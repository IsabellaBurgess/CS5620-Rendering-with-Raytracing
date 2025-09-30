// Isabella Burgess - u1408202

#define GLUT_DISABLE_ATEXIT_HACK


#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "include/cy/cyGL.h"
#include "include/cy/cyMatrix.h"

#include "include/tinyxml2.h"
#include "include/scene.h"
#include "include/objects.h"


#include "include/lodepng.h"

using namespace std;

// Sphere theSphere;

int LoadScene( RenderScene &scene, char const *filename );
void ShowViewport( RenderScene *scene );
bool treeTraversal( Node *node, Ray ray, HitInfo &hInfo);

bool hitTracker;
float oldT;

void BeginRender( RenderScene *scene ){
    RenderImage &sceneImage = scene->renderImage;
    Color24 *pixels = scene->renderImage.GetPixels();
    float *zBuf = scene->renderImage.GetZBuffer();

    Camera cam = scene->camera;
    Node rootNode = scene->rootNode;
    
    
    float imageWidth = sceneImage.GetWidth();
    float imageHeight = sceneImage.GetHeight();

    int camOffset = 1;
    float fov = cam.fov*(M_PI/180);
    
    float wsHeight = 2.0*camOffset*(tan(fov/2));
    float wsWidth = wsHeight*(imageWidth/imageHeight);

    printf("w = %f, h = %f\n", wsWidth, wsHeight);

    //TODO - add conversion to world space

    //Values used for world space conversion 


    Vec3f wsY = Vec3f(cam.up);
    Vec3f wsZ = Vec3f(cam.dir);
    Vec3f wsX = Vec3f(wsZ.Cross(wsY));
    

    Matrix4f initTransMatrix = Matrix4f(Vec4f(wsX, 0), Vec4f(wsY,0), Vec4f(wsZ,0), Vec4f(cam.pos,1));
    Matrix4f wsTransMatrix = initTransMatrix.GetTranspose();
    // cout<< to_string();

    
    
    // Ray rays[imageWidth*imageHeight];

    for(int i = 0; i < imageWidth; i++){
        for(int j = 0; j <imageHeight; j++){
            double r = 1;
            double g = double(i)/(imageHeight - 1);
            double b = double(j)/(imageWidth - 1);


            
            Ray currentRay; 

            //Camera location 
            currentRay.p = cam.pos;


            //Generate rays from camera
            //get to the top corner of the image, move over half pixel each time
            auto rayX = -(wsWidth/2) + ((wsWidth* (i + 1/2) / imageWidth));
            auto rayY = (wsHeight/2) - ((wsHeight* (j + 1/2) / imageHeight));

            currentRay.dir.Set(rayX, rayY, camOffset); //calculate the direction 
            
            currentRay.p = Vec3f(wsTransMatrix * Vec4f(currentRay.p, 1));
            currentRay.dir = Vec3f(wsTransMatrix * Vec4f(currentRay.dir, 1));
            // Normalize(currentRay.dir);

            // printf("The Current ray pos is [%f, %f, %f] \n", currentRay.p.x, currentRay.p.y, currentRay.p.z);

            // printf("The Current ray dir is [%f, %f, %f] \n", currentRay.dir.x, currentRay.dir.y, currentRay.dir.z);


            //quadratic equation for ray intersection
            Node *child = rootNode.GetChild(0);


            hitTracker = false;

            HitInfo hitInf = HitInfo();
            hitInf.Init();
            oldT = hitInf.z;


            bool hit = treeTraversal(&rootNode, currentRay, hitInf);

            
            Color24 newColor = Color24(r*255, g*255, b*255);
            Color24 secondNewColor = Color24(currentRay.dir.x*255, currentRay.dir.y*255, currentRay.dir.z*255);
         
            Color24 color;

            // printf("%d\n", hitTracker);
            if(hit == true)
            {
                // printf("true\n");
                color = Color24(1, 200, 255);
            }

            else{
                color.SetBlack();
            }

            std::atomic numPixel = j*imageWidth + i;

            pixels[numPixel] = color;
            zBuf[numPixel] = hitInf.z;
            
            // unsigned char color[3]; 
            // pixels[numPixel].GetValue(color);
            

            // printf("The Current Pixel Color is [%d, %d, %d] \n", color[0], color[1], color[2]);
            // printf("The Current Pixel is %d \n", sceneImage.GetNumRenderedPixels());
            scene->renderImage.IncrementNumRenderPixel(1);
        }
    }
    sceneImage.ComputeZBufferImage();

    sceneImage.SaveZImage("project1ZBuffer.png");
    sceneImage.SaveImage("project1.png");
}

void StopRender (){}


bool Sphere::IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide ) const{

    //Quadratic equation info 
    //Assume sphere is at 0, 0, 0, radius = 1

        // printf("in intersect Ray\n");

    float a = ray.dir.Dot(ray.dir);
    float b = 2.0*(ray.dir.Dot(ray.p));
    float c = ((ray.p.Dot(ray.p)) - 1.0);

    // printf("a b c = %f %f %f\n", a, b, c);


    float delta = (b*b) - (4.0*(a*c));

    float t = (-b - sqrt(delta))/(2.0*a);



    // printf("delta = %f\n", delta);
    if(t < 0)
    {
        // printf("false\n");

        return false;
    }
    
    if (t >= 0)
    {    

        if(t < hInfo.z ){
            hInfo.z = t;
        }


        // printf("true\n");
        //to be added to in project 2 and expanded to include full equation
        return true; 
    
    }

    return false;
}

int main (int argc, char** argv){
    
    RenderScene scene;
    
    LoadScene(scene, "project_1_scene.xml");


    ShowViewport(&scene);

}

bool treeTraversal( Node *node, Ray ray, HitInfo &hInfo)
{


    Ray transformedRay = node->ToNodeCoords(ray);


    for(int i = 0; i < node->GetNumChild(); i++){

        Node *child = node->GetChild(i);
        treeTraversal(child, transformedRay, hInfo);
    }
    
    // printf("Parent node: %s\n", node->GetName());

    Object* currentObj = node->GetNodeObj();
    
    // printf("in hitObject\n");
    
    if(currentObj){
        // printf("true2\n");
        if(currentObj->IntersectRay(transformedRay, hInfo, 1))
        {
            hitTracker = true;
            // printf("true %d\n ", hitTracker);
        }
    }

    return hitTracker;
}

