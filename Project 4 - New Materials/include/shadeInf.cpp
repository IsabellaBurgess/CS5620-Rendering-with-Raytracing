// Isabella Burgess - u1408202

#define GLUT_DISABLE_ATEXIT_HACK

#include <thread>

#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "cy/cyGL.h"
#include "cy/cyMatrix.h"
#include "headerFiles/shadeInf.h"
#include "headerFiles/ray.h"


using namespace cy;
using namespace std;

extern calculateRay rayCalculation; 

extern Node rootNode; 


