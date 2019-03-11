/*
 * Automatic primitive rasterization precision test.
 *
 * Draw prims at various sub-pixel offsets and examine where the quad is
 * actually drawn.
 * Check if the range of offsets which paint the right pixels falls within
 * OpenGL's specification.
 * In case of failures, report the coordinate bias needed to fix the problem.
 *
 * Note that even Mesa/swrast fails some line tests.  This is because some
 * window coordinates wind up as 53.9999 instead of 54, for example.  Enabling
 * the small translation factor below fixes that.  Revisit someday...
 *
 * Brian Paul
 * 28 Feb 2008
 */


#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "glut_wrap.h"


static bool NoSwap = false;
static int Width = 100, Height = 100;
static int Win;
static float Step = 1.0 / 16.0;
#if 0
/* This tiny offset fixes errors in Mesa/Xlib */
static float Xtrans = 0.5 * 0.125;
static float Ytrans = 0.5 * 0.125;
#else
static float Xtrans = 0.0;
static float Ytrans = 0.0;
#endif

enum prim {
   PRIM_POINTS,
   PRIM_LINES,
   PRIM_QUADS,
   PRIM_ALL
};

static enum prim TestPrim = PRIM_ALL;


static void
PointCalibrate(int xpos, int ypos)
{
   GLfloat rgba[4];
   float x, y;
   float xmin, ymin, xmax, ymax;

   xmin = ymin = 1000.0;
   xmax = ymax = -1000.0;

   for (y = -1.0; y <= 1.0; y += Step) {
      for (x = -1.0; x <= 1.0; x += Step) {
         glClear(GL_COLOR_BUFFER_BIT);
         glBegin(GL_POINTS);
         glVertex2f(xpos + x, ypos + y);
         glEnd();
         glReadPixels(xpos, ypos, 1, 1, GL_RGBA, GL_FLOAT, rgba);
         if (rgba[0] == 1.0 && rgba[1] == 1.0 && rgba[2] == 1.0) {
            /* hit */
            if (x < xmin)
               xmin = x;
            if (y < ymin)
               ymin = y;
            if (x > xmax)
               xmax = x;
            if (y > ymax)
               ymax = y;
         }

         if (!NoSwap)
            glutSwapBuffers();
      }
   }

   printf("Point at (%2d, %2d) drawn for x in [%7.4f, %7.4f] and y in [%7.4f, %7.4f]\n",
          xpos, ypos,
          xpos + xmin, xpos + xmax,
          ypos + ymin, ypos + ymax);

   if (xmax - xmin != 1.0 - Step) {
      printf("  => Inconsistant X-axis rasterization!\n");
   }
   if (ymax - ymin != 1.0 - Step) {
      printf("  => Inconsistant Y-axis rasterization!\n");
   }
   if (xmin < 0.0) {
      printf("  => Points should be X biased by about %f\n", xmin);
   }
   if (ymin < 0.0) {
      printf("  => Points should be Y biased by about %f\n", ymin);
   }
   if (xmax > 1.0) {
      printf("  => Points should be X biased by about %f\n", 1.0 - xmax);
   }
   if (ymax > 1.0) {
      printf("  => Points should be Y biased by about %f\n", 1.0 - ymax);
   }

   fflush(stdout);
}


