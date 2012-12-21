#ifndef __SPHERE3_H__
#define __SPHERE3_H__

#include <math.h> // We will need some math.
#include <algorithm>

#include "Point3.h"
#include "Vector3.h"
#include "Ray3.h"

struct Sphere3 {

	inline Sphere3(const float radius) {
		this->radius = radius;
		this->offset = Vector3(0,0,0);
	}
	
	inline Sphere3(const Point3 &center, const float radius) {
		this->center = center;
		this->radius = radius;
		this->offset = Vector3(0,0,0);
	}

	inline void setPosition(const Point3 & center) {
		this->center = center + this->offset;
		this->offset *= .98f;
	}

	inline void applyForce(Vector3 force) {
		this->offset += force;
		this->offset /= .98f;
	}

	inline void SetMaterial(const Color3f color) {
		SetMaterial(color / 4.0f, color, Color3f(.8, .8, .8));
	}

	inline void SetMaterial(const Color3f colorAmbient, const Color3f colorDiffuse, const Color3f colorSpecular) {
		this->colorAmbient = colorAmbient;
		this->colorDiffuse = colorDiffuse;
		this->colorSpecular = colorSpecular;
	}

	float IntersectionTest(Ray3 ray) {

		Vector3 relativeCenter = center - ray.endpoint;
		float dist = Vector3::Dot(ray.vector, relativeCenter);
		if(dist <= 0)
			return -1;
		Point3 projection = ray.Extend(dist);
		float distSquared = Point3::DistSq(projection, center);
		if(pow(radius, 2) < distSquared)
			return -1;

		return (projection - ray.endpoint).Length() - sqrt(pow(radius, 2) - distSquared);
	}

	float DiffuseIllumination(const Point3 surfacePoint, const Vector3 lightDirection) {
		Vector3 normal = surfacePoint - center;
		normal.Normalize();
		return std::max(Vector3::Dot(lightDirection, normal), 0.0f);
	}

	Ray3 ReflectRay(Ray3 incidentRay, float length) {
		incidentRay.vector.Normalize();
		Vector3 normal = incidentRay.Extend(length) - center;
		normal.Normalize();
		Vector3 reflectedVector = 2.0f * Vector3::Dot(-incidentRay.vector, normal) * normal + incidentRay.vector;
		return Ray3(incidentRay.Extend(length), reflectedVector);
	}

	// Local members:
	Point3 center;
	Vector3 offset;
	float radius;
	Color3f colorAmbient;
	Color3f colorDiffuse;
	Color3f colorSpecular;
};

#endif  // __SPHERE3_H__
