#include <iostream>
#include <thread>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "cy/cyGL.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

#include "headerFiles/lights.h"
#include "headerFiles/scene.h"
#include "headerFiles/ray.h"
#include "headerFiles/renderer.h"

extern Node rootNode;

using namespace cy;

extern calculateRay rayCalculation; 
extern RNG rng;
extern float shadowBias; 
extern float minShadowSamples;
extern float maxShadowSamples;

extern int photonCount;

Color PointLight::Illuminate( ShadeInfo const &sInfo, Vec3f &dir ) const{    
    Vec3f d = position - sInfo.P(); 
    dir = d.GetNormalized(); 

    Vec3f u;
    Vec3f v;
    dir.GetOrthonormals(u, v);

    
    float shadowValue = 0;

    int hitsFound = 0;
    int totalSamples = maxShadowSamples;
    Vec3f distance = Vec3f(1, 1, 1);

    for(int i = 0; i < maxShadowSamples; i++)
    {
        float ranX = Halton(i, 2) + rng.RandomFloat();
        float ranY = Halton(i, 3) + rng.RandomFloat();

        float ranRad = Halton(i, 5) + rng.RandomFloat();
        float ranAng = Halton(i, 7) + rng.RandomFloat();

        if(ranRad > 1){
            ranRad = ranRad - 1;
        }

        if(ranAng > 1){
            ranAng = ranAng - 1;
        }

        if(ranX > 1){
            ranX = ranX - 1;
        }

        if(ranY > 1){
            ranY = ranY - 1;
        }

        float radius = sqrt(ranX) * size;

        float angle = 2.0*Pi<float>() * ranY;

        float offsetU = radius * cos(angle);
        float offsetV = radius * sin(angle);

        Vec3f newPos = position + (u * offsetU) + (v * offsetV);
        Vec3f newDir = newPos - sInfo.P(); 

        Vec3f lightDir = newDir.GetNormalized();
        // newDir = newDir.GetNormalized();

        float currentShadowValue = sInfo.TraceShadowRay(lightDir, (float) newDir.Length());

        if(currentShadowValue > 0){
            hitsFound++;
        }
        shadowValue += currentShadowValue;

        if(i == minShadowSamples ){
            if(hitsFound == minShadowSamples || hitsFound == 0){
                totalSamples = minShadowSamples;        
      
                break;
            }
        }

        
    }

    if(attenuation > 0){
        distance = (position - sInfo.P());

        return (intensity * (float) ((float) shadowValue/ (float) totalSamples))/distance.LengthSquared(); 
    }  

    return (intensity * (float) ((float) shadowValue/ (float) totalSamples)); 

}

bool PointLight::IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide) const{
//Quadratic equation info
    //Assume sphere is at 0, 0, 0, radius = 1

    float a = ray.dir.Dot(ray.dir);
    float b = 2.0*(ray.dir.Dot(ray.p - position));
    float c = (((ray.p-position).Dot(ray.p - position)) - (pow(size, 2)));

    // printf("a b c = %f %f %f\n", a, b, c);
    float delta = (b*b) - (4.0*(a*c));
    float t1 = (-b - sqrt(delta))/(2.0*a);
    // float t2 = (-b + sqrt(delta))/(2.0*a);  

    // if(t1 < 0.01){
    //     t1 = (-b + sqrt(delta))/(2.0*a);
    // }
    // printf("delta = %f\n", delta);
    if (delta >= 0)
    {   
        if (t1 > 0.001)
        {
            hInfo.front = true;
            if(t1 < hInfo.z){
                hInfo.z = t1;
                hInfo.p = ray.p + ray.dir*t1;

                // hInfo.N = Normalize(hInfo.p);

                // float u = (atan2(hInfo.p.y, hInfo.p.x)/(2*Pi<float>())) + (1/2);
                // float v = (asin(hInfo.p.z)/(Pi<float>())) + (1/2);

                // hInfo.uvw = Vec3f(u, v, 0);
                hInfo.light = true;
                // printf("face hit %d", hInfo.front);
                return true;
            }
        }


        // printf("true\n");
    }

    // hInfo.light = true;

    return false;
}


void  PointLight::RandomPhoton( RNG &rng, Ray &r, Color &c ) const {
    float randU = rng.RandomFloat();
    float randV = rng.RandomFloat();

    float theta = 2.0 * M_PI * randU;
    float posZ = (1.0 - 2.0 * randV);

    float rProj = sqrtf(1.0 - posZ*posZ);

    float posX = rProj * cos(theta);
    float posY = rProj * sin(theta);

    Vec3f rayPos = Vec3f(posX, posY, posZ) ;


    r.p = (size*rayPos) +  position;

    float phiOffset = rng.RandomFloat();
    
    float thetaOffset = rng.RandomFloat();

    Vec3f u = Vec3f(0, 0, 0); 
    Vec3f v = Vec3f(0, 0, 0);

    Vec3f norm = (rayPos);
    norm.GetOrthonormals(u, v);


    float phi = (2.0*Pi<float>() * phiOffset);
    float cosTheta =  thetaOffset;

    float sinTheta = sqrt(1.0 - (cosTheta * cosTheta));

    Vec3f localDir = Vec3f(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);  
    Vec3f worldDir = (localDir.x * u) + (localDir.y * v) + (localDir.z * norm);

    r.dir = worldDir;

    c = (intensity * 4.0f * Pi<float>() * size * size)/photonCount;
}