static void
HLineCalibrate(int xpos, int ypos, int len)
{
   GLfloat rgba[2][4];
   float x, y;
   float ymin, ymax;
   float xmin_left, xmax_left, xmin_right, xmax_right;

   xmin_left = xmin_right = 1000.0;
   xmax_left = xmax_right = -1000.0;
   ymin = 1000;
   ymax = -1000.0;

   /*
    * First, check vertical positioning of the horizontal line
    */
   for (y = -1.0; y <= 1.0; y += Step) {
      glClear(GL_COLOR_BUFFER_BIT);
      glBegin(GL_LINES);
      glVertex2f(xpos,       ypos + y);
      glVertex2f(xpos + len, ypos + y);
      glEnd();

      glReadPixels(xpos + len / 2, ypos, 1, 1, GL_RGBA, GL_FLOAT, rgba);
      if (rgba[0][0] == 1.0) {
         /* hit */
         if (y < ymin)
            ymin = y;
         if (y > ymax)
            ymax = y;
      }

      if (!NoSwap)
         glutSwapBuffers();
   }

   printf("H-line at Y=%2d drawn for y in [%7.4f, %7.4f]\n",
          ypos,
          ypos + ymin, ypos + ymax);

   if (ymax - ymin != 1.0 - Step) {
      printf("  => Inconsistant Y-axis rasterization!\n");
   }

   if (ymin > 0.5 ) {
      printf("  => Lines should be Y biased by about %f\n", ymin - 0.5);
   }

   if (ymax < 0.5 ) {
      printf("  => Lines should be Y biased by about %f\n", 0.5 - ymax);
   }

   /*
    * Second, check endpoints  (for Y at 1/2 pixel)
    */
   for (x = -1.0; x <= 1.0; x += Step) {
      glClear(GL_COLOR_BUFFER_BIT);
      glBegin(GL_LINES);
      glVertex2f(xpos + x,       ypos + 0.5f);
      glVertex2f(xpos + x + len, ypos + 0.5f);
      glEnd();

      /* left end */
      glReadPixels(xpos - 1, ypos, 2, 1, GL_RGBA, GL_FLOAT, rgba);
      if (rgba[0][0] == 0.0 && rgba[1][0] == 1.0) {
         /* hit */
         if (x < xmin_left)
            xmin_left = x;
         if (x > xmax_left)
            xmax_left = x;
      }

      /* right end */
      glReadPixels(xpos + len - 1, ypos, 2, 1, GL_RGBA, GL_FLOAT, rgba);
      if (rgba[0][0] == 1.0 && rgba[1][0] == 0.0) {
         /* hit */
         if (x < xmin_right)
            xmin_right = x;
         if (x > xmax_right)
            xmax_right = x;
      }

      if (!NoSwap)
         glutSwapBuffers();
   }

   printf("H-line [%d..%d) hit left end for x in [%7.4f, %7.4f] "
          "hit right end for x in [%7.4f, %7.4f]\n",
          xpos, xpos + len,
          xpos + xmin_left, xpos + xmax_left,
          xpos + len + xmin_right, xpos + len + xmax_right);

   if (xmax_left - xmin_left > 1.0 - Step) {
      printf("  => Inconsistant left-end rasterization!\n");
   }
   if (xmax_right - xmin_right > 1.0 - Step) {
      printf("  => Inconsistant right-end rasterization!\n");
   }

   if (xmin_left != xmin_right ||
       xmax_left != xmax_right) {
      printf("  => Inconsistant length!\n");
   }

   if (xmin_left < 0.0) {
      printf("  => Coords should be X biased by about %f\n", xmin_left );
   }
   if (xmin_right < 0.0) {
      printf("  => Coords should be X biased by about %f\n", xmin_right );
   }
   if (xmax_left > 1.0) {
      printf("  => Coords should be X biased by about %f\n", -xmax_left + 1.0);
   }
   if (xmax_right > 1.0) {
      printf("  => Coords should be X biased by about %f\n", -xmax_right + 1.0);
   }

   fflush(stdout);
}


