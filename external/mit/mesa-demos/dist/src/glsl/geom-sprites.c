/**
 * Test using a geometry shader to implement point sprites.
 * XXX we should also demo point size attenuation.
 *
 * Brian Paul
 * March 2011
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"

static GLint WinWidth = 500, WinHeight = 500;
static GLint Win = 0;
static GLuint VertShader, GeomShader, FragShader, Program;
static GLboolean Anim = GL_TRUE;
static GLfloat Xrot = 0, Yrot = 0;
static int uPointSize = -1, uInverseViewportSize = -1;

static const int NumPoints = 50;
static float Points[100][3];

static const GLfloat Red[4] = {1, 0, 0, 1};
static const GLfloat Green[4] = {0, 1, 0, 0};


static void
CheckError(int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("GL Error %s (0x%x) at line %d\n",
             gluErrorString(err), (int) err, line);
   }
}


static void
Redisplay(void)
{
   int i;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glPushMatrix();
   glRotatef(Xrot, 1, 0, 0);
   glRotatef(Yrot, 0, 0, 1);

   glBegin(GL_POINTS);
   for (i = 0; i < NumPoints; i++) {
      glVertex3fv(Points[i]);
   }
   glEnd();

   glPopMatrix();

   glutSwapBuffers();
}


static void
Idle(void)
{
   int curTime = glutGet(GLUT_ELAPSED_TIME);
   Xrot = curTime * 0.02;
   Yrot = curTime * 0.05;
   glutPostRedisplay();
}


static void
Reshape(int width, int height)
{
   float ar = (float) width / height;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-ar, ar, -1, 1, 3, 25);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, -10);

   {
      GLfloat viewport[4];
      glGetFloatv(GL_VIEWPORT, viewport);
      glUniform2f(uInverseViewportSize, 1.0F / viewport[2], 1.0F / viewport[3]);
   }
}


static void
CleanUp(void)
{
   glDeleteShader(FragShader);
   glDeleteShader(VertShader);
   glDeleteShader(GeomShader);
   glDeleteProgram(Program);
   glutDestroyWindow(Win);
}


static void
Key(unsigned char key, int x, int y)
{
  (void) x;
  (void) y;

   switch(key) {
   case ' ':
   case 'a':
      Anim = !Anim;
      if (Anim) {
         glutIdleFunc(Idle);
      }
      else
         glutIdleFunc(NULL);
      break;
   case 27:
      CleanUp();
      exit(0);
      break;
   }
   glutPostRedisplay();
}


static GLuint
MakeTexture(void)
{
#define TEX_SIZE 32
   GLubyte image[TEX_SIZE][TEX_SIZE][3];
   GLuint i, j;
   GLuint tex;

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   /* 'X' pattern */
   for (i = 0; i < TEX_SIZE; i++) {
      for (j = 0; j < TEX_SIZE; j++) {
         int p1 = i - j, p2 = TEX_SIZE - 1 - i - j;
         p1 = (p1 >= -2 && p1 <= 2);
         p2 = (p2 >= -2 && p2 <= 2);
         if (p1 || p2) {             
            image[i][j][0] = 255;
            image[i][j][1] = 255;
            image[i][j][2] = 255;
         }
         else {
            image[i][j][0] = 50;
            image[i][j][1] = 50;
            image[i][j][2] = 50;
         }
      }
   }

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, TEX_SIZE, 0,
                GL_RGB, GL_UNSIGNED_BYTE, image);

   return tex;
}


static void
MakePoints(void)
{
   int i;
   for (i = 0; i < NumPoints; i++) {
      Points[i][0] = ((rand() % 2000) - 1000.0) / 500.0;
      Points[i][1] = ((rand() % 2000) - 1000.0) / 500.0;
      Points[i][2] = ((rand() % 2000) - 1000.0) / 500.0;
   }
}

