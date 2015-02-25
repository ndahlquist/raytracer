#!/usr/bin/env bash
# Installs the Android NDK r10d for 64-bit Linux.

curl http://dl.google.com/android/ndk/android-ndk-r10d-linux-x86_64.bin >> ndk_installer.bin
chmod a+x ndk_installer.bin
./ndk_installer.bin