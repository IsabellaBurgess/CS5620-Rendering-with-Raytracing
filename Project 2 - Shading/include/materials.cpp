#include "headerFiles/materials.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

using namespace cy;

extern MaterialList matList; 
extern Camera cam;

Color MtlBlinn::Shade(ShadeInfo const &shadeInfo) const
{
	Vec3f const camera = shadeInfo.V();
    Vec3f const hitPos = shadeInfo.P();
    Vec3f const norm = shadeInfo.N();

    float gloss = this->Glossiness();
    Vec3f lightDir = Vec3f(0,0,0 );
    // float lightIntensity = 1.0;

    Color ambientColor = Color(0, 0, 0);
    Color blinnColor = Color(0,0,0);
    
	Color baseColor = this->Diffuse();
    Color reflectColor = this->Specular();
    


    for(int i = 0; i < shadeInfo.NumLights(); i++){
        const Light* currentLight = shadeInfo.GetLight(i);
        Color intensity = currentLight->Illuminate(shadeInfo, lightDir);

        // printf("Current light is %s\n", currentLight->GetName());
        if(currentLight->IsAmbient()){
            ambientColor += intensity*baseColor;
        }


        else{
            

            Vec3f n = norm;
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

Color MtlPhong::Shade(ShadeInfo const &shadeInfo ) const
{
    return Color();
}

Color MtlMicrofacet::Shade(ShadeInfo const &shadeInfo ) const
{
    return Color();
}