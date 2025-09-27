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

extern double wsHeight;
extern double wsWidth;
extern float imageWidth;
extern float imageHeight;

extern Camera cam;
extern Color24 *pixels;
extern Node rootNode;
// extern MaterialList matList; 
// extern LightList lightList;

extern float *zBuf;


extern int camOffset;
extern float shadowBias;

using namespace cy;
using namespace std;


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
        // if (t1 <= 0.001 && t2 > 0.001){
        //     hInfo.front = false;

        //     if(t2 < hInfo.z ){

        //         hInfo.z = t2;
        //         hInfo.p = ray.p + ray.dir*t2;

        //         hInfo.N = -Normalize(hInfo.p);

        //         // printf("face hit %d", hInfo.front);
        //         return true;
        //     }

        // }


        if (t1 > 0.00)
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


// Color calculateRay::shootRay(Ray const &ray, HitInfo &hInfo, int bounceNum){
//     hInfo.node = &rootNode;
//     bool hit = sceneRenderer.TraceRay(ray, hInfo, hInfo.front);         
//     Color color;
//     printf("in shoot ray\n");
    
// //     if(ray.dir%hInfo.N > 0.0){
// //         hInfo.N = -hInfo.N;
// //         hInfo.front = false;
// //         // printf("back hit %d\n", hInfo.front);
// //     }

// //     else if(ray.dir%hInfo.N <= 0.0){
// //         hInfo.front = true;
// // //        printf("front hit\n");

// //     }
//         // printf("Node name is %s\n", hitInf.node->GetName());

//     // printf("%d\n", hitTracker);
//     if(hit == true)
//     {
//         // printf("true\n");
//             // printf("hit node: %s\n", hitInf.node->GetName());
//         // const Material *currentMat = hInfo.node->GetMaterial();
//         // color = currentMat->Shade(ray, hInfo, lightList, bounceNum);
//         color = Color(1, 200, 255);
//     }

//     else{
//         // printf("False\n");
//         color.SetBlack();
//     }

//     return color;
// }

