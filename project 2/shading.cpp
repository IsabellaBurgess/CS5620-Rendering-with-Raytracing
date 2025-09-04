// Isabella Burgess - u1408202

#define GLUT_DISABLE_ATEXIT_HACK

#include <iostream>
#include <thread>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "include/cy/cyGL.h"
#include "include/cy/cyMatrix.h"
#include "include/cy/cyVector.h"


#include "include/tinyxml2.h"
#include "include/scene.h"
#include "include/objects.h"
#include "include/materials.h"
#include "include/lights.h"

#include "include/lodepng.h"

using namespace std;

int LoadScene( RenderScene &scene, char const *filename );
void ShowViewport( RenderScene *scene );
bool treeTraversal( Node *node, Ray ray, HitInfo &hInfo);
void traceRaycer(int i, Matrix4f wsTransMatrix, RenderScene *scene);
Color Shade(Ray const &ray, HitInfo const &hInfo, LightList const &lights);
void SetViewportMaterial(int subMtlID=0);

// bool hitTracker;
const int numThreads = thread::hardware_concurrency();

//Scene information
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

void BeginRender( RenderScene *scene )
{
    RenderImage &sceneImage = scene->renderImage;
    pixels = scene->renderImage.GetPixels();
    zBuf = scene->renderImage.GetZBuffer();

    matList = scene->materials;
    lightList = scene->lights;

    cam = scene->camera;
    rootNode = scene->rootNode;

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
    
    // Matrix4f initTransMatrix = Matrix4f(Vec4f(wsX, 0), Vec4f(wsY,0), Vec4f(wsZ,0), Vec4f(cam.pos,1));
    // Matrix4f wsTransMatrix = initTransMatrix.GetTranspose();

    atomic<int> nextPixel{0}; 
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    auto trace = [&] 
    {
        while(true)
        {
            const int pixelIndex = nextPixel.fetch_add(1, std::memory_order_relaxed);
            if(pixelIndex >= imageHeight*imageWidth)
            {
                break;
            }
            traceRaycer(pixelIndex, wsTransMatrix, scene);
        }
    };
            
    for (int t = 0; t < numThreads; t++)
    {
        threads.emplace_back(trace);
    }

    for(auto& th : threads) th.join();
   
    // sceneImage.ComputeZBufferImage();
    // sceneImage.SaveZImage("project2ZBuffer.png");
    sceneImage.SaveImage("project2.png");
}

void StopRender (){}

void traceRaycer(int i, Matrix4f wsTransMatrix, RenderScene *scene){

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
    //quadratic equation for ray intersection

    HitInfo hitInf = HitInfo();
    hitInf.Init();

    bool hit = treeTraversal(&rootNode, currentRay, hitInf);         
    Color color;


    // printf("%d\n", hitTracker);
    if(hit == true)
    {
        // printf("true\n");
            // printf("hit node: %s\n", hitInf.node->GetName());
        const Material *currentMat = hitInf.node->GetMaterial();
        color = currentMat->Shade(currentRay, hitInf, lightList);
        // color = Color24(1, 200, 255);
    }

    else{
        color.SetBlack();
    }

    int numPixel = y*imageWidth + x;

    pixels[numPixel] = (Color24) color;
    zBuf[numPixel] = hitInf.z;
    scene->renderImage.IncrementNumRenderPixel(1);

}



bool Sphere::IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide ) const{
    //Quadratic equation info
    //Assume sphere is at 0, 0, 0, radius = 1

    float a = ray.dir.Dot(ray.dir);
    float b = 2.0*(ray.dir.Dot(ray.p));
    float c = ((ray.p.Dot(ray.p)) - 1.0);

    // printf("a b c = %f %f %f\n", a, b, c);
    float delta = (b*b) - (4.0*(a*c));
    float t = (-b - sqrt(delta))/(2.0*a);

    // printf("delta = %f\n", delta);
    if(t < 0)
    {
        // printf("false\n");
        return false;
    }
   
    if (t > 0)
    {    
        if(t < hInfo.z ){
            hInfo.z = t;
            hInfo.p = ray.p + ray.dir*t;

            hInfo.N = Normalize(hInfo.p);
            return true;
        }
        // printf("true\n");
   
    }
    return false;
}

int main (int argc, char** argv){   
    RenderScene scene;
   
    LoadScene(scene, "project_2_scene.xml");

    ShowViewport(&scene);
}


bool treeTraversal( Node *node, Ray ray, HitInfo &hInfo)
{
    bool hitTracker = false;
    Ray transformedRay = node->ToNodeCoords(ray);

    for(int i = 0; i < node->GetNumChild(); i++){
        Node *child = node->GetChild(i);
        if(treeTraversal(child, transformedRay, hInfo) == true)
        {
            hitTracker = true;
            // hInfo.node = child;
        }
    }
   
    // printf("Parent node: %s\n", node->GetName());

    Object* currentObj = node->GetNodeObj();
   
    // printf("in hitObject\n");
   
    if(currentObj){
        // printf("true2\n");
        if(currentObj->IntersectRay(transformedRay, hInfo, 1))
        {
            hitTracker = true;
            hInfo.node = node;
            // printf("true %d\n ", hitTracker);
        }
    }

    node->FromNodeCoords(hInfo);
    return hitTracker;
}







