#pragma once
#include "ray.h"
#include "vec3.h"
#include "object.h"
#include "sphere.h"
#include "raytracer.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <cassert>

struct BBox {
	vec3 Min;
	vec3 Max;

	BBox() {};
	BBox(const vec3& Mn, const vec3& Mx) : Min(Mn), Max(Mx) {}
	explicit BBox(const vec3& Point) : BBox(Point, Point) {}

	BBox& Extend(const BBox& other) {
		Min = min(Min, other.Min);
		Max = max(Max, other.Max);
	}

	vec3 Size() {
		return Max - Min;
	}

	int LargestAxis() {
		vec3 s = Size();
		int Axis = 0;
		Axis = s.x > max(s.y, s.z) ? 0 : s.y > s.z ? 1 : 2;
		return Axis;
	}

	// Calculate half of the surface area
	float HalfArea() {
		vec3 s = Size();
		return (s[0] + s[1]) * s[2] + s[0] * s[1];
	}

	static BBox Empty() {
		return BBox(
			vec3(+std::numeric_limits<float>::max()),
			vec3(-std::numeric_limits<float>::max());
		)
	}

};
