LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_ABI := armeabi-v7a
LOCAL_MODULE    := plasma
LOCAL_SRC_FILES := plasma.cpp
APP_OPTIM       := release
LOCAL_CFLAGS =  -O2 \
                -march=armv7 \
                -mfloat-abi=softfp \
                -mfpu=vfp
LOCAL_LDLIBS    := -lm -llog -ljnigraphics

include $(BUILD_SHARED_LIBRARY)
