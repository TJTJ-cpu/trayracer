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
		int Iter = 1000;
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

	int EvaluateSplit(Node* RootAxis, int Axis, float Pos) {
		int MaxSpheres = RootAxis->spheres.size();
		int IdealSphere = MaxSpheres / 2;
		std::vector<Sphere*> Spheres;


		for (auto sphere : RootAxis->spheres) {
			//if (sphere->center[Axis] <= Pos)
			//	Spheres.push_back(sphere);
			//else if (sphere->center[Axis] > Pos)
			//	Spheres.push_back(sphere);

			if (bInA(sphere, Axis, Pos)) {
				Spheres.push_back(sphere);
			}
			else if (bInAB(sphere, Axis, Pos)) {
				Spheres.push_back(sphere);
			}
		}

		int Result = IdealSphere - Spheres.size();
		return abs(Result);
	}

	void BVHTEST(Node* RootAxis, int Axis, float Pos) {
		int MaxSpheres = RootAxis->spheres.size();
		std::vector<Sphere*> Spheres;

		for (auto sphere : RootAxis->spheres) {
			if (bInA(sphere, Axis, Pos)) {
				Spheres.push_back(sphere);
			}
			else if (bInAB(sphere, Axis, Pos)) {
				Spheres.push_back(sphere);
			}
		}
		std::cout << "Total Sphere: " << MaxSpheres << std::endl;
		std::cout << "A Sphere: " << Spheres.size() << std::endl;
		std::cout << std::endl;
	}

	void SplitNode(Node* parent, int depth) {
		const int MaxDepth = 32;

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
			//vec3 size;
			//size = sphere->center;
			//std::cout << "Sphere Center: ";
			//std::cout << "x: " << size.x << ", y: " << size.y << ", z: " << size.z << std::endl;
			//std::cout << "Sphre Rad: " << sphere->radius << std::endl;
			//std::cout << "SpiltAxis: " << SpiltAxis << std::endl;
			//std::cout << "SplitPos: " << SplitPos << std::endl;
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

	void NormalSplit(Node* Root, int &SplitAxis, float &SplitPos) {
		vec3 size = Root->bounds.Size();
		SplitAxis = size.x > max(size.y, size.z) ? 0 : size.y > size.z ? 1 : 2;
		SplitPos = Root->bounds.Center[SplitAxis];
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
        if (sp->center[axis] - sp->radius <= Spos && sp->center[axis] + sp->radius >= Spos)
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
        if (sp->center[axis] - sp->radius <= Spos && sp->center[axis] + sp->radius <= Spos)
            return true;
        return false;
        
    }

	//std::vector<Sphere*>& BVHIntersect(Node* parent, const Ray& ray) {
	void BVHIntersect(Node* parent, const Ray& ray, std::vector<Sphere*> &s) {
		if (parent->ChildA == nullptr && parent->ChildB == nullptr) {
			for (auto sp : parent->spheres)
				s.push_back(sp);
			//s.insert(s.end(), parent->spheres.begin(), parent->spheres.end());
			return;
		}

		// Check intersection with ChildA's bounding box and recurse if hit
		if (parent->ChildA && parent->ChildA->bounds.BoxIntersection(ray)) {
			BVHIntersect(parent->ChildA, ray, s);
		}

		// Check intersection with ChildB's bounding box and recurse if hit
		if (parent->ChildB && parent->ChildB->bounds.BoxIntersection(ray)) {
			BVHIntersect(parent->ChildB, ray, s);
		}

	}

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

	//bool RaySphereTest(Ray& ray, HitResult& closestHit, std::vector<Sphere*> &spheres) {
	bool RaySphereTest(Ray& ray, HitResult& closestHit) {
		HitResult hit;
		bool isHit = false;
		std::stack<Node*> NodeStack;
		NodeStack.push(this);
		Node* curr = this;

		// boundingbox
		// is child
		// iter

		while (!NodeStack.empty()) {
			Node* Curr = NodeStack.top();
			NodeStack.pop();
			//std::cout << "Stack Size: " << NodeStack.size() << std::endl;
			if (Curr->ChildA == nullptr && Curr->ChildB == nullptr) {
				for (Sphere* object : Curr->spheres) {
					hit = object->Intersect(ray, hit.t);
					if (hit.HasValue())
					{
						if (hit.t < closestHit.t) {
						closestHit = hit;
						closestHit.object = object;
						isHit = true;
						}
					}
				}
			}
			else {
				if (Curr->ChildA && Curr->ChildA->bounds.BoxIntersection(ray))
					NodeStack.push(Curr->ChildA);
				if (Curr->ChildB && Curr->ChildB->bounds.BoxIntersection(ray))
					NodeStack.push(Curr->ChildB);
			}
		}
		return isHit;
	}
};












