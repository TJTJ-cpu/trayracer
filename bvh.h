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
	void GrowToInclude(Sphere* sphere) {
		Min.x = std::min(Min.x, sphere->center.x - sphere->radius);
		Min.y = std::min(Min.y, sphere->center.y - sphere->radius);
		Min.z = std::min(Min.z, sphere->center.z - sphere->radius);

		Max.x = std::max(Max.x, sphere->center.x + sphere->radius);
		Max.y = std::max(Max.y, sphere->center.y + sphere->radius);
		Max.z = std::max(Max.z, sphere->center.z + sphere->radius);
		this->Center = (Min + Max) / 2;
		this->bHasObject = true;
	}

	bool BoxIntersection(Ray ray) {
        float txMin = (this->Min.x - ray.Origin.x) * ray.InvRayDir.x;
        float txMax = (this->Max.x - ray.Origin.x) * ray.InvRayDir.x;
        float tyMin = (this->Min.y - ray.Origin.y) * ray.InvRayDir.y;
        float tyMax = (this->Max.y - ray.Origin.y) * ray.InvRayDir.y;
        float tzMin = (this->Min.z - ray.Origin.z) * ray.InvRayDir.z;
        float tzMax = (this->Max.z - ray.Origin.z) * ray.InvRayDir.z;

        float tMin = std::max(std::max(std::min(txMin, txMax), std::min(tyMin, tyMax)), std::min(tzMin, tzMax));
        float tMax = std::min(std::min(std::max(txMin, txMax), std::max(tyMin, tyMax)), std::max(tzMin, tzMax));

        if (tMax < 0)
            return false;

        if (tMin > tMax)
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
	std::vector<Sphere*> spheres;
	Node* ChildA = nullptr;
	Node* ChildB = nullptr;

	Node() {};

	Node(BoundingBox box, const std::vector<Sphere*> sp)
	: bounds(box), spheres(sp) {
		Build(sp);
		SplitNode(this, 0);
	}

	~Node() {
		delete ChildA;
		delete ChildB;
	}

	void AddSphere(Sphere* sphere) {
		spheres.push_back(sphere);
        this->bounds.GrowToInclude(sphere);
	}

	// Build BVH
	void Build(const std::vector<Sphere*> spheres) {
		for (auto sphere : spheres) 
			bounds.GrowToInclude(sphere);
		return;
	}

	int GetTreeSize(Node* root) {
		if (!this->ChildA && !this->ChildB)
			return 0;
		int left = GetTreeSize(this->ChildA);
		int right = GetTreeSize(this->ChildB);

		return left + right + 1;
	}

	void SplitNode(Node* parent, int depth) {
		const int MaxDepth = 10;

		if (depth == MaxDepth || !parent->bounds.bHasObject || parent->spheres.size() <= 3) {
			return;
		}

		vec3 size = parent->bounds.Size();
		int SpiltAxis = size.x > max(size.y, size.z) ? 0 : size.y > size.z ? 1 : 2;
		float SplitPos = parent->bounds.Center[SpiltAxis];
		std::cout << std::endl;
		//std::cout << "Depth-> " << depth << std::endl;
		//std::cout << "Object Count: " << parent->spheres.size() << std::endl;
		//std::cout << "SIZE-> x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
		//size = parent->bounds.Center;
		//std::cout << "Center-> x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
		//std::cout << "SpiltAxis: " << SpiltAxis << std::endl;
		//std::cout << "SplitPos: " << SplitPos << std::endl;
		//std::cout << std::endl;

		parent->ChildA = new Node();
		parent->ChildB = new Node();
		std::cout << std::endl;
		std::cout << "SpiltAxis: " << SpiltAxis << std::endl;
		std::cout << "SplitPos: " << SplitPos << std::endl;
		for (auto sphere : parent->spheres) {
			vec3 size;
			size = sphere->center;
			//std::cout << "Sphere Center: ";
			//std::cout << "x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
			//std::cout << "Sphre Rad: " << sphere->radius << std::endl;
			//std::cout << "SpiltAxis: " << SpiltAxis << std::endl;
			//std::cout << "SplitPos: " << SplitPos << std::endl;
            if (bInA(sphere, SpiltAxis, SplitPos)){
                parent->ChildA->AddSphere(sphere);
			}
			else if (bInB(sphere, SpiltAxis, SplitPos)) {
				parent->ChildB->AddSphere(sphere);
            } else if (bInAB(sphere, SpiltAxis, SplitPos)){
                parent->ChildA->AddSphere(sphere);
                parent->ChildB->AddSphere(sphere);
            } else {
				assert(false && "ERROR :: SPLITFUNCTION :: NO SUBDIVISION");
            }
			vec3 min = parent->ChildA->bounds.Min;
			vec3 max = parent->ChildA->bounds.Max;
			vec3 diff = max - min;
			float length = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

			//size = Child->bounds.Size();
			//std::cout << "x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
		}
		//std::cout << std::endl;
		//std::cout << "Parent ChildA Spheres: " << parent->ChildA->spheres.size() << std::endl;
		//std::cout << "Parent ChildB Spheres: " << parent->ChildB->spheres.size() << std::endl;
		//std::cout << std::endl;
		SplitNode(parent->ChildA, depth + 1);
		SplitNode(parent->ChildB, depth + 1);
	}

	void PrintBallVec() {
		int i = 1;
		vec3 size;
		for (auto s : this->spheres) {
			size = s->center;
			std::cout << "Sphere " << i << "->";
			std::cout << " x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
			i++;
		}
	}

    bool bInAB(Sphere* sp, int axis, float Spos){
		//std::cout << "bInAB: " << (sp->center[axis] - sp->radius) << " < " << Spos << " && " << (sp->center[axis] + sp->radius) << " > " << Spos << " => Condition: " << ((sp->center[axis] - sp->radius < Spos && sp->center[axis] + sp->radius > Spos) ? "true" : "false") << std::endl;
        if (sp->center[axis] - sp->radius < Spos && sp->center[axis] + sp->radius > Spos)
            return true;
        return false;
    }

    bool bInB(Sphere* sp, int axis, float Spos){
		//std::cout << "bInB: " << (sp->center[axis] - sp->radius) << " > " << Spos << " && " << (sp->center[axis] + sp->radius) << " > " << Spos << " => Condition: " << ((sp->center[axis] - sp->radius > Spos && sp->center[axis] + sp->radius > Spos) ? "true" : "false") << std::endl;
        if (sp->center[axis] - sp->radius > Spos && sp->center[axis] + sp->radius > Spos)
            return true;
        return false;
        
    }

    bool bInA(Sphere* sp, int axis, float Spos){
		//std::cout << "bInA: " << (sp->center[axis] - sp->radius) << " < " << Spos << " && " << (sp->center[axis] + sp->radius) << " < " << Spos << " => Condition: " << ((sp->center[axis] - sp->radius < Spos && sp->center[axis] + sp->radius < Spos) ? "true" : "false") << std::endl;
        if (sp->center[axis] - sp->radius < Spos && sp->center[axis] + sp->radius < Spos)
            return true;
        return false;
        
    }

	//std::vector<Sphere*>& BVHIntersect(Node* parent, const Ray& ray) {
	Node* BVHIntersect(Node* parent, const Ray& ray) {
		if (parent->ChildA == nullptr && parent->ChildB == nullptr) {
			return parent;
		}

		std::vector<Sphere*> hitA, hitB, nullSph;
		Node* ParA = new Node();
		Node* ParB = new Node();
		Node* ParT = new Node();

		// Check intersection with ChildA's bounding box and recurse if hit
		if (parent->ChildA && parent->ChildA->bounds.BoxIntersection(ray)) {
			ParA = BVHIntersect(parent->ChildA, ray);
		}

		// Check intersection with ChildB's bounding box and recurse if hit
		if (parent->ChildB && parent->ChildB->bounds.BoxIntersection(ray)) {
			ParB = BVHIntersect(parent->ChildB, ray);
		}

		return ParT;
	}

 //   std::vector<Sphere*>& BVHIntersect(Node *parent, const Ray& ray) {
	//	HitResult hit;

	//	if (parent->ChildA == nullptr && parent->ChildB == nullptr) 
	//	{
	//		return parent->spheres;
	//	}
	//	else 
	//		recursice
 //       std::vector<Sphere*> hitA, hitB;


	//	if (parent->ChildA)
	//		if (parent->ChildA->bounds.BoxIntersection(ray))
	//			BVHIntersect(parent->ChildA, ray, hitA);
	//		else
	//			return;


	//	if (parent->ChildB)
	//		if (parent->ChildB->bounds.BoxIntersection(ray))
	//			BVHIntersect(parent->ChildB, ray, hitB);
	//		else
	//			return;


	//	return;
	//}

	void LevelOrderTraversal() {
		if (!this)
			return;
		std::queue<Node*> Queue;
		Queue.push(this);
		while (!Queue.empty()) {
			Node* CurrNode = Queue.front();
			vec3 size = CurrNode->bounds.Size();
			std::cout << std::endl;
			std::cout << "Spheres count: " << CurrNode->spheres.size() << std::endl;
			std::cout << "Size->  x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
			if (CurrNode->ChildA)
				Queue.push(CurrNode->ChildA);
			if (CurrNode->ChildB)
				Queue.push(CurrNode->ChildB);
			Queue.pop();
		}
	}
};












