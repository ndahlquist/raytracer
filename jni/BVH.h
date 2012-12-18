#ifndef __BVH_H__
#define __BVH_H__

#include <math.h> // We will need some math.
#include <vector>

class BVH {

public:
	BVH() {};
	
	void Index(Sphere3 * sphere) {
		spheres.push_back(sphere);
	}
	
	std::vector<Sphere3 *> IntersectionCandidates(Ray3 r) {
		std::vector<Sphere3 *> candidates;
		for(int i=0; i<spheres.size(); i++) {
			if(RaySlabIntersection(r, * spheres[i]))
				candidates.push_back(spheres[i]);
		}
		return candidates;
	}

private:

	
	
	// Adapted from tavianator.com/2011/05/fast-branchless-raybounding-box-intersections/
	bool RaySlabIntersection(Ray3 r, Sphere3 s) {
		float tmin = -INFINITY, tmax = INFINITY;
		if(r.vector.x != 0.0) {
    			float tx1 = (s.center.x - s.radius - r.endpoint.x)/r.vector.x;
    			float tx2 = (s.center.x + s.radius - r.endpoint.x)/r.vector.x;
    			tmin = std::max(tmin, std::min(tx1, tx2));
    			tmax = std::min(tmax, std::max(tx1, tx2));
  		}
 
  		if(r.vector.y != 0.0) {
    			float ty1 = (s.center.y - s.radius - r.endpoint.y)/r.vector.y;
    			float ty2 = (s.center.y + s.radius - r.endpoint.y)/r.vector.y;
 
    			tmin = std::max(tmin, std::min(ty1, ty2));
    			tmax = std::min(tmax, std::max(ty1, ty2));
  		}
 
		if(tmax < tmin)
  			return false;
		return true;
	}

	std::vector<Sphere3 *> spheres;
};

#endif  // __BVH_H__
