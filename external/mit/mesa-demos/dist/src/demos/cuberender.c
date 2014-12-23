/*
 * Test rendering into a cube map texture with FBOs.
 *
 * Brian Paul
 * May 2011
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"


#define DEG_TO_RAD (3.14159 / 180.0)

#define ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

static GLint WindowWidth = 750, WindowHeight = 450;
static GLint Win;

static GLfloat Xrot = 10, Yrot = 0, Zrot = 0;

static GLfloat GroundColor[4] = {0.5, 0.5, 0.75, 1};
static GLfloat GroundY = -1.0f;

static GLfloat ViewDist = 30.0;

static GLfloat SpherePos[4] = { 0, 2.5, 0, 1 };
static GLfloat LightDist = 15;
static GLfloat LightLatitude = 30.0;
static GLfloat LightLongitude = 45.0;
static GLfloat LightPos[4];

static GLboolean Anim = GL_TRUE;

static GLboolean NeedNewCubeMap = GL_TRUE;
static GLuint CubeTexture;


struct cylinder
{
   float PosX, PosY;
   float Radius, Height;
   float Color[4];
};

#define NUM_CYLINDERS 30

static struct cylinder Cylinders[NUM_CYLINDERS];


static float
RandomFloat(float min, float max)
{
   const int k = 10000;
   float t = (rand() % k) / (float) (k - 1);  /* t in [0,1] */
   float r = min + t * (max - min);
   return r;
}


static void
CheckError(int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("GL Error 0x%x at line %d\n", (int) err, line);
   }
}


static void
GenerateCylinders(void)
{
   int i;
   for (i = 0; i < NUM_CYLINDERS; i++) {
      float r = RandomFloat(5.0, 9.0);
      float a = RandomFloat(0, 2 * M_PI);
      
      Cylinders[i].PosX = r * cos(a);
      Cylinders[i].PosY = r * sin(a);
      Cylinders[i].Radius = RandomFloat(0.25, 1.0);
      Cylinders[i].Height = RandomFloat(1.0, 7.0);
      Cylinders[i].Color[0] = RandomFloat(0.25, 1.0);
      Cylinders[i].Color[1] = RandomFloat(0.25, 1.0);
      Cylinders[i].Color[2] = RandomFloat(0.25, 1.0);
      Cylinders[i].Color[3] = RandomFloat(0.25, 1.0);
   }
}

static void
DrawCylinder(const struct cylinder *cyl)
{
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cyl->Color);
   glPushMatrix();
   glTranslatef(cyl->PosX, GroundY, cyl->PosY);
   glRotatef(-90, 1, 0, 0);
   glutSolidCone(cyl->Radius, cyl->Height, 12, 2);
   glPopMatrix();
}


static void
DrawLightSource(void)
{
   glPushMatrix();
   glTranslatef(LightPos[0], LightPos[1], LightPos[2]);
   glColor3f(1, 1, 1);
   glutSolidSphere(0.25, 8, 8);
   glPopMatrix();
}


static void
DrawScene(void)
{
   int i;

   glEnable(GL_LIGHTING);

   for (i = 0; i < NUM_CYLINDERS; i++) {
      DrawCylinder(&Cylinders[i]);
   }

   glDisable(GL_LIGHTING);

   if (1)
      DrawLightSource();

   /* ground plane */
   if (1) {
      GLfloat k = 10.0, g = GroundY;
      CheckError(__LINE__);
      glPushMatrix();
      glColor3fv(GroundColor);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, GroundColor);
      glNormal3f(0, 1, 0);
      glBegin(GL_POLYGON);
      glVertex3f(-k, g, -k);
      glVertex3f( k, g, -k);
      glVertex3f( k, g,  k);
      glVertex3f(-k, g,  k);
      glEnd();
      glPopMatrix();
   }

   CheckError(__LINE__);
}


/*
 * Draw sphere with cube map texture that reflects rest of the scene.
 */
static void
DrawShinySphere(void)
{
   glEnable(GL_TEXTURE_CUBE_MAP);

#if 0
   glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
   glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
   glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB);
#else
   glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_ARB);
   glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_ARB);
   glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_ARB);
