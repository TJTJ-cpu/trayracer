#pragma once
#include "ray.h"
#include "vec3.h"
#include "sphere.h"
#include "raytracer.h"

class Node;
class BoundingBox;

class BoundingBox 
{
public:
	Node root;
	vec3 Min;
	vec3 Max;
	vec3 Center = (Min + Max) / 2;
	bool bHasPoint = false;

	// Make the bouding box include the sphere that we pass in
	void GrowToInclude(const Sphere& sphere) {
		Min.x = std::min(Min.x, sphere.center.x - sphere.radius);
		Min.y = std::min(Min.y, sphere.center.y - sphere.radius);
		Min.z = std::min(Min.z, sphere.center.z - sphere.radius);

		Max.x = std::max(Max.x, sphere.center.x + sphere.radius);
		Max.y = std::max(Max.y, sphere.center.y + sphere.radius);
		Max.z = std::max(Max.z, sphere.center.z + sphere.radius);
	}

	void Spilt(Node* node, int depth) {
		const int MaxDepth = 32;
		const int MinSpherePerLeaf = 2;

		if (depth == MaxDepth) {
			node->isLeaf = true;
			return;
		}

		vec3 BoxSize = node->bounds.Size();
		int SplitAxis = 0;
		if (BoxSize.x > BoxSize.y && BoxSize.x > BoxSize.z)
			SplitAxis = 0;
		else if (BoxSize.y > BoxSize.z)
			SplitAxis = 1;
		else 
			SplitAxis = 2;
			
	}

	// Using slab method
	bool Intersects(const Ray& ray) const {
		float tMin = (Min.x - ray.RayStart.x) / ray.RayDir.x;
		float tMax = (Max.x - ray.RayStart.x) / ray.RayDir.x;
		if (tMin > tMax) 
			std::swap(tMin, tMax);

		float tyMin = (Min.y - ray.RayStart.y) / ray.RayDir.y;
		float tyMax = (Max.y - ray.RayStart.y) / ray.RayDir.y;
		if (tyMin > tyMax) 
			std::swap(tyMin, tyMax);

		if ((tMin > tyMax) || (tyMin > tMax)) 
			return false;

		if (tyMin > tMin) 
			tMin = tyMin;
		if (tyMax < tMax) 
			tMax = tyMax;

		float tzMin = (Min.z - ray.RayStart.z) / ray.RayDir.z;
		float tzMax = (Max.z - ray.RayStart.z) / ray.RayDir.z;
		if (tzMin > tzMax) 
			std::swap(tzMin, tzMax);

		if ((tMin > tzMax) || (tzMin > tMax)) 
			return false;

		return true;
	}

	vec3 Size() {
		return this->Max = this->Min;
	}

};

class Node 
{
public:
	BoundingBox bounds;
	std::vector<Sphere> spheres;
	Node* left = nullptr;
	Node* right = nullptr;
	bool isLeaf = false;

	Node(const BoundingBox& box, const std::vector<Sphere>& spheres)
	: bounds(box), spheres(spheres) {}
};