static void
Init(void)
{
   static const char *fragShaderText =
      "uniform sampler2D tex; \n"
      "void main() \n"
      "{ \n"
      "   gl_FragColor = texture2D(tex, gl_TexCoord[0].xy); \n"
      "} \n";
   static const char *vertShaderText =
      "void main() \n"
      "{ \n"
      "   gl_FrontColor = gl_Color; \n"
      "   gl_Position = ftransform(); \n"
      "} \n";
   static const char *geomShaderText =
      "#version 120 \n"
      "#extension GL_ARB_geometry_shader4: enable \n"
      "uniform vec2 InverseViewportSize; \n"
      "uniform float PointSize; \n"
      "void main() \n"
      "{ \n"
      "   vec4 pos = gl_PositionIn[0]; \n"
      "   vec2 d = vec2(PointSize * pos.w) * InverseViewportSize; \n"
      "   gl_FrontColor = gl_FrontColorIn[0]; \n"
      "   gl_TexCoord[0] = vec4(0, 0, 0, 1); \n"
      "   gl_Position = pos + vec4(-d.x, -d.y, 0, 0); \n"
      "   EmitVertex(); \n"
      "   gl_TexCoord[0] = vec4(1, 0, 0, 1); \n"
      "   gl_Position = pos + vec4( d.x, -d.y, 0, 0); \n"
      "   EmitVertex(); \n"
      "   gl_TexCoord[0] = vec4(0, 1, 0, 1); \n"
      "   gl_Position = pos + vec4(-d.x,  d.y, 0, 0); \n"
      "   EmitVertex(); \n"
      "   gl_TexCoord[0] = vec4(1, 1, 0, 1); \n"
      "   gl_Position = pos + vec4( d.x,  d.y, 0, 0); \n"
      "   EmitVertex(); \n"
      "} \n";

   if (!ShadersSupported())
      exit(1);

   if (!glutExtensionSupported("GL_ARB_geometry_shader4")) {
      fprintf(stderr, "Sorry, GL_ARB_geometry_shader4 is not supported.\n");
      exit(1);
   }

   VertShader = CompileShaderText(GL_VERTEX_SHADER, vertShaderText);
   FragShader = CompileShaderText(GL_FRAGMENT_SHADER, fragShaderText);
   GeomShader = CompileShaderText(GL_GEOMETRY_SHADER_ARB, geomShaderText);
   assert(GeomShader);

   Program = LinkShaders3(VertShader, GeomShader, FragShader);
   assert(Program);
   CheckError(__LINE__);

   /*
    * The geometry shader will convert incoming points to quads (4-vertex
    * triangle strips).
    */
   glProgramParameteriARB(Program, GL_GEOMETRY_INPUT_TYPE_ARB,
                          GL_POINTS);
   glProgramParameteriARB(Program, GL_GEOMETRY_OUTPUT_TYPE_ARB,
                          GL_TRIANGLE_STRIP);
   glProgramParameteriARB(Program,GL_GEOMETRY_VERTICES_OUT_ARB, 4);
   CheckError(__LINE__);

   glLinkProgramARB(Program);

   /* check link */
   {
      GLint stat;
      GetProgramiv(Program, GL_LINK_STATUS, &stat);
      if (!stat) {
         GLchar log[1000];
         GLsizei len;
         GetProgramInfoLog(Program, 1000, &len, log);
         fprintf(stderr, "Shader link error:\n%s\n", log);
      }
   }

   CheckError(__LINE__);

   glUseProgram(Program);
   CheckError(__LINE__);

   uInverseViewportSize = glGetUniformLocation(Program, "InverseViewportSize");
   uPointSize = glGetUniformLocation(Program, "PointSize");

   glUniform1f(uPointSize, 24.0);

   glClearColor(0.3f, 0.3f, 0.3f, 0.0f);

   printf("GL_RENDERER = %s\n",(const char *) glGetString(GL_RENDERER));

   assert(glIsProgram(Program));
   assert(glIsShader(FragShader));
   assert(glIsShader(VertShader));
   assert(glIsShader(GeomShader));

   glEnable(GL_DEPTH_TEST);

   MakeTexture();
   MakePoints();
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(WinWidth, WinHeight);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   Win = glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Redisplay);
   if (Anim)
      glutIdleFunc(Idle);

   Init();
   glutMainLoop();
   return 0;
}
