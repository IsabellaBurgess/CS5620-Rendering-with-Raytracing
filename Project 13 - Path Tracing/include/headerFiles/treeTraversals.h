#ifndef _TREE_TRAVERSALS_
#define _TREE_TRAVERSALS_
 
#include "scene.h"
#include "renderer.h"


class treeTraversals
{
    public:
        treeTraversals(){}
        bool rayTreeTraversal( Node *node, Ray ray, HitInfo &hInfo);

        // Color shootRay(Ray const &ray, HitInfo &hInfo, int bounceNum);
};

 
#endif
