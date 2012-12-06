
#ifndef __SCENE_H__
#define __SCENE_H__

#include <math.h> // We will need some math.
#include <vector>

#include "ColorUtils.h"

#include "Sphere3.h"
#include "Ray3.h"
#include "Light.h"

#define NUM_RECURSIVE_REFLECTIONS 4

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
		thisSphere->applyForce(-5.0f*Normal);
	}

	Color3f TraceRay(int x, int y) {
		Point3 samplePoint = Point3(mCamera.LensPlane, x/2.0f, y/2.0f);
		Ray3 ray = Ray3(mCamera.PinHole, samplePoint);
		ray.vector.Normalize();
		return TraceRay(ray, NUM_RECURSIVE_REFLECTIONS);
	}

	Color3f TraceRay(Ray3 ray, int recursion) {

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

		if(visibleSphere == -1) {
			if(recursion == NUM_RECURSIVE_REFLECTIONS)
				return background.SampleBackground(ray.vector);
			return lightProbe.SampleLightProbe(ray.vector);
		}

		// Ambient / emissive
		Color3f sum = Color3f(elements[visibleSphere].colorAmbient);

		// Diffuse
		Color3f mat = Color3f(elements[visibleSphere].colorDiffuse);
		for(int i=0; i < lights.size(); i++) {
			float diffuseMultiplier = elements[visibleSphere].DiffuseIllumination(ray.extend(dist), lights[i]);
			diffuseMultiplier *= lights[i].brightness;
			Color3f light = Color3f(lights[i].color);
			sum += diffuseMultiplier *light * mat;// * light
		}

		if(recursion <= 0)
			return sum;
		recursion--;

		if(elements[visibleSphere].colorSpecular == RGBAtoU32(0, 0, 0))
			return sum;

		// Specular
		mat = Color3f(elements[visibleSphere].colorSpecular);
		for(int i=0; i < lights.size(); i++) {
			float specularMultiplier = elements[visibleSphere].SpecularIllumination(ray.extend(dist), lights[i], ray.vector);
			specularMultiplier *= lights[i].brightness;
			Color3f light = Color3f(lights[i].color);
			sum += specularMultiplier * light * mat;
		}

		// Reflective
		Ray3 reflectedRay = elements[visibleSphere].ReflectRay(ray, dist);
		Color3f reflectedColor = this->TraceRay(reflectedRay, recursion);
		sum += mat / 255.0f * reflectedColor;

		return sum;
	}

	struct background {
		void * pixels;
		AndroidBitmapInfo info;

		Color3f SampleBackground(Vector3 vec) {
			vec.Normalize();
			float x = vec.y+.5f;
			float y = vec.z+.5f;
			return bilinearSample(info.width*x, info.height*y);
		}
	private:
		Color3f Lerp(Color3f c1, Color3f c2, float t) {
			return Color3f((1-t)*c1.r + t*c2.r, (1-t)*c1.g + t*c2.g, (1-t)*c1.b + t*c2.b);
		}

		Color3f bilinearSample(float x, float y) {
			Color3f bottomLeft = SampleBitmap(floor(x), floor(y));
			Color3f bottomRight = SampleBitmap(ceil(x), floor(y));
			Color3f topLeft = SampleBitmap(floor(x), ceil(y));
			Color3f topRight = SampleBitmap(ceil(x), ceil(y));

			Color3f bottomLerp = Lerp(bottomLeft, bottomRight, x - floor(x));
			Color3f topLerp = Lerp(topLeft, topRight, x - floor(x));

			return Lerp(bottomLerp, topLerp, y - floor(y));
		}

		Color3f SampleBitmap(int x, int y) {
			if(x >= info.width || x < 0|| y >= info.height || y < 0)
				return 0;
			return Color3f(* pixRef(info, pixels, x, y));
		}
	} background;

	struct lightProbe {
		void * pixels;
		AndroidBitmapInfo info;

		Color3f SampleLightProbe(Vector3 vec) {
			vec.Normalize();
			float x = vec.y/4.0f+.5f;
			float y = vec.z/4.0f+.5f;
			return SampleBitmap(info.width*x, info.height*y)*1.2f;
		}

		Color3f BilinearSampleLightProbe(Vector3 vec) {
			vec.Normalize();
			float x = vec.y/4.0f+.5f;
			float y = vec.z/4.0f+.5f;
			return bilinearSample(info.width*x, info.height*y);
		}

	private:
		Color3f Lerp(Color3f c1, Color3f c2, float t) {
			return Color3f((1-t)*c1.r + t*c2.r, (1-t)*c1.g + t*c2.g, (1-t)*c1.b + t*c2.b);
		}

		Color3f bilinearSample(float x, float y) {
			Color3f bottomLeft = SampleBitmap(floor(x), floor(y));
			Color3f bottomRight = SampleBitmap(ceil(x), floor(y));
			Color3f topLeft = SampleBitmap(floor(x), ceil(y));
			Color3f topRight = SampleBitmap(ceil(x), ceil(y));

			Color3f bottomLerp = Lerp(bottomLeft, bottomRight, x - floor(x));
			Color3f topLerp = Lerp(topLeft, topRight, x - floor(x));

			return Lerp(bottomLerp, topLerp, y - floor(y));
		}

		Color3f SampleBitmap(int x, int y) {
			if(x >= info.width || x < 0|| y >= info.height || y < 0)
				return 0;
			return Color3f(* pixRef(info, pixels, x, y));
		}
	} lightProbe;

private:
	std::vector<Sphere3> elements;
	std::vector<PointLight> lights;
	Camera mCamera;
};

#endif  // __SCENE_H__
