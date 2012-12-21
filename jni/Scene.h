#ifndef __SCENE_H__
#define __SCENE_H__

#include <math.h> // We will need some math.
#include <vector>

#include "Color.h"
#include "Sphere3.h"
#include "BAH.h"
#include "BVH.h"
#include "Ray3.h"

#define NUM_RECURSIVE_REFLECTIONS 2
#define FOCAL_LENGTH 2.2f

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

	void BuildAccelerationStructure() {
		BoundingAreaHierarchy.Initialize(Point3(0,0,0));
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
		Ray3 ray = Ray3(Vector3(FOCAL_LENGTH, x, y));
		ray.vector.Normalize();
		float dist;
		Sphere3 * thisSphere = BoundingAreaHierarchy.AcceleratedIntersection(ray, dist);

		if(thisSphere == NULL)
			return;

		Vector3 Normal = ray.Extend(dist) - thisSphere->center;
		Normal.Normalize();
		thisSphere->applyForce(-4.0f*Normal);
	}

	inline Color3f TraceRay(float x, float y, bool & doesIntersect) {
		doesIntersect = true;
		Ray3 ray = Ray3(Vector3(FOCAL_LENGTH, x, y));
		ray.vector.Normalize();
		return TraceRay(ray, 0, doesIntersect);
	}

	Color3f TraceRay(Ray3 ray, int recursion, bool & doesIntersect) {

		float dist;
		Sphere3 * visibleSphere;
		if(recursion == 0) { // Viewing rays
			visibleSphere = BoundingAreaHierarchy.AcceleratedIntersection(ray, dist);
			if(visibleSphere == NULL) {
				doesIntersect = false;
				return Color3f();
			}
		} else { // reflection rays
			visibleSphere = BoundingVolumeHierarchy.AcceleratedIntersection(ray, dist);
			if(visibleSphere == NULL)
				return lightProbe.SampleLightProbe(ray.vector);
		}

		// Ambient / emissive
		//Color3f sum = visibleSphere->colorAmbient;

		// Diffuse
		Color3f mat = visibleSphere->colorDiffuse;
		//sum += mat * visibleSphere->DiffuseIllumination(ray.extend(dist), lightDirection);
		Color3f sum = mat * visibleSphere->DiffuseIllumination(ray.Extend(dist), lightDirection);

		//if(visibleSphere->colorSpecular == Color3f(0, 0, 0))
		//	return sum;

		// Reflective
		if(recursion >= NUM_RECURSIVE_REFLECTIONS) {
			//mat = visibleSphere->colorSpecular;
			Ray3 reflectRay = visibleSphere->ReflectRay(ray, dist);
			Color3f reflectedColor = lightProbe.SampleLightProbe(reflectRay.vector);
			//sum += mat * reflectedColor;
			sum += reflectedColor;
			return sum;
		}
		recursion++;

		///mat = visibleSphere->colorSpecular;
		Ray3 reflectedRay = visibleSphere->ReflectRay(ray, dist);
		Color3f reflectedColor = this->TraceRay(reflectedRay, recursion, doesIntersect);
		//sum += mat * reflectedColor;
		sum += reflectedColor;

		return sum;
	}

	struct lightProbe {
		void * pixels;
		AndroidBitmapInfo info;

		// vec must be normalized
		inline Color3f SampleLightProbe(const Vector3 & vec) {
			float x = vec.y/4.0f+.5f; // TODO: do proper sphere mapping
			float y = vec.z/4.0f+.5f;
			return SampleBitmap(info.width*x, info.height*y);
		}

	private:
		inline Color3f SampleBitmap(const int x, const int y) {
#ifdef BOUNDS_CHECK
			if(x >= info.width || x < 0|| y >= info.height || y < 0)
				return 0;
#endif
			return Color3f(* pixRef(info, pixels, x, y));
		}
	} lightProbe;

private:

	std::vector<Sphere3> elements;
	Vector3 lightDirection;

	BAH BoundingAreaHierarchy;
	BVH BoundingVolumeHierarchy;

};

#endif  // __SCENE_H__