static void
VLineCalibrate(int xpos, int ypos, int len)
{
   GLfloat rgba[2][4];
   float x, y;
   float xmin, xmax;
   float ymin_bottom, ymax_bottom, ymin_top, ymax_top;

   ymin_bottom = ymin_top = 1000.0;
   ymax_bottom = ymax_top = -1000.0;
   xmin = 1000;
   xmax = -1000.0;

   /*
    * First, check horizontal positioning of the vertical line
    */
   for (x = -1.0; x <= 1.0; x += Step) {
      glClear(GL_COLOR_BUFFER_BIT);
      glBegin(GL_LINES);
      glVertex2f(xpos + x,   ypos);
      glVertex2f(xpos + x, ypos + len);
      glEnd();

      glReadPixels(xpos + len / 2, ypos, 1, 1, GL_RGBA, GL_FLOAT, rgba);
      glReadPixels(xpos, ypos + len / 2, 1, 1, GL_RGBA, GL_FLOAT, rgba);
      if (rgba[0][0] == 1.0) {
         /* hit */
         if (x < xmin)
            xmin = x;
         if (x > xmax)
            xmax = x;
      }

      if (!NoSwap)
         glutSwapBuffers();
   }

   printf("V-line at X=%d drawn for x in [%7.4f, %7.4f]\n",
          xpos,
          xpos + xmin, xpos + xmax);

   if (xmax - xmin != 1.0 - Step) {
      printf("  => Inconsistant X-axis rasterization!\n");
   }

   if (xmin > 0.5 ) {
      printf("  => Lines should be X biased by about %f\n", xmin - 0.5);
   }

   if (xmax < 0.5 ) {
      printf("  => Lines should be X biased by about %f\n", 0.5 - xmax);
   }

   /*
    * Second, check endpoints  (for X at 1/2 pixel)
    */
   for (y = -1.0; y <= 1.0; y += Step) {
      glClear(GL_COLOR_BUFFER_BIT);
      glBegin(GL_LINES);
      glVertex2f(xpos + 0.5f, ypos + y);
      glVertex2f(xpos + 0.5f, ypos + y + len);
      glEnd();

      /* bottom end */
      glReadPixels(xpos, ypos - 1, 1, 2, GL_RGBA, GL_FLOAT, rgba);
      if (rgba[0][0] == 0.0 && rgba[1][0] == 1.0) {
         /* hit */
         if (y < ymin_bottom)
            ymin_bottom = y;
         if (y > ymax_bottom)
            ymax_bottom = y;
      }

      /* top end */
      glReadPixels(xpos, ypos + len - 1, 1, 2, GL_RGBA, GL_FLOAT, rgba);
      if (rgba[0][0] == 1.0 && rgba[1][0] == 0.0) {
         /* hit */
         if (y < ymin_top)
            ymin_top = y;
         if (y > ymax_top)
            ymax_top = y;
      }

      if (!NoSwap)
         glutSwapBuffers();
   }

   printf("V-line [%d..%d) hit bottom end for y in [%7.4f, %7.4f] "
          "hit top end for y in [%7.4f, %7.4f]\n",
          ypos, ypos + len,
          ypos + ymin_bottom, ypos + ymax_bottom,
          ypos + len + ymin_top, ypos + len + ymax_top);

   if (ymax_bottom - ymin_bottom > 1.0 - Step) {
      printf("  => Inconsistant bottom-end rasterization!\n");
   }
   if (ymax_top - ymin_top > 1.0 - Step) {
      printf("  => Inconsistant top-end rasterization!\n");
   }

   if (ymin_bottom != ymin_top ||
       ymax_bottom != ymax_top) {
      printf("  => Inconsistant length!\n");
   }

   if (ymin_bottom < 0.0) {
      printf("  => Coords should be Y biased by about %f\n", ymin_bottom );
   }
   if (ymin_top < 0.0) {
      printf("  => Coords should be Y biased by about %f\n", ymin_top );
   }
   if (ymax_bottom > 1.0) {
      printf("  => Coords should be Y biased by about %f\n", -ymax_bottom + 1.0);
   }
   if (ymax_top > 1.0) {
      printf("  => Coords should be Y biased by about %f\n", -ymax_top + 1.0);
   }

   fflush(stdout);
}


static void
QuadCalibrate(int xpos, int ypos, int width, int height)
{
   GLfloat rgba1[2][4];
   GLfloat rgba2[2][4];
   float x, y;
   float xmin, ymin, xmax, ymax;

   xmin = ymin = 1000.0;
   xmax = ymax = -1000.0;

   for (y = -1.0; y <= 1.0; y += Step) {
      for (x = -1.0; x <= 1.0; x += Step) {
         glClear(GL_COLOR_BUFFER_BIT);
         glBegin(GL_QUADS);
         glVertex2f(xpos + x,         ypos + y);
         glVertex2f(xpos + x + width, ypos + y);
         glVertex2f(xpos + x + width, ypos + y + height);
         glVertex2f(xpos + x,         ypos + y + height);
         glEnd();

         /* horizontal measurement */
         glReadPixels(xpos - 1, ypos + 2, 2, 1, GL_RGBA, GL_FLOAT, rgba1);
         glReadPixels(xpos + width - 1, ypos + 2, 2, 1, GL_RGBA, GL_FLOAT, rgba2);
         if (rgba1[0][0] == 0.0 && rgba1[1][0] == 1.0 &&
             rgba2[0][0] == 1.0 && rgba2[1][0] == 0.0) {
            if (x < xmin)
               xmin = x;
            if (x > xmax)
               xmax = x;
         }

         /* vertical measurement */
         glReadPixels(xpos + 2, ypos - 1, 1, 2, GL_RGBA, GL_FLOAT, rgba1);
         glReadPixels(xpos + 2, ypos + height - 1, 1, 2, GL_RGBA, GL_FLOAT, rgba2);
         if (rgba1[0][0] == 0.0 && rgba1[1][0] == 1.0 &&
             rgba2[0][0] == 1.0 && rgba2[1][0] == 0.0) {
            if (y < ymin)
               ymin = y;
            if (y > ymax)
               ymax = y;
         }

         if (!NoSwap)
            glutSwapBuffers();
      }
   }

   printf("Quad at (%2d, %2d)..(%2d, %2d) drawn"
          " for x in [%7.4f, %7.4f] and y in [%7.4f, %7.4f]\n",
          xpos, ypos,
          xpos + width, ypos + height,
          xpos + xmin, xpos + xmax,
          ypos + ymin, ypos + ymax);

   if (xmax - xmin != 1.0 - Step) {
      printf("  => Inconsistant X-axis rasterization/size!\n");
   }
   if (ymax - ymin != 1.0 - Step) {
      printf("  => Inconsistant Y-axis rasterization/size!\n");
   }

   if (xmin < -0.5) {
      printf("  => Coords should be X biased by about %f\n", 0.5 + xmin );
   }
   if (ymin < -0.5) {
      printf("  => Coords should be Y biased by about %f\n", 0.5 + ymin);
   }
   if (xmax > 0.5) {
      printf("  => Coords should be X biased by about %f\n", -xmax + 0.5);
   }
   if (ymax > 0.5) {
      printf("  => Coords should be Y biased by about %f\n", -ymax + 0.5);
   }

   fflush(stdout);
}


