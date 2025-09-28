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

float maxT = BIGFLOAT;

int camOffset = 1;
int initBounceNum = 5;


const int numThreads = 16 ;
// const int numThreads = thread::hardware_concurrency();


Color shootRay(int x, int y, Ray const &ray, HitInfo &hInfo, int bounceNum);

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
        // sceneImage.SaveZImage("project2ZBuffer.png");
        sceneImage.SaveImage("renderedImage.png");
    }

    bool TraceRay(Ray const &ray, HitInfo &hInfo, int hitSide) const
    {
        bool hitTracker = rayCalculation.treeTraversal(&rootNode, ray, hInfo);
       
        return hitTracker;
    }

    bool TraceShadowRay(Ray const &ray, float t_max, int hitSide){
        HitInfo hitInf = HitInfo();
        hitInf.Init();

        bool hit =  rayCalculation.treeTraversal(&rootNode, ray, hitInf);
        
        if(hitInf.z > t_max) 
        {
            hit = false;
        }
                // printf("Hit tracker: %d\n ", hitTracker);

        return hit;
    }

    bool shadowRay(Node *node, Ray ray, HitInfo &hInfo){
        bool hitTracker = false;
        Ray transformedRay = node->ToNodeCoords(ray);
        // hInfo.node = node;
        Object* currentObj = node->GetNodeObj();
    
        // printf("in hitObject\n");
    
        if(currentObj != nullptr){
            // printf("true2\n");

            if(currentObj->IntersectRay(transformedRay, hInfo, 1))
            {
                // printf("true %d\n ", hitTracker);
                // node->FromNodeCoords(hInfo);

                return true;

            }
        }
        for(int i = 0; i < node->GetNumChild(); i++){
            Node *child = node->GetChild(i);
                // printf("true %d\n ", hitTracker);

            if(shadowRay(child, transformedRay, hInfo) == true)
            {
                return true;

            }
        }

        return false;
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

        // printf("Camera pos: [%f, %f, %f]\n", currentRay.p.x, currentRay.p.y, currentRay.p.z);

        // printf("The Current ray pos is [%f, %f, %f] \n", currentRay.p.x, currentRay.p.y, currentRay.p.z);

        // printf("The Current ray dir is [%f, %f, %f] \n", currentRay.dir.x, currentRay.dir.y, currentRay.dir.z);
        // quadratic equation for ray intersection

        HitInfo hitInf = HitInfo();
        hitInf.Init();
        hitInf.node = &rootNode;

        Color finalColor = shootRay(x, y, currentRay, hitInf, initBounceNum);
        int numPixel = y*imageWidth + x;

        pixels[numPixel] = (Color24) finalColor;
        zBuf[numPixel] = hitInf.z;
    }

};

RayTracer sceneRenderer;


int main (int argc, char** argv){
    RayTracer sceneRenderer;
    sceneRenderer.LoadScene("sceneFiles/boxScene.xml");

    ShowViewport(&sceneRenderer, false);
    return 0;
}


class Shadows : public ShadeInfo{
public:

    Shadows(std::vector<Light*> const &lightList) : ShadeInfo(lightList){}

    float TraceShadowRay( Ray const &ray, float t_max) const override
    {
        //    printf("hi\n");

        Ray rayCopy;
        rayCopy.dir = ray.dir;
        rayCopy.p = Vec3f(ray.p + (ray.dir * 0.0001));

        

        if(sceneRenderer.TraceShadowRay(ray, t_max, true))
        {
            return 0.0;
        }

        return 1.0;
    }
  
};



Color shootRay(int x, int y, Ray const &ray, HitInfo &hInfo, int bounceNum ){
    // hInfo.node = &rootNode;
    // printf("hit node: %s\n", hInfo.node->GetName());

    bool hit = sceneRenderer.TraceRay(ray, hInfo, hInfo.front);         
    Color color;

    // printf("%d\n", hit);
    if(hit == true && hInfo.node->GetNodeObj() != nullptr)
    {
        // printf("true\n");
            // printf("hit node: %s\n", hitInf.node->GetName());
        const Material *currentMat = hInfo.node->GetMaterial();

        Shadows shadeInf = Shadows(lightList);
        shadeInf.SetPixel(x, y);

        shadeInf.SetHit(ray, hInfo);
        color = currentMat->Shade(shadeInf);
        // color = Color(1, 200, 255);
    }

    else{
        // printf("F alse\n");
        color.SetBlack();
    }

    return color;
}


void StopRender(){}
