ifeq ($(shell uname),Darwin)
ifdef USE_AUDIOQUEUE
AUDIO_DRIVER ?= osx
LDFLAGS += -framework AudioToolbox
else
AUDIO_DRIVER ?= openal
LDFLAGS += -framework OpenAL
endif
else
CFLAGS  = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags alsa)
LDFLAGS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-L alsa) -lrt -lm
LDLIBS  = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-l --libs-only-other alsa)
AUDIO_DRIVER ?= alsa
endif

TARGET = jukebox
OBJS = $(TARGET).o appkey.o $(AUDIO_DRIVER)-audio.o audio.o lpd8806led.o lpd8806.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread  $^ -o $@ $(LDLIBS)
ifdef DEBUG
ifeq ($(shell uname),Darwin)
	install_name_tool -change @loader_path/../Frameworks/libspotify.framework/libspotify @rpath/libspotify.so $@
endif
endif

include common.mk


lpd8806led.o: lpd8806led.c lpd8806led.h
lpd8806.o: lpd8806.c lpd8806.h
audio.o: audio.c audio.h
alsa-audio.o: alsa-audio.c audio.h
dummy-audio.o: dummy-audio.c audio.h
osx-audio.o: osx-audio.c audio.h
openal-audio.o: openal-audio.c audio.h
jukebox.o: jukebox.c audio.h
playtrack.o: playtrack.c audio.h
