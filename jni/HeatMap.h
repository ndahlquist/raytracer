
#ifndef __HEATMAP_H__
#define __HEATMAP_H__

#include <math.h> // We will need some math.
#include <algorithm>

class HeatMap {
public:

#define REGION_SIZE 5
#define MAX_PROBABILITY 1.0f
#define DECAY_RATE .3f
#define RANDOM_SAMPLING_RATE 20

	HeatMap(unsigned int width, unsigned int height) {
		arrayWidth = width / REGION_SIZE;
		arrayHeight = height / REGION_SIZE;
		map = new float[arrayWidth * arrayHeight];
		for(int y=0; y<arrayHeight; y++) {
			for(int x=0; x<arrayWidth; x++) {
				map[y*arrayWidth + x] = MAX_PROBABILITY;
			}
		}
	}

	void Post(unsigned int xCoor, unsigned int yCoor) {
		const unsigned int x = xCoor / REGION_SIZE;
		const unsigned int y = yCoor / REGION_SIZE;
		*MapCoordinate(x, y) = MAX_PROBABILITY;
		*MapCoordinate(x+1, y+1) = MAX_PROBABILITY;
		*MapCoordinate(x+1, y-1) = MAX_PROBABILITY;
		*MapCoordinate(x-1, y+1) = MAX_PROBABILITY;
		*MapCoordinate(x-1, y-1) = MAX_PROBABILITY;
	}

	void DecayRegions() {
		for(int y=0; y<arrayHeight; y++) {
			for(int x=0; x<arrayWidth; x++) {
				map[y*arrayWidth + x] -= DECAY_RATE;
			}
		}
	}

	bool GetFlip(unsigned int xCoor, unsigned int yCoor) {
		const unsigned int x = xCoor / REGION_SIZE;
		const unsigned int y = yCoor / REGION_SIZE;
		const float p = *MapCoordinate(x, y);
		if(p > 0)
			return true;
		if(rand()%RANDOM_SAMPLING_RATE == 0)
			return true;
		return false;
	}

private:

	float * MapCoordinate(int x, int y) {
		x = std::min(x, (int) arrayWidth-1);
		x = std::max(0, x);
		y = std::min(y, (int) arrayHeight-1);
		y = std::max(0, y);
		return map + y*arrayWidth + x;
	}

	unsigned int arrayWidth;
	unsigned int arrayHeight;
	float * map;
};

#endif  // __HEATMAP_H__
