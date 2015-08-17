# evdev-jukebox
An evdev controller spotify jukebox which supports LED strips for cool effects

To ensure that the spi device shows up, I had to ad the following lines to /boot/config.txt
#device_tree=
dtparam=spi=on
