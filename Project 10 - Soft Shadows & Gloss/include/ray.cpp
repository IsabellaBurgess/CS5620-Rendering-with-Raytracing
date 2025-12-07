#include <iostream>
#include <thread>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "cy/cyGL.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

#include "headerFiles/scene.h"
#include "headerFiles/ray.h"
#include "headerFiles/objects.h"
#include "headerFiles/renderer.h"
#include "headerFiles/shadeInf.h"
#include "headerFiles/main.h"



extern double wsHeight;
extern double wsWidth;
extern float imageWidth;
extern float imageHeight;

extern Camera cam;
extern Color24 *pixels;
extern Node rootNode;
extern MaterialList matList; 
extern LightList lightList;
extern int bounceNum;
extern TexturedColor background;
extern TexturedColor env;


extern float *zBuf;

extern int camOffset;
extern float shadowBias;

using namespace cy;
using namespace std;

extern calculateRay rayCalculation; 
extern RayTracer sceneRenderer;
extern RNG rng;


Color calculateRay::shootRay(int x, int y, Ray const &ray, HitInfo &hInfo, int sampleCount){
    // hInfo.node = &rootNode;
    // printf("hit node: %s\n", hInfo.node->GetName());

    bool hit = TraceRay(ray, hInfo, hInfo.front);         
    Color color;

    // printf("%d\n", hit);
    if(hit == true && hInfo.node->GetNodeObj() != nullptr)
    {
        // printf("true\n");
            // printf("hit node: %s\n", hitInf.node->GetName());
        if(hInfo.light == true){
            printf("light\n");
            return Color(1, 1, 1);
        }  
        const Material *currentMat = hInfo.node->GetMaterial();

        Shadows shadeInf = Shadows(lightList, env, rng);
        shadeInf.SetPixel(x, y);

        shadeInf.SetHit(ray, hInfo);
        shadeInf.SetPixelSample(sampleCount);
        color = currentMat->Shade(shadeInf);
        // color = Color(1, 200, 255);
    }

    else{
        float u = (float) x / (float) cam.imgWidth ;
        float v = (float) y / (float) cam.imgHeight ;
        color = background.Eval(Vec3f(u, v, 0));


        // printf("F alse\n");


        // printf("background color is (%f, %f, %f)\n", color.r, color.b, color.g);
        // color.SetBlack();
    }

    return color;
}

bool calculateRay::TraceRay(Ray const &ray, HitInfo &hInfo, int hitSide) const
{
    bool hitTracker = rayCalculation.treeTraversal(&rootNode, ray, hInfo);
    
    for(int i = 0; i < lightList.size(); i++){
        if(lightList[i]->IsRenderable()){
            (lightList[i]->IntersectRay(ray, hInfo, hitSide));
                // hitTracker = true;
        }
    }
    return hitTracker;
}

bool calculateRay::treeTraversal( Node *node, Ray ray, HitInfo &hInfo)
{
    bool hitTracker = false;
    Ray transformedRay = node->ToNodeCoords(ray);
    // hInfo.node = node;
    Object* currentObj = node->GetNodeObj();
   
    // printf("in hitObject\n");
   
    for(int i = 0; i < node->GetNumChild(); i++){
        Node *child = node->GetChild(i);
        
        if(treeTraversal(child, transformedRay, hInfo) == true)
        {

            hitTracker = true;
            node->FromNodeCoords(hInfo);

            // hInfo.node = child;
        // node->FromNodeCoords(hInfo);

        }
    }

    if(currentObj != nullptr){

        if(currentObj->IntersectRay(transformedRay, hInfo, 1))
        {
            hitTracker = true;
            node->FromNodeCoords(hInfo);

            hInfo.node = node;

            // printf("true %d\n ", hitTracker);
        }
    }
    return hitTracker;
}

bool calculateRay::TraceShadowRay(Ray const &ray, float t_max, int hitSide) const{
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


//Shadows and shade info
