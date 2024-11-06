#pragma once
#include "vec3.h"

//------------------------------------------------------------------------------
/**
*/
class Ray
{
public:
    Ray(vec3 startpoint, vec3 dir) :
        RayStart(startpoint),
        RayDir(dir)
    {

    }

    ~Ray()
    {

    }

    vec3 PointAt(float t)
    {
        return { RayStart + RayDir * t };
    }

    // beginning of ray
    vec3 RayStart;
    // magnitude and direction of ray
    vec3 RayDir;
};