#pragma once
#include <array>
#include <cstddef>
#include <iostream>
#include "NodeBox.h"

struct BVH {
	std::vector<NewNode> Nodes;
	std::vector<size_t> SpheresIndex;

	BVH() {};

	static void Build(const BBox* box, const vec3* centers, size_t sphereCount) {
		BVH bvh;
		bvh.SpheresIndex.resize(sphereCount);
		// Add val to vec sequentially start from 0
		std::iota(bvh.SpheresIndex.begin(), bvh.SpheresIndex.end(), 0);

		bvh.Nodes.resize(2 * sphereCount - 1);
		bvh.Nodes[0].SpheresCount = sphereCount;
		bvh.Nodes[0].FirstIndex = 0;

		// Track number of node created
		size_t NodeCount = 1;
		BulidRecursive(bvh, 0, NodeCount, box, centers);
		// Resize after bulid
		bvh.Nodes.resize(NodeCount);
	}

	static void BulidRecursive(BVH& bvh, size_t NodeIndex, size_t& NodeCount, const BBox* bboxes, const vec3* centers) {
		auto& node = bvh.Nodes[NodeIndex];
		assert(node.IsLeaf());

		node.bbox = BBox::Empty();
		// Iterate throught sphree in node
		for (size_t i = 0; i < node.SpheresCount; i ++ ) {
			// Threshold set in NodeBox.h
			if (node.SpheresCount <= build_config.MaxPrim)
				return;
			// Get the boundign box using index, and expand node's boundin box
			node.bbox.Extend(bboxes[bvh.SpheresIndex[node.FirstIndex + i]]);
		}
	}
};


struct Split {
    int axis = 0;
    float cost = FLT_MAX;
    size_t rightBin = 0;

    operator bool () const {
        return rightBin != 0;
    }

    bool operator < (const Split& other) const {
        return *this && cost < other.cost;
    }
};

static Split findBestSplit(
    int axis,
    const BVH &bvh,
    const NewNode& node,
    const BBox* bboxes,
    const vec3 *center)
{
	// Declare an arr of Bin obj with size bin_count
    std::array<Bin, bin_count> bins;
    for (size_t i = 0; i < node.SpheresCount; i++){
        size_t sphereIndex = bvh.SpheresIndex[node.FirstIndex + i];
        auto &bin = bins[bin_index(axis, node.bbox, center[sphereIndex])];
        bin.bbox.Extend(bboxes[sphereIndex]);
        bin.SphereCount++;
    }
	std::array<float, bin_count> rightCost;
	Bin LeftAccumulation, RightAccumulation;
	for (size_t i = bin_count - 1; i > 0; i--) {
		RightAccumulation.extend(bins[i]);
		rightCost[i] = RightAccumulation.cost();
	}
	Split split{
		axis
	};
	for (size_t i = 0; i < bin_count - 1; i++) {
		LeftAccumulation.extend(bins[i]);
		float cost = LeftAccumulation.cost() + rightCost[i + 1];
		if (cost < split.cost) {
			split.cost = cost;
			split.rightBin = i + 1;
		}
	}
	return split;
}

