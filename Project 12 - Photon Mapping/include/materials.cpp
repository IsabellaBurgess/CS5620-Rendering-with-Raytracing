#include "headerFiles/materials.h"
#include "headerFiles/renderer.h"
#include "headerFiles/ray.h"



#include "cy/cyMatrix.h"
#include "cy/cyVector.h"
#include "headerFiles/scene.h"
#include "headerFiles/shadeInf.h"
#include "headerFiles/photonmap.h"
#include "headerFiles/main.h"


using namespace cy;

extern MaterialList matList; 
extern Camera cam;

extern calculateRay rayCalculation; 
extern RayTracer sceneRenderer;
extern RNG rng;
extern int montCarloBounceNum; 
extern int montCarloSamples;
extern int photonCount;

extern PhotonMap *photonMap;

 



Color MtlBlinn::Shade(ShadeInfo const &shadeInfo) const
{
	Vec3f const camera = shadeInfo.V();
    Vec3f const hitPos = shadeInfo.P();
    Vec3f const norm = shadeInfo.N();

    float gloss = Glossiness().GetValue();
    Vec3f lightDir = Vec3f(0,0,0 );
    // float lightIntensity = 1.0;

    Color ambientColor = Color(0.0, 0.0, 0.0);
    Color blinnColor = Color(0.0,0.0,0.0);
    Color diffuseLight = Color(0.0, 0.0, 0.0);
    Color specularLight = Color(0.0, 0.0, 0.0);
    
    
    Color absorbValue = this->absorption;

    //this!! this is what needs to get updated and changed
    // TexturedColor baseTex = this->Diffuse();
	Color baseColor = Diffuse().Eval(shadeInfo.UVW());
    Color reflectColor = Specular().Eval(shadeInfo.UVW());

    
    Color reflectValue = this->reflection.GetValue();
    Color refractValue = this->refraction.GetValue();
    
    Color reflection = Color(0, 0, 0);
    Color refraction = Color(0, 0, 0);

    
    Color irradianceColor = Color(0, 0, 0);
    Vec3f photonDir = Vec3f(0, 0, 0);
    sceneRenderer.map->EstimateIrradiance<50, PHOTONMAP_FILTER_CONSTANT>(irradianceColor, photonDir, 0.1f, hitPos, norm, 1.0f);

    


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
            float phiOffset = Halton(shadeInfo.CurrentPixelSample(), 2) + rng.RandomFloat();
            float thetaOffset = Halton(shadeInfo.CurrentPixelSample(), 3) + rng.RandomFloat();

            Vec3f u = Vec3f(0, 0, 0); 
            Vec3f v = Vec3f(0, 0, 0);

            norm.GetOrthonormals(u, v);

            if(phiOffset > 1.0){
                phiOffset = phiOffset - 1;
            }

            if(thetaOffset > 1.0){
                thetaOffset = thetaOffset - 1;
            }

            float phi = 2.0*Pi<float>() * phiOffset;
            float cosTheta = pow(1 - thetaOffset, 1.0/(glossiness.GetValue() + 1.0));

            float sinTheta = sqrt(1.0 - pow(cosTheta, 2.0));

            Vec3f localH = Vec3f(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);  
            Vec3f worldH = (localH.x * u) + (localH.y * v) + (localH.z * norm);

            Vec3f refractDir;

            refractDir = -eta * camera - (sqrt(refractCosThetaTSquared)- eta*(camera % worldH))*worldH;
                // printf("front hit 1\n");
 
            Ray refractRay; 
            refractRay.dir = refractDir ;

            refractRay.p = hitPos ;
            
            HitInfo refractHit = HitInfo();
            refractHit.Init(); 
            
            float dist = BIGFLOAT;

            refraction = shadeInfo.TraceSecondaryRay(refractRay, dist);

            
            if(!shadeInfo.IsFront()){
                refraction.r = refraction.r*exp(-absorbValue.r*dist);
                refraction.g = refraction.g*exp(-absorbValue.g*dist);
                refraction.b = refraction.b*exp(-absorbValue.b*dist);

            }

            //change refraction to the current texture point
            refraction = refraction * refractValue;
        }

    }
    
    if(reflectValue != Color(0, 0, 0) && shadeInfo.CanBounce())
    { 
        float phiOffset = Halton(shadeInfo.CurrentPixelSample(), 2) + rng.RandomFloat();
        float thetaOffset = Halton(shadeInfo.CurrentPixelSample(), 3) + rng.RandomFloat();

        Vec3f u = Vec3f(0, 0, 0); 
        Vec3f v = Vec3f(0, 0, 0);

        norm.GetOrthonormals(u, v);

        if(phiOffset > 1.0){
            phiOffset = phiOffset - 1;
        }

        if(thetaOffset > 1.0){
            thetaOffset = thetaOffset - 1;
        }

        float phi = 2.0*Pi<float>() * phiOffset;
        float cosTheta = pow(1 - thetaOffset, 1.0/(glossiness.GetValue() + 1.0));

        float sinTheta = sqrt(1.0 - pow(cosTheta, 2.0));

        Vec3f localH = Vec3f(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);  
        Vec3f worldH = (localH.x * u) + (localH.y * v) + (localH.z * norm);

        // Vec3f h = (norm * cosTheta) + (u )

        Vec3f reflectDir = 2.0*(worldH%camera)*worldH - camera;

        Ray reflectRay;
        reflectRay.dir = reflectDir;

        reflectRay.p = hitPos;

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

        // if(shadeInfo.CurrentBounce() < montCarloBounceNum){
            // for(int j = 0; j < montCarloSamples; j++){
            //     float phiOffset = Halton(shadeInfo.CurrentPixelSample(), 2) + rng.RandomFloat();
            //     float thetaOffset = Halton(shadeInfo.CurrentPixelSample(), 3) + rng.RandomFloat();

            //     Vec3f u = Vec3f(0, 0, 0); 
            //     Vec3f v = Vec3f(0, 0, 0);

            //     norm.GetOrthonormals(u, v);

            //     if(phiOffset > 1.0){
            //         phiOffset = phiOffset - 1;
            //     }

            //     if(thetaOffset > 1.0){
            //         thetaOffset = thetaOffset - 1.0;
            //     }

            //     float phi = (2.0*Pi<float>() * phiOffset);
            //     float cosTheta =  sqrt(thetaOffset);

            //     float sinTheta = sqrt(1.0 - (cosTheta * cosTheta));

            //     Vec3f localDir = Vec3f(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);  
            //     Vec3f worldDir = (localDir.x * u) + (localDir.y * v) + (localDir.z * norm);

            //     Ray montCarloRay = Ray(hitPos, worldDir);

            //     float dist = BIGFLOAT;

            //     ambientColor += (shadeInfo.TraceSecondaryRay(montCarloRay, dist))*baseColor ; 
            // }

            // ambientColor = (ambientColor/montCarloSamples);
        // }
        if(currentLight->IsAmbient()){
            ambientColor += intensity * baseColor;
        }

            // printf("Current light is %s\n", currentLight->GetName());

        lightDir = Normalize(lightDir);

        Vec3f n = shadeInfo.N();
        Vec3f h = Normalize(lightDir+camera);
        // printf("Normals are [%f, %f, %f]\n", n.x, n.y, n.z);
        // printf(Object name is %s\n", hInfo.node->GetName());    
            
        float cosPhi = Max((n % h), 0.0f);

        float cosTheta = Max((n % lightDir), 0.0f);
        
            
        specularLight =  (reflectColor * pow(cosPhi, gloss)) * ((gloss+2)/(8*Pi<float>()));
        diffuseLight = ( (cosTheta * baseColor) * (1/Pi<float>()));

        blinnColor += intensity * (diffuseLight + specularLight);

        
    }

    // printf("irradiance Color [%f, %f, %f]\n", irradianceColor.r, irradianceColor.g, irradianceColor.b);
    // blinnColor =  blinnColor ;
    
    // printf("Blinn color = [%f, %f, %f]\n", blinnColor.r, blinnColor.g, blinnColor.b);

    Color indirectColor = (shadeInfo.Eval(diffuse)) * (( irradianceColor/M_PI)) ;
    return (blinnColor  + reflection + refraction + indirectColor + emission.GetValue()) ;
    // return ambientColor + emission.GetValue();
}

