// Point3.h
#ifndef __Point3_H__
#define __Point3_H__

#include <math.h>

#include "Vector3.h"

/**
*  Simple struct to represent 3D points
*/
struct Point3
{

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
   // inline explicit Point3(const STVector3& v);

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
    //static inline float DistSq(const Point3& left, const Point3& right);

    float x, y, z;
};

inline Vector3 operator-(const Point3& left, const Point3& right) {
	return Vector3(left.x - right.x, left.y - right.y, left.z - right.z);
}

//inline Point3 operator+(const Point3& left, const STVector3& right);
//inline Point3 operator+(const STVector3& left, const Point3& right);
//inline Point3 operator-(const Point3& left, const STVector3& right);

#endif  // __Point3_H__
