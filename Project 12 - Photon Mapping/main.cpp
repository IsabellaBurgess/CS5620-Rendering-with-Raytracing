// Isabella Burgess - u1408202

#define GLUT_DISABLE_ATEXIT_HACK

#include <iostream>
#include <thread>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "include/cy/cyGL.h"
#include "include/cy/cyMatrix.h"
#include "include/cy/cyVector.h"
#include "include/cy/cyTriMesh.h"
#include "include/cy/cyColor.h"

#include "include/headerFiles/main.h"

#include "include/libs/tinyxml2.h"
#include "include/libs/lodepng.h"

#include "include/headerFiles/scene.h"
#include "include/headerFiles/objects.h"
#include "include/headerFiles/materials.h"
#include "include/headerFiles/lights.h"
#include "include/headerFiles/renderer.h"
#include "include/headerFiles/rng.h"
#include "include/headerFiles/photonmap.h"

#include "include/headerFiles/ray.h"
using namespace std;

Camera cam;
Color24 *pixels;
Node rootNode;
MaterialList matList; 
LightList lightList;
TexturedColor background;
TexturedColor env;


float *zBuf;
int *sampleCount;

double wsHeight;
double wsWidth;
float imageWidth;
float imageHeight;
Matrix4f wsTransMatrix;

float maxT = BIGFLOAT;
const int numThreads = 20 ;

int camOffset = 1;  
int bounceNum = 3;
int montCarloBounceNum = 1; 

int photonCount = 1000000;

int minSampleCount = 4;
int maxSampleCount = 16;
float minShadowSamples = 4;
float maxShadowSamples = 4;

int montCarloSamples = 4;
float errorThreshold = 0.01;

calculateRay rayCalculation; 
RayTracer sceneRenderer;
RNG rng;


int main (int argc, char** argv){

    sceneRenderer.LoadScene("sceneFiles/cornellBox.xml");
    ShowViewport(&sceneRenderer, false);
    return 0;
}

