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
extern float shadowBias;

using namespace cy;
using namespace std;




Color calculateRay::shootRay(Ray const &ray, HitInfo &hInfo, int bounceNum){

    bool hit = treeTraversal(&rootNode, ray, hInfo);         
    Color color;
    
//     if(ray.dir%hInfo.N > 0.0){
//         hInfo.N = -hInfo.N;
//         hInfo.front = false;
//         // printf("back hit %d\n", hInfo.front);
//     }

//     else if(ray.dir%hInfo.N <= 0.0){
//         hInfo.front = true;
// //        printf("front hit\n");

//     }
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
