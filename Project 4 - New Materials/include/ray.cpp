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
extern Color backgroundColor;

extern float *zBuf;

extern int camOffset;
extern float shadowBias;

using namespace cy;
using namespace std;

extern calculateRay rayCalculation; 
extern RayTracer sceneRenderer;


Color calculateRay::shootRay(int x, int y, Ray const &ray, HitInfo &hInfo){
    // hInfo.node = &rootNode;
    // printf("hit node: %s\n", hInfo.node->GetName());

    bool hit = TraceRay(ray, hInfo, hInfo.front);         
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

bool calculateRay::TraceRay(Ray const &ray, HitInfo &hInfo, int hitSide) const
{
    bool hitTracker = rayCalculation.treeTraversal(&rootNode, ray, hInfo);
    
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


        if (t1 > 0.001)
        {
            // hInfo.front = true;
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
float Shadows::TraceShadowRay( Ray const &ray, float t_max) const {
    
    Ray rayCopy;
    rayCopy.dir = ray.dir;
    rayCopy.p = Vec3f(ray.p + (ray.dir * 0.0001));

    if(rayCalculation.TraceShadowRay(ray, t_max, true))
    {
        return 0.0;
    }

    return 1.0;
    
}

bool Shadows::shadowRay(Node *node, Ray ray, HitInfo &hInfo){
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

Color Shadows::TraceSecondaryRay( Ray const &ray, float &dist ) const
{
    HitInfo newHit;
    newHit.Init();
    Color newColor = backgroundColor;

    bool hit = rayCalculation.TraceRay(ray, newHit, newHit.front);
    
    
    if(hit == true && hInfo.node->GetNodeObj() != nullptr)
    {
        Shadows shade = *this;
        shade.SetHit(ray, newHit);
        shade.IncrementBounce();

        dist = newHit.z;
        newColor = newHit.node->GetMaterial()->Shade(shade);
    }
    
    return newColor;
} 

bool Shadows::CanBounce() const
{
    if(bounceS >= bounceNum){
        return false;
    }

    else {
        return true;
    }
}