
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

#define RECURSION_BOTTOM 4

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

static void HorizontalWaveletDecompose(AndroidBitmapInfo & info, float * inputPixels, float * outputPixels, int width, int height) {
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x+=2) {
			for(int c=0; c < 3; c++) {
				float V0 = * getPixel(inputPixels, x, y, c, info.width);
				float V1 = * getPixel(inputPixels, x+1, y, c, info.width);
				* getPixel(outputPixels, x/2, y, c, info.width) = (V0+V1)/2;
				* getPixel(outputPixels, x/2+width/2, y, c, info.width) = (V0-V1)/2+128;
			}
		}
	}
}

static void HorizontalWaveletRecompose(AndroidBitmapInfo & info, float * inputPixels, float * outputPixels, int width, int height) {
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x+=2) {
			for(int c=0; c < 3; c++) {
				float V = * getPixel(inputPixels, x/2, y, c, info.width);
				float dV = * getPixel(inputPixels, x/2+width/2, y, c, info.width) - 128;
				* getPixel(outputPixels, x, y, c, info.width) = constrain(V+dV);
				* getPixel(outputPixels, x+1, y, c, info.width) = constrain(V-dV);
			}
		}
	}
}

static void VerticalWaveletDecompose(AndroidBitmapInfo & info, float * inputPixels, float * outputPixels, int width, int height) {
	for(int y = 0; y < height; y+=2) {
		for(int x = 0; x < width; x++) {
			for(int c=0; c < 3; c++) {
				float V0 = * getPixel(inputPixels, x, y, c, info.width);
				float V1 = * getPixel(inputPixels, x, y+1, c, info.width);
				* getPixel(outputPixels, x, y/2, c, info.width) = (V0+V1)/2;
				* getPixel(outputPixels, x, y/2+height/2, c, info.width) = (V0-V1)/2+128;
			}
		}
	}
}

static void VerticalWaveletRecompose(AndroidBitmapInfo & info, float * inputPixels, float * outputPixels, int width, int height) {
	for(int y = 0; y < height; y+=2) {
		for(int x = 0; x < width; x++) {
			for(int c=0; c < 3; c++) {
				float V = * getPixel(inputPixels, x, y/2, c, info.width);
				float dV = * getPixel(inputPixels, x, y/2+height/2, c, info.width) - 128;
				* getPixel(outputPixels, x, y, c, info.width) = constrain(V+dV);
				* getPixel(outputPixels, x, y+1, c, info.width) = constrain(V-dV);
			}
		}
	}
}

static void * NonstandardWaveletDecompose(AndroidBitmapInfo & info, void * inputPixels, int width = -1, int height = -1, float * buffer0 = NULL, float * buffer1 = NULL) {
	if(buffer0 == NULL) { // Begin recursive loop
		buffer0 = new float[info.width*info.height*3];
		buffer1 = new float[info.width*info.height*3];
		U32BuftoFloatBuf(inputPixels, buffer0, info.width * info.height);
		width = info.width;
		height = info.height;
	}
	HorizontalWaveletDecompose(info, buffer0, buffer1, width, height);
	VerticalWaveletDecompose(info, buffer1, buffer0, width, height);
	if(width == RECURSION_BOTTOM || height == RECURSION_BOTTOM) { // End recursive loop
		delete[] buffer1;
		return buffer0;
	}
	return NonstandardWaveletDecompose(info, inputPixels, width/2, height/2, buffer0, buffer1);
}

static void NonstandardWaveletRecompose(AndroidBitmapInfo & info, void * inputBuffer, void * outputPixels, int width = -1, int height = -1, float * buffer0 = NULL, float * buffer1 = NULL) {
	if(buffer0 == NULL) { // Begin recursive loop
		buffer0 = (float *) inputBuffer;
		buffer1 = new float[info.width*info.height*3];
		width = RECURSION_BOTTOM;
		height = RECURSION_BOTTOM;
	}
	VerticalWaveletRecompose(info, buffer0, buffer1, width, height);
	HorizontalWaveletRecompose(info, buffer1, buffer0, width, height);
	if(width == info.width || height == info.height) { // End recursive loop
		FloatBuftoU32Buf(buffer0, outputPixels, info.width * info.height);
		delete[] buffer0;
		delete[] buffer1;
		return;
	}
	NonstandardWaveletRecompose(info, inputBuffer, outputPixels, width*2, height*2, buffer0, buffer1);
}

