#pragma once
#include "ray.h"
#include "color.h"
#include <float.h>
#include <string>
#include <memory>

class Object;

//------------------------------------------------------------------------------
/**
*/
struct HitResult
{
    // hit point
    vec3 p;
    // normal
    vec3 normal;
    // hit object, or nullptr
    Object* object = nullptr;
    // intersection distance
    float t = FLT_MAX;

    bool HasValue() {
        if (object != nullptr)
            return true;
        else
            return false;
    }
};

//template<class TYPE>
//class Optional
//{
//public:
//    Optional() {}
//    Optional(HitResult hit) : hasValue(true)
//    {
//        this->value = std::make_shared<HitResult>(hit);
//    }
//
//    bool HasValue()
//    {
//        // check if this object has a value.
//        if (this->hasValue)
//        {
//            // doublecheck the value
//            if (value == nullptr)
//            {
//                return false;
//            }
//            if (value != nullptr)
//            {
//                // doublecheck the value content.
//                if (value->object == nullptr)
//                {
//                    return false;
//                }
//                if (value->normal.IsZero())
//                {
//                    return false;
//                }
//            }
//        }
//        else
//        {
//            return false;
//        }
//        return true;
//    }
//    HitResult Get()
//    {
//        assert(this->HasValue());
//        return *value;
//    }
//
//private:
//    bool hasValue = false;
//    std::shared_ptr<HitResult> value = nullptr;
//};

//------------------------------------------------------------------------------
/**
*/
class Object
{
public:
    Object() {}

    virtual ~Object() {}

    virtual HitResult Intersect(Ray ray, float maxDist) { return {}; };
    virtual Color GetColor() = 0;
    virtual Ray ScatterRay(Ray ray, vec3 point, vec3 normal) { return Ray({ 0,0,0 }, {1,1,1}); };
};