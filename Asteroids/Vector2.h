#pragma once
#include <corecrt_math.h>

// Custom struct for storing and manipulating 2-dimensional vectors
struct Vector2 {
	Vector2() {
		reset();
	}
	Vector2(float val) {
		x = val;
		y = val;
	}
	Vector2(float _x, float _y) {
		x = _x;
		y = _y;
	}
	static Vector2 lerp(Vector2 a, Vector2 b, float t) {
		return a * (1 - t) + b * t;
	}
	static Vector2 Unit(float angle) {
		return Vector2(cos(angle), sin(angle));

	}
	float x, y;

	void reset() {
		x = 0;
		y = 0;
	}
	float mag() {
		return sqrtf(powf(x, 2) + powf(y, 2));
	}
	float angle() {
		return atan2f(y, x);
	}
	void operator=(float val) {
		x = val;
		y = val;
	}
	Vector2 operator+(Vector2 vec) {
		Vector2 sum = { x + vec.x, y + vec.y };
		return sum;
	}
	Vector2 operator-(Vector2 vec) {
		Vector2 dif = { x - vec.x, y - vec.y };
		return dif;
	}
	Vector2 operator*(float factor) {
		Vector2 product = { x * factor, y * factor };
		return product;
	}
	Vector2 operator*(Vector2 vector) {
		Vector2 product = { x * vector.x, y * vector.y };
		return product;
	}
	Vector2 operator/(float factor) {
		Vector2 product = { x / factor, y / factor };
		return product;
	}
	void operator+=(Vector2 vec) {
		x += vec.x;
		y += vec.y;
	}
	void operator-=(Vector2 vec) {
		x -= vec.x;
		y -= vec.y;
	}
	void operator*=(float divisor) {
		x *= divisor;
		y *= divisor;
	}
	void operator/=(float divisor) {
		x /= divisor;
		y /= divisor;
	}
	bool operator==(Vector2 comparator) {
		return (x == comparator.x && y == comparator.y);
	}
};