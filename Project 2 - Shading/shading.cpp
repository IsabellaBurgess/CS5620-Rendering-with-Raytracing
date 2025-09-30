// Isabella Burgess - u1408202

#define GLUT_DISABLE_ATEXIT_HACK

#include <thread>

#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "include/cy/cyGL.h"
#include "include/cy/cyMatrix.h"

#include "include/headerFiles/scene.h"
#include "include/headerFiles/objects.h"
#include "include/headerFiles/renderer.h"
#include "include/headerFiles/ray.h"
#include "include/headerFiles/lights.h"
#include "include/headerFiles/materials.h"



#include "include/libs/tinyxml2.h"
#include "include/libs/lodepng.h"

using namespace std;

Camera cam;
Color24 *pixels;
Node rootNode;
MaterialList matList; 
LightList lightList;


float *zBuf;

double wsHeight;
double wsWidth;
float imageWidth;
float imageHeight;

int camOffset = 1;
int initBounceNum = 5;


const int numThreads = 16 ;
// const int numThreads = thread::hardware_concurrency();


Color shootRay(ShadeInfo shadeInf, Ray const &ray, HitInfo &hInfo, int bounceNum);

calculateRay rayCalculation; 


class RayTracer : public Renderer{
    public: 

    void BeginRender(){
        // printf("in begin render\n");
        scene = GetScene();
        RenderImage &sceneImage = GetRenderImage();
        pixels = sceneImage.GetPixels();
        zBuf = sceneImage.GetZBuffer();

        
        matList = scene.materials;
        lightList = scene.lights;
        
        cam = GetCamera();
        rootNode = scene.rootNode;

        Vec3f position = cam.pos;

        imageWidth = sceneImage.GetWidth();
        imageHeight = sceneImage.GetHeight();

        float fov = cam.fov*(M_PI/180);
    
        wsHeight = 2.0*camOffset*(tan(fov/2.0));
        wsWidth = wsHeight*(imageWidth/imageHeight);

        printf("w = %f, h = %f\n", wsWidth, wsHeight);

        //Values used for world space conversion

        Vec3f wsY = Vec3f(cam.up);
        Vec3f wsZ = Vec3f(cam.dir);
        Vec3f wsX = Vec3f(wsZ.Cross(wsY));
    
        Matrix3f testMat = Matrix3f(wsX, wsY, wsZ);
        testMat = testMat.GetTranspose();
        Matrix4f wsTransMatrix; 

        wsTransMatrix.SetColumn(0, Vec4f(wsX,0));
        wsTransMatrix.SetColumn(1, Vec4f(wsY,0));
        wsTransMatrix.SetColumn(2, Vec4f(-wsZ,0));
        wsTransMatrix.SetColumn(3, Vec4f(cam.pos,1));

        
        atomic<int> nextPixel{0}; 
        std::vector<std::thread> threads;
        threads.reserve(numThreads);


        int pixelIndex = 0;
        auto trace = [&] 
        {
            while(true)
            {
                const int pixelIndex = nextPixel.fetch_add(1, std::memory_order_relaxed);

                if(renderImage.IsRenderDone())
                {
                    break;
                }
                

                initalRays(pixelIndex, wsTransMatrix);   
                renderImage.IncrementNumRenderPixel(1);
         
            }
        };
                
        for (int t = 0; t < numThreads; t++)
        {
            threads.emplace_back(trace);
        }

        for(auto& th : threads) th.join();

        sceneImage.ComputeZBufferImage();
        sceneImage.SaveZImage("project2ZBuffer.png");
        sceneImage.SaveImage("renderedImage.png");
    }

    bool TraceRay(Ray const &ray, HitInfo &hInfo, int hitSide) const
    {
        const Node* node = hInfo.node;
        bool hitTracker = false;
        Ray transformedRay = node->ToNodeCoords(ray);
        // hInfo.node = node;
        const Object* currentObj = node->GetNodeObj();
    
        // printf("in hitObject\n");

        for(int i = 0; i < node->GetNumChild(); i++){
            const Node *child = node->GetChild(i);
            
            HitInfo childHitInf = HitInfo();
            childHitInf.Init();
            childHitInf.node = child;
 
            hInfo.node = child;

            if(TraceRay(transformedRay, hInfo, hitSide) == true)
            { 
                hitTracker = true;
                node->FromNodeCoords(hInfo);
            }
        }

            // printf("Parent node: %s\n", node->GetName());

        if(currentObj != nullptr)
        {
            if(currentObj->IntersectRay(transformedRay, hInfo, 1))
            {
                hitTracker = true;
                node->FromNodeCoords(hInfo);

                hInfo.node = node;

            }
        }

        return hitTracker;
    }

