/**
 * Test using a geometry shader to implement polygon outlining.
 *
 * Based on the technique "Single-Pass Wireframe Rendering" by Andreas
 * Bærentzen, Steen Lund Nielsen, Mikkel Gjael, Bent D. Larsen & Niels
 * Jaergen Christensen, SIGGRAPH 2006
 *
 * Brian Paul
 * May 2012
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"
#include "trackball.h"

static GLint WinWidth = 500, WinHeight = 500;
static GLint Win = 0;
static GLuint VertShader, GeomShader, FragShader, Program;
static GLboolean Anim = GL_TRUE;
static int uViewportSize = -1;

static const GLfloat Orange[4] = {1.0, 0.6, 0.0, 1};

static float CurQuat[4] = { 0, 0, 0, 1 };
static GLboolean ButtonDown = GL_FALSE;
static GLint ButtonX, ButtonY;


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
   GLfloat rot[4][4];

   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glColor4fv(Orange);

   glPushMatrix();
   build_rotmatrix(rot, CurQuat);
   glMultMatrixf(&rot[0][0]);

   if (0)
      glutSolidDodecahedron();
   else
      glutSolidSphere(2, 30, 20);

   glPopMatrix();

   glutSwapBuffers();
}


static void
Idle(void)
{
   static const float yAxis[3] = {0, 1, 0};
   static double t0 = -1.;
   float quat[4];
   double dt, t = glutGet(GLUT_ELAPSED_TIME) / 2000.0;
   if (t0 < 0.0)
      t0 = t;
   dt = t - t0;
   t0 = t;

   axis_to_quat(yAxis, 2.0 * dt, quat);
   add_quats(quat, CurQuat, CurQuat);

   glutPostRedisplay();
}


static void
Reshape(int width, int height)
{
   float ar = (float) width / height;
   WinWidth = width;
   WinHeight = height;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-ar, ar, -1, 1, 3, 25);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, -10);

   /* pass viewport dims to the shader */
   {
      GLfloat viewport[4];
      glGetFloatv(GL_VIEWPORT, viewport);
      glUniform2f(uViewportSize, viewport[2], viewport[3]);
   }
}


static void
MouseMotion(int x, int y)
{
   if (ButtonDown) {
      float x0 = (2.0 * ButtonX - WinWidth) / WinWidth;
      float y0 = (WinHeight - 2.0 * ButtonY) / WinHeight;
      float x1 = (2.0 * x - WinWidth) / WinWidth;
      float y1 = (WinHeight - 2.0 * y) / WinHeight;
      float q[4];

      trackball(q, x0, y0, x1, y1);
      ButtonX = x;
      ButtonY = y;
      add_quats(q, CurQuat, CurQuat);

      glutPostRedisplay();
   }
}


static void
MouseButton(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
     ButtonDown = GL_TRUE;
     ButtonX = x;
     ButtonY = y;
  }
  else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
     ButtonDown = GL_FALSE;
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


static void
Init(void)
{
   static const char *vertShaderText =
      "#version 120 \n"
      "void main() \n"
      "{ \n"
      "   gl_FrontColor = gl_Color; \n"
      "   gl_Position = ftransform(); \n"
      "} \n";
   static const char *geomShaderText =
      "#version 120 \n"
      "#extension GL_ARB_geometry_shader4: enable \n"
      "uniform vec2 ViewportSize; \n"
      "varying vec2 Vert0, Vert1, Vert2; \n"
      "\n"
      "// Transform NDC coord to window coord \n"
      "vec2 vpxform(vec4 p) \n"
      "{ \n"
      "   return (p.xy / p.w + 1.0) * 0.5 * ViewportSize; \n"
      "} \n"
      "\n"
      "void main() \n"
      "{ \n"
      "   gl_FrontColor = gl_FrontColorIn[0]; \n"
      "   Vert0 = vpxform(gl_PositionIn[0]); \n"
      "   Vert1 = vpxform(gl_PositionIn[1]); \n"
      "   Vert2 = vpxform(gl_PositionIn[2]); \n"
      "   gl_Position = gl_PositionIn[0]; \n"
      "   EmitVertex(); \n"
      "   gl_Position = gl_PositionIn[1]; \n"
      "   EmitVertex(); \n"
      "   gl_Position = gl_PositionIn[2]; \n"
      "   EmitVertex(); \n"
      "} \n";
   static const char *fragShaderText =
      "#version 120 \n"
      "#define LINE_WIDTH 2.5 \n"
      "varying vec2 Vert0, Vert1, Vert2; \n"
      "// Compute distance from a point to a line \n"
      "float point_line_dist(vec2 p, vec2 v1, vec2 v2) \n"
      "{ \n"
      "   float s = (v2.x - v1.x) * (v1.y - p.y) - (v1.x - p.x) * (v2.y - v1.y); \n"
      "   float t = length(v2 - v1); \n"
      "   return abs(s) / t; \n"
      "} \n"
      "\n"
      "void main() \n"
      "{ \n"
      "   float d0 = point_line_dist(gl_FragCoord.xy, Vert0, Vert1); \n"
      "   float d1 = point_line_dist(gl_FragCoord.xy, Vert1, Vert2); \n"
      "   float d2 = point_line_dist(gl_FragCoord.xy, Vert2, Vert0); \n"
      "   float m = min(d0, min(d1, d2)); \n"
      "   gl_FragColor = gl_Color * smoothstep(0.0, LINE_WIDTH, m); \n"
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

   Program = LinkShaders3(VertShader, GeomShader, FragShader);
   assert(Program);
   CheckError(__LINE__);

   /*
    * The geometry shader will receive and emit triangles.
    */
   glProgramParameteriARB(Program, GL_GEOMETRY_INPUT_TYPE_ARB,
                          GL_TRIANGLES);
   glProgramParameteriARB(Program, GL_GEOMETRY_OUTPUT_TYPE_ARB,
                          GL_TRIANGLE_STRIP);
   glProgramParameteriARB(Program,GL_GEOMETRY_VERTICES_OUT_ARB, 3);
   CheckError(__LINE__);

   /* relink */
   glLinkProgramARB(Program);

   assert(glIsProgram(Program));
   assert(glIsShader(FragShader));
   assert(glIsShader(VertShader));
   assert(glIsShader(GeomShader));

   glUseProgram(Program);

   uViewportSize = glGetUniformLocation(Program, "ViewportSize");

   glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
   glEnable(GL_DEPTH_TEST);

   printf("GL_RENDERER = %s\n",(const char *) glGetString(GL_RENDERER));
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
   glutMotionFunc(MouseMotion);
   glutMouseFunc(MouseButton);
   if (Anim)
      glutIdleFunc(Idle);

   Init();
   glutMainLoop();
   return 0;
}
