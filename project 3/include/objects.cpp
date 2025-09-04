#include "objects.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

using namespace cy;


bool Sphere::IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide ) const{
    //Quadratic equation info
    //Assume sphere is at 0, 0, 0, radius = 1

    float a = ray.dir.Dot(ray.dir);
    float b = 2.0*(ray.dir.Dot(ray.p));
    float c = ((ray.p.Dot(ray.p)) - 1.0);

    // printf("a b c = %f %f %f\n", a, b, c);
    float delta = (b*b) - (4.0*(a*c));
    float t = (-b - sqrt(delta))/(2.0*a);

    // printf("delta = %f\n", delta);
    if(t < 0)
    {
        // printf("false\n");
        return false;
    }
   
    if (t > 0)
    {    
        if(t < hInfo.z ){
            hInfo.z = t;
            hInfo.p = ray.p + ray.dir*t;

            hInfo.N = Normalize(hInfo.p);
            return true;
        }
        // printf("true\n");
   
    }
    return false;
}