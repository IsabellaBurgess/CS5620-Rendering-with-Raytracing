#include <iostream>
#include <thread>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "cy/cyGL.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

#include "headerFiles/lights.h"
#include "headerFiles/scene.h"
#include "headerFiles/ray.h"
#include "headerFiles/objects.h"

extern double wsHeight;
extern double wsWidth;
extern float imageWidth;
extern float imageHeight;

extern Camera cam;
extern Color24 *pixels;
extern Node rootNode;
extern MaterialList matList; 
extern LightList lightList;

extern float *zBuf;

extern RenderScene scene;

extern int camOffset;
extern int bounceNum;
extern float shadowBias;

using namespace cy;
using namespace std;




Color calculateRay::shootRay(Ray const &ray, HitInfo &hInfo){

    bool hit = treeTraversal(&rootNode, ray, hInfo);         
    Color color;

        // printf("Node name is %s\n", hitInf.node->GetName());

    // printf("%d\n", hitTracker);
    if(hit == true && hInfo.node->GetNodeObj() != nullptr)

    {
        // printf("true\n");
            // printf("hit node: %s\n", hitInf.node->GetName());
        const Material *currentMat = hInfo.node->GetMaterial();
        color = currentMat->Shade(ray, hInfo, lightList, bounceNum);
        // color = Color24(1, 200, 255);
    }

    else{
        color.SetBlack();
    }

    return color;

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
    float t = (-b - sqrt(delta))/(2.0*a);
  

    // if(t < shadowBias){
    //     t = (-b + sqrt(delta))/(2.0*a);
    // }
    // printf("delta = %f\n", delta);
    if(t < 0)
    {
        // printf("false\n");
        return false;
    }
   
    if (t >= 0)
    {    
        
        if(t < hInfo.z ){

            hInfo.z = t;
            hInfo.p = ray.p + ray.dir*t;

            hInfo.N = Normalize(hInfo.p);

            if(ray.dir%hInfo.N > 0.0){
                hInfo.N = -hInfo.N;
                hInfo.front = false;
            }

            else{
                hInfo.front = true;
            }
            return true;
        }
        // printf("true\n");
    }


    return false;
}
