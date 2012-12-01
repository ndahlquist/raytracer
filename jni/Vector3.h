// Vector3.h
#ifndef __STVECTOR3_H__
#define __STVECTOR3_H__

#include <math.h>

/**
* Vector3 represents a 3-vector
*/
struct Vector3
{
    //
    // Initalization
    //
    inline Vector3(){
		this->x = 0.0f;
		this->y = 0.0f;
		this->z = 0.0f;
	}
    //inline Vector3(const Vector3& v);
   // inline explicit Vector3(const Point3& p);
   // inline Vector3(float x, float y, float z);
    //inline Vector3(float s);
	inline Vector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	/*
    //
    // Assignment
    //
    inline Vector3& operator=(const Vector3& v);

    //
    // Overloaded operators
    //
    inline Vector3& operator*=(float right);
    inline Vector3& operator/=(float right);
    inline Vector3& operator+=(const Vector3& right);
    inline Vector3& operator-=(const Vector3& right);

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
    /*inline static Vector3 Cross(const Vector3& left, const Vector3& right);
    inline static float Dot(const Vector3& left, const Vector3& right);
    inline static Vector3 DirectProduct(const Vector3& left, const Vector3& right);
    inline static Vector3 Lerp(const Vector3& left, const Vector3& right, float s);
    inline static Vector3 ComponentMax(const Vector3& left, const Vector3& right);
    inline static Vector3 ComponentMin(const Vector3& left, const Vector3& right);*/

    // Local members
    float x, y, z;

};

/*inline Vector3 operator*(const Vector3& left, float right);
inline Vector3 operator*(float left, const Vector3& right);
inline Vector3 operator/(const Vector3& left, float right);
inline Vector3 operator+(const Vector3& left, const Vector3& right);
inline Vector3 operator-(const Vector3& left, const Vector3& right);
inline Vector3 operator-(const Vector3& v);*/

//inline Vector3 operator-(const Point3& left, const Point3& right)

#endif  // __STVECTOR3_H__

