#pragma once
#include "vec3.h"

//------------------------------------------------------------------------------
/**
*/
class Ray
{
public:
    // beginning of ray
    vec3 Origin;
    // magnitude and direction of ray
    vec3 RayDir;
    vec3 InvRayDir;
    int sign[3];

    Ray(vec3 startpoint, vec3 dir) :
        Origin(startpoint),
        RayDir(dir)
    {
		// FOR OPTIMIZTION
        InvRayDir = vec3(1 / dir.x, 1 / dir.y, 1 / dir.z);
        // CHECK FOR WHICH DIRECTION THE RAY IS SHOOTING AT
        sign[0] = (InvRayDir.x < 0);
        sign[1] = (InvRayDir.y < 0);
        sign[2] = (InvRayDir.z < 0);
    }

    ~Ray() {}

    vec3 PointAt(float t) {
        return { Origin + RayDir * t };
    }

};