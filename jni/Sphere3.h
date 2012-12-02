
#ifndef __SPHERE3_H__
#define __SPHERE3_H__

#include <math.h> // We will need some math.
#include <algorithm>

#include "Point3.h"
#include "Vector3.h"
#include "Ray3.h"
#include "Light.h"

struct Sphere3 {

	inline Sphere3(const Point3 &center, const float radius) {
		this->center = center;
		this->radius = radius;
	}
	inline Sphere3(const float x, const float y, const float z, const float radius) {
		this->center = Point3(x, y, z);
		this->radius = radius;
	}

	inline void SetMaterial(const uint32_t colorDiffuse) {
		uint8_t R, G, B;
		RGBAfromU32(colorDiffuse, R, G, B);
		SetMaterial(RGBAtoU32(R/4, G/4, B/4), colorDiffuse, RGBAtoU32(200, 200, 200));
	}

	inline void SetMaterial(const uint32_t colorAmbient, const uint32_t colorDiffuse, const uint32_t colorSpecular) {
		this->colorAmbient = colorAmbient;
		this->colorDiffuse = colorDiffuse;
		this->colorSpecular = colorSpecular;
	}

	float IntersectionTest(Ray3 ray) {

		Vector3 relativeCenter = center - ray.endpoint;
		ray.vector.Normalize();
		float dist = Vector3::Dot(ray.vector, relativeCenter);
		if(dist <= 0)
			return -1;
		Point3 projection = ray.extend(dist);
		float distSquared = Point3::DistSq(projection, center);
		if(pow(radius, 2) < distSquared)
			return -1;

		return (projection - ray.endpoint).Length() - sqrt(pow(radius, 2) - distSquared);
	}


	float DiffuseIllumination(Point3 surfacePoint, PointLight pLight) {

		Vector3 normal = surfacePoint - center;
		normal.Normalize();
		Vector3 light = pLight.position - surfacePoint;
		light.Normalize();
		return std::max(Vector3::Dot(light, normal), 0.0f);

	}

	Ray3 ReflectRay(Ray3 incidentRay, float length) {
		incidentRay.vector.Normalize();
		Vector3 normal = incidentRay.extend(length) - center;
		normal.Normalize();
		Vector3 reflectedVector = 2.0f * Vector3::Dot(-incidentRay.vector, normal) * normal + incidentRay.vector;
		return Ray3(incidentRay.extend(length), reflectedVector);
	}

	// Local members:
	Point3 center;
	float radius;
	uint32_t colorAmbient;
	uint32_t colorDiffuse;
	uint32_t colorSpecular;
};

#endif  // __SPHERE3_H__
