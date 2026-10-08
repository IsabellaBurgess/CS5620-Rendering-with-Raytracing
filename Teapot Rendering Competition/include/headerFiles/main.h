// Isabella Burgess
// CS 6620 - Rendering with Ray Tracing

#ifndef _MAIN_HEADER_
#define _MAIN_HEADER_

#include "renderer.h"
#include "scene.h"


class RayTracer : public Renderer
{
public: 
    void BeginRender();

    Color takeSample(int x, int y, int i, HitInfo &hitInf);

    float tValues [71] = { 12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228,
                           2.201, 2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093, 2.086,
                           2.080, 2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042,
                           2.040, 2.037, 2.035, 2.032, 2.030, 2.028, 2.026, 2.024, 2.023, 2.021,
                           2.020, 2.018, 2.017, 2.015, 2.014, 2.013, 2.012, 2.011, 2.010, 2.009,
                           2.000, 2.000, 2.000, 2.000, 2.000, 2.000, 2.000, 2.000, 2.000, 2.000,
                           1.994, 1.994, 1.994, 1.994, 1.994, 1.994, 1.994, 1.994, 1.994, 1.994};
    void fillPhotonMap(PhotonMap* map, PhotonMap* causticMap);
    void bouncePhoton(PhotonMap* map, PhotonMap* caustics, DirSampler::Info &si, Ray ray, Color power);

    PhotonMap const* GetPhotonMap() const{return map;}
    PhotonMap const* GetCausticsMap() const{return caustics;}



    PhotonMap* map;
    PhotonMap* caustics; 
};
#endif