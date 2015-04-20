# This is for a raspberry pi

apt-get install -y libasound2-dev pkg-config build-essential alsa-utils
# Ensure we are using the jack output
amixer cset numid=3 1
