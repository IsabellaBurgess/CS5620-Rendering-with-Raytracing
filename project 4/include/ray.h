#ifndef _RAY_GENERATION_
#define _RAY_GENERATION_
 
#include "scene.h"


class calculateRay
{
    public:
        calculateRay(){}
        Color shootRay(Ray const &ray, HitInfo &hInfo);
        
        bool treeTraversal( Node *node, Ray ray, HitInfo &hInfo);
};

 
#endif