#endif
   glEnable(GL_TEXTURE_GEN_S);
   glEnable(GL_TEXTURE_GEN_T);
   glEnable(GL_TEXTURE_GEN_R);

   glPushMatrix();
   glTranslatef(SpherePos[0], SpherePos[1], SpherePos[2]);
   glColor3f(0.75, 0.75, 0.75);

   if (1) {
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glRotatef(-Yrot, 0, 1, 0);
      glRotatef(-Xrot, 1, 0, 0);
   }

   glutSolidSphere(2.5, 16, 16);

   glMatrixMode(GL_MODELVIEW);

   glPopMatrix();

   glDisable(GL_TEXTURE_CUBE_MAP);
   glDisable(GL_TEXTURE_GEN_S);
   glDisable(GL_TEXTURE_GEN_T);
   glDisable(GL_TEXTURE_GEN_R);
}


/**
 * Setup modelview and projection for drawing a cube face.
 */
static void
SetupCubeFaceView(GLenum face, const GLfloat centerPos[4])
{
   GLfloat near_val = 0.5, far_val = 20.0;

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-near_val, near_val, -near_val, near_val, near_val, far_val);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   switch (face) {
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      glRotatef(180, 1, 0, 0);
      glRotatef(-90, 0, 1, 0);
      break;
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      glRotatef(180, 1, 0, 0);
      glRotatef(90, 0, 1, 0);
      break;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      glRotatef(-90, 1, 0, 0);
      break;
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      glRotatef(90, 1, 0, 0);
      break;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      glRotatef(180, 1, 0, 0);
      break;
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      glRotatef(0, 1, 0, 0);
      glRotatef(180, 0, 0, 1);
      break;
   }

   glTranslatef(-centerPos[0], -centerPos[1], -centerPos[2]);

   glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
}



static void
ComputeLightPos(GLfloat dist, GLfloat latitude, GLfloat longitude,
                GLfloat pos[4])
{
   pos[0] = dist * sin(longitude * DEG_TO_RAD);
   pos[1] = dist * sin(latitude * DEG_TO_RAD);
   pos[2] = dist * cos(latitude * DEG_TO_RAD) * cos(longitude * DEG_TO_RAD);
   pos[3] = 1;
}


static GLint
ChooseCubeSize(void)
{
   if (WindowWidth >= 1024 && WindowHeight >= 1024) {
      return 1024;
   }
   else if (WindowWidth >= 512 && WindowHeight >= 512) {
      return 512;
   }
   else if (WindowWidth >= 256 && WindowHeight >= 256) {
      return 256;
   }
   else {
      return 128;
   }
}


static GLuint
CreateCubeTexture(GLint size)
{
   GLboolean linearFilter = GL_TRUE;
   GLuint cube, face;

   glGenTextures(1, &cube);
   glBindTexture(GL_TEXTURE_CUBE_MAP, cube);

   /* Set the filter mode so that the texture is texture-complete.
    * Otherwise it will cause the framebuffer to fail the framebuffer
    * completeness test.
    */
   if (linearFilter) {
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   }
   else {
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   }

   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   for (face = 0; face < 6; face++) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA,
		   size, size, 0,
		   GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   }

   CheckError(__LINE__);

   return cube;
}


/**
 * Render the six faces of a cube map.
 */
static void
RenderCubeMap(void)
{
   GLuint fbo, face, depthStencilRb;
   GLint cubeSize;

   CheckError(__LINE__);

   cubeSize = ChooseCubeSize();
   printf("Rendering %d x %d cube texture\n", cubeSize, cubeSize);

   CubeTexture = CreateCubeTexture(cubeSize);

   /*
    * Make FBO.
    */
   glGenFramebuffersEXT(1, &fbo);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

   /*
    * Make depth/stencil renderbuffer.
    */
   glGenRenderbuffersEXT(1, &depthStencilRb);
   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthStencilRb);
   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL,
                            cubeSize, cubeSize);
   glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                GL_DEPTH_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT, depthStencilRb);
   glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                GL_STENCIL_ATTACHMENT_EXT,
                                GL_RENDERBUFFER_EXT, depthStencilRb);

   glViewport(0, 0, cubeSize, cubeSize);

   /*
    * Render into cube faces.
    */
   for (face = 0; face < 6; face++) {
      GLenum status;

      /* Render color into face of cubemap */
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				CubeTexture, 0);

      status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
      CheckError(__LINE__);
      if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
	 fprintf(stderr, "FBO not complete!  status = 0x%04x\n", status);
	 assert(status == GL_FRAMEBUFFER_COMPLETE_EXT);
      }

      CheckError(__LINE__);

      /* Render the depth image into cube face */
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      SetupCubeFaceView(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, SpherePos);

      DrawScene();
   }

   glDeleteRenderbuffersEXT(1, &depthStencilRb);
   glDeleteFramebuffersEXT(1, &fbo);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

   CheckError(__LINE__);
}


