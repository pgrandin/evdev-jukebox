# This is for a raspberry pi

apt-get install -y libasound2-dev pkg-config build-essential alsa-utils

if [[ `pkg-config --cflags libspotify` == "" ]]; then
    wget -c https://developer.spotify.com/download/libspotify/libspotify-12.1.103-Linux-armv6-bcm2708hardfp-release.tar.gz
    tar xfz libspotify-12.1.103-Linux-armv6-bcm2708hardfp-release.tar.gz
    pushd libspotify-12.1.103-Linux-armv6-bcm2708hardfp-release/
    make install 
    popd
fi

# Ensure we are using the jack output
amixer cset numid=3 1
