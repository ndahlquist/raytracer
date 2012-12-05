
#ifndef __HEATMAP_H__
#define __HEATMAP_H__

#include <math.h> // We will need some math.
#include <algorithm>

class HeatMap {
public:

#define MAX_PROBABILITY 1.0f
#define MIN_PROBABILITY 0.01f
#define PROBABILITY_DECAY .02f

	HeatMap(unsigned int width, unsigned int height, unsigned int frequency=10) {
		this->frequency = frequency;
		arrayWidth = width / frequency;
		arrayHeight = height / frequency;
		map = new float[arrayWidth * arrayHeight];
		for(int y=0; y<arrayHeight; y++) {
			for(int x=0; x<arrayWidth; x++) {
				map[y*arrayWidth + x] = 0.0f;
			}
		}
	}

	void Post(unsigned int xCoor, unsigned int yCoor) {
		unsigned int x = xCoor / frequency;
		unsigned int y = yCoor / frequency;
		*MapCoordinate(x, y) = MAX_PROBABILITY-MIN_PROBABILITY;
		*MapCoordinate(x+1, y+1) = MAX_PROBABILITY-MIN_PROBABILITY;
		*MapCoordinate(x+1, y-1) = MAX_PROBABILITY-MIN_PROBABILITY;
		*MapCoordinate(x-1, y+1) = MAX_PROBABILITY-MIN_PROBABILITY;
		*MapCoordinate(x-1, y-1) = MAX_PROBABILITY-MIN_PROBABILITY;
	}

	bool GetFlip(unsigned int xCoor, unsigned int yCoor) {
		float p = GetProbability(xCoor, yCoor);
		if(rand() % 1024 > p * 1024)
			return false;
		return true;
	}

	float GetProbability(int xCoor, int yCoor) {
		unsigned int x = xCoor / frequency;
		unsigned int y = yCoor / frequency;
		float currentP = * MapCoordinate(x, y);
		* MapCoordinate(x, y) = std::max(0.0f, *MapCoordinate(x, y)-PROBABILITY_DECAY);
		return currentP + MIN_PROBABILITY;
	}

private:

	float * MapCoordinate(int x, int y) {
		x = std::min(x, (int) arrayWidth-1);
		x = std::max(0, x);
		y = std::min(y, (int) arrayHeight-1);
		y = std::max(0, y);
		return map + y*arrayWidth + x;
	}

	unsigned int frequency;
	unsigned int arrayWidth;
	unsigned int arrayHeight;
	unsigned int imageWidth;
	unsigned int imageHeight;
	float * map;
};

#endif  // __HEATMAP_H__
