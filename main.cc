#include <stdio.h>
#include "window.h"
#include "vec3.h"
#include "raytracer.h"
#include "sphere.h"
#include <chrono>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define degtorad(angle) angle * MPI / 180


int main(int argc, char* argv[])
{ 
    unsigned width = 300;
    unsigned height = 300;
    int RaysPerPixel = 1;
    int SphereAmount = 36;
    int maxBounces = 5;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            width = std::stoi(argv[i + 1]);
			std::cout << "width: " << width << std::endl;
        }
        else if (strcmp(argv[i], "-h") == 0) {
            height = std::stoi(argv[i + 1]);
			std::cout << "height: " << height << std::endl;
        }
        else if (strcmp(argv[i], "-rpp") == 0) {
            RaysPerPixel = std::stoi(argv[i + 1]);
            std::cout << "ray per pixel: " << RaysPerPixel << std::endl;
        }
        else if (strcmp(argv[i], "-s") == 0) {
            SphereAmount = std::stoi(argv[i + 1]);
            std::cout << "SphereAmount: " << SphereAmount << std::endl;
        } 
        else if (strcmp(argv[i], "-b") == 0) {
            maxBounces = std::stoi(argv[i + 1]);
            std::cout << "MaxBounce: " << maxBounces << std::endl;
        } 
    }
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;
    std::cout << "Ray Per Pixel: " << RaysPerPixel << std::endl;
    std::cout << "Sphere Amount: " << SphereAmount << std::endl;
    Display::Window wnd;
    wnd.SetTitle("TrayRacer");
    
    if (!wnd.Open())
        return 1;

    std::vector<Color> framebuffer;

    framebuffer.resize(width * height);
    

    Raytracer rt = Raytracer(width, height, framebuffer, RaysPerPixel, maxBounces);

    // Create some objects
    Material* mat = new Material();
    mat->type = "Lambertian";
    mat->color = { 0.5,0.5,0.5 };
    mat->roughness = 0.3;
    Sphere* ground = new Sphere(1000, { 0,-1000, -1 }, mat);
    rt.AddObject(ground);
    
    std::vector<std::string>MaterialType;
    std::vector<float> SpanVec;
    MaterialType = { "Lambertian", "Conductor", "Dielectric"};
    SpanVec = { 10.0f, 30.0f, 25.0f };
    for (int it = 0; it < SphereAmount; it++)
    {
		Material* mat = new Material();
		mat->type = MaterialType[it % 3];
		float r = RandomFloat();
		float g = RandomFloat();
		float b = RandomFloat();
		mat->color = { r,g,b };
		mat->roughness = RandomFloat();
        Sphere* ground = new Sphere(
		RandomFloat() * 0.7f + 0.2f,
		{
			RandomFloatNTP() * SpanVec[it % 3],
			RandomFloat() * SpanVec[it % 3] + 0.2f,
			RandomFloatNTP() * SpanVec[it % 3] 
		},
		mat);
		rt.AddObject(ground);
    }
    
    bool exit = false;

    // camera
    bool resetFramebuffer = false;
    vec3 camPos = { 0,1.0f,10.0f };
    vec3 moveDir = { 0,0,0 };

    wnd.SetKeyPressFunction([&exit, &moveDir, &resetFramebuffer](int key, int scancode, int action, int mods)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            exit = true;
            break;
        case GLFW_KEY_W:
            moveDir.z -= 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_S:
            moveDir.z += 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_A:
            moveDir.x -= 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_D:
            moveDir.x += 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_SPACE:
            moveDir.y += 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            moveDir.y -= 1.0f;
            resetFramebuffer |= true;
            break;
        default:
            break;
        }
    });

    float pitch = 0;
    float yaw = 0;
    float oldx = 0;
    float oldy = 0;

    wnd.SetMouseMoveFunction([&pitch, &yaw, &oldx, &oldy, &resetFramebuffer](double x, double y)
    {
        x *= -0.1;
        y *= -0.1;
        yaw = x - oldx;
        pitch = y - oldy;
        resetFramebuffer |= true;
        oldx = x;
        oldy = y;
    });

    float rotx = 0;
    float roty = 0;

    // number of accumulated frames
    int frameIndex = 0;

    std::vector<Color> framebufferCopy;
    framebufferCopy.resize(width * height);

    /// RENDERING LOOP
    //while (wnd.IsOpen() && !exit)
    {
        resetFramebuffer = false;
        moveDir = {0,0,0};
        pitch = 0;
        yaw = 0;

        // poll input
        wnd.Update();

        rotx -= pitch;
        roty -= yaw;

        moveDir = normalize(moveDir);

        mat4 xMat = (rotationx(rotx));
        mat4 yMat = (rotationy(roty));
        mat4 cameraTransform = multiply(yMat, xMat);

        camPos = camPos + transform(moveDir * 0.2f, cameraTransform);
        
        cameraTransform.m30 = camPos.x;
        cameraTransform.m31 = camPos.y;
        cameraTransform.m32 = camPos.z;

        rt.SetViewMatrix(cameraTransform);
        
        if (resetFramebuffer)
        {
            rt.Clear();
            frameIndex = 0;
        }
        double RayNum;
		auto start = std::chrono::high_resolution_clock::now();
		RayNum = rt.Raytrace();
		auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> frameDuration = end - start;
        double DoubleFrameDuration = frameDuration.count();
        // Calculating MRay/S
        std::cout << "Duration: " << frameDuration.count() << " sec" << std::endl;
        std::cout << "Total Number of Rays: " << RayNum << std::endl;
        std::cout << "Total Rays: " << (RayNum / 1'000'000)/DoubleFrameDuration << "MRays/s" << std::endl;

        frameIndex++;

        // Get the average distribution of all samples
		std::vector<uint8_t> ImageData;
        {
            size_t p = 0;
            for (Color const& pixel : framebuffer)
            {
                framebufferCopy[p] = pixel;
                framebufferCopy[p].r /= frameIndex;
                framebufferCopy[p].g /= frameIndex;
                framebufferCopy[p].b /= frameIndex;
                ImageData.push_back(255 * framebufferCopy[p].r);
                ImageData.push_back(255 * framebufferCopy[p].g);
                ImageData.push_back(255 * framebufferCopy[p].b);
                p++;
            }
        }
        stbi_flip_vertically_on_write(1);
		stbi_write_png("Frame.png", width, height, 3, ImageData.data(), width * 3);

        glClearColor(0, 0, 0, 1.0);
        glClear( GL_COLOR_BUFFER_BIT );

        wnd.Blit((float*)&framebufferCopy[0], width, height);
        wnd.SwapBuffers();
    }

    if (wnd.IsOpen())
        wnd.Close();

    return 0;
} 
