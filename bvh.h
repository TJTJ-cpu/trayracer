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

	int GetLargetstAxis() {
		vec3 size = Size();
		return size.x > max(size.y, size.z) ? 0 : size.y > size.z ? 1 : 2;
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

	void FindNoOverlapSplit(Node* Root, int& SplitAxis, float& SplitPos) {
		int Iter = 50000;
		int CurrentBestResult = 2147483647;
		float Candidate, Length;
		float BestCadidate = FLT_MAX;
		int Result;
		for (int j = 0; j < 3; j++) {
			float Scale = (this->bounds.Max[j] - this->bounds.Min[j]) / Iter;
			Length = this->bounds.Max[j] - this->bounds.Min[j];
			for (int i = 0; i < Iter; i++) {
				Candidate = (Length / Iter) * i;
				Result = EvaluateNoOverlapSplit(Root, j, Candidate);
				if (Result < CurrentBestResult) {
					SplitAxis = j;
					SplitPos = Candidate;
					CurrentBestResult = Result;
				}
			}
		}

	}

	int EvaluateNoOverlapSplit(Node* RootAxis, int Axis, float Pos) {
		int MaxSpheres = RootAxis->spheres.size();
		int IdealSphere = MaxSpheres / 2;
		std::vector<Sphere*> Spheres;

		for (auto sphere : RootAxis->spheres) {
			if (bInAB(sphere, Axis, Pos))
				return 2147483647;

			if (bInA(sphere, Axis, Pos)) 
				Spheres.push_back(sphere);
		}

		int Result = IdealSphere - Spheres.size();
		return abs(Result);
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
		//BVHTEST(Root, SplitAxis, SplitPos);
	}	
	
	void NormalSplit(Node* Root, int &SplitAxis, float &SplitPos) {
		vec3 size = Root->bounds.Size();
		SplitAxis = size.x > max(size.y, size.z) ? 0 : size.y > size.z ? 1 : 2;
		SplitPos = Root->bounds.Center[SplitAxis];
	}

	float EvaluateSplit(Node* RootAxis, int Axis, float Pos) {
		//int MaxSpheres = RootAxis->spheres.size();
		//int IdealSphere = MaxSpheres / 2;
		//std::vector<Sphere*> Spheres;
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

	/// TO DO
	/*	
	func that get volume from vec<sphere>

	*/


	void SplitNode(Node* parent, int depth) {
		const int MaxDepth = 4;
		//std::cout << "Depth: " << MaxDepth << std::endl;

		if (depth == MaxDepth || !parent->bounds.bHasObject || parent->spheres.size() <= 5) {
			return;
		}

		int SpiltAxis;
		float SplitPos;

		FindBestSplit(parent, SpiltAxis, SplitPos);
		//NormalSplit(parent, SpiltAxis, SplitPos);
		//FindNoOverlapSplit(parent, SpiltAxis, SplitPos);
		parent->ChildA = new Node();
		parent->ChildB = new Node();
		//std::cout << std::endl;
		//std::cout << "SpiltAxis: " << SpiltAxis << std::endl;
		//std::cout << "SplitPos: " << SplitPos << std::endl;
		for (auto sphere : parent->spheres) {
			vec3 size;
			size = sphere->center;
			//std::cout << "Sphe/*re Center: ";
			//std::cout << "x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
			//std::cout << "Sphre Rad: " << sphere->radius << std::endl;
			//std::cout << "SpiltAxis: " << SpiltAxis << std::endl;
			//std::cout << "SplitPos*/: " << SplitPos << std::endl;
			// Center Split
			//if (sphere->center[SpiltAxis] <= SplitPos)
			//	parent->ChildA->AddSphere(sphere);
			//else if (sphere->center[SpiltAxis] > SplitPos)
			//	parent->ChildB->AddSphere(sphere);
			//std::cout << std::endl;
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
			//std::cout << "Child A Size: " << parent->ChildA->spheres.size() << std::endl;
			//std::cout << "Child B Size: " << parent->ChildB->spheres.size() << std::endl;
			//std::cout << std::endl;
			//vec3 min = parent->ChildA->bounds.Min;
			//vec3 max = parent->ChildA->bounds.Max;
			//vec3 diff = max - min;
			//float length = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
			//size = Child->bounds.Size();
			//std::cout << "x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
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


	// old
  //  bool bInAB(Sphere* sp, int axis, float Spos){
		////std::cout << "bInAB: " << (sp->center[axis] - sp->radius) << " < " << Spos << " && " << (sp->center[axis] + sp->radius) << " > " << Spos << " => Condition: " << ((sp->center[axis] - sp->radius < Spos && sp->center[axis] + sp->radius > Spos) ? "true" : "false") << std::endl;
  //      if (sp->center[axis] - sp->radius < Spos && sp->center[axis] + sp->radius > Spos)
  //          return true;
  //      return false;
  //  }

  //  bool bInB(Sphere* sp, int axis, float Spos){
		////std::cout << "bInB: " << (sp->center[axis] - sp->radius) << " > " << Spos << " && " << (sp->center[axis] + sp->radius) << " > " << Spos << " => Condition: " << ((sp->center[axis] - sp->radius > Spos && sp->center[axis] + sp->radius > Spos) ? "true" : "false") << std::endl;
  //      if (sp->center[axis] - sp->radius > Spos && sp->center[axis] + sp->radius > Spos)
  //          return true;
  //      return false;
  //      
  //  }

  //  bool bInA(Sphere* sp, int axis, float Spos){
		////std::cout << "bInA: " << (sp->center[axis] - sp->radius) << " < " << Spos << " && " << (sp->center[axis] + sp->radius) << " < " << Spos << " => Condition: " << ((sp->center[axis] - sp->radius < Spos && sp->center[axis] + sp->radius < Spos) ? "true" : "false") << std::endl;
  //      if (sp->center[axis] - sp->radius < Spos && sp->center[axis] + sp->radius < Spos)
  //          return true;
  //      return false;
  //      
  //  }

	//std::vector<Sphere*>& BVHIntersect(Node* parent, const Ray& ray) {
	//void BVHIntersect(Node* parent, const Ray& ray, std::vector<Sphere*> &s) {
	//	if (parent->ChildA == nullptr && parent->ChildB == nullptr) {
	//		for (auto sp : parent->spheres)
	//			s.push_back(sp);
	//		//s.insert(s.end(), parent->spheres.begin(), parent->spheres.end());
	//		return;
	//	}

	//	// Check intersection with ChildA's bounding box and recurse if hit
	//	if (parent->ChildA && parent->ChildA->bounds.BoxIntersection(ray)) {
	//		BVHIntersect(parent->ChildA, ray, s);
	//	}

	//	// Check intersection with ChildB's bounding box and recurse if hit
	//	if (parent->ChildB && parent->ChildB->bounds.BoxIntersection(ray)) {
	//		BVHIntersect(parent->ChildB, ray, s);
	//	}

	//}
};












