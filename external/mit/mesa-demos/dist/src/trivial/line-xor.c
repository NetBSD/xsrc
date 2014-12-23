/*
 * Draw lines in XOR mode.
 * Note that the last pixel in a line segment should not be drawn by GL
 * so that pixels aren't touched twice at the shared vertex of connected lines.
 *
 * Brian Paul
 * 7 Oct 2010
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "glut_wrap.h"


static GLboolean xor = GL_TRUE;


static void
Reshape(int width, int height)
{
   glViewport(0, 0, (GLint)width, (GLint)height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
}


static void 
Key(unsigned char key, int x, int y)
{
   if (key == 'x') {
      xor = !xor;
      printf("XOR mode: %s\n", xor ? "on" : "off");
   }
   else if (key == 27)
      exit(0);
   glutPostRedisplay();
}


static void
Draw(void)
{
   glClear(GL_COLOR_BUFFER_BIT); 

   glLogicOp(GL_XOR);
   if (xor)
      glEnable(GL_COLOR_LOGIC_OP);
   else
      glDisable(GL_COLOR_LOGIC_OP);

   glColor3f(0, 1, 0);

   /* outer line rect */
   glBegin(GL_LINE_LOOP);
   glVertex2f(-0.9, -0.9);
   glVertex2f( 0.9, -0.9);
   glVertex2f( 0.9,  0.9);
   glVertex2f(-0.9,  0.9);
   glEnd();

   /* middle line rect */
   glBegin(GL_LINE_STRIP);
   glVertex2f(-0.8, -0.8);
   glVertex2f( 0.8, -0.8);
   glVertex2f( 0.8,  0.8);
   glVertex2f(-0.8,  0.8);
   glVertex2f(-0.8, -0.8);
   glEnd();

   /* inner line rect */
   glBegin(GL_LINES);
   glVertex2f(-0.7, -0.7);
   glVertex2f( 0.7, -0.7);

   glVertex2f( 0.7, -0.7);
   glVertex2f( 0.7,  0.7);

   glVertex2f( 0.7,  0.7);
   glVertex2f(-0.7,  0.7);

   glVertex2f(-0.7,  0.7);
   glVertex2f(-0.7, -0.7);
   glEnd();

   /* inner + pattern */
   glBegin(GL_LINES);
   glVertex2f(-0.6,  0.0);
   glVertex2f( 0.6,  0.0);
   glVertex2f( 0.0,  0.6);
   glVertex2f( 0.0, -0.6);
   glEnd();


   glutSwapBuffers();
}


int
main(int argc, char **argv)
{
   glutInit(&argc, argv);

   glutInitWindowSize(250, 250);

   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   glutCreateWindow(argv[0]);
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Draw);

   fprintf(stderr, "GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
   fprintf(stderr, "GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
   fprintf(stderr, "GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
   fprintf(stderr, "NOTE: there should be no pixels missing at the corners"
           " of the line loops.\n");
   fprintf(stderr, "There should be a missing pixel at the center of the '+'.\n");
   fprintf(stderr, "Resize the window to check for any pixel drop-outs.\n");
   fprintf(stderr, "Press 'x' to toggle XOR mode on/off.\n");

   glutMainLoop();

   return 0;
}
