
//-------------------------------------------------------------------------------
///
/// \file       lights.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    13.0
/// \date       October 25, 2025
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#ifndef _LIGHTS_H_INCLUDED_
#define _LIGHTS_H_INCLUDED_
 
#include "renderer.h"
 
using namespace cy;
//-------------------------------------------------------------------------------
 
class GenLight : public Light
{
protected:
    void SetViewportParam( int lightID, ColorA const &ambient, ColorA const &intensity, Vec4f const &pos ) const;
};
 
//-------------------------------------------------------------------------------
 
class AmbientLight : public GenLight
{
public:
#ifdef LEGACY_SHADING_API
    Color Illuminate( ShadeInfo const &sInfo, Vec3f &dir ) const override { return intensity; }
#endif
    Color Intensity() const override { return intensity; }
    bool  IsAmbient() const override { return true; }
    void  SetViewportLight( int lightID ) const override { SetViewportParam(lightID,ColorA(intensity),ColorA(0.0f),Vec4f(0,0,0,1)); }
    void  Load( Loader const &loader ) override;
 
    bool GenerateSample( SamplerInfo const &sInfo, Vec3f &dir, Info &si ) const override
    {
        si.prob=1; si.mult=intensity; si.dist=0; dir=sInfo.N(); si.lobe=DirSampler::Lobe::ALL; return true;
    }
 
protected:
    Color intensity = Color(0,0,0);
};
 
//-------------------------------------------------------------------------------
 
class DirectLight : public GenLight
{
public:
#ifdef LEGACY_SHADING_API
    Color Illuminate( ShadeInfo const &sInfo, Vec3f &dir ) const override { dir=-direction; return intensity * sInfo.TraceShadowRay(-direction); }
#endif
    Color Intensity() const override { return intensity; }
    void  SetViewportLight( int lightID ) const override { SetViewportParam(lightID,ColorA(0.0f),ColorA(intensity),Vec4f(-direction,0.0f)); }
    void  Load( Loader const &loader ) override;
 
    bool GenerateSample( SamplerInfo const &sInfo, Vec3f &dir, Info &si ) const override
    {
        si.prob=1; si.mult=intensity; si.dist=BIGFLOAT; dir=-direction; si.lobe=DirSampler::Lobe::ALL; return true;
    }
 
protected:
    Color intensity = Color(0,0,0);
    Vec3f direction = Vec3f(0,0,0);
};
 
//-------------------------------------------------------------------------------
 
class PointLight : public GenLight
{
public:
#ifdef LEGACY_SHADING_API
    Color Illuminate( ShadeInfo const &sInfo, Vec3f &dir ) const override;
#endif
    Color Radiance( SamplerInfo const &sInfo ) const override { return intensity / (Pi<float>()*size*size); }
    Color Intensity     () const override { return intensity; }
    bool  IsRenderable  () const override { return size > 0.0f; }
    bool  IsPhotonSource() const override { return true; }
    void  RandomPhoton( RNG &rng, Ray &r, Color &c ) const override;
    void  SetViewportLight( int lightID ) const override;
    void  Load( Loader const &loader ) override;
 
    bool IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const override;
    Box  GetBoundBox() const override { return Box( position-size, position+size ); }
    void ViewportDisplay( Material const *mtl ) const override; // used for OpenGL display
 
    bool GenerateSample( SamplerInfo const &sInfo, Vec3f       &dir, Info &si ) const override;
    void GetSampleInfo ( SamplerInfo const &sInfo, Vec3f const &dir, Info &si ) const override;
 
protected:
    Color intensity   = Color(0,0,0);
    Vec3f position    = Vec3f(0,0,0);
    float size        = 0.0f;
    float attenuation = 0.0f;   // Zero means no attenuation. If non-zero, light distance is scaled by attenuation.
};
 
//-------------------------------------------------------------------------------
 
#endif
