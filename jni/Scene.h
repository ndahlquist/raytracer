
#ifndef __SCENE_H__
#define __SCENE_H__

#include <math.h> // We will need some math.
#include <vector>

#include "ColorUtils.h"

#include "Sphere3.h"
#include "Ray3.h"

class Scene {
public:

	Scene() {
		colorBackground = RGBAtoU32(0, 0, 0);
	}

	void Add(Sphere3 sphere) {
		elements.push_back(sphere);
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

		float diffuseMultiplier = elements[visibleSphere].DiffuseIllumination(ray.extend(dist));
		return RGBAtoU32(0, 100* diffuseMultiplier, 0);
		//Ray3 reflection = elements[visibleSphere].ReflectRay(ray, dist);

			return elements[visibleSphere].colorAmbient;
	}

	uint32_t colorBackground;

private:
	std::vector<Sphere3> elements;

};

#endif  // __SCENE_H__
