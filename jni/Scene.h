
#ifndef __SCENE_H__
#define __SCENE_H__

#include <math.h> // We will need some math.
#include <vector>

#include "ColorUtils.h"

#include "Sphere3.h"
#include "Ray3.h"

class Scene {
public:

	Scene() { }

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

		if(visibleSphere == 0)
			return RGBAtoU32(0, constrain(dist), 0);
		else if(visibleSphere == 1)
			return RGBAtoU32(constrain(dist), 0, 0);
		else if(visibleSphere == 2)
			return RGBAtoU32(0, 0, constrain(dist));
		else
			return RGBAtoU32(0, 100, 100);
	}

private:
	std::vector<Sphere3> elements;

};

#endif  // __SCENE_H__
