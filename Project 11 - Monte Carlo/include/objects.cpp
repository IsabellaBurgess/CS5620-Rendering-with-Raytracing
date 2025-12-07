#include "headerFiles/objects.h"
#include "headerFiles/xmlload.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"
#include "cy/cyTriMesh.h"
#include "cy/cyBVH.h"
#include <cfloat>

using namespace cy;
using namespace std;

float closest = __FLT_MAX__;

Vec3f vecMin(const Vec3f& vec1, const Vec3f& vec2);
Vec3f vecMax(const Vec3f& vec1, const Vec3f& vec2);

BVHTriMesh bvhTree;

#define FAST_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FAST_MAX(a, b) ((a) > (b) ? (a) : (b))


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


                float u = (atan2(hInfo.p.y, hInfo.p.x)/(2*Pi<float>())) + (1/2);
                float v = (asin(hInfo.p.z)/(Pi<float>())) + (1/2);
                hInfo.uvw = Vec3f(u, v, 0);

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

                float u = (atan2(hInfo.p.y, hInfo.p.x)/(2*Pi<float>())) + (1/2);
                float v = (asin(hInfo.p.z)/(Pi<float>())) + (1/2);

                hInfo.uvw = Vec3f(u, v, 0);
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

                        
            if(ray.dir.z > 0 ){

                hInfo.front = false;
                // hInfo.N = -hInfo.N;
            }


            else{
                hInfo.front = true;
            }

            hInfo.z = t;
            hInfo.p = ray.p + ray.dir*t;

             
            Vec2f uv = (Vec2f(hInfo.p) + Vec2f(1, 1))/2;
            hInfo.uvw = Vec3f(uv, 0);

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
    bvhTree = this->bvh;
    
    bool hitFound = false;

    

    if(TraceBVHNode(ray, hInfo, hitSide, bvhTree.GetRootNodeID()))
    {
            // printf("traced node\n");

        return true;
            }
    

    // if(boundBox.IntersectRay(ray, __FLT_MAX__)){
    //     // printf("in box\n");
        // for(int i = 0; i < this->nf; i++) {
        //     // printf("face number is %d\n", i);
        //     if(this->IntersectTriangle(ray, hInfo, hitSide, i)){
        //         hitFound=true;
        //     }
        // }
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
    
    if (t > 0.001){
        if(t < hInfo.z){
            
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
        if(area0 > 0.0 && area1 > 0.0 && area2 > 0.0)
        {
            insideTriangle = true;
        }
        
        if(area0 < 0.0 && area1 < 0.0 && area2 < 0.0){
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

            hInfo.N = GetNormal(faceID, Vec3f(b0, b1, b2)).GetNormalized();

            float backCheck = hInfo.N%ray.dir;
            if(backCheck > 0 ){
                
                hInfo.front = false;
                hInfo.N = -hInfo.N;
            }

            else{
                hInfo.front = true;
            }

            // printf("Bari cords = %f\n", b0 + b1 + b2);
            hInfo.z = t;
            hInfo.p = ray.p + ray.dir*t;


            if(this->HasTextureVertices()){
                hInfo.uvw = GetTexCoord(faceID, Vec3f(b0, b1, b2));
            }
            else{
                hInfo.uvw = Vec3f(0, 0, 0);
            }

            return true;
        }
        
    }
}

    return false;
    
}

