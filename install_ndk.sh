#!/usr/bin/env bash
# Installs the Android NDK r10d for 64-bit Linux.

set -e

if [ -e android-ndk-r10d ];
then
	echo "The NDK is already installed. Skipping installation."
	exit 0
fi

curl http://dl.google.com/android/ndk/android-ndk-r10d-linux-x86_64.bin >> ndk_installer.bin
chmod a+x ndk_installer.bin
./ndk_installer.bin

sudo ln -s android-ndk-r10d/ndk-build /usr/bin/ndk-build