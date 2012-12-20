#ifndef __BVH_H__
#define __BVH_H__

#include <math.h> // We will need some math.
#include <vector>

class BVH {

public:
	BVH() {}
	
	inline void Initialize() {}
	
	inline void Index(Sphere3 * sphere) {
		spheres.push_back(sphere);
	}
	
	inline void Sort() {}
	
	inline Sphere3 * AcceleratedIntersection(const Ray3 & r, float & intersectDist) {
		intersectDist = INFINITY;
		Sphere3 * visibleSphere = NULL;
		for(int i=0; i<spheres.size(); i++) {
			float this_dist = spheres[i]->IntersectionTest(r);
			if(this_dist >= 0 && this_dist < intersectDist) {
				intersectDist = this_dist;
				visibleSphere = spheres[i];
			}
		}
		return visibleSphere;
	}

private:

	
	
	// Adapted from tavianator.com/2011/05/fast-branchless-raybounding-box-intersections/
	/*bool RaySlabIntersection(Ray3 r, Sphere3 s) {
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
	}*/

	std::vector<Sphere3 *> spheres;
};

#endif  // __BVH_H__
