#pragma once

#include <algorithm>
#include <vector>
#include <memory>
#include "sphere.h"
#include "aabb.h"
#include "mat4.h"


class QuadTree
{
public:
    QuadTree() {}
    QuadTree(AABB Bound) : Boundary(Bound) {}

    AABB Boundary;
    std::vector<vec2>Points;
    const int MAX_CAPACITY = 4;
    bool bIsDivide = false;

    //Children
    QuadTree* northWest;
    QuadTree* northEast;
    QuadTree* southWest;
    QuadTree* southEast;

    // Function
    // queryRange(AABB Range)
    bool Insert(vec2 p); 
    void Subdivide();

};

bool QuadTree::Insert(vec2 p)
{
    if (!this->Boundary.Contain(p))
        return false;
    if (this->Points.size() < MAX_CAPACITY) {
        this->Points.push_back(p);
        return true;
    }
    else {
        if (!bIsDivide) {
			Subdivide();
            bIsDivide = true;
        }
        if (this->northEast->Insert(p))
            return true;
        else if (this->northWest->Insert(p))
			return true;
        else if (this->southEast->Insert(p))
			return true;
        else if (this->southWest->Insert(p))
			return true;
    }
}

void QuadTree::Subdivide()
{
    AABB nw(this->Boundary.x - this->Boundary.w/2, this->Boundary.y - this->Boundary.h/2, this->Boundary.w/2, this->Boundary.h/2);
    AABB ne(this->Boundary.x + this->Boundary.w/2, this->Boundary.y - this->Boundary.h/2, this->Boundary.w/2, this->Boundary.h/2);
    AABB sw(this->Boundary.x - this->Boundary.w/2, this->Boundary.y + this->Boundary.h/2, this->Boundary.w/2, this->Boundary.h/2);
    AABB se(this->Boundary.x + this->Boundary.w/2, this->Boundary.y + this->Boundary.h/2, this->Boundary.w/2, this->Boundary.h/2);

    this->northWest = new QuadTree(nw);
    this->northEast = new QuadTree(ne);
    this->southWest = new QuadTree(sw);
    this->southEast = new QuadTree(se);
}
