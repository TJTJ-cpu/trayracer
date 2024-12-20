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
#include <stack>


class Node;
class BoundingBox;

template<typename T>
bool IsInVec(std::vector<T> vec, T target) {
	return (std::find(vec.begin(), vec.end(), target) != vec.end());
}


class BoundingBox 
{
public:
	vec3 Min = vec3(0,0,0);
	vec3 Max = vec3(0,0,0);
	vec3 Center = (Min + Max) / 2;

	// Make the bouding box include the sphere that we pass in
	void GrowToInclude(Sphere* sphere) {
		Min.x = std::min(Min.x, sphere->center.x - sphere->radius);
		Min.y = std::min(Min.y, sphere->center.y - sphere->radius);
		Min.z = std::min(Min.z, sphere->center.z - sphere->radius);

		Max.x = std::max(Max.x, sphere->center.x + sphere->radius);
		Max.y = std::max(Max.y, sphere->center.y + sphere->radius);
		Max.z = std::max(Max.z, sphere->center.z + sphere->radius);
		this->Center = (Min + Max) / 2;
		//this->bHasObject = true;
	}

	bool BoxIntersection(Ray ray, float maxDist) {
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

		if (tMin > maxDist)
			return false;

        return true;
	}

	float DistanceTo(const vec3& point) const {
		float dx = max(max(this->Min.x - point.x, 0.0f), point.x - this->Max.x);
		float dy = max(max(this->Min.y - point.y, 0.0f), point.y - this->Max.y);
		float dz = max(max(this->Min.z - point.z, 0.0f), point.z - this->Max.z);
		return sqrt(dx * dx + dy * dy + dz * dz);
	}

	vec3 Size() {
		return this->Max - this->Min;
	}

	float SurfaceArea() {
		float width = Max.x - Min.x;
		float height = Max.y - Min.y;
		float depth = Max.z - Min.z;
		return 2 * (width * height + width * depth + height * depth);
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

	bool IsLeaf() {
		if (!this->ChildA && !this->ChildB)
			return true;
		return false;
	}

	int GetTreeSize(Node* root) {
		if (!this->ChildA && !this->ChildB)
			return 0;
		int left = GetTreeSize(this->ChildA);
		int right = GetTreeSize(this->ChildB);

		return left + right + 1;
	}

	void FindBestSplit(Node* Root, int &SplitAxis, float &SplitPos) {
		int Iter = 10000;
		int CurrentBestResult = 2147483647;
		float Candidate, Length;
		float BestCadidate = FLT_MAX;
		int Result;
		for (int j = 0; j < 3; j++) {
			float Scale = (this->bounds.Max[j] - this->bounds.Min[j]) / Iter;
			Length = this->bounds.Max[j] - this->bounds.Min[j];
			for (int i = 0; i < Iter; i++) {
				Candidate = (Length / Iter) * i;
				Result = EvaluateSplit(Root, j, Candidate);
				if (Result < CurrentBestResult) {
					SplitAxis = j;
					SplitPos = Candidate;
					CurrentBestResult = Result;
				}
			}
		}
	}	
	
	void NormalSplit(Node* Root, int &SplitAxis, float &SplitPos) {
		vec3 size = Root->bounds.Size();
		SplitAxis = size.x > max(size.y, size.z) ? 0 : size.y > size.z ? 1 : 2;
		SplitPos = Root->bounds.Center[SplitAxis];
	}

	float EvaluateSplit(Node* RootAxis, int Axis, float Pos) {
		Node ASpheres;
		Node BSpheres;
		unsigned int OverlappedCount = 0;

		for (auto sphere : RootAxis->spheres) {
			if (bInA(sphere, Axis, Pos)) {
				ASpheres.AddSphere(sphere);
			}
			else if (bInB(sphere, Axis, Pos)) {
				BSpheres.AddSphere(sphere);
			}
			else if (bInAB(sphere, Axis, Pos)) {
				ASpheres.AddSphere(sphere);
				BSpheres.AddSphere(sphere);
				OverlappedCount++;
			}
			else {
				assert(false && "ERROR :: SPLITFUNCTION :: NO SUBDIVISION");
			}
		}

		// SURFACE AREA HEURISTIC
		float surfaceAreaA = ASpheres.bounds.SurfaceArea();
		float surfaceAreaB = BSpheres.bounds.SurfaceArea();
		float surfaceAreaParent = RootAxis->bounds.SurfaceArea();

		float costA = (surfaceAreaA / surfaceAreaParent) * (ASpheres.spheres.size());
		float costB = (surfaceAreaB / surfaceAreaParent) * (BSpheres.spheres.size());

		// DISCOURAGE OVERLAPPING * CONSTANT
		float overlapPenalty = OverlappedCount * 1.5;

		float totalCost = costA + costB + overlapPenalty;
		return totalCost;
	}

	void SplitNode(Node* parent, int depth) {
		const int MaxDepth = 4;

		if (depth == MaxDepth || parent->spheres.size() <= 4) {
			return;
		}

		int SpiltAxis;
		float SplitPos;

		FindBestSplit(parent, SpiltAxis, SplitPos);
		parent->ChildA = new Node();
		parent->ChildB = new Node();
		for (auto sphere : parent->spheres) {
			vec3 size;
			size = sphere->center;
            if (bInA(sphere, SpiltAxis, SplitPos)) {
				parent->ChildA->AddSphere(sphere);
			}
			else if (bInB(sphere, SpiltAxis, SplitPos)) {
				parent->ChildB->AddSphere(sphere);
            }
			else if (bInAB(sphere, SpiltAxis, SplitPos)) {
                parent->ChildA->AddSphere(sphere);
                parent->ChildB->AddSphere(sphere);
            } else {
				assert(false && "ERROR :: SPLITFUNCTION :: NO SUBDIVISION");
            }
		}
		SplitNode(parent->ChildA, depth + 1);
		SplitNode(parent->ChildB, depth + 1);
	}

	bool bInA(Sphere* sp, int axis, float Spos) {
		return sp->center[axis] + sp->radius <= Spos;
	}

	bool bInB(Sphere* sp, int axis, float Spos) {
		return sp->center[axis] - sp->radius >= Spos;
	}

	bool bInAB(Sphere* sp, int axis, float Spos) {
		return (sp->center[axis] - sp->radius < Spos) && (sp->center[axis] + sp->radius > Spos);
	}
};

