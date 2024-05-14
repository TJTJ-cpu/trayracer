#pragma once

#include "mat4.h"
#include "ray.h"


struct vec2
{
	vec2() {}
	vec2(float _x, float _y) : x(_x), y(_y) {}

	float x;
	float y;
};



class AABB
{
public:

	AABB() {}
	AABB(float X, float Y,float width ,float height) : x(X), y(Y), w(width), h(height) {}

	float x;
	float y;
	float w;
	float h;

	// Functions
	bool Contain(vec2 other);
	// bool Contain(vec3 point)
	// bool Overlaps(AABB other)

};


bool AABB::Contain(vec2 point)
{
	return (
		point.x >= this->x - this->w &&
		point.x <= this->x + this->w &&
		point.y >= this->y - this->h &&
		point.y <= this->y + this->h);
}
