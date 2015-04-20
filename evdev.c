#include <linux/input.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "findpowermate.h"
#include "evdev.h"

int clic_start_time;
int last_scroll_event;

char *events[EV_MAX + 1] = {
        [0 ... EV_MAX] = NULL,
        [EV_SYN] = "Sync",                      [EV_KEY] = "Key",
        [EV_REL] = "Relative",                  [EV_ABS] = "Absolute",
        [EV_MSC] = "Misc",                      [EV_LED] = "LED",
        [EV_SND] = "Sound",                     [EV_REP] = "Repeat",
        [EV_FF] = "ForceFeedback",              [EV_PWR] = "Power",
        [EV_FF_STATUS] = "ForceFeedbackStatus",
};

void powermate_pulse_led(int static_brightness, int pulse_speed, int pulse_table, int pulse_asleep, int pulse_awake)
{
  int fd;

  fd = find_powermate(O_WRONLY);

  if(fd < 0){
    fprintf(stderr, "Unable to locate powermate\n");
    return;
  }
  struct input_event ev;
  memset(&ev, 0, sizeof(struct input_event));

  static_brightness &= 0xFF;

  if(pulse_speed < 0)
    pulse_speed = 0;
  if(pulse_speed > 510)
    pulse_speed = 510;
  if(pulse_table < 0)
    pulse_table = 0;
  if(pulse_table > 2)
    pulse_table = 2;
  pulse_asleep = !!pulse_asleep;
  pulse_awake = !!pulse_awake;

  ev.type = EV_MSC;
  ev.code = MSC_PULSELED;
  ev.value = static_brightness | (pulse_speed << 8) | (pulse_table << 17) | (pulse_asleep << 19) | (pulse_awake << 20);

  if(write(fd, &ev, sizeof(struct input_event)) != sizeof(struct input_event))
    fprintf(stderr, "write(): %s\n", strerror(errno));
}

void *
 process_evdev_events(void )
{
     int evdev_fd;
     struct input_event ev[64];
     int clic_duration;
     int i;

  int static_brightness = 0x80;
  int pulse_speed = 255;
  int pulse_table = 0;
  int pulse_asleep = 1;
  int pulse_awake = 0;

    evdev_fd = find_powermate(O_WRONLY);

  if(evdev_fd < 0){
    fprintf(stderr, "Unable to locate powermate\n");
    return 1;
  }
  pulse_awake=1;
  powermate_pulse_led(static_brightness, pulse_speed, pulse_table, pulse_asleep, pulse_awake);

        if ((evdev_fd = open("/dev/input/by-id/usb-Griffin_Technology__Inc._Griffin_PowerMate-event-if00", O_RDONLY)) < 0) {
                perror("jukebox");
                return 1;
        }
     printf("Starting thread loop\n");
     for (;;) {
         int rd = read(evdev_fd, ev, sizeof(struct input_event) * 64);
         if (rd < (int) sizeof(struct input_event)) {
                 perror("\njukebox: error reading");
                 pthread_exit(NULL);
         }
        for (i = 0; i < rd / sizeof(struct input_event); i++){
                     if ( ev[i].type == 1 ) {
                             if ( ev[i].value == 1 ) {
                                     clic_start_time=ev[i].time.tv_sec * 1000000 + ev[i].time.tv_usec;
                             } else {
                                     clic_duration=( ev[i].time.tv_sec * 1000000 + ev[i].time.tv_usec - clic_start_time ) /1000;
                                     printf("That's a release, duration %ld ms\n", clic_duration );
                                     if ( clic_duration > 3000 ) {
                                             sync();
                                             // reboot(RB_POWER_OFF);
                                     } else if ( clic_duration > 500 ) {
                                             printf ("That was a long clic!\n");
                                     } else {
                                             printf ("That was a short clic!\n");
                                             media_toggle_playback();
                                     }
                             }
                     } else if ( ev[i].type == 2 ) {
                                SetAlsaMasterVolume(ev[i].value);
                                last_scroll_event=ev[i].time.tv_sec * 1000000 + ev[i].time.tv_usec;
                     }
                     if(0) {
                     printf("EventC: time %ld.%06ld, type %d (%s), code %d, value %d\n",
                             ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
                             events[ev[i].type] ? events[ev[i].type] : "?",
                             ev[i].code,
                             ev[i].value);
                     }
        }
     }
     printf("Thread loop finished\n");
     close(evdev_fd);
     pthread_exit(NULL);
}

