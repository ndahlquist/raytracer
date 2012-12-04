
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

static HeatMap * interestMap;
static int num_threads = 8;
static int num_rays;

struct thread_args {
	AndroidBitmapInfo * info;
	void * pixels;
	Scene * scene;

	int threadNum;
};

void * workerThread(void * ptr){
	struct thread_args args = * (struct thread_args *) ptr;
	for(int x = 0; x < args.info->width; x++) {
		for(int y = 0; y < args.info->height; y++) {
			if((x*x*x + y) % num_threads != args.threadNum)
				continue;
			if(!interestMap->GetFlip(x, y))
				continue;
			uint32_t * oldValue = pixRef(*args.info, args.pixels, x, y);
			uint32_t newValue = args.scene->TraceRay(x - args.info->width / 2.0f, y - args.info->height / 2.0f);
			interestMap->Post(x, y, * oldValue, newValue);
			* oldValue = newValue;
			num_rays++;
		}
	}
	return NULL;
}

static void ThreadedRayTrace(AndroidBitmapInfo & info, void * pixels, int nframe) {

	Scene mScene;
	float frame = (float) nframe / 200.0f;

	Sphere3 sphere0 = Sphere3(100, -90, -100, 100);
	sphere0.SetMaterial(RGBAtoU32(100, 0, 0));
	mScene.Add(sphere0);

	Sphere3 sphere1 = Sphere3(80, 80+40*sin(frame/34.0f + 6), -90*sin(frame/43.0f), 50);
	sphere1.SetMaterial(RGBAtoU32(0, 100, 0));
	mScene.Add(sphere1);

	Sphere3 sphere2 = Sphere3(80, -40+10*sin(frame/54.0f + 5), 20+40*sin(frame/30.0f), 40);
	sphere2.SetMaterial(RGBAtoU32(0, 0, 100));
	mScene.Add(sphere2);

	Sphere3 sphere3 = Sphere3(5, 50+10*sin(frame/20.0f), 20+10*cos(frame/20.0f), 10);
	sphere3.SetMaterial(RGBAtoU32(100, 100, 0));
	mScene.Add(sphere3);

	Sphere3 sphere4 = Sphere3(400, -100, 140, 340);
	sphere4.SetMaterial(RGBAtoU32(20, 20, 20), RGBAtoU32(150, 150, 150), RGBAtoU32(0, 0, 0));
	mScene.Add(sphere4);

	PointLight light0 = PointLight(Point3(0, 200, -100), .01f);
	mScene.Add(light0);

	PointLight light1 = PointLight(Point3(0, -400, -100), .005f);
	mScene.Add(light1);

	Camera camera0;
	camera0.PinHole = Point3(-800, 0, 0);
	camera0.LensPlane = 0;
	mScene.Add(camera0);

	mScene.BuildAccelerationStructure();

	pthread_t * threads[num_threads];
	struct thread_args * args_array[num_threads];

	for(int i=0; i < num_threads; i++) {
		struct thread_args * args = new struct thread_args;
		args->info = &info;
		args->pixels = pixels;
		args->scene = &mScene;
		args->threadNum = i;
		threads[i] = new pthread_t;
		args_array[i] = args;
		pthread_create(threads[i],NULL,&workerThread,(void *)args);

	}
	for(int i=0; i < num_threads; i++) {
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
	if(interestMap == NULL)
		interestMap = new HeatMap(info.width, info.height);

}

extern "C"
JNIEXPORT jint JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_RayTrace(JNIEnv * env, jobject obj, jobject mBitmap, jint frame) {

	AndroidBitmapInfo info;
	void * mPixels;

	if(!VerifyBitmap(env, mBitmap, info))
		return 0;

	if(AndroidBitmap_lockPixels(env, mBitmap, &mPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	num_rays = 0;
	ThreadedRayTrace(info, mPixels, frame);
	AndroidBitmap_unlockPixels(env, mBitmap);
	return num_rays;

}
