#include "headerFiles/objects.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

using namespace cy;

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
    
    if (t > 0.001){
        
        if(t < hInfo.z){
            hInfo.z = t;
            hInfo.p = ray.p + ray.dir*t;

            hInfo.N = Vec3f(0, 0, 1);

            // printf("face hit %d", hInfo.front);
            return true;
        }
    }

    return false;
}

bool TriObj::IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide ) const{
    const Object *currentObj = hInfo.node->GetNodeObj();
    Box boundBox = currentObj->GetBoundBox();

    // if(IntersectBox(boundBox, ray)){
        for(int i = 0; i < this->nf; i++) {
            this->IntersectTriangle(ray, hInfo, hitSide, i);
                return true;
        }

    // }
    return false;
}

bool TriObj::IntersectTriangle( Ray const &ray, HitInfo &hInfo, int hitSide, unsigned int faceID ) const{
    
    TriMesh::TriFace currentFace = F(faceID);

    Vec3f v0 = V(currentFace.v[0]);
    Vec3f v1 = V(currentFace.v[1]);
    Vec3f v2 = V(currentFace.v[2]);

    Vec3f e1 = v1 - v0;
    Vec3f e2 = v2 - v0;

    Vec3f norm = e1.Cross(e2);

    float h = -v0 % norm;

    float t = -((ray.p % norm) + h)/(ray.dir % norm);

    if (t > 0.001){
            
        Vec2f newV0;
        Vec2f newV1;
        Vec2f newV2;
        Vec2f tempX;

        Vec3f tempHit = ray.p + ray.dir*t;


        if(abs(norm.x) > abs(norm.y) && abs(norm.x) > abs(norm.z)){
            newV0 = Vec2f(v0.y, v0.z);
            newV1 = Vec2f(v1.y, v1.z);
            newV2 = Vec2f(v2.y, v2.z);
            tempX = Vec2f(tempHit.y, tempHit.z);
        }

        else if(abs(norm.y) > abs(norm.z) && abs(norm.y) > abs(norm.z)){
            newV0 = Vec2f(v0.x, v0.z);
            newV1 = Vec2f(v1.x, v1.z);
            newV2 = Vec2f(v2.x, v2.z);
            tempX = Vec2f(tempHit.x, tempHit.z);
        }

        else if(abs(norm.z) > abs(norm.y) && abs(norm.z) > abs(norm.y)){
            newV0 = Vec2f(v0.x, v0.y);
            newV1 = Vec2f(v1.x, v1.y);
            newV2 = Vec2f(v2.x, v2.y);
            tempX = Vec2f(tempHit.x, tempHit.y);
        }

        float area0 = (newV2 - newV1).Cross(tempX - newV1);
        float area1 = (newV0 - newV2).Cross(tempX - newV2);
        float area2 = (newV1 - newV0).Cross(tempX - newV0);

        if(area0 >= 0.0 && area1 >= 0.0 && area2 >= 0.0)
        {
            if(t < hInfo.z){
                hInfo.z = t;
                hInfo.p = ray.p + ray.dir*t;

                hInfo.N = norm;

                // printf("face hit %d", hInfo.front);
                return true;
            }
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