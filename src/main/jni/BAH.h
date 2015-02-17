#ifndef __BAH_H__
#define __BAH_H__

#include <math.h> // We will need some math.
#include <vector>
#include <algorithm> // sort

class BAH {

public:
	BAH() {}
	
	inline void Initialize(const Point3 eye) {
		this->eye = eye;
		min_z_spheres.clear();
		max_z_spheres.clear();
	}
	
	inline void Index(Sphere3 * s) {
		Vector3 toCenter = s->center - eye;
		float distToTangent = sqrt(toCenter.LengthSq() - s->radius);
		toCenter.Normalize();
		float sinOfAngle = s->radius / distToTangent;
		float cosOfAngle = 1 - pow(s->radius / distToTangent, 2);

		// Find the line perpendicular to toCenter, in the plane that also contains Vector3(0,1,0)
		Vector3 perpendicular = Vector3::Cross(toCenter, Vector3::Cross(toCenter, Vector3(0,1,0)));
		perpendicular.Normalize();
		
		struct sphereBound bounds;
		
		Vector3 toTangent = toCenter * cosOfAngle + perpendicular * sinOfAngle;
		bounds.max_y = toTangent.y;

		toTangent = toCenter * cosOfAngle - perpendicular * sinOfAngle;
		bounds.min_y = toTangent.y;

		// Find the line perpendicular to toCenter, in the plane that also contains Vector3(0,0,1)
		perpendicular = Vector3::Cross(toCenter, Vector3::Cross(toCenter, Vector3(0,0,1)));
		perpendicular.Normalize();

		toTangent = toCenter * cosOfAngle + perpendicular * sinOfAngle;
		bounds.max_z = toTangent.z;

		toTangent = toCenter * cosOfAngle - perpendicular * sinOfAngle;
		bounds.min_z = toTangent.z;
		
		bounds.sphere = s;
		min_z_spheres.push_back(bounds);
	}
	
	inline void Sort() {
		
		max_z_spheres = min_z_spheres;
		
		for(int i=0; i<min_z_spheres.size(); i++)
			min_z_spheres[i].direction=2;
		for(int i=0; i<max_z_spheres.size(); i++)
			max_z_spheres[i].direction=3;
	
		struct sphereBound cmp;
		std::sort(min_z_spheres.begin(), min_z_spheres.end(), cmp);
		std::sort(max_z_spheres.begin(), max_z_spheres.end(), cmp);
		
		median_z = (min_z_spheres[min_z_spheres.size() / 2].min_z + max_z_spheres[max_z_spheres.size() / 2].max_z) / 2.0f;
	}
	
	inline Sphere3 * AcceleratedIntersection(const Ray3 & r, float & intersectDist) {
		intersectDist = INFINITY;
		Sphere3 * visibleSphere = NULL;
		
		if(r.vector.z < median_z) {
			for(int i=0; i<min_z_spheres.size(); i++) {
				if(r.vector.z < min_z_spheres[i].min_z)
					break;
				if(r.vector.z > min_z_spheres[i].max_z)
					continue;
				if(r.vector.y < min_z_spheres[i].min_y)
					continue;
				if(r.vector.y > min_z_spheres[i].max_y)
					continue;
				float this_dist = min_z_spheres[i].sphere->IntersectionTest(r);
				if(this_dist >= 0 && this_dist < intersectDist) {
					intersectDist = this_dist;
					visibleSphere = min_z_spheres[i].sphere;
				}
			}
		} else {
			for(int i=max_z_spheres.size()-1; i>=0; i--) {
				if(r.vector.z > max_z_spheres[i].max_z)
					break;
				if(r.vector.z < max_z_spheres[i].min_z)
					continue;
				if(r.vector.y < max_z_spheres[i].min_y)
					continue;
				if(r.vector.y > max_z_spheres[i].max_y)
					continue;
				float this_dist = max_z_spheres[i].sphere->IntersectionTest(r);
				if(this_dist >= 0 && this_dist < intersectDist) {
					intersectDist = this_dist;
					visibleSphere = max_z_spheres[i].sphere;
				}
			}
		}
		
		return visibleSphere;
	}

private:

	struct sphereBound {
		Sphere3 * sphere;
		float min_y;
		float max_y;
		float min_z;
		float max_z;
		char direction;
		bool operator() (const sphereBound &c1, const sphereBound &c2) {
			if(c1.direction==0) // TODO: this is super hacky
				return (c1.min_y < c2.min_y);
			if(c1.direction==1)
				return (c1.max_y < c2.max_y);
			if(c1.direction==2)
				return (c1.min_z < c2.min_z);
			else // direction==3
				return (c1.max_z < c2.max_z);
		}
	};
	std::vector<sphereBound> min_z_spheres;
	std::vector<sphereBound> max_z_spheres;
	float median_z;
	Point3 eye;
};

#endif  // __BAH_H__
