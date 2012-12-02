
#ifndef __SCENE_H__
#define __SCENE_H__

#include <math.h> // We will need some math.
#include <vector>

#include "ColorUtils.h"

#include "Sphere3.h"
#include "Ray3.h"
#include "Light.h"

class Scene {
public:

	Scene() {
		colorBackground = RGBAtoU32(0, 100, 100);
	}

	void Add(Sphere3 sphere) {
		elements.push_back(sphere);
	}

	void Add(PointLight light) {
		lights.push_back(light);
	}

	uint32_t TraceRay(Ray3 ray) {

		float dist = FLT_MAX;
		int visibleSphere = -1;
		for(int i=0; i < elements.size(); i++) {
			float this_dist = elements[i].IntersectionTest(ray);
			if(this_dist >= 0 && this_dist < dist) {
				dist = this_dist;
				visibleSphere = i;
			}
		}

		if(visibleSphere == -1)
			return colorBackground;

		float R=0;
		float G=0;
		float B=0;
		uint8_t matR, matG, matB;
		RGBAfromU32(elements[visibleSphere].colorDiffuse, matR, matG, matB);
		for(int i=0; i < lights.size(); i++) {
			float diffuseMultiplier = elements[visibleSphere].DiffuseIllumination(ray.extend(dist)); // TODO
			diffuseMultiplier *= lights[i].brightness;
			uint8_t lightR, lightG, lightB;
			RGBAfromU32(lights[i].color, lightR, lightG, lightB);
			R += diffuseMultiplier * lightR * matR;
			G += diffuseMultiplier * lightG * matG;
			B += diffuseMultiplier * lightB * matB;
		}
		return RGBAtoU32(constrain(R), constrain(G), constrain(B));
		//Ray3 reflection = elements[visibleSphere].ReflectRay(ray, dist);
	}

	uint32_t colorBackground;

private:
	std::vector<Sphere3> elements;
	std::vector<PointLight> lights;

};

#endif  // __SCENE_H__
