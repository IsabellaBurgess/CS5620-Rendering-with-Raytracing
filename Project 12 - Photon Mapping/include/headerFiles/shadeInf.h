 
#ifndef _SHADE_INF_HEADER_
#define _SHADE_INF_HEADER_

#include "renderer.h"
#include "scene.h"
#include "ray.h"
#include "objects.h"

using namespace cy;
using namespace std;


class Shadows : public ShadeInfo
{
public: 
    Shadows( std::vector<Light*> const &lightList, TexturedColor const &environment, RNG &r ) : ShadeInfo(lightList, environment, r) {}

    // calculateRay const* renderer;
    float TraceShadowRay( Ray const &ray, float t_max) const override;
    bool shadowRay(Node *node, Ray ray, HitInfo &hInfo);

    Color TraceSecondaryRay( Ray const &ray, float &dist, bool reflection, bool montCarlo) const override; 

    bool CanBounce() const override;

//kl

};
#endif 