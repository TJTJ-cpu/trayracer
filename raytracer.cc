#include "raytracer.h"
#include <chrono>
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
    SpawnThread();
}

Raytracer::~Raytracer() {
    Stop();
}
//------------------------------------------------------------------------------
/**
*/

void Raytracer::SetUpNode(BoundingBox Box, std::vector<Sphere*> Spheres) {
    MainNode = new Node(Box, Spheres);
}

unsigned int 
Raytracer::AssignJob()
{
    // Max Pixels
    int Cores = std::thread::hardware_concurrency();
    CurrentThread.store(0);
	 // Cores = 1;

    // ThreadPool
    /// FIND THE OPTIMAL NUMBER LATER
    const int NumChunk = 10;
    int ChunkSize = (height + NumChunk - 1) / NumChunk;
    // ROUND UP CHUNK
    int totalChunk = (height + ChunkSize - 1) / ChunkSize;

    for (int i = 0; i < totalChunk; i++) {
		// min y
        float my = i * ChunkSize;
		// max y
        float mx = min((i + 1) * ChunkSize, height);
        //QueueJob(this, chunk);
        QueueChunk(vec2(my, mx));
        AvailableThreads.fetch_sub(1);
    }

    //while (AvailableThreads > 0)
    while (CurrentThread > NumChunk)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));


    std::cout << "Cur thread: " << CurrentThread;
    std::cout << ", NumChunk: " << NumChunk << std::endl;
    return RayNum;


     //Spawn  threads
    //PixelCounter = 0;
    //size_t MaxPixel = width * height;
    //MaxPixel = this->height * this->width;
  //  for (int i = 0; i < Cores; i++) {
  //      Threads.emplace_back([&]() {
  //          while (true) {
  //              size_t index = PixelCounter.fetch_add(1);

  //              // Base Case
  //              if (index >= MaxPixel)
  //                  break;

  //              auto [x, y] = indexToXY(index);
		//		float u = ((float(x) + RandomFloat()) * (1.0f / width)) * 2.0f - 1.0f;
		//		float v = ((float(y) + RandomFloat()) * (1.0f / height)) * 2.0f - 1.0f;
  //              
  //              Color color;

  //              for (int z = 0; z < rpp; z++) {
  //                  color += GetColor(u, v, x, y);
  //                  RayNum++;
  //              }

  //              AssignColor(color, x, y);
  //          }
		//});
  //  }

  //  for (auto& thread : Threads) {
  //      thread.join();
  //  }

    return RayNum;
}

