#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "Color.h"
#include "Point3.h"
#include "Vector3.h"
#include "Ray3.h"
#include "Sphere3.h"
#include "Scene.h"

#define  LOG_TAG    "libplasma"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define NUM_THREADS 8

static Scene * scene;
static int interlace_lines = 2;
static int frame_num = 0;
static int num_rays;
static int width;
static int height;

	struct background {
		void * pixels;
		AndroidBitmapInfo info;

		Color3f SampleBackground(float x, float y) {
			return bilinearSample(info.width*x, info.height*y);
		}
	private:
		Color3f Lerp(Color3f c1, Color3f c2, float t) {
			return Color3f((1-t)*c1.r + t*c2.r, (1-t)*c1.g + t*c2.g, (1-t)*c1.b + t*c2.b);
		}

		Color3f bilinearSample(float x, float y) {
			Color3f bottomLeft = SampleBitmap(floor(x), floor(y));
			Color3f bottomRight = SampleBitmap(ceil(x), floor(y));
			Color3f topLeft = SampleBitmap(floor(x), ceil(y));
			Color3f topRight = SampleBitmap(ceil(x), ceil(y));

			Color3f bottomLerp = Lerp(bottomLeft, bottomRight, x - floor(x));
			Color3f topLerp = Lerp(topLeft, topRight, x - floor(x));

			return Lerp(bottomLerp, topLerp, y - floor(y));
		}

		Color3f SampleBitmap(int x, int y) {
			if(x >= info.width || x < 0|| y >= info.height || y < 0) // TODO
				return 0;
			return Color3f(* pixRef(info, pixels, x, y));
		}
	} background;

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

struct thread_args {
	AndroidBitmapInfo * info;
	void * pixels;
	int threadNum;
};

void * workerThread(void * ptr){
	struct thread_args args = * (struct thread_args *) ptr;
	for(int y = args.threadNum; y < height; y+=NUM_THREADS) {
	//for(int y = args.threadNum*interlace_lines+frame_num%interlace_lines; y < height; y+=NUM_THREADS*interlace_lines) {
		if((frame_num+y) % interlace_lines != 0)
			continue;
		for(int x = 0; x < width; x++) {
			bool doesIntersect;
			Color3f newValue = scene->TraceRay((float) x / width - .5f, (float) y / width - .5, doesIntersect);
			uint32_t * oldValue = pixRef(*args.info, args.pixels, x, y);
			if(doesIntersect)
				* oldValue = AlphaMask(newValue.U32(), 255);
			else if(* oldValue >> 24 != 0) // If the background is not already drawn, draw background.
				* oldValue = AlphaMask(background.SampleBackground((float) x/width,(float) y/height).U32(), 0);
			num_rays++;
		}
	}
	return NULL;
}

static void ThreadedRayTrace(AndroidBitmapInfo & info, void * pixels, long timeElapsed) {

	static float frame;
	frame += timeElapsed / 3000.0f;
	frame_num++;

	Sphere3 * sphere0 = scene->ReturnSphere(0); // Red
	sphere0->setPosition(Point3(950+50*sin(frame/70), -80+40*sin(frame/70), 10*cos(frame/60)));

	Sphere3 * sphere1 = scene->ReturnSphere(1); // Blue
	sphere1->setPosition(Point3(880, 80+40*sin(frame/34.0f + 6), 100-90*sin(frame/43.0f)));

	Sphere3 * sphere2 = scene->ReturnSphere(2); // Green
	sphere2->setPosition(Point3(880, -40+10*sin(frame/54.0f + 5), 120+40*sin(frame/30.0f)));

	Sphere3 * sphere3 = scene->ReturnSphere(3); // Yellow
	sphere3->setPosition(Point3(805, 50+23*sin(frame/19.0f), 120+23*cos(frame/20.0f)));

	Sphere3 * sphere4 = scene->ReturnSphere(4); // Purple
	sphere4->setPosition(Point3(880+200*sin(frame/23.0f), 20+40*cos(frame/11.0f), 170+150*cos(frame/23.0f)));

	scene->BuildAccelerationStructure();

	pthread_t * threads[NUM_THREADS];
	struct thread_args * args_array[NUM_THREADS];

	for(int i=0; i < NUM_THREADS; i++) {
		struct thread_args * args = new struct thread_args;
		args->info = &info;
		args->pixels = pixels;
		args->threadNum = i;
		threads[i] = new pthread_t;
		args_array[i] = args;
		pthread_create(threads[i],NULL,&workerThread,(void *)args);
	}
	for(int i=0; i < NUM_THREADS; i++) {
		pthread_join(*threads[i], NULL);
		delete threads[i];
		delete args_array[i];
	}
}