void RayTracer::BeginRender(){

    // printf("in begin render\n");
    scene = GetScene();
    RenderImage &sceneImage = GetRenderImage();
    pixels = sceneImage.GetPixels();
    zBuf = sceneImage.GetZBuffer();
    sampleCount = sceneImage.GetSampleCount();

    matList = scene.materials;
    lightList = scene.lights;
    env = scene.environment;
    background = scene.background;
    cam = GetCamera();

    rootNode = scene.rootNode;
    camOffset = cam.focaldist;
    
    PhotonMap *photonMap = new PhotonMap;
    PhotonMap *causticMap = new PhotonMap;


    causticMap->Resize(photonCount*lightList.size());

    photonMap->Resize(photonCount*lightList.size());
    printf("remaining photons %d\n", photonMap->RemainingSpace());

    fillPhotonMap(photonMap, causticMap);

    map = photonMap;
    caustics = causticMap;

    thread t([&]{
        Vec3f position = cam.pos;

        imageWidth = sceneImage.GetWidth();
        imageHeight = sceneImage.GetHeight();

        float fov = cam.fov*(M_PI/180);

        wsHeight = 2.0*camOffset*(tan(fov/2.0));
        wsWidth = wsHeight*(imageWidth/imageHeight);

        printf("w = %f, h = %f\n", wsWidth, wsHeight);

        //Values used for world space conversion

        Vec3f wsY = Vec3f(cam.up);
        Vec3f wsZ = Vec3f(cam.dir);
        Vec3f wsX = Vec3f(wsZ.Cross(wsY));

        Matrix3f testMat = Matrix3f(wsX, wsY, wsZ);
        testMat = testMat.GetTranspose();

        wsTransMatrix.SetColumn(0, Vec4f(wsX,0));
        wsTransMatrix.SetColumn(1, Vec4f(wsY,0));
        wsTransMatrix.SetColumn(2, Vec4f(-wsZ,0));
        wsTransMatrix.SetColumn(3, Vec4f(cam.pos,1));

        
        atomic<int> nextPixel{0}; 
        std::vector<std::thread> threads;
        threads.reserve(numThreads);

        // printf("number of photons start is %d\n", photonMap->NumPhotons());

        int pixelIndex = 0;
        auto trace = [&] 
        {
            while(true)
            {
                const int pixelIndex = nextPixel.fetch_add(1, std::memory_order_relaxed);

                if(renderImage.IsRenderDone())
                {
                    break;
                }
                
                int x = pixelIndex%(int)imageWidth;
                int y = pixelIndex/imageWidth;

                Color S1 = Color(0, 0, 0);
                Vec3f S2 = Vec3f(0.0, 0.0, 0.0);
                HitInfo hitInf = HitInfo();

                float errorR = BIGFLOAT;
                float errorG = BIGFLOAT;
                float errorB = BIGFLOAT;

                
                // initalRays(pixelIndex, wsTransMatrix);   
                int finalSampleCount = 0;
                while(errorR > errorThreshold && errorG > errorThreshold && errorB > errorThreshold)
                {
                    if(finalSampleCount > maxSampleCount){
                        break;
                    }

                    Color sample = takeSample(x, y, finalSampleCount, hitInf); 
                    S1 += sample; 
                    S2 += Vec3f(pow(sample.r, 2), pow(sample.g, 2), pow(sample.b, 2));
                    
                    finalSampleCount++;

                    if(finalSampleCount < minSampleCount){
                        continue;
                    }

                    float stanDevR = (1.0/(finalSampleCount - 1)) * (S2.x - ((pow(S1.r, 2.0))/finalSampleCount));
                    float stanDevG = (1.0/(finalSampleCount - 1)) * (S2.y - ((pow(S1.g, 2.0))/finalSampleCount));
                    float stanDevB = (1.0/(finalSampleCount - 1)) * (S2.z - ((pow(S1.b, 2.0))/finalSampleCount));

                    // Color standardDeviation = (1/(finalSampleCount - 1)) * (S2 - ((pow(S1.r, 2), pow(S1.g, 2), pow(S1.b, 2))/finalSampleCount));

                    errorR = tValues[finalSampleCount-1] * (sqrt(stanDevR)/sqrt(finalSampleCount));
                    errorG = tValues[finalSampleCount-1] * (sqrt(stanDevG)/sqrt(finalSampleCount));
                    errorB = tValues[finalSampleCount-1] * (sqrt(stanDevB)/sqrt(finalSampleCount));
                }

                int numPixel = y*imageWidth + x;

                Color finalColor =  (S1/finalSampleCount).Linear2sRGB();
                

                // if(cam.sRGB == true){
                //     finalColor = finalColor.sRGB2Linear();
                // }

                pixels[numPixel] = (Color24) finalColor;
                zBuf[numPixel] = hitInf.z;
                sampleCount[numPixel] = finalSampleCount;
                
                // printf("samples is %d\n", finalSampleCount);
                renderImage.IncrementNumRenderPixel(1);
            
            }
        };
                
        for (int t = 0; t < numThreads; t++)
        {
            threads.emplace_back(trace);
        }

        for(auto& th : threads) th.join();

        
        sceneImage.ComputeZBufferImage();
        sceneImage.ComputeSampleCountImage();
        // sceneImage.SaveSampleCountImage("sampleImage.png");
        sceneImage.SaveImage("renderedImage.png");

        // printf("number of photons left is %d\n", photonMap->RemainingSpace());
        
    });

    t.detach();


}

Color RayTracer::takeSample(int x, int y, int i, HitInfo &hitInf)
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

    float radius = sqrt(ranRad);
    float angle = 2 * 2*Pi<float>() * ranAng;

    float cartX = radius * cam.dof * cos(angle);
    float cartY = radius * cam.dof * sin(angle);

    Vec3f cartCords = Vec3f(cartX, cartY, 0.0);

    Vec3f worldSpaceCartCords =Vec3f(wsTransMatrix * (Vec4f(cartCords, 0.0) ));
    Ray currentRay;

    //Camera location
    currentRay.p = cam.pos + worldSpaceCartCords; 

    //Generate rays from camera
    //get to the top corner of the image, move over half pixel each time
    auto rayX = -(wsWidth/2) + ((wsWidth* (x + 1/2 + ranX) / imageWidth)) - cartX;
    auto rayY = (wsHeight/2) - ((wsHeight* (y + 1/2 + ranY) / imageHeight)) - cartY;

    
    currentRay.dir.Set(rayX, rayY, -camOffset); //calculate the direction

    // currentRay.p = Vec3f(wsTransMatrix * Vec4f(currentRay.p, 0.0));
    currentRay.dir = Vec3f(wsTransMatrix * (Vec4f(currentRay.dir, 0.0) ));

    // currentRay.dir = currentRay.dir - worldSpaceCartCords;
    currentRay.dir.Normalize();

    hitInf.Init();
    hitInf.node = &rootNode;
    
    return rayCalculation.shootRay(x, y, currentRay, hitInf, i);
}