Color 
Raytracer::GetColor2(int x, int y){
     Color color;
    for (int i = 0; i < rpp; ++i) {
        float u = ((float(x) + RandomFloat()) * (1.0f / width)) * 2.0f - 1.0f;
        float v = ((float(y) + RandomFloat()) * (1.0f / height)) * 2.0f - 1.0f;
        vec3 direction = vec3(u, v, -1.0f);
        direction = transform(direction, this->frustum);
        Ray ray(get_position(this->view), direction);
        color += this->TracePath(ray, 0); // Recursive or iterative tracing
    }
    return color;
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
Raytracer::AssignColor(Color &color, int x, int y) {
    //std::cout << "x: " << x << ", y: " << std::endl;
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
Raytracer::TracePath(Ray &ray, unsigned n)
{
    vec3 hitPoint;
    vec3 hitNormal;
    Object* hitObject = nullptr;
    Color color;
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
Raytracer::BVHRaycast(Ray &ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, 
    float& distance, std::vector<Sphere*> const &world)
{
    HitResult closestHit;
    std::stack<Node*> StackNode;
    StackNode.push(this->MainNode);
    int count = this->MainNode->spheres.size();
    //int numHits = 0;
    //HitResult hit;
    Node* curr;
    while (!StackNode.empty()) {
        curr = StackNode.top();
        StackNode.pop();
            
        // CONTINUE IF IT DIDN'T HIT THE BOUNDING BOX
        if (!curr->bounds.BoxIntersection(ray))
            continue;

        // ITERATE THROUGHT THE LEAF NODE
        if (curr->IsLeaf()) {
            //isHit = this->MainNode->HitTest(curr, closestHit, ray);
            this->HitTest(curr, closestHit, ray);
        }
        // PUSH THE NODE TO THE STACK
        else {
            StackNode.push(curr->ChildA);
            StackNode.push(curr->ChildB);
        }

    }

    hitPoint = closestHit.p;
    hitNormal = closestHit.normal;
    hitObject = closestHit.object;
    distance = closestHit.t;
    
    if (closestHit.object)
        return true;
    return false;
}


void 
Raytracer::HitTest(Node*& node, HitResult& closestHit, Ray ray) {
    HitResult hit;
    bool isHit = false;
    //std::cout << "Spheres Test Count: " << node->spheres.size() << std::endl;
    for (Sphere* sphere : node->spheres)
    {
        hit = sphere->Intersect(ray, hit.t);
        if (hit.HasValue())
        {
            //assert(hit.t < closestHit.t);
            if (hit.t < closestHit.t) {
                closestHit = hit;
                closestHit.object = sphere;
                isHit = true;
            }
        }
    }
}
void Raytracer::RayTraceChunk(vec2 Chunk) {
    size_t MinY = Chunk.x;
    size_t MaxY = Chunk.y;
    //std::cout << "MinY: " << MinY << ", MaxY: " << MaxY << std::endl;

    static int leet = 1337;
    std::mt19937 generator(leet++);
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    for (int y = MinY; y < MaxY; y++) {
        for (int x = 0; x < this->width;x++) {
			//std::cout << "x: " << x << ", y: "<< y << std::endl;
            Color color;
            for (int i = 0; i < this->rpp; i++) {
                /// !!! CHECK WHICH IS BETTER LATER !!!
                float u = ((float(x) + RandomFloat()) * (1.0f / this->width)) * 2.0f - 1.0f;
                float v = ((float(y) + RandomFloat()) * (1.0f / this->width)) * 2.0f - 1.0f;
                //float u = ((float(x) + dis(generator)) * (1.0f / this->width)) * 2.0f - 1.0f;
                //float v = ((float(y) + dis(generator)) * (1.0f / this->height)) * 2.0f - 1.0f;

                vec3 direction = vec3(u, v, -1.0f);
                direction = transform(direction, this->frustum);

                Ray ray = Ray(get_position(this->view), direction);
                color += this->TracePath(ray, 0);
                AssignColor(color, x, y);
            }
        }
    }
    CurrentThread.fetch_add(1);
}


void
Raytracer::QueueChunk(vec2 Chunk) {
    {
        std::unique_lock<std::mutex> lock(QueueMutex);
        ChunkInfo.push(Chunk);
        //std::cout << "Queued chunk: [" << Chunk.x << ", " << Chunk.y << "]" << std::endl;;
    }
    Mutex.notify_one();
}

void 
Raytracer::SpawnThread() {
    int Cores = std::thread::hardware_concurrency();
    for (int i = 0; i < Cores; i++) {
        Threads.emplace_back(&Raytracer::ThreadLoop, this);
    }
}

void 
Raytracer::ThreadLoop() {
    while (true) {
        vec2 Chunk;
        {
            std::unique_lock<std::mutex> lock(QueueMutex);
            Mutex.wait(lock, [this] {
                //std::cout << "Waiting: ChunkInfo.empty() = " << ChunkInfo.empty()
                //    << ", ShouldTerminate = " << bShouldTerminate << std::endl;;
                return !ChunkInfo.empty() || bShouldTerminate;
                });
            if (bShouldTerminate)
                return;
            Chunk = ChunkInfo.front();
            ChunkInfo.pop();
        }
        RayTraceChunk(Chunk);
    }
}

void 
Raytracer::Stop() {
    {
        std::unique_lock<std::mutex> lock(QueueMutex);
        bShouldTerminate = true;
    }
    Mutex.notify_all();
    for (std::thread& ActiveThread : Threads) {
        ActiveThread.join();
    }
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
        hit = object->Intersect(ray, hit.t);
        if (hit.HasValue())
        {
            closestHit = hit;
            closestHit.object = object;
            isHit = true;
            numHits++;
        }
    }
    if (closestHit.object == nullptr)

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