Color MtlPhong::Shade(ShadeInfo const &shadeInfo ) const
{
    return Color();
}

Color MtlMicrofacet::Shade(ShadeInfo const &shadeInfo ) const
{
    return Color();
}

bool MtlPhong::GenerateSample( SamplerInfo const &sInfo, Vec3f &dir, Info &si ) const{
   return false;
}

bool MtlMicrofacet::GenerateSample( SamplerInfo const &sInfo, Vec3f &dir, Info &si ) const{
   return false;
}

bool MtlBlinn::GenerateSample( SamplerInfo const &sInfo, Vec3f &dir, Info &si ) const{
    Vec3f const camera = sInfo.V();
    Vec3f const hitPos = sInfo.P();
    Vec3f const norm = sInfo.N();

    float gloss = Glossiness().GetValue();
    Vec3f lightDir = Vec3f(0,0,0 );
    // float lightIntensity = 1.0;

    Color blinnColor = Color(0.0,0.0,0.0);
    
    Color absorbValue = this->absorption;

    //this!! this is what needs to get updated and changed
    // TexturedColor baseTex = this->Diffuse();
	Color baseColor = this->Diffuse().Eval(sInfo.UVW());
    Color reflectColor = this->Specular().Eval(sInfo.UVW());

    Color reflectValue = this->reflection.GetValue();
    Color refractValue = this->refraction.GetValue();

    float ranNum = rng.RandomFloat();

    //Probablility set up with the help of Austin Kim.
    // Color diff = sInfo.Eval(diffuse);
    // Color spec = sInfo.Eval(specular);
    // Color refract = sInfo.Eval(refraction);

    // float rSum = diff.r + spec.r + refract.r;
    // float gSum = diff.g + spec.g + refract.g;
    // float bSum = diff.b + spec.b + refract.b;

    // float diffSum = diff.Sum();
    // float specSum = spec.Sum();
    // float refractSum = refract.Sum();

    // float totalSum = diffSum + specSum + refractSum;

    // float Pr = Max(rSum, gSum, bSum);

    // float diffProb = (diffSum / totalSum) * Pr; 
    // float specProb = (specSum / totalSum) * Pr;
    // float refractProb = (refractProb / totalSum) * Pr;

    // float totalProb = diffProb + specProb + refractProb;

        // printf("diffuse is %f\n", diffuseProb);
        // printf("rand is %f\n", ranNum);

    float diffProb = diffuse.GetValue().Max();
    float specProb = reflection.GetValue().Max();
    float refractProb = refraction.GetValue().Max();
    if(ranNum <= diffProb){
        // si.mult = Color(1, 0, 0);

        float phiOffset = rng.RandomFloat();
    
        float thetaOffset = rng.RandomFloat();

        Vec3f u = Vec3f(0, 0, 0); 
        Vec3f v = Vec3f(0, 0, 0);

        norm.GetOrthonormals(u, v);

        float phi = (2.0*Pi<float>() * phiOffset);
        float dirCosTheta =  thetaOffset;

        float dirSinTheta = sqrt(1.0 - (dirCosTheta * dirCosTheta));

        Vec3f localDir = Vec3f(dirSinTheta * cos(phi), dirSinTheta * sin(phi), dirCosTheta);  
        Vec3f finalDir = (localDir.x * u) + (localDir.y * v) + (localDir.z * norm);

        lightDir = Normalize(dir);

        Vec3f n = Normalize(norm);
        Vec3f h = Normalize(lightDir+camera);
        // printf("Normals are [%f, %f, %f]\n", n.x, n.y, n.z);
        // printf(Object name is %s\n", hInfo.node->GetName());    
            
        float cosPhi = Max((n % h), 0.0f);

        float cosTheta = Max((n % lightDir), 0.0f);
        
            
        // Color specularLight =  (reflectColor * pow(cosPhi, gloss)) * ((gloss+2)/(8*Pi<float>()));
        Color diffuseLight = (cosTheta * baseColor) * (1/Pi<float>());
        
        si.mult = sInfo.Eval(diffuse) / ((2*M_PI)* diffProb);
        si.prob = diffProb; 
        si.lobe = Lobe::DIFFUSE;

        dir = Normalize(finalDir);

        return true; 
    }   

    // else if(ranNum <= diffProb + specProb){
        
        // float phiOffset = rng.RandomFloat();
        // float thetaOffset = rng.RandomFloat();

        // Vec3f u = Vec3f(0, 0, 0); 
        // Vec3f v = Vec3f(0, 0, 0);

        // norm.GetOrthonormals(u, v);

        // float phi = 2.0*Pi<float>() * phiOffset;
        // float cosTheta = pow(1 - thetaOffset, 1.0/(glossiness.GetValue() + 1.0));

        // float sinTheta = sqrt(1.0 - pow(cosTheta, 2.0));

        // Vec3f localH = Vec3f(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);  
        // Vec3f worldH = (localH.x * u) + (localH.y * v) + (localH.z * norm);

        // // Vec3f h = (norm * cosTheta) + (u )

    //     Vec3f reflectDir = 2.0*(norm%camera)*norm - camera;
        
    //     si.mult = sInfo.Eval(reflection);
    //     si.prob = specProb; 
    //     si.lobe = Lobe::SPECULAR;

    //     dir = Normalize(reflectDir);

    //     return true;
    // }

    else if(ranNum <= diffProb + specProb){
        float ior = this->IOR();
        float eta = 1/ior;

        if(!sInfo.IsFront())
        {
            Vec3f norm = -norm;
            eta = ior/1;
        }

        float refractCosThetaO = pow((camera % norm),2.0);
        float refractCosThetaTSquared = 1.0 - (pow(eta, 2.0)*(1.0-(refractCosThetaO)));

            float phiOffset = rng.RandomFloat();
            float thetaOffset = rng.RandomFloat();

            Vec3f u = Vec3f(0, 0, 0); 
            Vec3f v = Vec3f(0, 0, 0);

            norm.GetOrthonormals(u, v);

            float phi = 2.0*Pi<float>() * phiOffset;
            float cosTheta = pow(1 - thetaOffset, 1.0/(glossiness.GetValue() + 1.0));

            float sinTheta = sqrt(1.0 - pow(cosTheta, 2.0));

            Vec3f localH = Vec3f(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);  
            Vec3f worldH = (localH.x * u) + (localH.y * v) + (localH.z * norm);

            Vec3f refractDir;

            refractDir = -eta * camera - (sqrt(refractCosThetaTSquared)- eta*(camera % worldH))*worldH;

            si.mult = sInfo.Eval(refraction);
            si.prob = refractProb; 
            si.lobe = Lobe::TRANSMISSION;

            dir = Normalize(refractDir);

            return true;
        
    }

    else{
        return false;
    }   
}

