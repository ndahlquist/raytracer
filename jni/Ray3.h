#ifndef __RAY3_H__
#define __RAY3_H__

#include <math.h> // We will need some math.

#include "Vector3.h"
#include "Point3.h"

struct Ray3 {
     //
     // Initalization
     //
     //inline Ray3();
     inline Ray3(const Vector3& v){
		this->endpoint = Point3(0,0,0);
		this->vector = v;
     }
     // inline explicit Ray3(const Point3& p);
     // inline Ray3(float x, float y, float z);
     //inline Ray3(float s);
	inline Ray3(const Point3& origin, const Point3& endpoint) {
		this->endpoint = origin;
		this->vector = endpoint - origin;
	}

	inline Ray3(const Point3& origin, const Vector3& vector) {
		this->endpoint = origin;
		this->vector = vector;
	}

	inline Point3 Extend(float dist) const {
		Vector3 newVector = vector;
		newVector.Normalize();
		newVector *= dist;
		return endpoint + newVector;
	}
	/*
    //
    // Assignment
    //
    inline Ray3& operator=(const Ray3& v);

    //
    // Overloaded operators
    //
    inline Ray3& operator*=(float right);
    inline Ray3& operator/=(float right);
    inline Ray3& operator+=(const Ray3& right);
    inline Ray3& operator-=(const Ray3& right);

    //
    // Normalization
    //
    inline void Normalize();
    inline void SetLength(float newLength);

    //
    // Math
    //
    inline float Length() const;
    inline float LengthSq() const;

    //
    // Validation
    //
    inline bool Valid() const;

    //
    // Component accessors
    //
    inline float& Component(unsigned int index)
    {
        return ((float *)this)[index];
    }

    inline float Component(unsigned int index) const
    {
        return ((const float *)this)[index];
    }




    //
    // Static math functions
    //
    /*inline static Ray3 Cross(const Ray3& left, const Ray3& right);
    inline static float Dot(const Ray3& left, const Ray3& right);
    inline static Ray3 DirectProduct(const Ray3& left, const Ray3& right);
    inline static Ray3 Lerp(const Ray3& left, const Ray3& right, float s);
    inline static Ray3 ComponentMax(const Ray3& left, const Ray3& right);
    inline static Ray3 ComponentMin(const Ray3& left, const Ray3& right);*/

    // Local members
    Point3 endpoint;
    Vector3 vector;

};

/*inline Ray3 operator*(const Ray3& left, float right);
inline Ray3 operator*(float left, const Ray3& right);
inline Ray3 operator/(const Ray3& left, float right);
inline Ray3 operator+(const Ray3& left, const Ray3& right);
inline Ray3 operator-(const Ray3& left, const Ray3& right);
inline Ray3 operator-(const Ray3& v);*/

//inline Ray3 operator-(const Point3& left, const Point3& right)

#endif  // __RAY3_H__

