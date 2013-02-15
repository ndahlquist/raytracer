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
#define FOCAL_LENGTH 2.6f

class Scene {
public:

	bool reflectionsEnabled;
	bool lightProbeEnabled;
	
	Scene() {
		reflectionsEnabled = true;
		lightProbeEnabled = true;
	}

	void Add(Sphere3 sphere) {
		elements.push_back(sphere);
	}

	Sphere3 * SphereFromIndex(int index) {
		return & elements[index];
	}

	int IndexFromSphere(Sphere3 * sphere) {
		for(int i=0; i < elements.size(); i++) {
				if(& elements[i] == sphere)
					return i;
		}
		return -1;
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

	Sphere3 * TraceSphere(float x, float y) {
		Ray3 ray = Ray3(Vector3(FOCAL_LENGTH, x, y));
		ray.vector.Normalize();
		float dist;
		return BoundingAreaHierarchy.AcceleratedIntersection(ray, dist);
	}

	Sphere3 * MoveSphere(float x, float y, Sphere3 * sphere) {
		Ray3 ray = Ray3(Vector3(FOCAL_LENGTH, x, y));
		Point3 newPosition = ray.XPlaneIntersect(sphere->center.x);
		sphere->offsetPosition(newPosition);
		//ray.vector.Normalize();
		//Vector3 Normal = ray.Extend(10.0f/*dist*/) - sphere->center;
		//Normal.Normalize();
		//Vector3 force = Normal * Vector3(0.0f, .8f * sphere->radius, .8f * sphere->radius);
		//sphere->applyForce(force);
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
		} else { // Reflection rays
			visibleSphere = BoundingVolumeHierarchy.AcceleratedIntersection(ray, dist);
			if(visibleSphere == NULL) {
				if(lightProbeEnabled)
					return lightProbe.SampleLightProbe(ray.vector);
				else
					return Color3f(0.0f, 0.0f, 0.0f);
			}
		}
		
		// Diffuse
		Color3f mat = visibleSphere->colorDiffuse;
		Color3f sum = mat * visibleSphere->DiffuseIllumination(ray.Extend(dist), lightDirection);

		//if(visibleSphere->colorSpecular == Color3f(0, 0, 0))
		//	return sum;

		if(!reflectionsEnabled || recursion >= NUM_RECURSIVE_REFLECTIONS) {
			if(lightProbeEnabled){
				Ray3 reflectRay = visibleSphere->ReflectRay(ray, dist);
				sum += lightProbe.SampleLightProbe(reflectRay.vector);
			}
			return sum;
		}
		recursion++;

		// Reflective
		Ray3 reflectedRay = visibleSphere->ReflectRay(ray, dist);
		Color3f reflectedColor = this->TraceRay(reflectedRay, recursion, doesIntersect);
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
