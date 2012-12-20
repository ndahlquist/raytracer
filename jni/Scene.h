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
#define FOCAL_LENGTH 2.6f

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

		Vector3 Normal = ray.extend(dist) - thisSphere->center;
		Normal.Normalize();
		thisSphere->applyForce(-4.0f*Normal);
	}

	inline Color3f TraceRay(float x, float y, bool & doesIntersect) {
		doesIntersect = true;
		Ray3 ray = Ray3(Vector3(FOCAL_LENGTH, x, y));
		ray.vector.Normalize();
		return TraceRay(ray, NUM_RECURSIVE_REFLECTIONS, doesIntersect);
	}

	Color3f TraceRay(Ray3 ray, int recursion, bool & doesIntersect) {

		float dist;
		Sphere3 * visibleSphere;
		if(ray.endpoint == Point3(0,0,0)) { // Viewing rays // TODO
			//visibleSphere = BoundingVolumeHierarchy.AcceleratedIntersection(ray, dist);
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
	Vector3 lightDirection;

	BAH BoundingAreaHierarchy;
	BVH BoundingVolumeHierarchy;

};

#endif  // __SCENE_H__
