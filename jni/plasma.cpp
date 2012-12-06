#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "ColorUtils.h"
#include "Point3.h"
#include "Vector3.h"
#include "Ray3.h"
#include "Sphere3.h"
#include "Scene.h"
#include "HeatMap.h"

#define  LOG_TAG    "libplasma"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define NUM_THREADS 8

static Scene * scene;
static HeatMap * heatMap;
static bool HeatMapEnabled = true;
static int interlace_lines = 2;
static int frame_num = 0;
static int num_rays;
static int width;
static int height;

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
		if(interlace_lines >= 1 && (frame_num+y) % interlace_lines != 0)
			continue;
		for(int x = 0; x < width; x++) {
			if(HeatMapEnabled && !heatMap->GetFlip(x, y))
				continue;
			uint32_t * oldValue = pixRef(*args.info, args.pixels, x, y);
			uint32_t newValue = scene->TraceRay(x - width / 2.0f, y - height / 2.0f).U32();
			if(* oldValue != newValue) {
				if(HeatMapEnabled)
					heatMap->Post(x, y);
				* oldValue = newValue;
			}
			num_rays++;
		}
	}
	return NULL;
}

static void ThreadedRayTrace(AndroidBitmapInfo & info, void * pixels, long timeElapsed) {

	static float frame;
	frame += timeElapsed / 3000.0f;
	frame_num++;

	Sphere3 * sphere0 = scene->ReturnSphere(0);
	sphere0->setPosition(Point3(100, -90, -100));

	Sphere3 * sphere1 = scene->ReturnSphere(1);
	sphere1->setPosition(Point3(80, 80+40*sin(frame/34.0f + 6), -90*sin(frame/43.0f)));

	Sphere3 * sphere2 = scene->ReturnSphere(2);
	sphere2->setPosition(Point3(80, -40+10*sin(frame/54.0f + 5), 20+40*sin(frame/30.0f)));

	Sphere3 * sphere3 = scene->ReturnSphere(3);
	sphere3->setPosition(Point3(5, 50+23*sin(frame/19.0f), 20+23*cos(frame/20.0f)));

	Sphere3 * sphere4 = scene->ReturnSphere(4);
	sphere4->setPosition(Point3(80+200*sin(frame/23.0f), 20+40*cos(frame/11.0f), 70+150*cos(frame/23.0f)));

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

	AndroidBitmapInfo info;
	if(!VerifyBitmap(env, mBitmap, info))
		return;
	width = info.width;
	height = info.height;
	if(heatMap == NULL)
		heatMap = new HeatMap(info.width, info.height);
	if(scene == NULL) {
		scene = new Scene();

		Sphere3 sphere0 = Sphere3(100, -90, -100, 100);
		sphere0.SetMaterial(RGBAtoU32(100, 0, 0));
		scene->Add(sphere0);

		Sphere3 sphere1 = Sphere3(80, 80, 0, 50);
		sphere1.SetMaterial(RGBAtoU32(0, 100, 0));
		scene->Add(sphere1);

		Sphere3 sphere2 = Sphere3(80, -40, 20, 40);
		sphere2.SetMaterial(RGBAtoU32(0, 0, 100));
		scene->Add(sphere2);

		Sphere3 sphere3 = Sphere3(5, 50, 20, 30);
		sphere3.SetMaterial(RGBAtoU32(100, 100, 0));
		scene->Add(sphere3);

		Sphere3 sphere4 = Sphere3(480, 100, 300, 15);
		sphere4.SetMaterial(RGBAtoU32(100, 0, 100));
		scene->Add(sphere4);

		PointLight light0 = PointLight(Point3(0, 200, -100), .01f);
		scene->Add(light0);

		PointLight light1 = PointLight(Point3(0, -400, -100), .005f);
		scene->Add(light1);

		Camera camera0;
		camera0.PinHole = Point3(-800, 0, 0);
		camera0.LensPlane = 0;
		scene->Add(camera0);
	}
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
	return;

}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_PassBackground(JNIEnv * env, jobject obj, jobject mBitmap) {

	AndroidBitmapInfo info;
	void * mPixels;

	if(!VerifyBitmap(env, mBitmap, info))
		return;

	if(AndroidBitmap_lockPixels(env, mBitmap, &mPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	scene->background.pixels = mPixels;
	scene->background.info = info;

	AndroidBitmap_unlockPixels(env, mBitmap);
	return;

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
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_ToggleAdaptiveSampling(JNIEnv * env, jobject obj, jboolean enabled) {
	HeatMapEnabled = enabled;
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
	scene->PokeSphere(x - width / 2.0f, y - height / 2.0f);
}