void RayTracer::fillPhotonMap(PhotonMap* map, PhotonMap* caustics){
    bool inPhotonMap = true;
    while(inPhotonMap){
        for(int j = 0; j < lightList.size(); j++){
            Light* currentLight = lightList[j];

            if(!currentLight->IsPhotonSource()){
                continue; 
            }

            Ray currentRay;
            Color currentColor = Color(1, 1, 1);
            currentLight->RandomPhoton(rng, currentRay, currentColor);

            HitInfo hitInf = HitInfo();

            hitInf.Init();
            hitInf.node = &rootNode;

            
            // printf("ray position: [%f, %f, %f]\n", currentRay.dir.x, currentRay.dir.y, currentRay.dir.z);
            if(rayCalculation.TraceRay(currentRay, hitInf, HIT_FRONT_AND_BACK)){

                ShadeInfo shadeInf(lightList, env, rng);
                shadeInf.SetHit(currentRay, hitInf);

                DirSampler::Info info;
                info.SetVoid();
                

                if(map->RemainingSpace() == 0){
                    inPhotonMap = false;
                    break;
                }
                if(hitInf.node->GetMaterial()->IsPhotonSurface()){
                    map->AddPhoton(hitInf.p, currentRay.dir, currentColor);
                }

                if(hitInf.node->GetMaterial()->GenerateSample(shadeInf, currentRay.dir, info)){
                    currentRay.p = hitInf.p;
                    // printf("color is [%f, %f, %f]\n", info.mult.r, info.mult.g, info.mult.b);
                    bouncePhoton(map, caustics, info, currentRay, currentColor);
                }                    
            
            } 
        }
    }

    map->ScalePhotonPowers((float) (M_PI/(photonCount*lightList.size())));
    caustics->ScalePhotonPowers((float) (M_PI/(caustics->NumPhotons()*lightList.size())));

    map->PrepareForIrradianceEstimation();
    caustics->PrepareForIrradianceEstimation();
}

void RayTracer::bouncePhoton(PhotonMap* map, PhotonMap* caustics, DirSampler::Info &si, Ray ray, Color power){
    HitInfo hitInf = HitInfo();

    hitInf.Init();
    hitInf.node = &rootNode;

    if(rayCalculation.TraceRay(ray, hitInf, hitInf.front)){
                        // printf("ray position after: [%f, %f, %f]\n", hitInf.p.x, hitInf.p.y, hitInf.p.z);

        ShadeInfo shadeInf(lightList, env, rng);
        shadeInf.SetHit(ray, hitInf);

        if(map->RemainingSpace() == 0){
            return;
        }
        
        DirSampler::Lobe prevLobe = si.lobe;

        Vec3f newDir = Vec3f(0.0, 0.0, 0.0);
            // printf("ray position: [%f, %f, %f]\n", newDir.x, newDir.y, newDir.z);

        // printf("color is [%f, %f, %f]\n", si.mult.r, si.mult.g, si.mult.b);
        if(hitInf.node->GetMaterial()->GenerateSample(shadeInf, newDir, si)){
            Ray newRay = Ray(hitInf.p, newDir);
            Color newPower = power*(si.mult);


            if(si.lobe == DirSampler::Lobe::DIFFUSE){
                if(prevLobe == DirSampler::Lobe::TRANSMISSION|| prevLobe == DirSampler::Lobe::SPECULAR){
                    // return;
                    caustics->AddPhoton(hitInf.p, newRay.dir, newPower);
                }
                else{
                    map->AddPhoton(hitInf.p, newRay.dir, newPower);

                }

                    
            }

            bouncePhoton(map, caustics, si, newRay, newPower);
        }

    }

    return;

}


void StopRender(){}