    void initalRays(int i, Matrix4f wsTransMatrix){

        int x = i%(int)imageWidth;
        int y = i/imageWidth;
            
        Ray currentRay;

        //Camera location
        currentRay.p = cam.pos;

        //Generate rays from camera
        //get to the top corner of the image, move over half pixel each time
        auto rayX = -(wsWidth/2) + ((wsWidth* (x + 1/2) / imageWidth));
        auto rayY = (wsHeight/2) - ((wsHeight* (y + 1/2) / imageHeight));

        currentRay.dir.Set(rayX, rayY, -camOffset); //calculate the direction

        // currentRay.p = Vec3f(wsTransMatrix * Vec4f(currentRay.p, 0.0));
        currentRay.dir = Vec3f(wsTransMatrix * Vec4f(currentRay.dir, 0.0));
        currentRay.dir.Normalize();

        ShadeInfo shade = ShadeInfo(lightList);
        // printf("Camera pos: [%f, %f, %f]\n", currentRay.p.x, currentRay.p.y, currentRay.p.z);

        // printf("The Current ray pos is [%f, %f, %f] \n", currentRay.p.x, currentRay.p.y, currentRay.p.z);

        // printf("The Current ray dir is [%f, %f, %f] \n", currentRay.dir.x, currentRay.dir.y, currentRay.dir.z);
        // quadratic equation for ray intersection

        HitInfo hitInf = HitInfo();
        hitInf.Init();
        hitInf.node = &rootNode;

        shade.SetPixel(x, y);

        Color finalColor = shootRay(shade, currentRay, hitInf, initBounceNum);
        int numPixel = y*imageWidth + x;

        pixels[numPixel] = (Color24) finalColor;
        zBuf[numPixel] = hitInf.z;
    }

};

RayTracer sceneRenderer;

Color shootRay(ShadeInfo shadeInf, Ray const &ray, HitInfo &hInfo, int bounceNum ){
    hInfo.node = &rootNode;
    bool hit = sceneRenderer.TraceRay(ray, hInfo, hInfo.front);         
    Color color;

    // printf("%d\n", hitTracker);
    if(hit == true)
    {
        // printf("true\n");
            // printf("hit node: %s\n", hitInf.node->GetName());
        const Material *currentMat = hInfo.node->GetMaterial();


        shadeInf.SetHit(ray, hInfo);
        color = currentMat->Shade(shadeInf);
        // color = Color(1, 200, 255);
    }

    else{
        // printf("False\n");
        color.SetBlack();
    }

    return color;
}


bool Sphere::IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide ) const{
    //Quadratic equation info
    //Assume sphere is at 0, 0, 0, radius = 1

    float a = ray.dir.Dot(ray.dir);
    float b = 2.0*(ray.dir.Dot(ray.p));
    float c = ((ray.p.Dot(ray.p)) - 1.0);

    // printf("a b c = %f %f %f\n", a, b, c);
    float delta = (b*b) - (4.0*(a*c));
    float t1 = (-b - sqrt(delta))/(2.0*a);
    float t2 = (-b + sqrt(delta))/(2.0*a);  

    // if(t1 < 0.01){
    //     t1 = (-b + sqrt(delta))/(2.0*a);
    // }
    if (delta >= 0)
    {    
        if (t1 <= 0.001 && t2 > 0.001){
            hInfo.front = false;

            if(t2 < hInfo.z ){

                hInfo.z = t2;
                hInfo.p = ray.p + ray.dir*t2;

                hInfo.N = -Normalize(hInfo.p);

                // printf("face hit %d", hInfo.front);
                return true;
            }

        }


        else if (t1 > 0.001)
        {
            hInfo.front = true;
            if(t1 < hInfo.z){
                hInfo.z = t1;
                hInfo.p = ray.p + ray.dir*t1;

                hInfo.N = Normalize(hInfo.p);

                // printf("face hit %d", hInfo.front);
                return true;
            }
        }

        // printf("true\n");
    }


    return false;
}


int main (int argc, char** argv){
    RayTracer sceneRenderer;
    sceneRenderer.LoadScene("sceneFiles/project_2_scene.xml");

    ShowViewport(&sceneRenderer, false);
    return 0;
}

void StopRender(){}
