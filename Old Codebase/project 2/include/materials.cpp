#include "materials.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

using namespace cy;

extern MaterialList matList; 
extern Camera cam;

Color MtlBlinn::Shade(Ray const &ray, HitInfo const &hInfo, LightList const &lights) const
{
	Vec3f camera = -ray.dir;


    float gloss = this->Glossiness();
    Vec3f lightDir;
    // float lightIntensity = 1.0;

    Color ambientColor = Color(0, 0, 0);
    Color blinnColor = Color(0,0,0);
    
	Color baseColor = this->Diffuse();
    Color reflectColor = this->Specular();
    


    for(int i = 0; i < lights.size(); i++){
        Light* currentLight = lights.at(i);
        Color intensity = currentLight->Illuminate(hInfo.p, hInfo.N);

        // printf("Current light is %s\n", currentLight->GetName());
        if(currentLight->IsAmbient()){
            ambientColor += intensity*baseColor;
        }


        else{
            lightDir = -currentLight->Direction(hInfo.p);

            Vec3f n = hInfo.N;
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

    return blinnColor + ambientColor;
}

Color MtlPhong::Shade(Ray const &ray, HitInfo const &hInfo, LightList const &lights) const
{
    return Color();
}

Color MtlGGX::Shade(Ray const &ray, HitInfo const &hInfo, LightList const &lights) const
{
    return Color();
}