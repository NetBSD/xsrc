/**
 * Test using a geometry shader to implement wide lines.
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
static GLboolean UseGeomShader = GL_TRUE;
static GLfloat LineWidth = 10.0;
static GLfloat MaxLineWidth;
static GLfloat Xrot = 0, Yrot = 0;
static int uLineWidth = -1, uInverseViewportSize = -1;

static const int NumPoints = 50;
static float Points[100][3], Colors[100][3];

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

   if (UseGeomShader) {
      glUseProgram(Program);
      glUniform1f(uLineWidth, LineWidth);
   }
   else {
      glUseProgram(0);
      glLineWidth(LineWidth);
   }

   glBegin(GL_LINES);
   for (i = 0; i < NumPoints; i++) {
      glColor3fv(Colors[i]);
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
   case 'g':
      UseGeomShader = !UseGeomShader;
      printf("Use geometry shader? %d\n", UseGeomShader);
      break;
   case 'w':
      LineWidth -= 0.5;
      if (LineWidth < 1.0)
         LineWidth = 1.0;
      printf("Line width: %f\n", LineWidth);
      break;
   case 'W':
      LineWidth += 0.5;
      if (LineWidth > MaxLineWidth)
         LineWidth = MaxLineWidth;
      printf("Line width: %f\n", LineWidth);
      break;

   case 27:
      CleanUp();
      exit(0);
      break;
   }
   glutPostRedisplay();
}


static void
MakePoints(void)
{
   int i;
   for (i = 0; i < NumPoints; i++) {
      Colors[i][0] = (rand() % 1000) / 1000.0;
      Colors[i][1] = (rand() % 1000) / 1000.0;
      Colors[i][2] = (rand() % 1000) / 1000.0;
      Points[i][0] = ((rand() % 2000) - 1000.0) / 500.0;
      Points[i][1] = ((rand() % 2000) - 1000.0) / 500.0;
      Points[i][2] = ((rand() % 2000) - 1000.0) / 500.0;
   }
}

static void
Init(void)
{
   static const char *fragShaderText =
      "void main() \n"
      "{ \n"
      "   gl_FragColor = gl_Color; \n"
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
      "uniform float LineWidth; \n"
      "void main() \n"
      "{ \n"
      "   vec4 pos0 = gl_PositionIn[0]; \n"
      "   vec4 pos1 = gl_PositionIn[1]; \n"
      "   vec4 dir = abs(pos1 - pos0); \n"
      "   vec2 d0 = vec2(LineWidth * pos0.w) * InverseViewportSize; \n"
      "   vec2 d1 = vec2(LineWidth * pos1.w) * InverseViewportSize; \n"
      "   // this conditional could be avoided \n"
      "   if (dir.x > dir.y) { \n"
      "      // X-major line \n"
      "      d0.x = 0.0; \n"
      "      d1.x = 0.0; \n"
      "   } \n"
      "   else { \n"
      "      // Y-major line \n"
      "      d0.y = 0.0; \n"
      "      d1.y = 0.0; \n"
      "   } \n"
      "   gl_FrontColor = gl_FrontColorIn[0]; \n"
      "   gl_TexCoord[0] = vec4(0, 0, 0, 1); \n"
      "   gl_Position = pos0 + vec4( d0.x, -d0.y, 0, 0); \n"
      "   EmitVertex(); \n"
      "   gl_FrontColor = gl_FrontColorIn[1]; \n"
      "   gl_TexCoord[0] = vec4(1, 0, 0, 1); \n"
      "   gl_Position = pos1 + vec4( d1.x, -d1.y, 0, 0); \n"
      "   EmitVertex(); \n"
      "   gl_FrontColor = gl_FrontColorIn[0]; \n"
      "   gl_TexCoord[0] = vec4(0, 1, 0, 1); \n"
      "   gl_Position = pos0 + vec4(-d0.x,  d0.y, 0, 0); \n"
      "   EmitVertex(); \n"
      "   gl_FrontColor = gl_FrontColorIn[1]; \n"
      "   gl_TexCoord[0] = vec4(1, 1, 0, 1); \n"
      "   gl_Position = pos1 + vec4(-d1.x,  d1.y, 0, 0); \n"
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
    * The geometry shader will convert incoming lines to quads (4-vertex
    * triangle strips).
    */
   glProgramParameteriARB(Program, GL_GEOMETRY_INPUT_TYPE_ARB,
                          GL_LINES);
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
   uLineWidth = glGetUniformLocation(Program, "LineWidth");

   glClearColor(0.3f, 0.3f, 0.3f, 0.0f);

   printf("GL_RENDERER = %s\n",(const char *) glGetString(GL_RENDERER));

   assert(glIsProgram(Program));
   assert(glIsShader(FragShader));
   assert(glIsShader(VertShader));
   assert(glIsShader(GeomShader));

   glEnable(GL_DEPTH_TEST);

   {
      GLfloat r[2];
      glGetFloatv(GL_LINE_WIDTH_RANGE, r);
      MaxLineWidth = r[1];
   }

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
