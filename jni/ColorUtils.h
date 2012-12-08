
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

class Color3f {
public:
	float r, g, b;

	Color3f() {
		r = 0;
		g = 0;
		b = 0;
	}

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
		return RGBAtoU32(constrain(r), constrain(g), constrain(b));
	}

	float Luminance() {
		return 0.299f*b + 0.587f*g + 0.114f*b;
	}

	inline Color3f& operator+=(const Color3f& right) {
		r += right.r;
		g += right.g;
		b += right.b;
		return *this;
	}

};

inline Color3f operator*(const Color3f& v, float a) {
	return Color3f(v.r*a, v.g*a, v.b*a);
}
inline Color3f operator*(float a, const Color3f& v) {
	return Color3f(v.r*a, v.g*a, v.b*a);
}
inline Color3f operator*(const Color3f& a, const Color3f& b) {
	return Color3f(a.r*b.r, a.g*b.g, a.b*b.b);
}
inline Color3f operator+(const Color3f& left, const Color3f& right) {
	return Color3f(left.r + right.r, left.g + right.g, left.b + right.b);
}
inline Color3f operator-(const Color3f& left, const Color3f& right) {
	return Color3f(left.r - right.r, left.g - right.g, left.b - right.b);
}
inline Color3f operator/(const Color3f& c, float a) {
	return Color3f(c.r/a, c.g/a, c.b/a);
}
inline bool operator==(const Color3f& left, const Color3f& right) {
	return left.r==right.r && left.g==right.g && left.b==right.b;
}

#endif
