 
#ifndef _SHADE_INF_HEADER_
#define _SHADE_INF_HEADER_

#include "renderer.h"
#include "scene.h"
#include "ray.h"
#include "objects.h"

using namespace cy;
using namespace std;


class Shadows : public SamplerInfo
{
public: 
    Shadows( std::vector<Light*> const &lightList, TexturedColor const &environment, RNG &r ) : SamplerInfo( r) {}

    // calculateRay const* renderer;
    bool shadowRay(Node *node, Ray ray, HitInfo &hInfo);



//kl

};
#endif 