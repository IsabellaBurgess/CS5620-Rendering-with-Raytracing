#ifndef _RAY_GENERATION_
#define _RAY_GENERATION_

#include "renderer.h"
#include "scene.h"

class calculateRay : public Renderer
{
    public:
        calculateRay(){};

        bool TraceRay(Ray const &ray, HitInfo &hInfo, int hitSide) const ;

        bool treeTraversal( Node *node, Ray ray, HitInfo &hInfo);
        Color shootRay(int x, int y, Ray const &ray, HitInfo &hInfo);

        bool TraceShadowRay(Ray const &ray, float t_max, int hitSide) const ;

        // Color shootRay(Ray const &ray, HitInfo &hInfo, int bounceNum);
};


#endif
