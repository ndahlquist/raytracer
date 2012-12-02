
#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ColorUtils.h"

#include "Point3.h"
#include "Vector3.h"
#include "Ray3.h"
#include "Sphere3.h"
#include "Scene.h"

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

static void FunnyColors(AndroidBitmapInfo & info, void * pixels, int frame) {

	Scene mScene;

	Sphere3 sphere0 = Sphere3(100, -90, -100, 100);
	sphere0.SetMaterial(RGBAtoU32(100, 0, 0));
	mScene.Add(sphere0);

	Sphere3 sphere1 = Sphere3(100, 120, -90, 100);
	sphere1.SetMaterial(RGBAtoU32(0, 100, 0));
	mScene.Add(sphere1);

	Sphere3 sphere2 = Sphere3(100, -40, 40*sin(frame/10.0f), 40);
	sphere2.SetMaterial(RGBAtoU32(0, 0, 100));
	mScene.Add(sphere2);

	PointLight light0 = PointLight(Point3(0, 100, 100), .01f);
	mScene.Add(light0);

	PointLight light1 = PointLight(Point3(0, -400, -100), .005f);
	//mScene.Add(light1);

	for(int y = 0; y < info.height; y++) {
		for(int x = 0; x < info.width; x++) {
			uint8_t R, G, B;
			uint32_t * p = pixRef(info, pixels, x, y);
			RGBAfromU32(*p, R, G, B);

			Point3 eye = Point3(-800,0,0);
			Point3 samplePoint = Point3(0, (x - info.width / 2.0f)/2.0f, (y - info.height / 2.0f)/2.0f);
			Ray3 ray = Ray3(eye, samplePoint);
			*p = mScene.TraceRay(ray);

		}
	}
}

/*******************************************************************************************/

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nicd_raytracer_MainActivity_RayTrace(JNIEnv * env, jobject obj, jobject mBitmap, jint frame) {

	AndroidBitmapInfo info;
	void * mPixels;

	if(!VerifyBitmap(env, mBitmap, info))
		return;

	if(AndroidBitmap_lockPixels(env, mBitmap, &mPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");

	FunnyColors(info, mPixels, frame);
	AndroidBitmap_unlockPixels(env, mBitmap);

}
