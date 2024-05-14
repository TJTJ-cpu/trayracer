#pragma once
#include "vec3.h"

//------------------------------------------------------------------------------
/**
*/
class Ray
{
public:
    Ray () {}

    Ray(vec3 startpoint, vec3 dir) :
        orig(startpoint),
        dir(dir)
    {

    }

    ~Ray()
    {

    }

    vec3 PointAt(float t)
    {
        return {orig + dir * t};
    }

    // beginning of ray
    vec3 orig;
    // magnitude and direction of ray
    vec3 dir;

    vec3 Direction() const { return dir; }
    vec3 Origin() const { return orig; }
};