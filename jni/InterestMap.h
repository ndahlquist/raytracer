
#ifndef __INTERESTMAP_H__
#define __INTERESTMAP_H__

#include <math.h> // We will need some math.
#include <algorithm>

class InterestMap {
public:
	InterestMap(unsigned int width, unsigned int height, unsigned int frequency=20) {
		this->frequency = frequency;
		arrayWidth = width / frequency;
		arrayHeight = height / frequency;
		map = new float[arrayWidth * arrayHeight];
		int i=0;
		for(int y=0; y<arrayHeight; y++) {
			for(int x=0; x<arrayWidth; x++) {
				map[y*arrayWidth + x] = 0.0f;
				i++;
			}
		}
	}

#define MAX_PROBABILITY 1.0f
#define MIN_PROBABILITY .05f
#define PROBABILITY_DECAY .98f

	void Post(unsigned int xCoor, unsigned int yCoor, uint32_t oldValue, uint32_t newValue) {
		unsigned int x = xCoor / frequency;
		unsigned int y = yCoor / frequency;
		x = std::min(x, arrayWidth-1);
		y = std::min(y, arrayHeight-1);
		if(oldValue != newValue)
			map[y*arrayWidth + x] = MAX_PROBABILITY-MIN_PROBABILITY;
		else
			map[y*arrayWidth + x] *= PROBABILITY_DECAY;
	}

	bool Get(unsigned int xCoor, unsigned int yCoor) {
		unsigned int x = xCoor / frequency;
		unsigned int y = yCoor / frequency;
		x = std::min(x, arrayWidth-1);
		y = std::min(y, arrayHeight-1);
		float p = map[y*arrayWidth + x] + MIN_PROBABILITY;
		if(rand() % 1000 > p * 1000)
			return false;
		return true;
	}
private:
	unsigned int frequency;
	unsigned int arrayWidth;
	unsigned int arrayHeight;
	unsigned int imageWidth;
	unsigned int imageHeight;
	float * map;
};

#endif  // __INTERESTMAP_H__
