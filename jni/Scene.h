#ifndef __SCENE_H__
#define __SCENE_H__

#include <math.h> // We will need some math.
#include <vector>

#include "ColorUtils.h"

#include "Sphere3.h"
#include "BAH.h"
#include "BVH.h"
#include "Ray3.h"
#include "Light.h"

#define NUM_RECURSIVE_REFLECTIONS 0 //4 TODO 

struct Camera {
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

	void Add(PointLight light) {
		lights.push_back(light);
	}

	void Add(Camera camera) {
		mCamera = camera;
	}

	void BuildAccelerationStructure() {
		BoundingAreaHierarchy.Initialize(mCamera.PinHole);
		for(int i=0; i < elements.size(); i++) {
			BoundingAreaHierarchy.Index(& elements[i]);
		}
		BoundingAreaHierarchy.Sort();
	};

	void PokeSphere(float x, float y) { // TODO: this reuses a lot of code: try to combine with TraceRay
		/*Point3 samplePoint = Point3(mCamera.LensPlane, x/2.0f, y/2.0f);
		Ray3 ray = Ray3(mCamera.PinHole, samplePoint);
		ray.vector.Normalize();
		float dist = FLT_MAX;
		int visibleSphere = -1;
		for(int i=0; i < elements.size(); i++) {
			float this_dist = elements[i].AcceleratedIntersectionTest(ray); // TODO: implement b-search over accel structure.
			if(this_dist >= 0 && this_dist < dist) {
				dist = this_dist;
				visibleSphere = i;
			}
		}

		if(visibleSphere == -1)
			return;

		Sphere3 * thisSphere = & elements[visibleSphere];

		Vector3 Normal = ray.extend(dist) - thisSphere->center;
		Normal.Normalize();
		thisSphere->applyForce(-5.0f*Normal);*/
	}

	Color3f TraceRay(int x, int y, bool & doesIntersect) {
		doesIntersect = true;
		Point3 samplePoint = Point3(mCamera.LensPlane, x/2.0f, y/2.0f);
		Ray3 ray = Ray3(mCamera.PinHole, samplePoint);
		ray.vector.Normalize();
		return TraceRay(ray, NUM_RECURSIVE_REFLECTIONS, doesIntersect);
	}

	Color3f TraceRay(Ray3 ray, int recursion, bool & doesIntersect) {

		float dist = FLT_MAX;
		Sphere3 * visibleSphere = NULL;
		
		if(ray.endpoint == mCamera.PinHole) { // Viewing rays
			std::vector<Sphere3 *> candidates = BoundingAreaHierarchy.IntersectionCandidates(ray);
			if(candidates.size()==0)
				return Color3f(0,200,0);
			//if(candidates.size()==1)
			//	return Color3f(0,0,200);
				
			for(int i=0; i < candidates.size(); i++) {
				float this_dist = candidates[i]->IntersectionTest(ray);
				if(this_dist >= 0 && this_dist < dist) {
					dist = this_dist;
					visibleSphere = candidates[i];
				}
			}
		} /*else for(int i=0; i < elements.size(); i++) {
			float this_dist = elements[i].IntersectionTest(ray);
			if(this_dist >= 0 && this_dist < dist) {
				dist = this_dist;
				visibleSphere = & elements[i];
			}
		}*/

		if(visibleSphere == NULL) {
			if(recursion == NUM_RECURSIVE_REFLECTIONS) {
				doesIntersect = false;
				return Color3f(0,0,0);
			}
			return lightProbe.SampleLightProbe(ray.vector);
		}

		// Ambient / emissive
		Color3f sum = Color3f(visibleSphere->colorAmbient);

		// Diffuse
		Color3f mat = Color3f(visibleSphere->colorDiffuse);
		for(int i=0; i < lights.size(); i++) {
			float diffuseMultiplier = visibleSphere->DiffuseIllumination(ray.extend(dist), lights[i]);
			Color3f light = lights[i].color;
			sum += diffuseMultiplier * light * mat;
		}

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
	std::vector<PointLight> lights;
	Camera mCamera;
	
	BAH BoundingAreaHierarchy;
	
};

#endif  // __SCENE_H__
