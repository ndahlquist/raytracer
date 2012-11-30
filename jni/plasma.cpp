
#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "stats.h"

#define  LOG_TAG    "libplasma"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static void RGBAfromU32(const uint32_t in, uint8_t & R, uint8_t & G, uint8_t & B) {
	R = in;
	G = in >> 8;
	B = in >> 16;
}

static __inline__ uint32_t RGBAtoU32(const uint8_t R, const uint8_t G, const uint8_t B) {
	uint32_t output = R;
	output |= G << 8;
	output |= B << 16;
	return output;
}

static __inline__ float constrain(float x) {
	if(x < 0)
		return 0;
	else if(x > 255)
		return 255;
	return x;
}

static __inline__ uint32_t * pixRef(AndroidBitmapInfo & info, void * pixels, unsigned int x, unsigned int y) {
	pixels = (char*) pixels + y * info.stride;
	uint32_t * line = (uint32_t *) pixels;
	return line + x;
}

float * getPixel(float * inputPixels, unsigned int x, unsigned int y, unsigned int channel, unsigned int width) {
	return inputPixels + 3 * y * width  + 3 * x + channel;
}

void U32BuftoFloatBuf(void * inputPixels, float * outputBuffer, int length) {
	for(int i = 0; i < length; i++) {
		uint32_t * p = (uint32_t *) inputPixels;
		p += i;
		uint8_t R, G, B;
		RGBAfromU32(*p, R, G, B);

		*(outputBuffer + 3*i) = R;
		*(outputBuffer + 3*i + 1) = G;
		*(outputBuffer + 3*i + 2) = B;
	}
}

void FloatBuftoU32Buf(float * inputBuffer, void * outputPixels, int length) {
	for(int i = 0; i < length; i++) {
		uint8_t R, G, B;
		R = *(inputBuffer + 3*i);
		G = *(inputBuffer + 3*i + 1);
		B = *(inputBuffer + 3*i + 2);

		uint32_t * p = (uint32_t *) outputPixels;
		p += i;
		*p = RGBAtoU32(R, G, B);
	}
}

static bool VerifyBitmap(JNIEnv * env, jobject bitmap, AndroidBitmapInfo & info) {
	int ret;
	if((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return false;
	}
	if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap format is not RGBA_8888 !");
		return false;
	}
	return true;
}

static void FunnyColors(AndroidBitmapInfo & info, void * pixels) {
	for(int y = 0; y < info.height; y++) {
		for(int x = 0; x < info.width; x++) {
			uint8_t R, G, B;
			uint32_t * p = pixRef(info, pixels, x, y);
			RGBAfromU32(*p, R, G, B);

			R = rand();

			*p = RGBAtoU32(R, G, B);
		}
	}
}

/*******************************************************************************************/

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_NonstandardWaveletRecompose(JNIEnv * env, jobject obj, jobject mBitmap) {

	AndroidBitmapInfo info;
	void * mPixels;

	if(!VerifyBitmap(env, mBitmap, info))
		return;

	if(AndroidBitmap_lockPixels(env, mBitmap, &mPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	FunnyColors(info, mPixels);
	AndroidBitmap_unlockPixels(env, mBitmap);

}
