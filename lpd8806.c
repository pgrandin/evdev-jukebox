/*
 * Copyright 2012 Christopher De Vries
 * This program is distributed under the Artistic License 2.0, a copy of which
 * is included in the file LICENSE.txt
 */
#include "lpd8806led.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>
#include "jukebox.h"

pthread_mutex_t playback_status_lock;

static const char *device = "/dev/spidev0.0";
static const int leds = 500;

void HSVtoRGB(double h, double s, double v, double *r, double *g, double *b);

void * spi_handler(void)
{
  lpd8806_buffer buf;
  int fd;
  int return_value;
  lpd8806_color *p;
  int i;
  double h, r, g, b;

  /* Open the device file using Low-Level IO */
  fd = open(device,O_WRONLY);
  if(fd<0) {
    fprintf(stderr,"Error %d: %s\n",errno,strerror(errno));
    pthread_exit(NULL);
  }

  /* Initialize the SPI bus for Total Control Lighting */
  return_value = spi_init(fd);
  if(return_value==-1) {
    fprintf(stderr,"SPI initialization error %d: %s\n",errno, strerror(errno));
    pthread_exit(NULL);
  }

  /* Initialize pixel buffer */
  if(lpd8806_init(&buf,leds)<0) {
    fprintf(stderr,"Pixel buffer initialization error: Not enough memory.\n");
    pthread_exit(NULL);
  }

  /* Set the gamma correction factors for each color */
  set_gamma(2.2,2.2,2.2);

  /* Blank the pixels */

  for(i=0;i<leds;i++) {
    write_gamma_color(&buf.pixels[i],0x00,0x00,0x00);
  }

  send_buffer(fd,&buf);

  h = 0.0;
  printf("Entering spi loop\n");
  /* Loop while continue_looping is true */
  for(;;){
    pthread_mutex_lock(&playback_status_lock);
    if(is_playing){
      h+=2.0;
      if(h>=360.0) h=0.0;
      memmove(buf.pixels+1,buf.pixels,sizeof(lpd8806_color)*(leds-2));
      HSVtoRGB(h,1.0,1.0,&r,&g,&b);
      p = &buf.pixels[0];
      write_gamma_color(p,(int)(r*255.0),(int)(g*255.0),(int)floor(b*255.0));
      send_buffer(fd,&buf);
    } else {
      for(i=0;i<leds;i++) {
         p = &buf.pixels[i];
         write_gamma_color(p,0x00,0x00,0x00);
      }
    send_buffer(fd,&buf);
   }
   pthread_mutex_unlock(&playback_status_lock);
   usleep(10000);
  }
for(i=0;i<leds;i++) {
    p = &buf.pixels[i];
      write_gamma_color(p,0x00,0x00,0x00);
      
  }
    send_buffer(fd,&buf);
  lpd8806_free(&buf);
  close(fd);
  printf("lpd thread terminated.\n");

  pthread_exit(NULL);
}

/* Convert hsv values (0<=h<360, 0<=s<=1, 0<=v<=1) to rgb values (0<=r<=1, etc) */
void HSVtoRGB(double h, double s, double v, double *r, double *g, double *b) {
  int i;
  double f, p, q, t;

  if( s < 1.0e-6 ) {
    /* grey */
    *r = *g = *b = v;
    return;
  }

  h /= 60.0;			        /* Divide into 6 regions (0-5) */
  i = (int)floor( h );
  f = h - (double)i;			/* fractional part of h */
  p = v * ( 1.0 - s );
  q = v * ( 1.0 - s * f );
  t = v * ( 1.0 - s * ( 1.0 - f ) );

  switch( i ) {
    case 0:
      *r = v;
      *g = t;
      *b = p;
      break;
    case 1:
      *r = q;
      *g = v;
      *b = p;
      break;
    case 2:
      *r = p;
      *g = v;
      *b = t;
      break;
    case 3:
      *r = p;
      *g = q;
      *b = v;
      break;
    case 4:
      *r = t;
      *g = p;
      *b = v;
      break;
    default:		// case 5:
      *r = v;
      *g = p;
      *b = q;
      break;
  }
}