/**
 * Misc/disabled code for debugging.
 */
static void
DebugTest(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);

   glColor3f(.5, .5, .5);

   glBegin(GL_LINES);
   glVertex2f(30, 35.5);
   glVertex2f(54, 35.5);
   glVertex2f(54, 35.5);
   glVertex2f(66, 35.5);
   glEnd();

   glDisable(GL_BLEND);
   glColor3f(1,1,1);
}


static void
Draw(void)
{
   //   glClear(GL_COLOR_BUFFER_BIT);

   glPushMatrix();
   glTranslatef(Xtrans, Ytrans, 0);

   printf("Probe step size: %f\n", Step);

   if (TestPrim == PRIM_POINTS || TestPrim == PRIM_ALL) {
      PointCalibrate(1, 1);
      PointCalibrate(50, 50);
      PointCalibrate(28, 17);
      PointCalibrate(17, 18);
      printf("\n");
   }

   if (TestPrim == PRIM_LINES || TestPrim == PRIM_ALL) {
      HLineCalibrate(5, 10, 10);
      HLineCalibrate(25, 22, 12);
      HLineCalibrate(54, 33, 12);
      HLineCalibrate(54+12, 33, 12);
      printf("\n");

      VLineCalibrate(5, 10, 10);
      VLineCalibrate(25, 22, 12);
      VLineCalibrate(54, 33, 12);
      VLineCalibrate(54+12, 33, 12);
      printf("\n");
   }

   if (TestPrim == PRIM_QUADS || TestPrim == PRIM_ALL) {
      QuadCalibrate(2, 2, 10, 10);
      QuadCalibrate(50, 50, 10, 10);
      QuadCalibrate(28, 17, 12, 12);
      QuadCalibrate(17, 28, 12, 12);
   }

   (void) DebugTest;

   glPopMatrix();

   if (NoSwap)
      glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
   Width = width;
   Height = height;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, width, 0, height, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


static void
Key(unsigned char key, int x, int y)
{
   (void) x;
   (void) y;
   switch (key) {
   case 27:
      glutDestroyWindow(Win);
      exit(0);
      break;
   }
   glutPostRedisplay();
}


static void
Init(void)
{
   printf("Measurement/callibration for basic prim rasterization...\n");
   printf("GL_RENDERER: %s\n", (char*) glGetString(GL_RENDERER));
   printf("GL_VERSION: %s\n", (char*) glGetString(GL_VERSION));
}


int
main(int argc, char *argv[])
{
   int i;

   glutInit(&argc, argv);

   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--noswap") == 0)
         NoSwap = true;
      else if (strcmp(argv[i], "--points") == 0)
         TestPrim = PRIM_POINTS;
      else if (strcmp(argv[i], "--lines") == 0)
         TestPrim = PRIM_LINES;
      else if (strcmp(argv[i], "--quads") == 0)
         TestPrim = PRIM_QUADS;
   }

   glutInitWindowPosition(0, 0);
   glutInitWindowSize(Width, Height);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   Win = glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Draw);
   Init();
   glutMainLoop();
   return 0;
}
