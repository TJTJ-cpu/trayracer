#pragma once
#include <cmath>
#include <initializer_list>
#include <assert.h>

#define MPI 3.14159265358979323846

class vec3
{
public:
    double x, y, z;

    vec3(float a) : vec3(a, a, a) {};
    vec3() : x(0), y(0), z(0) {}
    vec3(double x, double y, double z) : x(x), y(y), z(z) {}


    vec3(std::initializer_list<double> const il)
    {
        assert(il.size() == 3);

        int i = 0;
        for (auto v : il)
        {
            double* d = reinterpret_cast<double*>(this);
            d += i;
            *d = v;
            i++;
        }
    }


    ~vec3()
    {
    }

    vec3(vec3 const& rhs)
    {
        this->x = rhs.x;
        this->y = rhs.y;
        this->z = rhs.z;

    }

    vec3 operator+(vec3 const& rhs) { return {x + rhs.x, y + rhs.y, z + rhs.z};}
    vec3 operator-(vec3 const& rhs) { return {x - rhs.x, y - rhs.y, z - rhs.z};}
    vec3 operator-() { return {-x, -y, -z};}
    vec3 operator*(float const c) { return {x * c, y * c, z * c};}
    vec3 operator/(double const c) const { return vec3(x / c, y / c, z / c); }

    double& operator[](const int i)
    {
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
        else if (i == 2)
            return z;
        else
            assert(false && "Out of bound");
    }
    double const& operator[](const int i) const
    {
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
        else if (i == 2)
            return z;
        else
            assert(false && "Out of bound");
    }

};

// Get length of 3D vector
inline double len(vec3 const& v)
{

    return float(sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
}

// Get normalized version of v
inline vec3 normalize(vec3 v)
{
    double l = len(v);
    if (l == 0)
        return vec3(0,0,0);

    vec3 ret = vec3(v.x / l, v.y / l, v.z / l);
    return vec3(ret);

    return vec3(v.x / l, v.y / l, v.z / l);
}

inline vec3 min(const vec3& a, const vec3& b) {
    return vec3(
        a.x < b.x ? a.x : b.x,
        a.y < b.y ? a.y : b.y,
        a.z < b.z ? a.z : b.z
    );
}

inline vec3 max(const vec3& a, const vec3& b) {
    return vec3(
        a.x > b.x ? a.x : b.x,
        a.y > b.y ? a.y : b.y,
        a.z > b.z ? a.z : b.z
    );
}

inline int max(const int& a, const int& b) {
    return a >= b ? a : b;
}

// piecewise multiplication between two vectors
inline vec3 mul(vec3 a, vec3 b)
{
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

// piecewise add between two vectors
inline vec3 add(vec3 a, vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline float dot(vec3 a, vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 reflect(vec3 v, vec3 n)
{
    return v - n * (2 * dot(v,n));
}

inline vec3 cross(vec3 a, vec3 b)
{
    return { a.y * b.z - a.z * b.y,
             a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x, };
}

class vec2 {
public:
    explicit vec2(float a) : vec2(a, a) {};
    float x, y;

    vec2(float a, float b) : x(a), y(b){}
    vec2(){}

};

inline float min(const float& a, const float& b) {
    return (a <= b) ? a : b;
}
