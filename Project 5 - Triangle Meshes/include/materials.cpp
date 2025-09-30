#include "headerFiles/materials.h"
#include "headerFiles/renderer.h"
#include "headerFiles/ray.h"



#include "cy/cyMatrix.h"
#include "cy/cyVector.h"
#include "headerFiles/scene.h"
#include "headerFiles/shadeInf.h"


using namespace cy;

extern MaterialList matList; 
extern Camera cam;

extern calculateRay rayCalculation; 
 



Color MtlBlinn::Shade(ShadeInfo const &shadeInfo) const
{
	Vec3f const camera = shadeInfo.V();
    Vec3f const hitPos = shadeInfo.P();
    Vec3f const norm = shadeInfo.N();

    float gloss = this->Glossiness();
    Vec3f lightDir = Vec3f(0,0,0 );
    // float lightIntensity = 1.0;

    Color ambientColor = Color(0.0, 0.0, 0.0);
    Color blinnColor = Color(0.0,0.0,0.0);
    
    Color absorbValue = this->absorption;

	Color baseColor = this->Diffuse();
    Color reflectColor = this->Specular();

    
    Color reflectValue = this->reflection;
    Color refractValue = this->refraction;

    Color reflection = Color(0, 0, 0);
    Color refraction = Color(0, 0, 0);

  if(refractValue != Color(0, 0, 0) && shadeInfo.CanBounce()){
        // printf("glass!\n");
        float ior = this->IOR();
        float eta = 1/ior;

        if(!shadeInfo.IsFront())
        {
            eta = ior/1;
        }

        float refractCosThetaO = pow((camera % norm),2.0);
        float refractCosThetaTSquared = 1.0 - (pow(eta, 2.0)*(1.0-(refractCosThetaO)));

        if(refractCosThetaTSquared <= 0.0f){
            reflectValue.SetWhite();
        }

        else{
            Vec3f refractDir;
            float refractCosThetaT = sqrt(refractCosThetaTSquared);

            refractDir = -eta * camera - (sqrt(refractCosThetaTSquared)- eta*(camera % norm))*norm;
                // printf("front hit 1\n");

            Ray refractRay; 
            refractRay.dir = refractDir ;

            refractRay.p = hitPos ;
            
            HitInfo refractHit = HitInfo();
            refractHit.Init(); 
            
            float dist = BIGFLOAT;

            refraction = shadeInfo.TraceSecondaryRay(refractRay, dist);

            
            // if(!shadeInfo.IsFront()){
            //     refraction.r = refraction.r*exp(-absorbValue.r*dist);
            //     refraction.g = refraction.g*exp(-absorbValue.g*dist);
            //     refraction.b = refraction.b*exp(-absorbValue.b*dist);

            // }

            refraction = refraction *refractValue;
        }

    }
    
    if(reflectValue != Color(0, 0, 0) && shadeInfo.CanBounce())
    {


        Vec3f reflectDir = 2.0*(norm%camera)*norm - camera;

        Ray reflectRay;
        reflectRay.dir = reflectDir;

        reflectRay.p = hitPos + (reflectRay.dir * 0.01);

        HitInfo reflectHit = HitInfo();
        reflectHit.Init();

        //create a shootSecondaryRay method. pass in the bounce number, subtract 1 each time. 
        //thats it thats the only difference. check that bounce number is > 0 
                    float dist = BIGFLOAT;

        reflection = shadeInfo.TraceSecondaryRay(reflectRay, dist) * reflectValue;
        // return reflectColor;
    }

    for(int i = 0; i < shadeInfo.NumLights(); i++){
        const Light* currentLight = shadeInfo.GetLight(i);
        // printf("light number is %s\n", currentLight->GetName());

        Color intensity = currentLight->Illuminate(shadeInfo, lightDir);

        if(currentLight->IsAmbient()){
            ambientColor += intensity*baseColor;
        }


        else{
            // printf("Current light is %s\n", currentLight->GetName());

            lightDir = Normalize(lightDir);

            Vec3f n = shadeInfo.N();
            Vec3f h = Normalize(lightDir+camera);
            // printf("Normals are [%f, %f, %f]\n", n.x, n.y, n.z);
            // printf("Object name is %s\n", hInfo.node->GetName());
            
                
            float cosPhi = Max((n % h), 0.0f);

            float cosTheta = Max((n % lightDir), 0.0f);
                
            Color specularLight =  reflectColor*pow(cosPhi, gloss);
            Color difuseLight = (cosTheta*baseColor);
            blinnColor += (intensity*(difuseLight + specularLight));


        }
    }
    // printf("Blinn color = [%f, %f, %f]\n", blinnColor.r, blinnColor.g, blinnColor.b);

    return blinnColor + ambientColor + reflection + refraction;
}

Color MtlPhong::Shade(ShadeInfo const &shadeInfo ) const
{
    return Color();
}

Color MtlMicrofacet::Shade(ShadeInfo const &shadeInfo ) const
{
    return Color();
}