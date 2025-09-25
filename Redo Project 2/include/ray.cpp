#include <iostream>
#include <thread>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "cy/cyGL.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

#include "headerFiles/scene.h"
#include "headerFiles/ray.h"
#include "headerFiles/objects.h"
#include "headerFiles/renderer.h"

extern double wsHeight;
extern double wsWidth;
extern float imageWidth;
extern float imageHeight;

extern Camera cam;
extern Color24 *pixels;
extern Node rootNode;
// extern MaterialList matList; 
// extern LightList lightList;

extern float *zBuf;


extern int camOffset;
extern float shadowBias;

using namespace cy;
using namespace std;


 

// Color calculateRay::shootRay(Ray const &ray, HitInfo &hInfo, int bounceNum){
//     hInfo.node = &rootNode;
//     bool hit = sceneRenderer.TraceRay(ray, hInfo, hInfo.front);         
//     Color color;
//     printf("in shoot ray\n");
    
// //     if(ray.dir%hInfo.N > 0.0){
// //         hInfo.N = -hInfo.N;
// //         hInfo.front = false;
// //         // printf("back hit %d\n", hInfo.front);
// //     }

// //     else if(ray.dir%hInfo.N <= 0.0){
// //         hInfo.front = true;
// // //        printf("front hit\n");

// //     }
//         // printf("Node name is %s\n", hitInf.node->GetName());

//     // printf("%d\n", hitTracker);
//     if(hit == true)
//     {
//         // printf("true\n");
//             // printf("hit node: %s\n", hitInf.node->GetName());
//         // const Material *currentMat = hInfo.node->GetMaterial();
//         // color = currentMat->Shade(ray, hInfo, lightList, bounceNum);
//         color = Color(1, 200, 255);
//     }

//     else{
//         // printf("False\n");
//         color.SetBlack();
//     }

//     return color;
// }

