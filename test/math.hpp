#include <array>

typedef std::array<float, 2> vec2;

vec2 operator+(const vec2& a, const vec2& b) {
	return {a[0] + b[0], a[1] + b[1]};
}

vec2 operator+=(vec2& a, const vec2& b) {
	a[0] += b[0];
	a[1] += b[1];
	return a;
}

vec2 operator-(const vec2& a, const vec2& b) {
	return {a[0] - b[0], a[1] - b[1]};
}

vec2 operator*(const vec2& a, float b) {
	return {a[0] * b, a[1] * b};
}

vec2 operator*=(vec2& a, float b) {
	a[0] *= b;
	a[1] *= b;
	return a;
}

vec2 operator/=(vec2& a, float b) {
	a[0] /= b;
	a[1] /= b;
	return a;
}

vec2 operator/(const vec2& a, float b) {
	return {a[0] / b, a[1] / b};
}
