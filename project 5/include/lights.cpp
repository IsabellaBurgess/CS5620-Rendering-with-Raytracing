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

extern RenderScene scene;
extern Node rootNode;

using namespace cy;

extern calculateRay rayCalculation; 
extern float shadowBias;


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



float GenLight::Shadow( Ray const &ray, float t_max ){
    HitInfo hitInf = HitInfo();
    hitInf.Init();
    Ray rayCopy;
    rayCopy.dir = ray.dir;
    rayCopy.p = Vec3f(ray.p + (ray.dir * shadowBias));

    if(rayCalculation.treeTraversal(&rootNode, ray, hitInf))
    {
        if(hitInf.z <= t_max){
        //    printf("hi\n");
            return 0.0;
        }
    }

    return 1.0;
}