bool TriObj::TraceBVHNode( Ray const &ray, HitInfo &hInfo, int hitSide, unsigned int nodeID ) const{

    Box nodeBox = bvhTree.GetNodeBounds(nodeID);
    

    if(nodeBox.IntersectRay(ray, __FLT_MAX__) == false){

        return false;
    }

    if(bvhTree.IsLeafNode(nodeID)){
          // printf("leaf\n");
          bool hitTracker = false;
        for(int i = 0; i < bvhTree.GetNodeElementCount(nodeID); i++){

            if(IntersectTriangle(ray, hInfo, hitSide, bvhTree.GetNodeElements(nodeID)[i])){
                hitTracker = true;
                // int siblingNode = bvhTree.GetSiblingNode(nodeID);
                // if(bvhTree.GetNodeBounds(siblingNode)[2] > hInfo.z){
                //     return false;
                // }
                // else{
                //     return false;
                // }
            }
            // printf("in leaf\n");

        }
            return hitTracker; 


    }


    unsigned int childOneID, childTwoID; 

    bvhTree.GetChildNodes(nodeID, childOneID, childTwoID);

    
    // printf("child 1 = %d\n", childOneID);
    // printf("child 2 = %d\n", childTwoID);
    HitInfo hitChild1;
    hitChild1.Init();

    HitInfo hitChild2;
    hitChild2.Init();
    bool child1 = TraceBVHNode(ray, hitChild1, hitSide, childOneID);

    bool child2 = TraceBVHNode(ray, hitChild2, hitSide, childTwoID);


    if(child1 || child2){
        if(hitChild1.z < hitChild2.z){
            hInfo = hitChild1;
            // printf("returning 1\n");
            return true;
        }
        
        else{
            hInfo = hitChild2;
            // printf("returning 2\n");

            return true;
        }
    }


    
    //get node info, check if the ray hits in the box
        //if no hit, return false! easy peasy

    //if yes hit check if its a leaf
        //if yes is a leaf, run intersectTriangles on each triangle in the leaf

    //if not a leaf, recursion time! run on both children 

    return false;

}


//Method implemented with help from Devin Fink
//Ray-AABB intersection 
bool Box::IntersectRay(Ray const &r, float t_max) const{
    Vec3f boxMax = pmax;
    Vec3f boxMin = pmin;

    Vec3f dir = r.dir;
    Vec3f p = r.p;
    auto safeInv = [](float d){
        return (fabs(d) > 1e-8f) ? (1.0f/d):
        std::numeric_limits<float>::infinity();
    };

    Vec3f invR = Vec3f(safeInv(dir.x), safeInv(dir.y), safeInv(dir.z));

    Vec3f tMin = vecMin(Vec3f(invR * (boxMax - p)), Vec3f(invR * (boxMin - p)));
    Vec3f tMax = vecMax(Vec3f(invR * (boxMax - p)), Vec3f(invR * (boxMin - p)));

    float t0 = Max(tMin.x, tMin.y, tMin.z);
    float t1 = Min(tMax.x, tMax.y, tMax.z);

    return t0 <= t1 && t1 >= 0.0f;
}

// bool Box::IntersectRay(Ray const &r, float t_max) const{
//     float invDirX = -(r.dir.x);
//     float invDirY = -(r.dir.y);
//     float invDirZ = -(r.dir.z);

//     float t1 = (pmax.x - r.p.x) * invDirX;
//     float t2 = (pmin.x - r.p.x) * invDirX;
//     float t3 = (pmax.y - r.p.y) * invDirY;
//     float t4 = (pmin.y - r.p.y) * invDirY;
//     float t5 = (pmax.z - r.p.z) * invDirZ;
//     float t6 = (pmin.z - r.p.z) * invDirZ;

//     float tminX = FAST_MIN(t1, t2);
//     float tmaxX = FAST_MAX(t1, t2);
//     float tminY = FAST_MIN(t3, t4);
//     float tmaxY = FAST_MAX(t3, t4);
//     float tminZ = FAST_MIN(t5, t6);
//     float tmaxZ = FAST_MAX(t5, t6);

//     float tmin = FAST_MAX(FAST_MAX(tminX, tminY), tminZ);
//     float tmax = FAST_MIN(FAST_MIN(tmaxX, tmaxY), tmaxZ);

//     return tmax >= tmin && tmax >= 0.0f;
// }

Vec3f vecMin(const Vec3f& vec1, const Vec3f& vec2){
    return Vec3f(Min(vec1.x, vec2.x), Min(vec1.y, vec2.y), Min(vec1.z, vec2.z));
}

Vec3f vecMax(const Vec3f& vec1, const Vec3f& vec2){
    return Vec3f(Max(vec1.x, vec2.x), Max(vec1.y, vec2.y), Max(vec1.z, vec2.z));
}
