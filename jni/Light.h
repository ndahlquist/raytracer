#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <math.h> // We will need some math.

#include "Point3.h"

struct PointLight {
	PointLight(Point3 position, float brightness) {
		this->position = position;
		this->brightness = brightness;
		this->color = RGBAtoU32(255, 255, 255);
	}

	Point3 position;
	float brightness;
	uint32_t color;
};

#endif
