#ifndef __BAH_H__
#define __BAH_H__

#include <math.h> // We will need some math.
#include <vector>
#include <algorithm> // sort

class BAH {
private:

	struct sphereBound {
		float bound;
		int sphereIndexMask;
		bool operator() (const sphereBound &c1, const sphereBound &c2) {
			return (c1.bound < c2.bound);
		}
	};

public:

	BAH() {}
	
	// Initialization functions (called once per frame)
	void Initialize(const Point3 eye) {
		this->eye = eye;
		spheres.clear();
		miny.clear();
		maxy.clear();
		minz.clear();
		maxz.clear();
	}
	
	void Index(Sphere3 * s) {
		spheres.push_back(s);
	}
	
	void Sort() {
		for(int i=0; i<spheres.size(); i++) {
			Sphere3 * s = spheres[i];
			Vector3 toCenter = s->center - eye;
			float distToTangent = sqrt(toCenter.LengthSq() - s->radius);
			toCenter.Normalize();
			float sinOfAngle = s->radius / distToTangent;
			float cosOfAngle = 1 - pow(s->radius / distToTangent, 2);

			// Find the line perpendicular to toCenter, in the plane that also contains Vector3(0,1,0)
			Vector3 perpendicular = Vector3::Cross(toCenter, Vector3::Cross(toCenter, Vector3(0,1,0))); // TODO: simplify this.
			perpendicular.Normalize();
		
			Vector3 toTangent = toCenter * cosOfAngle - perpendicular * sinOfAngle;
			struct sphereBound thisminy;
			thisminy.sphereIndexMask = 1 << i;
			thisminy.bound = toTangent.y;
			miny.push_back(thisminy);
			
			toTangent = toCenter * cosOfAngle + perpendicular * sinOfAngle;
			struct sphereBound thismaxy;
			thismaxy.sphereIndexMask = 1 << i;
			thismaxy.bound = toTangent.y;
			maxy.push_back(thismaxy);

			// Find the line perpendicular to toCenter, in the plane that also contains Vector3(0,0,1)
			perpendicular = Vector3::Cross(toCenter, Vector3::Cross(toCenter, Vector3(0,0,1))); // TODO: simplify this.
			perpendicular.Normalize();

			toTangent = toCenter * cosOfAngle - perpendicular * sinOfAngle;
			struct sphereBound thisminz;
			thisminz.sphereIndexMask = 1 << i;
			thisminz.bound = toTangent.z;
			minz.push_back(thisminz);

			toTangent = toCenter * cosOfAngle + perpendicular * sinOfAngle;
			struct sphereBound thismaxz;
			thismaxz.sphereIndexMask = 1 << i;
			thismaxz.bound = toTangent.z;
			maxz.push_back(thismaxz);
		}
		
		struct sphereBound cmp;
		
		std::sort(miny.begin(), miny.end(), cmp);
		VectorMaskCascade(miny);
		std::sort(maxy.begin(), maxy.end(), cmp);
		VectorMaskCascade(maxy);
		std::sort(minz.begin(), minz.end(), cmp);
		std::sort(maxz.begin(), maxz.end(), cmp);
		
		
	}
	
	void VectorMaskCascade(std::vector<sphereBound> & v) {
		int sphereSet = 0;
		for(int i=0; i<v.size(); i++) {
			sphereSet |= v[i].sphereIndexMask;
			v[i].sphereIndexMask = sphereSet;
		}
	}
	
	// Called once per viewing ray.
	std::vector<Sphere3 *> IntersectionCandidates(Ray3 r) {
		std::vector<Sphere3 *> candidates;
		
		// TODO: cache y or z preference
		int candidateSet = getYCandidates(r);
		if(candidateSet == 0)
			return candidates;

		candidateSet &= getZCandidates(r);
		if(candidateSet == 0)
			return candidates;

		for(int i=0; i<spheres.size(); i++) {
			int mask = 1 << i;
			if((candidateSet & mask) != 0)
				candidates.push_back(spheres[i]);
		}

		return candidates;
	}

private:

	// Returns a set representation of the spheres inside y bounds.
	int getYCandidates(Ray3 ray) {
		//int low = 0, high = miny.size()-1;
		//int mid = (low+high+1)/2, prev = -1;
		/*while(mid != prev) {
			prev = mid;
			if(ray.vector.y > miny[mid].bound)
				low = mid;
			else
				high = mid;
			mid = (low+high+1)/2;
		}
		
		int minCandidates = miny[low].sphereIndexMask;
		return minCandidates;*/
		int mid = miny.size()-1;
		while(miny[mid].bound > ray.vector.y) {
			mid--;
			if(mid <= 0)
				break;
		}
		return miny[mid].sphereIndexMask;
		
		/*mid = 0;
		while(maxy[mid].bound < ray.vector.y) {
			mid++;
		}
		int maxCandidates = ~0;//maxy[mid].sphereIndexMask;
		return (minCandidates & maxCandidates);
		//for(int i=0; i<miny.size(); i++) {
		//	if(ray.vector.y > miny[i].bound)
		//		minCandidates = miny[i].sphereIndexMask;
		//}
		/*low = 0; high = maxy.size()-1;
		mid = (low+high+1)/2; prev = -1; // TODO
		while(mid != prev) {
			prev = mid;
			if(ray.vector.y < maxy[mid].bound)
				low = mid;
			else
				high = mid;
			mid = (low+high+1)/2;
		}
		
		int maxCandidates = maxy[low].sphereIndexMask;
		/*for(int i=0; i<maxy.size(); i++) {
			if(ray.vector.y < maxy[i].bound)
				maxCandidates |= maxy[i].sphereIndexMask;
		}*/
		//return maxCandidates;
		
		//return (minCandidates & maxCandidates);
	}

	// Returns a set representation of the spheres inside z bounds.
	int getZCandidates(Ray3 ray) {
		int mincandidates = 0;
		for(int i=0; i<minz.size(); i++) {
			if(ray.vector.z > minz[i].bound)
				mincandidates |= minz[i].sphereIndexMask;
		}
		int maxcandidates = 0;
		for(int i=0; i<maxz.size(); i++) {
			if(ray.vector.z < maxz[i].bound)
				maxcandidates |= maxz[i].sphereIndexMask;
		}
		
		return (mincandidates & maxcandidates);
	}

	Point3 eye;
	std::vector<Sphere3 *> spheres;
	std::vector<sphereBound> miny;
	std::vector<sphereBound> maxy;
	std::vector<sphereBound> minz;
	std::vector<sphereBound> maxz;
};

#endif  // __BAH_H__
