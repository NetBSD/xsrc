/**
 * Test glPolygonMode w/ different front/back face modes.
 *
 * The left half of the window draws a torus with different front/back
 * polygon modes.
 *
 * The right half of the windows tries to draw the same thing, but with
 * a different approach:
 *   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 *   glEnable(GL_CULL_FACE);
 *   glCullFace(GL_FRONT);
 *   draw_object();
 *   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 *   glEnable(GL_CULL_FACE);
 *   glCullFace(GL_BACK);
 *   draw_object();
 *
 * The outcome is often the same, but not always (turn on blending with 'b').
 *
 * Brian Paul
 * 2 Feb 2016
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "glut_wrap.h"

static int Win;
static int WinWidth = 800, WinHeight = 400;
static GLfloat Xrot = 0;
static GLboolean Anim = GL_TRUE;
static double t0;
static GLboolean Blend = GL_TRUE;


static void
Idle(void)
{
   double t1 = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
   Xrot += (t1 - t0) * 20;  /* 20 degrees per second */
   t0 = t1;
   glutPostRedisplay();
}


static void
Draw(void)
{
   int hw = WinWidth / 2;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if (Blend) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBlendFunc(GL_ONE, GL_ONE);
   }
   else {
      glDisable(GL_BLEND);
   }

   glPushMatrix();
   glRotatef(Xrot, 1, 0, 0);

   /* draw left half */
   glViewport(0, 0, hw, WinHeight);

   glPolygonMode(GL_FRONT, GL_LINE);
   glPolygonMode(GL_BACK, GL_FILL);
   glDisable(GL_CULL_FACE);
   glutSolidTorus(0.75, 2.0, 10, 20);

   /* draw right half */
   glViewport(hw, 0, WinWidth - hw, WinHeight);

   /* draw back faces */
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_FRONT);
   glutSolidTorus(0.75, 2.0, 10, 20);

   /* draw front faces */
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glutSolidTorus(0.75, 2.0, 10, 20);

   glPopMatrix();

   glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
   WinWidth = width;
   WinHeight = height;
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-1.0, 1.0, -1.0, 1.0, 3.0, 25.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -10.0);
   glutPostRedisplay();
}


static void
Key(unsigned char key, int x, int y)
{
   (void) x;
   (void) y;
   switch (key) {
   case 'a':
      Anim = !Anim;
      if (Anim) {
         t0 = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
         glutIdleFunc(Idle);
      }
      else
         glutIdleFunc(NULL);
      break;
   case 'b':
      Blend = !Blend;
      break;
   case 27:
      glutDestroyWindow(Win);
      exit(0);
      break;
   }
   glutPostRedisplay();
}


static void
SpecialKey(int key, int x, int y)
{
   const GLfloat step = 3.0;
   (void) x;
   (void) y;
   switch (key) {
   case GLUT_KEY_UP:
      Xrot -= step;
      break;
   case GLUT_KEY_DOWN:
      Xrot += step;
      break;
   }
   glutPostRedisplay();
}


static void
Init(void)
{
   static const float yellow[4] = {1, 1, 0, 0.5};
   static const float blue[4] = {0.2, 0.2, 1, 0.5};
   static const float gray[4] = {0.2, 0.2, 0.2, 1.0};

   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, gray);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
   glMaterialfv(GL_BACK, GL_DIFFUSE, blue);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
   glShadeModel(GL_FLAT);

   glClearColor(0.25, 0.25, 0.25, 1.0);
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(WinWidth, WinHeight);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   Win = glutCreateWindow(argv[0]);
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutSpecialFunc(SpecialKey);
   glutDisplayFunc(Draw);
   if (Anim)
      glutIdleFunc(Idle);
   Init();
   glutMainLoop();
   return 0;
}
