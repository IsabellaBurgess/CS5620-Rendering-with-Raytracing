#include "headerFiles/objects.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"
#include "cy/cyTriMesh.h"
#include <cfloat>

using namespace cy;

float closest = __FLT_MAX__;

bool IntersectBox(Box box, Ray const &ray);


bool Sphere::IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide ) const{
    //Quadratic equation info
    //Assume sphere is at 0, 0, 0, radius = 1

    float a = ray.dir.Dot(ray.dir);
    float b = 2.0*(ray.dir.Dot(ray.p));
    float c = ((ray.p.Dot(ray.p)) - 1.0);

    // printf("a b c = %f %f %f\n", a, b, c);
    float delta = (b*b) - (4.0*(a*c));
    float t1 = (-b - sqrt(delta))/(2.0*a);
    float t2 = (-b + sqrt(delta))/(2.0*a);  

    // if(t1 < 0.01){
    //     t1 = (-b + sqrt(delta))/(2.0*a);
    // }
    // printf("delta = %f\n", delta);
    if (delta >= 0)
    {    
        if (t1 <= 0.001 && t2 > 0.001){
            hInfo.front = false;

            if(t2 < hInfo.z ){

                hInfo.z = t2;
                hInfo.p = ray.p + ray.dir*t2;

                hInfo.N = -Normalize(hInfo.p);

                // printf("face hit %d", hInfo.front);
                return true;
            }

        }


        else if (t1 > 0.001)
        {
            hInfo.front = true;
            if(t1 < hInfo.z){
                hInfo.z = t1;
                hInfo.p = ray.p + ray.dir*t1;

                hInfo.N = Normalize(hInfo.p);

                // printf("face hit %d", hInfo.front);
                return true;
            }
        }

        // printf("true\n");
    }


    return false;
}

bool Plane::IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide) const{

    float t = -(ray.p.z/ray.dir.z);

    Vec3f hitPoint = ray.p + ray.dir*t;
    float hitPointX = hitPoint.x;
    float hitPointY = hitPoint.y;

    if(hitPointX <= 1 && hitPointX >= -1 && hitPointY <= 1 && hitPointY >= -1 )
    {
    
    if (t > 0.001){
        
        if(t < hInfo.z){
      
            hInfo.z = t;
            hInfo.p = ray.p + ray.dir*t;


            hInfo.N =  Vec3f(0, 0, 1);

            // printf("face hit %d", hInfo.front);
            return true;
        }
    }
}

    return false;
}

bool TriObj::IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide ) const{

    Box boundBox = this->GetBoundBox();
    
    bool hitFound = false;

    // if(IntersectBox(boundBox, ray)){
        for(int i = 0; i < this->nf; i++) {
            if(this->IntersectTriangle(ray, hInfo, hitSide, i)){
                hitFound=true;
            }
        }
    // }

    return hitFound;
}

bool TriObj::IntersectTriangle( Ray const &ray, HitInfo &hInfo, int hitSide, unsigned int faceID ) const{
    
    TriMesh::TriFace currentFace = F(faceID);
    constexpr float epsilon = std::numeric_limits<float>::epsilon();

    Vec3f v0 = V(currentFace.v[0]);
    Vec3f v1 = V(currentFace.v[1]);
    Vec3f v2 = V(currentFace.v[2]);

    Vec3f e1 = v1 - v0;
    Vec3f e2 = v2 - v0;
    
    Vec3f norm = Normalize(e2.Cross(e1));

    float h = -(v0 % norm);

    float t = -(((ray.p % norm) + h)/(ray.dir % norm));

    norm = norm;

    
    if (t > 0.001 && t < hInfo.z){
            
        Vec2f newV0;
        Vec2f newV1;
        Vec2f newV2;
        Vec2f tempX;

        Vec3f tempHit = ray.p + ray.dir*t;

        if( (norm.x) >=  (norm.y) &&  (norm.x) >=  (norm.z)){
            newV0 = Vec2f(v0.y, v0.z);
            newV1 = Vec2f(v1.y, v1.z);
            newV2 = Vec2f(v2.y, v2.z);
            tempX = Vec2f(tempHit.y, tempHit.z);
        }

        else if( (norm.y) >=  (norm.z) &&  (norm.y) >=  (norm.x)){
            newV0 = Vec2f(v0.x, v0.z);
            newV1 = Vec2f(v1.x, v1.z);
            newV2 = Vec2f(v2.x, v2.z);
            tempX = Vec2f(tempHit.x, tempHit.z);
        }

        else if( (norm.z) >=  (norm.y) &&  (norm.z) >=  (norm.x)){
            newV0 = Vec2f(v0.x, v0.y);
            newV1 = Vec2f(v1.x, v1.y);
            newV2 = Vec2f(v2.x, v2.y);
            tempX = Vec2f(tempHit.x, tempHit.y);
        }

        else{
            printf("missed everythign\n");
        }

        float area0 = (newV2 - newV1).Cross(tempX - newV1);
        float area1 = (newV0 - newV2).Cross(tempX - newV2);
        float area2 = (newV1 - newV0).Cross(tempX - newV0);

  
        bool insideTriangle = false;
        if(area0 >= 0.0 && area1 >= 0.0 && area2 >= 0.0)
        {
            insideTriangle = true;
        }
        
        if(area0 <= 0.0 && area1 <= 0.0 && area2 <= 0.0){
            insideTriangle = true;
        }

        if( insideTriangle == true){
            float areaFull = area0 + area1 + area2;
            TriMesh::TriFace normFace = FN(faceID);

            Vec3f norm0 = VN(normFace.v[0]);
            Vec3f norm1 = VN(normFace.v[1]);
            Vec3f norm2 = VN(normFace.v[2]);
            
            float b0 = area0/areaFull;
            float b1 = area1/areaFull;
            float b2 = 1 - (b0+b1);

            // hInfo.N = Normalize(norm);
            hInfo.N = (b0*norm0 + b1*norm1 + b2*norm2);

            // printf("Bari cords = %f\n", b0 + b1 + b2);
            hInfo.z = t;
            hInfo.p = ray.p + ray.dir*t;


            return true;
        }
        
    }

    return false;
    
}


bool IntersectBox(Box box, Ray const &ray){ 
    Vec3f cord1 = box.pmax;
    Vec3f cord2 = box.pmin;
    float t;
    float cords[] = {cord1.x, cord1.y, cord1.z, cord2.x, cord2.y, cord2.z};
    // for(int i = 0; i < 6; i++){
    float t1 = -((ray.p.x + cord1.x)/ray.dir.x);
    if(t1 > 0.0001){
        return true;
    }

    float t2 = -((ray.p.x + cord2.x)/ray.dir.x);
    if(t2 > 0.0001){
        return true;
    }
   
    float t3 = -((ray.p.y + cord1.y)/ray.dir.y);
    if(t3 > 0.0001){
        return true;
    }

    float t4 = -((ray.p.y + cord2.y)/ray.dir.y);
    if(t4 > 0.0001){
        return true;
    }
    
    float t5 = -((ray.p.z + cord1.z)/ray.dir.z);
    if(t5 > 0.0001){
        return true;
    }

    float t6 = -((ray.p.z + cord2.z)/ray.dir.z);
    if(t6 > 0.0001){
        return true;
    }

    return false;
}