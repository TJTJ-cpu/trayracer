#pragma once
#include "ray.h"
#include "vec3.h"
#include "object.h"
#include "sphere.h"
#include "raytracer.h"
#include <algorithm>
#include <iostream>


class Node;
class BoundingBox;


class BoundingBox 
{
public:
	vec3 Min = vec3(0,0,0);
	vec3 Max = vec3(0,0,0);
	vec3 Center = (Min + Max) / 2;
	bool bHasObject = false;

	// Make the bouding box include the sphere that we pass in
	void GrowToInclude(const Sphere& sphere) {
		Min.x = std::min(Min.x, sphere.center.x - sphere.radius);
		Min.y = std::min(Min.y, sphere.center.y - sphere.radius);
		Min.z = std::min(Min.z, sphere.center.z - sphere.radius);

		Max.x = std::max(Max.x, sphere.center.x + sphere.radius);
		Max.y = std::max(Max.y, sphere.center.y + sphere.radius);
		Max.z = std::max(Max.z, sphere.center.z + sphere.radius);
		Center = (Min + Max) / 2;
		this->bHasObject = true;
	}

	bool BoxIntersection2(Ray ray) {

	}

	bool BoxIntersection(Ray ray) {
		float tMin, tMax, tyMin, tyMax, tzMin, tzMax;

		tMin = (this->Min.x - ray.Origin.x) * ray.InvRayDir.x;
		tMax = (this->Max.x - ray.Origin.x) * ray.InvRayDir.x;

		tyMin = (this->Min.y - ray.Origin.y) * ray.InvRayDir.y;
		tyMax = (this->Max.y - ray.Origin.y) * ray.InvRayDir.y;

		// CHECK POINT ALONG Y-AXIS
		if ((tMin > tyMax) || (tyMin > tMax))
			return false;
		// GET THE EARLIEST POINT THAT THE RAY ENTER THE BOX
		if (tyMin > tMin)
			tMin = tyMin;
		// GET THE LATEST POINT THAT THE RAY EXIT THE BOX
		if (tyMax < tMax)
			tMax = tyMax;

		tzMin = (this->Min.z - ray.Origin.z) * ray.InvRayDir.z;
		tzMax = (this->Max.z - ray.Origin.z) * ray.InvRayDir.z;
		if ((tMin > tzMax) || (tzMin > tMax))
			return false;

		return true;
	}

	vec3 Size() {
		return this->Max - this->Min;
	}

};


class Node 
{
public:
	BoundingBox bounds;
	std::vector<Sphere> spheres;
	Node* ChildA = nullptr;
	Node* ChildB = nullptr;

	Node() {};

	Node(BoundingBox box)
	: bounds(box){
	}

	Node(BoundingBox box, const std::vector<Sphere> spheres)
	: bounds(box), spheres(spheres) {
		Build(spheres);
		//SplitNode(this, 0);
	}

	~Node() {
		delete ChildA;
		delete ChildB;
	}

	void AddSphere(Sphere sphere) {
		spheres.push_back(sphere);
	}

	// Build BVH
	void Build(const std::vector<Sphere>& spheres) {
		for (const auto& sphere : spheres) 
			bounds.GrowToInclude(sphere);
		return;
	}

	void Spilt(Node* node, int depth) {
		// std::cout << "SplitNode" << std::endl;
		const int MaxDepth = 32;
		const int MinSpherePerLeaf = 2;

		if (depth == MaxDepth) {
			// std::cout << "MaxDepth Reached" << std::endl;
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

	void SplitNode(Node* parent, int depth) {
		const int MaxDepth = 32;

		if (depth == MaxDepth || !parent->bounds.bHasObject || parent->spheres.size() <= 3) {
			return;
		}

		vec3 size = parent->bounds.Size();
		int SpiltAxis = size.x > max(size.y, size.z) ? 0 : size.y > size.z ? 1 : 2;
		float SplitPos = parent->bounds.Center[SpiltAxis];
		//std::cout << std::endl;
		//std::cout << "Depth-> " << depth << std::endl;
		//std::cout << "Object Count: " << parent->spheres.size() << std::endl;
		//std::cout << "SIZE-> x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
		//std::cout << "SpiltAxis: " << SpiltAxis << std::endl;
		//std::cout << "SplitPos: " << SplitPos << std::endl;
		//std::cout << std::endl;

		parent->ChildA = new Node();
		parent->ChildB = new Node();

		for (Sphere sphere : parent->spheres) {
			bool bInA = sphere.center[SpiltAxis] < SplitPos;
			Node *Child = bInA ? parent->ChildA : parent->ChildB;
			Child->AddSphere(sphere);
			Child->bounds.GrowToInclude(sphere);
			//size = Child->bounds.Size();
			//std::cout << "x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
		}
		SplitNode(parent->ChildA, depth + 1);
		SplitNode(parent->ChildB, depth + 1);
	}
	//
	// HitResult BVHIntersect(Node *parent, const Ray& ray) const {
    std::vector<HitResult> BVHIntersect(Node *parent, const Ray& ray)  {
        std::vector<HitResult> HitVec;
		HitResult hit;

		//if (parent->spheres.empty())
		//	return HitVec;

		if (parent->ChildA == nullptr && parent->ChildB == nullptr) 
		{
			/// TO DO 
			// std::cout << "SphereCount: " << parent->spheres.size() << std::endl;
			// LOOP THROUGH IT AND THEN GET THE CLOSEST SPHERE
			for (Sphere sphere : parent->spheres) {
				HitResult SphereHit = sphere.Intersect(ray);
				if (SphereHit.HasValue() && SphereHit.t < hit.t)
                    HitVec.push_back(SphereHit);
			}
			if (HitVec.size() > 0)
				return HitVec;
			else 
				return HitVec;
		}
        std::vector<HitResult> hitA, hitB;

		//if (parent->ChildA && parent->ChildA->bounds.BoxIntersection(ray))
		//	hitA = BVHIntersect(parent->ChildA, ray);
		//if (parent->ChildB && parent->ChildB->bounds.BoxIntersection(ray))
		//	hitB = BVHIntersect(parent->ChildB, ray);

		if (parent->ChildA)
			if (parent->ChildA->bounds.BoxIntersection(ray))
				hitA = BVHIntersect(parent->ChildA, ray);
			else
				return hitA;

		if (parent->ChildB)
			if (parent->ChildB->bounds.BoxIntersection(ray))
				hitB = BVHIntersect(parent->ChildB, ray);
			else
				return hitB;

		// if (hitA.HasValue() && hitA.t < hit.t)
		// 	hit = hitA;
		//
		// if (hitB.HasValue() && hitB.t < hit.t)
		// 	hit = hitB;

		return HitVec;
	}

	void LevelOrderTraversal() {
		if (!this)
			return;
		std::queue<Node*> Queue;
		Queue.push(this);
		while (!Queue.empty()) {
			Node* CurrNode = Queue.front();
			vec3 size = CurrNode->bounds.Size();
			//std::cout << std::endl;
			//std::cout << "Spheres count: " << CurrNode->spheres.size() << std::endl;
			//std::cout << "Size->  x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
			if (CurrNode->ChildA)
				Queue.push(CurrNode->ChildA);
			if (CurrNode->ChildB)
				Queue.push(CurrNode->ChildB);
			Queue.pop();
		}
	}
};












