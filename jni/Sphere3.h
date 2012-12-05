
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
		this->offset = Vector3(0,0,0);
	}
	inline Sphere3(const float x, const float y, const float z, const float radius) {
		this->center = Point3(x, y, z);
		this->radius = radius;
		this->offset = Vector3(0,0,0);
	}

	inline void setPosition(const Point3 & center) {
		this->center = center + this->offset;
		this->offset *= .98f;
	}

	inline void applyForce(Vector3 force) {
		this->offset += force;
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

	void BuildAccelerationStructure(Point3 eye) {

		Vector3 toCenter = center - eye;
		float distToTangent = sqrt(toCenter.LengthSq() - radius);
		toCenter.Normalize();
		float sinOfAngle = radius / distToTangent;
		float cosOfAngle = 1 - pow(radius / distToTangent, 2);

		// Find the line perpendicular to toCenter, in the plane that also contains Vector3(0,1,0)
		Vector3 perpendicular = Vector3::Cross(toCenter, Vector3::Cross(toCenter, Vector3(0,1,0))); // TODO: simplify this.
		perpendicular.Normalize();

		Vector3 toTangent = toCenter * cosOfAngle + perpendicular * sinOfAngle;
		AccelerationStructure.east_bound = toTangent.y;

		toTangent = toCenter * cosOfAngle - perpendicular * sinOfAngle;
		AccelerationStructure.west_bound = toTangent.y;

		// Find the line perpendicular to toCenter, in the plane that also contains Vector3(0,0,1)
		perpendicular = Vector3::Cross(toCenter, Vector3::Cross(toCenter, Vector3(0,0,1))); // TODO: simplify this.
		perpendicular.Normalize();

		toTangent = toCenter * cosOfAngle + perpendicular * sinOfAngle;
		AccelerationStructure.north_bound = toTangent.z;

		toTangent = toCenter * cosOfAngle - perpendicular * sinOfAngle;
		AccelerationStructure.south_bound = toTangent.z;
	}

	float AcceleratedIntersectionTest(Ray3 ray) {
		if(ray.vector.z > AccelerationStructure.north_bound)
			return -2;
		if(ray.vector.z < AccelerationStructure.south_bound)
			return -2;
		if(ray.vector.y < AccelerationStructure.west_bound)
			return -2;
		if(ray.vector.y > AccelerationStructure.east_bound)
			return -2;

		return IntersectionTest(ray);
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

	float SpecularIllumination(Point3 surfacePoint, PointLight pLight, Vector3 viewingRay) {
		Vector3 Normal = surfacePoint - center;
		Normal.Normalize();
		viewingRay.Normalize();
		Vector3 HalfwayVector = viewingRay + (surfacePoint - pLight.position);
		HalfwayVector.Normalize();

		float specular = Vector3::Dot(Normal, -HalfwayVector);
		if(specular <= 0)
			return 0;
		return pow(specular, 512);
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
	Vector3 offset;
	float radius;
	uint32_t colorAmbient;
	uint32_t colorDiffuse;
	uint32_t colorSpecular;
	struct AccelerationStructure {
		float west_bound;
		float east_bound;
		float south_bound;
		float north_bound;
	} AccelerationStructure;
};

#endif  // __SPHERE3_H__
