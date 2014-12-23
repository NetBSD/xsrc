#ifndef _FRAMEWORK_H_
#define _FRAMEWORK_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"

extern void InitFramework (int *argc, char *argv[]);

extern void InitScene (void);
extern void RenderScene (void);

extern GLboolean CheckObjectStatus (GLhandleARB);

#endif

