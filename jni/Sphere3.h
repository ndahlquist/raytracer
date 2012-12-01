// Sphere3.h
#ifndef __SPHERE3_H__
#define __SHPERE3_H__

#include <math.h> // We will need some math.

#include "Point3.h"
#include "Vector3.h"
#include "Ray3.h"

struct Sphere3 {

	inline Sphere3(const Point3 &center, const float radius) {
		this->center = center;
		this->radius = radius;
	}
	inline Sphere3(const float x, const float y, const float z, const float radius) {
		this->center = Point3(x, y, z);
		this->radius = radius;
	}

	inline float IntersectionTest(const Ray3 ray) {
		float a = pow(ray.vector.x, 2) + pow(ray.vector.y, 2) + pow(ray.vector.z, 2);
		float b = 2*ray.vector.x*(ray.endpoint.x - center.x) + 2*ray.vector.y*(ray.endpoint.y - center.y) + 2*ray.vector.z*(ray.endpoint.z - center.z);
		float c = pow(center.x, 2) + pow(center.y, 2) + pow(center.z, 2) +
				  pow(ray.endpoint.x, 2) + pow(ray.endpoint.y, 2) + pow(ray.endpoint.z, 2) +
                  -2*(center.x*ray.endpoint.x + center.y*ray.endpoint.y + center.z*ray.endpoint.z) - pow(radius, 2);
		return pow(b, 2) - 4*a*c;
	}

	// Local members:
	Point3 center;
	float radius;
};

#endif  // __SPHERE3_H__
