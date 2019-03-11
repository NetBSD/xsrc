/*
 * Draw samples of wide lines, smooth lines and stippled lines in
 * all combinations.
 *
 * Brian Paul
 * 13 Feb 2015
 */


#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <GL/glew.h> // just for GL_ALIASED_LINE_WIDTH_RANGE
#include "glut_wrap.h"


static int w, h;
static float g, lw = 3.0;

static GLboolean Blend = GL_TRUE;
static GLboolean FlatShade = GL_TRUE;


static void
draw_line_sample(GLboolean smooth, GLboolean stipple, GLfloat width)
{
   float r0 = 0.4;
   float r1 = 0.8;
   float r2 = 0.9;
   float r3 = 0.95;
   int i;

   glLineWidth(width);

   if (stipple) {
      glEnable(GL_LINE_STIPPLE);
   }
   else {
      glDisable(GL_LINE_STIPPLE);
   }

   if (smooth) {
      glEnable(GL_LINE_SMOOTH);
      if (Blend)
         glEnable(GL_BLEND);
      else
         glDisable(GL_BLEND);
    }
   else {
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_BLEND);
   }

   /* spokes */
   glBegin(GL_LINES);
   for (i = 0; i < 360; i += 10) {
      float x0 = r0 * cos(i * M_PI / 180.0);
      float y0 = r0 * sin(i * M_PI / 180.0);
      float x1 = r1 * cos(i * M_PI / 180.0);
      float y1 = r1 * sin(i * M_PI / 180.0);

      glColor3f(.5, .5, 1);
      glVertex2f(x0, y0);
      glColor3f(1, 1, 1);
      glVertex2f(x1, y1);
   }
   glEnd();

   /* circle */
   glBegin(GL_LINE_STRIP);
   for (i = 0; i <= 360; i += 10) {
      float x = r2 * cos(i * M_PI / 180.0);
      float y = r2 * sin(i * M_PI / 180.0);

      glVertex2f(x, y);
   }
   glEnd();

   /* box */
   glBegin(GL_LINE_STRIP);
   glVertex2f(-r3, -r3);
   glVertex2f( r3, -r3);
   glVertex2f( r3,  r3);
   glVertex2f(-r3,  r3);
   glVertex2f(-r3, -r3);
   glEnd();
}


static void
Init(void)
{
   GLfloat range[2];
   char title[1000];

   printf("GL_RENDERER = %s\n", (const char *) glGetString(GL_RENDERER));
   printf("GL_VERSION = %s\n", (const char *) glGetString(GL_VERSION));

   glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
   printf("Aliased line width range %.5f .. %.5f\n", range[0], range[1]);
   glGetFloatv(GL_LINE_WIDTH_RANGE, range);
   printf("Smooth line width range %.5f .. %.5f\n", range[0], range[1]);
   glGetFloatv(GL_LINE_WIDTH_GRANULARITY, &g);
   printf("line width granularity %f\n", g);
   fflush(stdout);

   glClearColor(0.0, 0.0, 0.0, 0.0);

   glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   /* pattern: |- -- --- ----   | */
   glLineStipple(2, 0x1eed);

   sprintf(title, "Line Sampler (%s)", (const char *) glGetString(GL_RENDERER));
   glutSetWindowTitle(title);
}


static void
Draw(void)
{
   int i;

   glClearColor(0.2, 0.2, 0.2, 0.2);
   glClear(GL_COLOR_BUFFER_BIT);

   glShadeModel(FlatShade ? GL_FLAT : GL_SMOOTH);

   for (i = 0; i < 2; i++) {
      float width = i ? lw : 1.0;
      glViewport(0*w, i*h, w, h);
      draw_line_sample(GL_FALSE, GL_FALSE, width);
      glViewport(1*w, i*h, w, h);
      draw_line_sample(GL_TRUE, GL_FALSE, width);
      glViewport(2*w, i*h, w, h);
      draw_line_sample(GL_FALSE, GL_TRUE, width);
      glViewport(3*w, i*h, w, h);
      draw_line_sample(GL_TRUE, GL_TRUE, width);
   }

    glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
   w = width / 4;
   h = height / 2;
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


static void
Key(unsigned char key, int x, int y)
{
   if (key == 'w') {
      lw -= g;
      if (lw < g)
         lw = g;
   }
   else if (key == 'W') {
      lw += g;
   }
   else if (key == 'f') {
      FlatShade = !FlatShade;
   }
   else if (key == 'b') {
      Blend = !Blend;
   }
   if (key == 27)
      exit(0);

   printf("line width %.5f  FlatShade = %d  Blend = %d\n", lw, FlatShade, Blend);
   fflush(stdout);

   glutPostRedisplay();
}


int
main(int argc, char **argv)
{
   glutInit(&argc, argv);

   glutInitWindowPosition(0, 0);
   glutInitWindowSize(1000, 500);

   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

   if (!glutCreateWindow("Line Sampler")) {
      exit(1);
   }

   Init();

   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Draw);
   glutMainLoop();

   return 0;
}
