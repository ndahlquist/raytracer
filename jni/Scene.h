
#ifndef __SCENE_H__
#define __SCENE_H__

#include <math.h> // We will need some math.
#include <vector>

#include "ColorUtils.h"

#include "Sphere3.h"
#include "Ray3.h"
#include "Light.h"

struct Camera {
	Point3 PinHole;
	float LensPlane; // TODO: Generalize this to a ray
};

class Scene {
public:

	Scene() {
		colorBackground = RGBAtoU32(0, 0, 0);
	}

	void Add(Sphere3 sphere) {
		elements.push_back(sphere);
	}

	void Add(PointLight light) {
		lights.push_back(light);
	}

	void Add(Camera camera) {
		mCamera = camera;
	}

	void BuildAccelerationStructure() {
		for(int i=0; i < elements.size(); i++) {
			elements[i].BuildAccelerationStructure(mCamera.PinHole);
		}
	};

	uint32_t TraceRay(int x, int y, int recursion = 5) {
		Point3 samplePoint = Point3(mCamera.LensPlane, x/2.0f, y/2.0f);
		Ray3 ray = Ray3(mCamera.PinHole, samplePoint);
		ray.vector.Normalize();
		return TraceRay(ray, recursion);
	}

	uint32_t TraceRay(Ray3 ray, int recursion) {

		//if(elements[1].AcceleratedIntersectionTest(ray) == -2)
		//	return RGBAtoU32(255, 0, 0);

		float dist = FLT_MAX;
		int visibleSphere = -1;
		for(int i=0; i < elements.size(); i++) {
			float this_dist;
			if(ray.endpoint == mCamera.PinHole)
				this_dist = elements[i].AcceleratedIntersectionTest(ray);
			else
				this_dist = elements[i].IntersectionTest(ray);
			if(this_dist >= 0 && this_dist < dist) {
				dist = this_dist;
				visibleSphere = i;
			}
		}

		if(visibleSphere == -1)
			return colorBackground;

		// Ambient / emissive
		uint8_t matR, matG, matB;
		RGBAfromU32(elements[visibleSphere].colorAmbient, matR, matG, matB);
		float R=matR;
		float G=matG;
		float B=matB;

		// Diffuse
		RGBAfromU32(elements[visibleSphere].colorDiffuse, matR, matG, matB);
		for(int i=0; i < lights.size(); i++) {
			float diffuseMultiplier = elements[visibleSphere].DiffuseIllumination(ray.extend(dist), lights[i]);
			diffuseMultiplier *= lights[i].brightness;
			uint8_t lightR, lightG, lightB;
			RGBAfromU32(lights[i].color, lightR, lightG, lightB);
			R += diffuseMultiplier * lightR * matR;
			G += diffuseMultiplier * lightG * matG;
			B += diffuseMultiplier * lightB * matB;
		}

		if(recursion <= 0)
			return RGBAtoU32(constrain(R), constrain(G), constrain(B));
		recursion--;

		if(elements[visibleSphere].colorSpecular == RGBAtoU32(0, 0, 0))
			return RGBAtoU32(constrain(R), constrain(G), constrain(B));

		// Specular / reflective
		RGBAfromU32(elements[visibleSphere].colorSpecular, matR, matG, matB);
		Ray3 reflectedRay = elements[visibleSphere].ReflectRay(ray, dist);
		uint32_t reflectedColor = this->TraceRay(reflectedRay, recursion);
		uint8_t rR, rG, rB;
		RGBAfromU32(reflectedColor, rR, rG, rB);
		R += .7 * rR;
		G += .7 * rG;
		B += .7 * rB;

		return RGBAtoU32(constrain(R), constrain(G), constrain(B));
	}

	uint32_t colorBackground;

private:
	std::vector<Sphere3> elements;
	std::vector<PointLight> lights;
	Camera mCamera;
};

#endif  // __SCENE_H__
