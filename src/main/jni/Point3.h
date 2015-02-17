#ifndef __Point3_H__
#define __Point3_H__

#include <math.h> // We will need some math.

#include "Vector3.h"

/**
*  Simple struct to represent 3D points
*/
struct Point3 {

    inline Point3() {
    	this->x = 0.0f;
    	this->y = 0.0f;
    	this->z = 0.0f;
    }
    inline Point3(float x, float y, float z) {
    	this->x = x;
    	this->y = y;
    	this->z = z;
    }
    inline explicit Point3(const Vector3& v) {
    	this->x = v.x;
    	this->y = v.y;
    	this->z = v.z;
    }

    //inline Point3& operator+=(const STVector3& right);
  //  inline Point3& operator-=(const STVector3& right);

    /**
    * Returns distance between two points
    * Called as Point3::Dist(left, right)
    */
    //static inline float Dist(const Point3& left, const Point3& right);

    /**
    * Returns distance squared between two points
    * Called as Point3::DistSq(left, right)
    */
    static inline float DistSq(const Point3& left, const Point3& right) {
    	return pow(left.x - right.x, 2) + pow(left.y - right.y, 2) + pow(left.z - right.z, 2);
    }

    static inline Point3 Lerp(const Point3& left, const Point3& right, float t) {
    	float x = (1.0f - t) * left.x + t * right.x;
    	float y = (1.0f - t) * left.y + t * right.y;
    	float z = (1.0f - t) * left.z + t * right.z;
    	return Point3(x,y,z);
    }

    float x, y, z;
};

inline Vector3 operator-(const Point3& left, const Point3& right) {
	return Vector3(left.x - right.x, left.y - right.y, left.z - right.z);
}

inline Point3 operator+(const Point3& left, const Vector3& right) {
	return Point3(left.x + right.x, left.y + right.y, left.z + right.z);
}
//inline Point3 operator+(const STVector3& left, const Point3& right);
//inline Point3 operator-(const Point3& left, const STVector3& right);
inline bool operator==(const Point3& left, const Point3& right) {
	return left.x==right.x && left.y==right.y && left.z==right.z;
}

#endif  // __Point3_H__
