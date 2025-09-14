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


#include "include/libs/tinyxml2.h"
#include "include/libs/lodepng.h"

#include "include/headerFiles/scene.h"
#include "include/headerFiles/objects.h"
#include "include/headerFiles/materials.h"
#include "include/headerFiles/lights.h"


#include "include/headerFiles/ray.h"

using namespace std;
using namespace cy;

int LoadScene( RenderScene &scene, char const *filename );
void ShowViewport( RenderScene *scene );
void initalRays(int i, Matrix4f wsTransMatrix); 

void SetViewportMaterial(int subMtlID=0);

// const int numThreads = 1;
const int numThreads = thread::hardware_concurrency();

//Scene information
Camera cam;
Color24 *pixels;
Node rootNode;
MaterialList matList; 
LightList lightList;

calculateRay rayCalculation; 
RenderScene scene;

float *zBuf;

double wsHeight;
double wsWidth;
float imageWidth;
float imageHeight;

int camOffset = 1;
int initBounceNum = 5;

float shadowBias = 0.0001;


int main (int argc, char** argv){   

    LoadScene(scene, "sceneFiles/boxScene.xml");

    ShowViewport(&scene);
}

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
            initalRays(pixelIndex, wsTransMatrix);
        }
    };
            
    for (int t = 0; t < numThreads; t++)
    {
        threads.emplace_back(trace);
    }

    for(auto& th : threads) th.join();
   
    // sceneImage.ComputeZBufferImage();
    // sceneImage.SaveZImage("project2ZBuffer.png");
    sceneImage.SaveImage("renderedImage.png");
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
    //quadratic equation for ray intersection

    HitInfo hitInf = HitInfo();
    hitInf.Init();

    Color finalColor = rayCalculation.shootRay(currentRay, hitInf, initBounceNum);
    int numPixel = y*imageWidth + x;

    pixels[numPixel] = (Color24) finalColor;
    zBuf[numPixel] = hitInf.z;
    scene.renderImage.IncrementNumRenderPixel(1);

}

void StopRender (){}


