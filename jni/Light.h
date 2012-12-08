#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <math.h> // We will need some math.

#include "Point3.h"

struct PointLight {
	PointLight(Point3 position, Color3f color) {
		this->position = position;
		this->color = color;
	}

	Point3 position;
	Color3f color;
};

#endif