/*******************************************************************************************/

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_Initialize(JNIEnv * env, jobject obj, jobject mBitmap) {

	// Store the bitmap's height and width
	AndroidBitmapInfo info;
	if(!VerifyBitmap(env, mBitmap, info))
		return;
	width = info.width;
	height = info.height;
	// Initialize elements of the scene.
	if(scene == NULL) {
		scene = new Scene();

		Sphere3 sphere0 = Sphere3(100);
		sphere0.SetMaterial(Color3f(180, 0, 0));
		scene->Add(sphere0);

		Sphere3 sphere1 = Sphere3(50);
		sphere1.SetMaterial(Color3f(0, 120, 0));
		scene->Add(sphere1);

		Sphere3 sphere2 = Sphere3(40);
		sphere2.SetMaterial(Color3f(0, 0, 250));
		scene->Add(sphere2);

		Sphere3 sphere3 = Sphere3(30);
		sphere3.SetMaterial(Color3f(130, 130, 0));
		scene->Add(sphere3);

		Sphere3 sphere4 = Sphere3(15);
		sphere4.SetMaterial(Color3f(150, 0, 150));
		scene->Add(sphere4);

		scene->SetLighting(Vector3(-10, 10, -5));
	}
	void * mPixels;
	if(AndroidBitmap_lockPixels(env, mBitmap, &mPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");
	// Initialize alpha channel to 255 (intersection flag)
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			* pixRef(info, mPixels, x, y) = AlphaMask(RGBtoU32(0, 0, 0), 255);
		}
	}
	AndroidBitmap_unlockPixels(env, mBitmap);
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_PassLightProbe(JNIEnv * env, jobject obj, jobject mBitmap) {

	AndroidBitmapInfo info;
	void * mPixels;

	if(!VerifyBitmap(env, mBitmap, info))
		return;

	if(AndroidBitmap_lockPixels(env, mBitmap, &mPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	scene->lightProbe.pixels = mPixels;
	scene->lightProbe.info = info;

	AndroidBitmap_unlockPixels(env, mBitmap);

}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_PassBackground(JNIEnv * env, jobject obj, jobject mBitmap) {

	AndroidBitmapInfo info;
	void * mPixels;

	if(!VerifyBitmap(env, mBitmap, info))
		return;

	if(AndroidBitmap_lockPixels(env, mBitmap, &mPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	background.pixels = mPixels;
	background.info = info;

	AndroidBitmap_unlockPixels(env, mBitmap);

}

extern "C"
JNIEXPORT jint JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_RayTrace(JNIEnv * env, jobject obj, jobject mBitmap, jlong timeElapsed) {

	AndroidBitmapInfo info;
	void * mPixels;

	if(!VerifyBitmap(env, mBitmap, info))
		return 0;

	if(AndroidBitmap_lockPixels(env, mBitmap, &mPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	num_rays = 0;
	ThreadedRayTrace(info, mPixels, timeElapsed);
	AndroidBitmap_unlockPixels(env, mBitmap);
	return num_rays;

}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_SetInterlacing(JNIEnv * env, jobject obj, jint interlacing) {
	if(interlacing >= 1)
		interlace_lines = interlacing;
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_TouchEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y) {
	if(scene == NULL)
		return;
	scene->PokeSphere((float) x / width - .5f, (float) y / width - .5);
}
