#ifndef _RAY_GENERATION_
#define _RAY_GENERATION_
 
#include "scene.h"
#include "renderer.h"


class calculateRay
{
    public:
        calculateRay(){}
        bool treeTraversal( Node *node, Ray ray, HitInfo &hInfo);

        // Color shootRay(Ray const &ray, HitInfo &hInfo, int bounceNum);
};

 
#endif
