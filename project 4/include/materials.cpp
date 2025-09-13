#include "headerFiles/materials.h"
#include "headerFiles/ray.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

using namespace cy;

extern MaterialList matList; 
extern Camera cam;
extern calculateRay rayCalculation; 

Color MtlBlinn::Shade(Ray const &ray, HitInfo const &hInfo, LightList const &lights, int bounceNum) const
{
	Vec3f camera = -ray.dir;

    float gloss = this->Glossiness();
    Vec3f lightDir;
    // float lightIntensity = 1.0;

    Color ambientColor = Color(0, 0, 0);
    Color blinnColor = Color(0,0,0);
    
	Color baseColor = this->Diffuse();
    Color specularColor = this->Specular();

    Color reflectValue = this->reflection;
    Color refractValue = this->refraction;


        // Color specularColor = Color(1,1,1);
    Vec3f n = hInfo.N;

    Color reflection = Color(0, 0, 0);
    Color refraction = Color(0, 0, 0);

    
    //implement bounce hit!!!
    if(refractValue != Color(0, 0, 0)){
        // printf("glass!\n");
        float ior = this->IOR();
        float eta = 1/ior;

        float refractCosThetaO = pow(Max((n % lightDir), 0.0f),2);
        float refractCosThetaT = (1 - pow(eta, 2)*(1-(refractCosThetaO)));

        if(refractCosThetaT < 0){
            reflectValue = this->refraction;
        }

        else{
            refractCosThetaT = sqrt(refractCosThetaT);

            Vec3f refractDir = -(eta * camera) - (refractCosThetaT - eta*(refractCosThetaO))*n;
            Ray refractRay; 
            refractRay.p = hInfo.p;
            refractRay.dir = refractDir;
            
            HitInfo refractHit = HitInfo();
            refractHit.Init();

            return refraction = rayCalculation.shootRay(refractRay, refractHit) ;
        }

    }

    //implement bounce hit
    if(reflectValue != Color(0, 0, 0))
    {
        Vec3f reflectDir = 2.0*(n%camera)*n - camera;

        Ray reflectRay;
        reflectRay.p = hInfo.p;
        reflectRay.dir = reflectDir;

        HitInfo reflectHit = HitInfo();
        reflectHit.Init();

        //create a shootSecondaryRay method. pass in the bounce number, subtract 1 each time. 
        //thats it thats the only difference. check that bounce number is > 0 
        reflection = rayCalculation.shootRay(reflectRay, reflectHit) * reflectValue * specularColor;
        // return reflection;
    }
  
    for(int i = 0; i < lights.size(); i++){
        Light* currentLight = lights.at(i);
        Color intensity = currentLight->Illuminate(hInfo.p, hInfo.N);

        // printf("Current light is %s\n", currentLight->GetName());
        if(currentLight->IsAmbient()){
            ambientColor += intensity*baseColor;
        }


        else{
            lightDir = -currentLight->Direction(hInfo.p);

            Vec3f h = Normalize(lightDir+camera);
            
            // printf("Normals are [%f, %f, %f]\n", n.x, n.y, n.z);
            // printf("Object name is %s\n", hInfo.node->GetName());
            
                
            float cosPhi = Max((n % h), 0.0f);

            float cosTheta = Max((n % lightDir), 0.0f);
                
            Color specularLight =  specularColor*pow(cosPhi, gloss);
            Color difuseLight = (cosTheta*baseColor);
            blinnColor += (intensity*(difuseLight + specularLight));


        }
    }
    // printf("Blinn color = [%f, %f, %f]\n", blinnColor.r, blinnColor.g, blinnColor.b);

    return blinnColor + ambientColor + reflection + refraction;
}

Color MtlPhong::Shade(Ray const &ray, HitInfo const &hInfo, LightList const &lights, int bounceNum) const
{
    return Color();
}

Color MtlMicrofacet::Shade(Ray const &ray, HitInfo const &hInfo, LightList const &lights, int bounceNum) const
{
    return Color();
}