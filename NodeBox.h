#pragma once
#include <cfloat>
#include <cstddef>
#include <ios>
#include <vector>
#include <numeric>

#include "vec3.h"

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
			vec3(-std::numeric_limits<float>::max()));
	}
};


struct NewNode {
	BBox bbox;
	unsigned int SpheresCount;
	unsigned int FirstIndex;

	NewNode() {};
	NewNode(const BBox& box, unsigned int sphereCount, unsigned int firstIndex) :
		SpheresCount(sphereCount), FirstIndex(firstIndex) {}

	bool IsLeaf() const {
		return FirstIndex != 0;
	}
};

struct BuildConfig {
	size_t MinPrim;
	size_t MaxPrim;
	float TraversalCost;

};
static constexpr BuildConfig build_config = { 2,8,1.0f };

struct Bin{
    BBox bbox = BBox::Empty();
    size_t SphereCount = 0;

    Bin& extend(const Bin &other){
        bbox.Extend(other. bbox);
        SphereCount += other.SphereCount;
        return *this;
    }

    float cost()  {
        return bbox.HalfArea() * SphereCount;
    }
};
static constexpr size_t bin_count = 16;

static size_t bin_index(int axis, const BBox& bbox, const vec3& center) {
    int index = (center[axis] - bbox.Min[axis]) * (bin_count / (bbox.Max[axis] - bbox.Min[axis]));
    return std::min(bin_count - 1, static_cast<size_t>(std::max(0, index)));
}

