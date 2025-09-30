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

bool calculateRay::treeTraversal( Node *node, Ray ray, HitInfo &hInfo)
{
    bool hitTracker = false;
    Ray transformedRay = node->ToNodeCoords(ray);
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

    if(currentObj != 0x0){

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
