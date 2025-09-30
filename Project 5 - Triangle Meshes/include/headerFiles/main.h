 
#ifndef _MAIN_HEADER_
#define _MAIN_HEADER_

#include "renderer.h"


class RayTracer : public Renderer
{
public: 
    void BeginRender();

    void initalRays(int i, Matrix4f wsTransMatrix);

};
#endif