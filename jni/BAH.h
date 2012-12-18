#ifndef __BAH_H__
#define __BAH_H__

#include <math.h> // We will need some math.
#include <vector>

class BAH {

public:
	BAH() {}
	
	void Initialize(const Point3 eye) {
		this->eye = eye;
		spheres.clear();
	}
	
	void Index(Sphere3 * s) {
		Vector3 toCenter = s->center - eye;
		float distToTangent = sqrt(toCenter.LengthSq() - s->radius);
		toCenter.Normalize();
		float sinOfAngle = s->radius / distToTangent;
		float cosOfAngle = 1 - pow(s->radius / distToTangent, 2);

		// Find the line perpendicular to toCenter, in the plane that also contains Vector3(0,1,0)
		Vector3 perpendicular = Vector3::Cross(toCenter, Vector3::Cross(toCenter, Vector3(0,1,0))); // TODO: simplify this.
		perpendicular.Normalize();
		
		struct sphereBound bounds;
		
		Vector3 toTangent = toCenter * cosOfAngle + perpendicular * sinOfAngle;
		bounds.east_bound = toTangent.y;

		toTangent = toCenter * cosOfAngle - perpendicular * sinOfAngle;
		bounds.west_bound = toTangent.y;

		// Find the line perpendicular to toCenter, in the plane that also contains Vector3(0,0,1)
		perpendicular = Vector3::Cross(toCenter, Vector3::Cross(toCenter, Vector3(0,0,1))); // TODO: simplify this.
		perpendicular.Normalize();

		toTangent = toCenter * cosOfAngle + perpendicular * sinOfAngle;
		bounds.north_bound = toTangent.z;

		toTangent = toCenter * cosOfAngle - perpendicular * sinOfAngle;
		bounds.south_bound = toTangent.z;
		
		bounds.sphere = s;
		spheres.push_back(bounds);
	}
	
	std::vector<Sphere3 *> IntersectionCandidates(Ray3 r) {
		std::vector<Sphere3 *> candidates;
		for(int i=0; i<spheres.size(); i++) {
			if(SimpleIntersectionTest(r, i))
				candidates.push_back(spheres[i].sphere);
		}
		return candidates;
	}

private:

	bool SimpleIntersectionTest(Ray3 ray, int index) {
		if(ray.vector.z > spheres[index].north_bound)
			return false;
		if(ray.vector.z < spheres[index].south_bound)
			return false;
		if(ray.vector.y < spheres[index].west_bound)
			return false;
		if(ray.vector.y > spheres[index].east_bound)
			return false;

		return true;
	}

	struct sphereBound {
		Sphere3 * sphere;
		float west_bound;
		float east_bound;
		float south_bound;
		float north_bound;
	};
	std::vector<sphereBound> spheres;
	Point3 eye;
};

#endif  // __BAH_H__
