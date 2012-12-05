
#ifndef __COLORUTILS_H__
#define __COLORUTILS_H__

static void RGBAfromU32(const uint32_t in, uint8_t & R, uint8_t & G, uint8_t & B) {
	R = in;
	G = in >> 8;
	B = in >> 16;
}

static __inline__ uint32_t RGBAtoU32(const uint8_t R, const uint8_t G, const uint8_t B) {
	uint32_t output = R;
	output |= G << 8;
	output |= B << 16;
	return output;
}

static __inline__ uint32_t RGBAtoU32(const uint8_t R, const uint8_t G, const uint8_t B, const uint8_t A) {
	uint32_t output = R;
	output |= G << 8;
	output |= B << 16;
	output |= A << 24;
	return output;
}

static __inline__ float constrain(float x) {
	if(x < 0)
		return 0;
	else if(x > 255)
		return 255;
	return x;
}

static __inline__ uint32_t * pixRef(AndroidBitmapInfo & info, void * pixels, unsigned int x, unsigned int y) {
	pixels = (char*) pixels + y * info.stride;
	uint32_t * line = (uint32_t *) pixels;
	return line + x;
}

float * getPixel(float * inputPixels, unsigned int x, unsigned int y, unsigned int channel, unsigned int width) {
	return inputPixels + 3 * y * width  + 3 * x + channel;
}

void U32BuftoFloatBuf(void * inputPixels, float * outputBuffer, int length) {
	for(int i = 0; i < length; i++) {
		uint32_t * p = (uint32_t *) inputPixels;
		p += i;
		uint8_t R, G, B;
		RGBAfromU32(*p, R, G, B);

		*(outputBuffer + 3*i) = R;
		*(outputBuffer + 3*i + 1) = G;
		*(outputBuffer + 3*i + 2) = B;
	}
}

void FloatBuftoU32Buf(float * inputBuffer, void * outputPixels, int length) {
	for(int i = 0; i < length; i++) {
		uint8_t R, G, B;
		R = *(inputBuffer + 3*i);
		G = *(inputBuffer + 3*i + 1);
		B = *(inputBuffer + 3*i + 2);

		uint32_t * p = (uint32_t *) outputPixels;
		p += i;
		*p = RGBAtoU32(R, G, B);
	}
}

class Color3f {
public:
	float r, g, b;

	Color3f(float R, float G, float B) {
		r = R;
		g = G;
		b = B;
	}

	Color3f(uint32_t v) {
		uint8_t R, G, B;
		RGBAfromU32(v, R, G, B);
		r = R;
		g = G;
		b = B;
	}

	uint32_t U32() {
		return RGBAtoU32((uint8_t) r, (uint8_t) g, (uint8_t) b);
	}

};

inline Color3f operator*(const Color3f& v, float a) {
	return Color3f(v.r*a, v.g*a, v.b*a);
}
inline Color3f operator*(float a, const Color3f& v) {
	return Color3f(v.r*a, v.g*a, v.b*a);
}
inline Color3f operator+(const Color3f& left, const Color3f& right) {
	return Color3f(left.r + right.r, left.g + right.g, left.b + right.b);
}
inline Color3f operator-(const Color3f& left, const Color3f& right) {
	return Color3f(left.r - right.r, left.g - right.g, left.b - right.b);
}

#endif