static void ImageDifference(AndroidBitmapInfo & info, void * inputPixels, void * outputPixels) {
	for(int y = 0; y < info.height; y++) {
		for(int x = 0; x < info.width; x++) {
			uint8_t R0, G0, B0, R1, G1, B1;
			uint32_t * p = pixRef(info, inputPixels, x, y);
			RGBAfromU32(*p, R0, G0, B0);
			p = pixRef(info, outputPixels, x, y);
			RGBAfromU32(*p, R1, G1, B1);

			*p = RGBAtoU32(128+R0-R1, 128+G0-G1, 128+B0-B1);
		}
	}
}

/*******************************************************************************************/

extern "C"
JNIEXPORT jint JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_NonstandardWaveletDecompose(JNIEnv * env, jobject obj, jobject inputBitmap) {

	AndroidBitmapInfo info;
	void * inputPixels;

	if(!VerifyBitmap(env, inputBitmap, info))
		return -1;

	if(AndroidBitmap_lockPixels(env, inputBitmap, &inputPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	int outputPtr = (int) NonstandardWaveletDecompose(info, inputPixels);

	AndroidBitmap_unlockPixels(env, inputBitmap);

	return outputPtr;
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_NonstandardWaveletRecompose(JNIEnv * env, jobject obj, jint inputPtr, jobject outputBitmap) {

	AndroidBitmapInfo info;
	void * outputPixels;

	if(!VerifyBitmap(env, outputBitmap, info))
		return;

	if(AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	NonstandardWaveletRecompose(info, (void *) inputPtr, outputPixels);
	AndroidBitmap_unlockPixels(env, outputBitmap);

}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_MapWavelet(JNIEnv * env, jobject obj, jint inputPtr, jint width, jint height, jfloat level) {
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(y<RECURSION_BOTTOM && x<RECURSION_BOTTOM)
				continue; // We want to keep the scaled image (color profile) in the linear quantization space.
			for(int c=0; c < 3; c++) {
				float V = * getPixel((float *) inputPtr, x, y, c, width);
				V -= 128;
				V = 1 / V;
				V *= level;
				V += 128;
				* getPixel((float *) inputPtr, x, y, c, width) = constrain(V);
			}
		}
	}
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_ThresholdWavelet(JNIEnv * env, jobject obj, jint inputPtr, jint width, jint height, jfloat level) {
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(y<RECURSION_BOTTOM && x<RECURSION_BOTTOM)
				continue; // We want to keep the scaled image (color profile) in the linear quantization space.
			for(int c=0; c < 3; c++) {
				float V = * getPixel((float *) inputPtr, x, y, c, width);
				if(abs(V - 128) < level)
					V = 128;
				* getPixel((float *) inputPtr, x, y, c, width) = V;
			}
		}
	}
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_ExportWavelet(JNIEnv * env, jobject obj, jint inputPtr, jobject outputBitmap, int compression) {

	AndroidBitmapInfo info;
	void * outputPixels;

	if(compression < 0)
		compression = 0;

	if(!VerifyBitmap(env, outputBitmap, info))
		return;

	if(AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	uint8_t mask = ~0;
	mask = mask << compression;

	for(int y = 0; y < info.height; y++) {
		for(int x = 0; x < info.width; x++) {
			uint8_t R = * getPixel((float *) inputPtr, x, y, 0, info.width);
			uint8_t G = * getPixel((float *) inputPtr, x, y, 1, info.width);
			uint8_t B = * getPixel((float *) inputPtr, x, y, 2, info.width);
			* pixRef(info, outputPixels, x, y) = RGBAtoU32(R & mask, G & mask, B & mask);
		}
	}

	AndroidBitmap_unlockPixels(env, outputBitmap);

}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_ImportWavelet(JNIEnv * env, jobject obj, jobject inputBitmap, jint outputPtr) {

	AndroidBitmapInfo info;
	void * inputPixels;

	if(!VerifyBitmap(env, inputBitmap, info))
		return;

	if(AndroidBitmap_lockPixels(env, inputBitmap, &inputPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	for(int y = 0; y < info.height; y++) {
		for(int x = 0; x < info.width; x++) {
			uint32_t * p = pixRef(info, inputPixels, x, y);
			uint8_t R, G, B;
			RGBAfromU32(*p, R, G, B);

			*getPixel((float *) outputPtr, x, y, 0, info.width) = R;
			*getPixel((float *) outputPtr, x, y, 1, info.width) = G;
			*getPixel((float *) outputPtr, x, y, 2, info.width) = B;
		}
	}

	AndroidBitmap_unlockPixels(env, inputBitmap);

}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_ImageDifference(JNIEnv * env, jobject obj, jobject inputBitmap, jobject outputBitmap) {

	AndroidBitmapInfo info;
	void * inputPixels;
	void * outputPixels;

	if(!VerifyBitmap(env, inputBitmap, info) || !VerifyBitmap(env, outputBitmap, info))
		return;

	if(AndroidBitmap_lockPixels(env, inputBitmap, &inputPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");
	if(AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	ImageDifference(info, inputPixels, outputPixels);

	AndroidBitmap_unlockPixels(env, inputBitmap);
	AndroidBitmap_unlockPixels(env, outputBitmap);

}

extern "C"
JNIEXPORT jfloat JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_PSNR(JNIEnv * env, jobject obj, jobject reference, jobject reconstruction) {

	AndroidBitmapInfo info;
	void * referencePixels;
	void * reconstuctedPixels;

	if(!VerifyBitmap(env, reference, info) || !VerifyBitmap(env, reconstruction, info))
		return -1;

	if(AndroidBitmap_lockPixels(env, reference, &referencePixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");
	if(AndroidBitmap_lockPixels(env, reconstruction, &reconstuctedPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	float MSE = 0;
	for(int i = 0; i < info.width * info.height; i++) {
		uint8_t R0, G0, B0, R1, B1, G1;
		uint32_t * p = (uint32_t *) referencePixels;
		p += i;
		RGBAfromU32(*p, R0, G0, B0);
		p = (uint32_t *) reconstuctedPixels;
		p += i;
		RGBAfromU32(*p, R1, G1, B1);
		MSE += pow(R0 - R1, 2);
		MSE += pow(G0 - G1, 2);
		MSE += pow(B0 - B1, 2);
	}
	MSE /= (info.width * info.height * 3);

	AndroidBitmap_unlockPixels(env, reference);
	AndroidBitmap_unlockPixels(env, reconstruction);

	return 10*log10(pow(128,2)/MSE);
}

extern "C"
JNIEXPORT jfloat JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_CalculateEntropy(JNIEnv * env, jobject obj, jobject inputBitmap) {

	AndroidBitmapInfo info;
	void * pixels;

	if(!VerifyBitmap(env, inputBitmap, info))
		return -1;

	if(AndroidBitmap_lockPixels(env, inputBitmap, &pixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	unsigned int * histogram = new unsigned int[256];
	for(int i=0; i < 256; i++) {
		histogram[i] = 0;
	}
	for(int i = 0; i < info.width * info.height; i++) {
		uint32_t * p = (uint32_t *) pixels;
		p += i;
		uint8_t R, G, B;
		RGBAfromU32(*p, R, G, B);
		*(histogram + R) = *(histogram + R) + 1;
		*(histogram + G) = *(histogram + G) + 1;
		*(histogram + B) = *(histogram + B) + 1;
	}

	AndroidBitmap_unlockPixels(env, inputBitmap);

	float entropy = 0;
	for(int i=0; i < 256; i++) {
		if(histogram[i] == 0)
			continue;
		float frequency = (float) histogram[i] / info.width * info.height * 3;
		entropy -= histogram[i] * log(histogram[i]) / log(2);
	}

	delete[] histogram;

	return entropy;
}
