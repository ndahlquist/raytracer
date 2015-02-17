all: jniLibs

jniLibs: jni/*
	ndk-build
	rm -R jniLibs | true
	mv libs jniLibs
