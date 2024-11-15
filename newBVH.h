#pragma once
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

