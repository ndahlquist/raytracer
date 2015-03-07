all: jniLibs

jniLibs: jni/*
if which ndk-build; then
	ndk-build
elif [ -n "$ANDROID_NDK_HOME" ]
	$ANDROID_NDK_HOME/ndk-build
else
	echo "The Android NDK is not installed."
	exit 1
fi
	rm -R jniLibs | true
	mv libs jniLibs
