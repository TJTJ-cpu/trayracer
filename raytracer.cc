#include "raytracer.h"
#include "threadPool.h"
#include <random>
#include <thread>
#include <future>
#include <atomic>
#include <iostream>

//------------------------------------------------------------------------------
/**
*/
Raytracer::Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, unsigned rpp, unsigned bounces) :
    frameBuffer(frameBuffer),
    rpp(rpp),
    bounces(bounces),
    width(w),
    height(h)
{
    Pool.SpawnThread();
}

Raytracer::~Raytracer() {
    Pool.Stop();
}
//------------------------------------------------------------------------------
/**
*/

void Raytracer::SetUpNode(BoundingBox Box, std::vector<Sphere> Spheres) {
    MainNode = new Node(Box, Spheres);
}

unsigned int 
Raytracer::AssignJob()
{
    // Max Pixels
    size_t MaxPixel = width * height;
    int Cores = std::thread::hardware_concurrency();
  //  if (Cores > 6)
		//Cores = 1;
    MaxPixel = this->height * this->width;
    PixelCounter = 0;
	std::vector<std::thread> Threads;

    // Spawn  threads
    for (int i = 0; i < Cores; i++) {
        Threads.emplace_back([&]() {
		//Pool.QueueJob([&]() { 
            while (true) {
                size_t index = PixelCounter.fetch_add(1);

                // Base Case
                if (index >= MaxPixel)
                    break;

                auto [x, y] = indexToXY(index);
				float u = ((float(x) + RandomFloat()) * (1.0f / width)) * 2.0f - 1.0f;
				float v = ((float(y) + RandomFloat()) * (1.0f / height)) * 2.0f - 1.0f;
                
                Color color;

                for (int z = 0; z < rpp; z++) {
                    color += GetColor(u, v, x, y);
                    RayNum++;
                }

                AssignColor(color, x, y);
            }
		});
    }

    //while (Pool.Busy())
    //{
    //}

    for (auto& thread : Threads) {
        thread.join();
    }

    return RayNum;
}

Color Raytracer::GetColor(float u, float v, int x, int y) {

	vec3 direction = vec3(u, v, -1.0f);
	direction = transform(direction, this->frustum);
    Color color;

	Ray ray = Ray(get_position(this->view), direction);
	color = this->TracePath(ray, 0);
    return color;
}

void
Raytracer::AssignColor(Color color, int x, int y) {
	color.r /= this->rpp;
	color.g /= this->rpp;
	color.b /= this->rpp;

	this->frameBuffer[y * this->width + x] += color;

}

std::pair<int, int> Raytracer::indexToXY(size_t index) const {
    int x = index % width;
    int y = index / width;
    return std::make_pair(x, y);
}

//------------------------------------------------------------------------------
/**
*/

unsigned int 
Raytracer::Raytrace()
{
    static int leet = 1337;
    std::mt19937 generator (leet++);
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    unsigned int RayNum = 0;

    for (int x = 0; x < this->width; ++x)
    {
        for (int y = 0; y < this->height; ++y)
        {
            Color color;
            for (int i = 0; i < this->rpp; ++i)
            {
                float u = ((float(x + dis(generator)) * (1.0f / this->width)) * 2.0f) - 1.0f;
                float v = ((float(y + dis(generator)) * (1.0f / this->height)) * 2.0f) - 1.0f;

                vec3 direction = vec3(u, v, -1.0f);
                direction = transform(direction, this->frustum);
                
                Ray* ray = new Ray(get_position(this->view), direction);
                color += this->TracePath(*ray, 0);
                delete ray;
                RayNum++;
            }

            // divide by number of samples per pixel, to get the average of the distribution
            color.r /= this->rpp;
            color.g /= this->rpp;
            color.b /= this->rpp;

            this->frameBuffer[y * this->width + x] += color;
        }
    }
    return RayNum;
}

//------------------------------------------------------------------------------
/**
 * @parameter n - the current bounce level
*/
Color
Raytracer::TracePath(Ray ray, unsigned n)
{
    vec3 hitPoint;
    vec3 hitNormal;
    Object* hitObject = nullptr;
    float distance = FLT_MAX;

    if (BVHRaycast(ray, hitPoint, hitNormal, hitObject, distance, this->objects))
    {
        Ray scatteredRay =  Ray(hitObject->ScatterRay(ray, hitPoint, hitNormal));
        if (n < this->bounces)
        {
            return hitObject->GetColor() * this->TracePath(scatteredRay, n + 1);
        }

        if (n == this->bounces)
        {
            return {0,0,0};
        }
    }
    return this->Skybox(ray.RayDir);
}

//------------------------------------------------------------------------------
/**
*/

bool
Raytracer::BVHRaycast(Ray ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, 
    float& distance, std::vector<Sphere*> const &world)
{

    bool isHit = false;
    HitResult closestHit;
    int numHits = 0;
    //HitResult hit;

    // if (MainNode->bounds.BoxIntersection(ray)){
    // }
    HitResult hit;
    std::vector<HitResult> HitVec;
    std::vector<Sphere*> Spheres;
    MainNode->BVHIntersect(this->MainNode, ray, Spheres);

    for (Sphere* object : Spheres){
        hit = object->Intersect(ray);
        if (hit.HasValue())
        {
            closestHit = hit;
            closestHit.object = object;
            isHit = true;
            numHits++;
        }
    }

    hitPoint = closestHit.p;
    hitNormal = closestHit.normal;
    hitObject = closestHit.object;
    distance = closestHit.t;
    
    return isHit;
}

bool
Raytracer::Raycast(Ray ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, 
    float& distance, std::vector<Sphere*> const &world)
{
    bool isHit = false;
    HitResult closestHit;
    int numHits = 0;
    HitResult hit;

    for (Sphere* object : world)
    {
        hit = object->Intersect(ray);
        if (hit.HasValue())
        {
            closestHit = hit;
            closestHit.object = object;
            isHit = true;
            numHits++;
        }
    }

    hitPoint = closestHit.p;
    hitNormal = closestHit.normal;
    hitObject = closestHit.object;
    distance = closestHit.t;
    
    return isHit;
}


//------------------------------------------------------------------------------
/**
*/
void
Raytracer::Clear()
{
    for (auto& color : this->frameBuffer)
    {
        color.r = 0.0f;
        color.g = 0.0f;
        color.b = 0.0f;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
Raytracer::UpdateMatrices()
{
    mat4 inverseView = inverse(this->view); 
    mat4 basis = transpose(inverseView);
    this->frustum = basis;
}

//------------------------------------------------------------------------------
/**
*/
Color
Raytracer::Skybox(vec3 direction)
{
    float t = 0.5*(direction.y + 1.0);
    vec3 vec = vec3(1.0, 1.0, 1.0) * (1.0 - t) + vec3(0.5, 0.7, 1.0) * t;
    return {(float)vec.x, (float)vec.y, (float)vec.z};
}
