// Isabella Burgess - u1408202

#define GLUT_DISABLE_ATEXIT_HACK

#include <thread>

#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "cy/cyGL.h"
#include "cy/cyMatrix.h"
#include "headerFiles/shadeInf.h"
#include "headerFiles/ray.h"


using namespace cy;
using namespace std;

extern calculateRay rayCalculation; 
extern Node rootNode; 

extern Color backgroundColor;
extern int bounceNum;

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


