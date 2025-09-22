#include "headerFiles/materials.h"
#include "headerFiles/ray.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

using namespace cy;

extern MaterialList matList; 
extern Camera cam;
extern calculateRay rayCalculation; 
extern int initBounceNum;


Color MtlBlinn::Shade(Ray const &ray, HitInfo const &hInfo, LightList const &lights, int bounceNum) const
{
	Vec3f camera = -ray.dir.GetNormalized();

    float gloss = this->Glossiness();
    Vec3f lightDir;
    // float lightIntensity = 1.0;

    Color ambientColor = Color(0, 0, 0);
    Color blinnColor = Color(0,0,0);
    
	Color baseColor = this->Diffuse();
    Color specularColor = this->Specular();

    Color reflectValue = this->reflection;
    Color refractValue = this->refraction;

    Color absorbValue = this->absorption;
    // bool hitFront;
        // Color specularColor = Color(1,1,1);
    Vec3f n = hInfo.N;

    // if(ray.dir%hInfo.N > 0.0){
    //     n = (-hInfo.N);
    //     hitFront = false;
    //     printf("back hit 1\n");
    // }

    // else{
    //     hitFront = true;
    // }

    Color reflection = Color(0, 0, 0);
    Color refraction = Color(0, 0, 0);

    
    //implement bounce hit!!!
    if(refractValue != Color(0, 0, 0) && bounceNum > 0 ){
        bounceNum--;
        // printf("glass!\n");
        float ior = this->IOR();
        float eta = 1/ior;

        if(hInfo.front == false)
        {
            eta = ior/1;
        }

        float refractCosThetaO = pow((camera % n),2.0);
        float refractCosThetaTSquared = 1.0 - (pow(eta, 2.0)*(1.0-(refractCosThetaO)));

        if(refractCosThetaTSquared <= 0.0f){
            reflectValue.SetWhite();
        }

        else{
            Vec3f refractDir;
            float refractCosThetaT = sqrt(refractCosThetaTSquared);

            refractDir = -eta * camera - (sqrt(refractCosThetaTSquared)- eta*(camera % n))*n;
                // printf("front hit 1\n");

            Ray refractRay; 
            refractRay.dir = refractDir ;

            refractRay.p = hInfo.p ;
            
            HitInfo refractHit = HitInfo();
            refractHit.Init(); 
            

            refraction = rayCalculation.shootRay(refractRay, refractHit, bounceNum) * refractValue;

            // if(!hInfo.front){
            //     refraction.r = refraction.r*exp(-absorbValue.r*hInfo.z);
            //     refraction.g = refraction.g*exp(-absorbValue.g*hInfo.z);
            //     refraction.b = refraction.b*exp(-absorbValue.b*hInfo.z);

            // }
        }

    }

      if(reflectValue != Color(0, 0, 0) && bounceNum > 0)
    {    bounceNum--;

        if(hInfo.front == false){

        }
        Vec3f reflectDir = 2.0*(n%camera)*n - camera;

        Ray reflectRay;
        reflectRay.dir = reflectDir;

    
        if(bounceNum == initBounceNum - 1){
            reflectRay.p = hInfo.p + (reflectRay.dir);

        }

        else{
            reflectRay.p = hInfo.p + (reflectRay.dir * 0.01);

        }

        HitInfo reflectHit = HitInfo();
        reflectHit.Init();

        //create a shootSecondaryRay method. pass in the bounce number, subtract 1 each time. 
        //thats it thats the only difference. check that bounce number is > 0 
        
        reflection = rayCalculation.shootRay(reflectRay, reflectHit, bounceNum) * reflectValue ;
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