
#ifndef __SCENE_H__
#define __SCENE_H__

#include <math.h> // We will need some math.
#include <vector>

#include "ColorUtils.h"

#include "Sphere3.h"
#include "Ray3.h"
#include "Light.h"

struct Camera {
	Point3 PinHole;
	float LensPlane; // TODO: Generalize this to a ray
};

class Scene {
public:

	Scene() {}

	void Add(Sphere3 sphere) {
		elements.push_back(sphere);
	}

	Sphere3 * ReturnSphere(int index) {
		return & elements[index];
	}

	void Add(PointLight light) {
		lights.push_back(light);
	}

	void Add(Camera camera) {
		mCamera = camera;
	}

	void BuildAccelerationStructure() {
		for(int i=0; i < elements.size(); i++) {
			elements[i].BuildAccelerationStructure(mCamera.PinHole);
		}
	};

	void PokeSphere(float x, float y) { // TODO: this reuses a lot of code: try to combine with TraceRay
		Point3 samplePoint = Point3(mCamera.LensPlane, x/2.0f, y/2.0f);
		Ray3 ray = Ray3(mCamera.PinHole, samplePoint);
		ray.vector.Normalize();
		float dist = FLT_MAX;
		int visibleSphere = -1;
		for(int i=0; i < elements.size(); i++) {
			float this_dist = elements[i].AcceleratedIntersectionTest(ray); // TODO: implement b-search over accel structure.
			if(this_dist >= 0 && this_dist < dist) {
				dist = this_dist;
				visibleSphere = i;
			}
		}

		if(visibleSphere == -1)
			return;

		Sphere3 * thisSphere = & elements[visibleSphere];

		Vector3 Normal = ray.extend(dist) - thisSphere->center;
		Normal.Normalize();
		thisSphere->applyForce(-10.0f*Normal);
	}

	uint32_t TraceRay(int x, int y, int recursion = 4) {
		Point3 samplePoint = Point3(mCamera.LensPlane, x/2.0f, y/2.0f);
		Ray3 ray = Ray3(mCamera.PinHole, samplePoint);
		ray.vector.Normalize();
		return TraceRay(ray, recursion);
	}

	uint32_t TraceRay(Ray3 ray, int recursion) {

		//if(elements[1].AcceleratedIntersectionTest(ray) == -2)
		//	return RGBAtoU32(255, 0, 0);

		float dist = FLT_MAX;
		int visibleSphere = -1;
		for(int i=0; i < elements.size(); i++) {
			float this_dist;
			if(ray.endpoint == mCamera.PinHole)
				this_dist = elements[i].AcceleratedIntersectionTest(ray); // TODO: implement b-search over accel structure.
			else
				this_dist = elements[i].IntersectionTest(ray);
			if(this_dist >= 0 && this_dist < dist) {
				dist = this_dist;
				visibleSphere = i;
			}
		}

		if(visibleSphere == -1)
			return lightProbe.SampleLightProbe(ray.vector);

		// Ambient / emissive
		uint8_t matR, matG, matB;
		RGBAfromU32(elements[visibleSphere].colorAmbient, matR, matG, matB);
		float R=matR;
		float G=matG;
		float B=matB;

		// Diffuse
		RGBAfromU32(elements[visibleSphere].colorDiffuse, matR, matG, matB);
		for(int i=0; i < lights.size(); i++) {
			float diffuseMultiplier = elements[visibleSphere].DiffuseIllumination(ray.extend(dist), lights[i]);
			diffuseMultiplier *= lights[i].brightness;
			uint8_t lightR, lightG, lightB;
			RGBAfromU32(lights[i].color, lightR, lightG, lightB);
			R += diffuseMultiplier * lightR * matR;
			G += diffuseMultiplier * lightG * matG;
			B += diffuseMultiplier * lightB * matB;
		}

		if(recursion <= 0)
			return RGBAtoU32(constrain(R), constrain(G), constrain(B));
		recursion--;

		if(elements[visibleSphere].colorSpecular == RGBAtoU32(0, 0, 0))
			return RGBAtoU32(constrain(R), constrain(G), constrain(B));

		// Specular
		for(int i=0; i < lights.size(); i++) {
			RGBAfromU32(elements[visibleSphere].colorSpecular, matR, matG, matB);
			float specularMultiplier = elements[visibleSphere].SpecularIllumination(ray.extend(dist), lights[i], ray.vector);
			specularMultiplier *= lights[i].brightness;
			uint8_t lightR, lightG, lightB;
			RGBAfromU32(lights[i].color, lightR, lightG, lightB);
			R += specularMultiplier * lightR * matR;
			G += specularMultiplier * lightG * matG;
			B += specularMultiplier * lightB * matB;
		}

		// Reflective
		Ray3 reflectedRay = elements[visibleSphere].ReflectRay(ray, dist);
		uint32_t reflectedColor = this->TraceRay(reflectedRay, recursion);
		uint8_t rR, rG, rB;
		RGBAfromU32(reflectedColor, rR, rG, rB);
		R += matR / 255.0f * rR;
		G += matG / 255.0f * rG;
		B += matB / 255.0f * rB;

		return RGBAtoU32(constrain(R), constrain(G), constrain(B));
	}

	struct lightProbe {
		void * pixels;
		AndroidBitmapInfo info;

		uint32_t SampleLightProbe(Vector3 vec) {
			vec.Normalize();
			float x = vec.y/4.0f+.5f;
			float y = vec.z/4.0f+.5f;
			return * pixRef(info, pixels, x*info.width, y*info.height);
		}
	} lightProbe;

private:
	std::vector<Sphere3> elements;
	std::vector<PointLight> lights;
	Camera mCamera;
};

#endif  // __SCENE_H__
