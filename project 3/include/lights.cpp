#include <iostream>
#include <thread>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "cy/cyGL.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

#include "lights.h"

extern RenderScene scene;
extern Node rootNode;


bool shadowRay(Node *node, Ray ray, HitInfo &hInfo){
    bool hitTracker = false;
    Ray transformedRay = node->ToNodeCoords(ray);
    // hInfo.node = node;
    // Object* currentObj = node->GetNodeObj();
   
    // printf("in hitObject\n");
   
    if(node->GetNodeObj() != nullptr){
        // printf("true2\n");
        Object* currentObj = node->GetNodeObj();

        if(currentObj->IntersectRay(transformedRay, hInfo, 1))
        {
            // printf("true %d\n ", hitTracker);
    node->FromNodeCoords(hInfo);

            return true;

        }
    }
    for(int i = 0; i < node->GetNumChild(); i++){
        Node *child = node->GetChild(i);
            // printf("true %d\n ", hitTracker);

        if(shadowRay(child, transformedRay, hInfo) == true)
        {
            child->FromNodeCoords(hInfo);

            return true;

        }
    }

    return false;
}


float GenLight::Shadow( Ray const &ray, float t_max ){
    HitInfo hitInf = HitInfo();
    hitInf.Init();

    if(shadowRay(&rootNode, ray, hitInf))
    {
        if(hitInf.z <= t_max){
        //    printf("hi\n");
            return 0.0;
        }
    }

    return 1.0;
}