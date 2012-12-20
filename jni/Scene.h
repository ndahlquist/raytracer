#ifndef __SCENE_H__
#define __SCENE_H__

#include <math.h> // We will need some math.
#include <vector>

#include "ColorUtils.h"

#include "Sphere3.h"
#include "BAH.h"
#include "BVH.h"
#include "Ray3.h"

#define NUM_RECURSIVE_REFLECTIONS 0 //3 TODO 

struct Camera { // TODO
	Point3 PinHole;
	float LensPlane; // TODO: Generalize this to a ray
};

class Scene {
public:

	Scene() {}

	void Add(Sphere3 sphere) {
		elements.push_back(sphere);
	}

	Sphere3 * ReturnSphere(int index) {
		return & elements[index];
	}

	void SetLighting(Vector3 lightDirection) {
		lightDirection.Normalize();
		this->lightDirection=lightDirection;
	}

	void SetCamera(Camera camera) {
		mCamera = camera;
	}

	void BuildAccelerationStructure() {
		BoundingAreaHierarchy.Initialize(mCamera.PinHole);
		for(int i=0; i < elements.size(); i++) {
			BoundingAreaHierarchy.Index(& elements[i]);
		}
		BoundingAreaHierarchy.Sort();

		BoundingVolumeHierarchy.Initialize();
		for(int i=0; i < elements.size(); i++) {
			BoundingVolumeHierarchy.Index(& elements[i]);
		}
		BoundingVolumeHierarchy.Sort();
	};

	void PokeSphere(float x, float y) {
		Point3 samplePoint = Point3(mCamera.LensPlane, x/2.0f, y/2.0f);
		Ray3 ray = Ray3(mCamera.PinHole, samplePoint);
		ray.vector.Normalize();
		float dist;
		Sphere3 * thisSphere = BoundingAreaHierarchy.AcceleratedIntersection(ray, dist);

		if(thisSphere == NULL)
			return;

		Vector3 Normal = ray.extend(dist) - thisSphere->center;
		Normal.Normalize();
		thisSphere->applyForce(-4.0f*Normal);
	}

	Color3f TraceRay(int x, int y, bool & doesIntersect) {
		doesIntersect = true;
		Point3 samplePoint = Point3(mCamera.LensPlane, x/2.0f, y/2.0f);
		Ray3 ray = Ray3(mCamera.PinHole, samplePoint);
		ray.vector.Normalize();
		return TraceRay(ray, NUM_RECURSIVE_REFLECTIONS, doesIntersect);
	}

	Color3f TraceRay(Ray3 ray, int recursion, bool & doesIntersect) {

		float dist;
		Sphere3 * visibleSphere;
		if(ray.endpoint == mCamera.PinHole) { // Viewing rays
			visibleSphere = BoundingAreaHierarchy.AcceleratedIntersection(ray, dist);
			if(visibleSphere == NULL) {
				doesIntersect = false;
				return Color3f(0,0,0);
			}
		} else { // reflection rays
			visibleSphere = BoundingVolumeHierarchy.AcceleratedIntersection(ray, dist);
			if(visibleSphere == NULL)
				return lightProbe.SampleLightProbe(ray.vector);
		}

		// Ambient / emissive
		//Color3f sum = Color3f(visibleSphere->colorAmbient);
		Color3f sum = Color3f(0,0,0);

		// Diffuse
		Color3f mat = Color3f(visibleSphere->colorDiffuse);
		sum += mat * visibleSphere->DiffuseIllumination(ray.extend(dist), lightDirection);

		if(recursion <= 0)
			return sum;
		recursion--;

		if(visibleSphere->colorSpecular == Color3f(0, 0, 0))
			return sum;

		// Reflective
		mat = visibleSphere->colorSpecular;
		Ray3 reflectedRay = visibleSphere->ReflectRay(ray, dist);
		Color3f reflectedColor = this->TraceRay(reflectedRay, recursion, doesIntersect);
		sum += mat * reflectedColor;

		return sum;
	}

	struct lightProbe {
		void * pixels;
		AndroidBitmapInfo info;

		Color3f SampleLightProbe(Vector3 vec) {
			vec.Normalize();
			float x = vec.y/4.0f+.5f;
			float y = vec.z/4.0f+.5f;
			return SampleBitmap(info.width*x, info.height*y)*1.2f;
		}

	private:
		Color3f SampleBitmap(int x, int y) {
			if(x >= info.width || x < 0|| y >= info.height || y < 0)
				return 0;
			return Color3f(* pixRef(info, pixels, x, y));
		}
	} lightProbe;

private:

	std::vector<Sphere3> elements;
	Camera mCamera;
	Vector3 lightDirection;

	BAH BoundingAreaHierarchy;
	BVH BoundingVolumeHierarchy;

};

#endif  // __SCENE_H__