/**
 * Redraw window image
 */
static void
Display(void)
{
   const GLfloat ar = (GLfloat) WindowWidth / (GLfloat) WindowHeight;

   ComputeLightPos(LightDist, LightLatitude, LightLongitude, LightPos);

   if (NeedNewCubeMap) {
      /* regenerate cube map faces */
      RenderCubeMap();
      NeedNewCubeMap = GL_FALSE;
   }

   glViewport(0, 0, WindowWidth, WindowHeight);

   /* draw scene from camera's view */
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-ar, ar, -1.0, 1.0, 4.0, 50.0);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -ViewDist);
   glRotatef(Xrot, 1, 0, 0);
   glRotatef(Yrot, 0, 1, 0);
   glRotatef(Zrot, 0, 0, 1);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glLightfv(GL_LIGHT0, GL_POSITION, LightPos);

   DrawScene();
   DrawShinySphere();

   glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
   WindowWidth = width;
   WindowHeight = height;
   NeedNewCubeMap = GL_TRUE;
}


static void
Idle(void)
{
   static double t0 = -1.;
   double dt, t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
   if (t0 < 0.0)
      t0 = t;
   dt = t - t0;
   t0 = t;
   Yrot += 40.0 * dt;
   glutPostRedisplay();
}


static void
Key(unsigned char key, int x, int y)
{
   const GLfloat step = 3.0;
   (void) x;
   (void) y;
   switch (key) {
   case 'a':
      Anim = !Anim;
      if (Anim)
         glutIdleFunc(Idle);
      else
         glutIdleFunc(NULL);
      break;
   case 'r':
      GenerateCylinders();
      NeedNewCubeMap = GL_TRUE;
      break;
   case 'z':
      Zrot -= step;
         break;
   case 'Z':
      Zrot += step;
      break;
   case 27:
      glutDestroyWindow(Win);
      exit(0);
      break;
   }
   fflush(stdout);
   glutPostRedisplay();
}


static void
SpecialKey(int key, int x, int y)
{
   const GLfloat step = 3.0;
   const int mod = glutGetModifiers();
   (void) x;
   (void) y;
   switch (key) {
      case GLUT_KEY_UP:
         if (mod)
            //LightLatitude += step;
            SpherePos[1] += .1*step;
         else
            Xrot += step;
         break;
      case GLUT_KEY_DOWN:
         if (mod)
            //LightLatitude -= step;
            SpherePos[1] -= .1*step;
         else
            Xrot -= step;
         break;
      case GLUT_KEY_LEFT:
         if (mod)
            LightLongitude += step;
         else
            Yrot += step;
         break;
      case GLUT_KEY_RIGHT:
         if (mod)
            LightLongitude -= step;
         else
            Yrot -= step;
         break;
   }
   if (mod)
      NeedNewCubeMap = GL_TRUE;

   glutPostRedisplay();
}


static void
Init(void)
{
   const char *extensions[3] = {
      "GL_ARB_depth_texture",
      "GL_ARB_texture_cube_map",
      "GL_EXT_framebuffer_object"
   };
   int i;

   for (i = 0; i < ELEMENTS(extensions); i++) {
      if (!glutExtensionSupported(extensions[i])) {
         printf("Sorry, this demo requires %s\n", extensions[i]);
         exit(1);
      }
   }

   glClearColor(0.25, 0.25, 0.25, 1.0);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   GenerateCylinders();

   printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
}


static void
PrintHelp(void)
{
   printf("Keys:\n");
   printf("  a = toggle animation\n");
   printf("  cursor keys = rotate scene\n");
   printf("  <shift> + up/down = move sphere\n");
   printf("  <shift> + left/right = move light\n");
   fflush(stdout);
}


int
main(int argc, char *argv[])
{
   glutInitWindowSize(WindowWidth, WindowHeight);
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
   Win = glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutSpecialFunc(SpecialKey);
   glutDisplayFunc(Display);
   if (Anim)
      glutIdleFunc(Idle);
   Init();
   PrintHelp();
   glutMainLoop();
   return 0;
}
