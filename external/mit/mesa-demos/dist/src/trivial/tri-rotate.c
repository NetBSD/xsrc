/*
 * Draw a triangle with rotation
 */

#include <stdio.h>
#include <stdlib.h>
#include "glut_wrap.h"


static int win;
static float rot = 0.0f;


static void
Reshape(int width, int height)
{
   glViewport(0, 0, width, height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


static void
Key(unsigned char key, int x, int y)
{
   if (key == 27) {
      glutDestroyWindow(win);
      exit(0);
   }
   else if (key == 'r') {
      rot += 5.0f;
      glutPostRedisplay();
   }
   else if (key == 'R') {
      rot -= 5.0f;
      glutPostRedisplay();
   }
}


static void
Draw(void)
{
   glClear(GL_COLOR_BUFFER_BIT); 

   glPushMatrix();
   glRotatef(rot, 0.0, 0.0, 1.0f);

   glBegin(GL_TRIANGLES);
   glColor3f(1.0f, 0.0f, 0.0f); 
   glVertex3f(-0.8f, -0.8f, 0.0f);
   glColor3f(0.0f, 1.0f, 0.0f); 
   glVertex3f(0.8f, -0.8f, 0.0f);
   glColor3f(0.0f, 0.0f, 1.0f); 
   glVertex3f(0.0f, 0.8f, 0.0f);
   glEnd();

   glPopMatrix();

   glutSwapBuffers();
}


static void
Init(void)
{
   fprintf(stderr, "GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
   fprintf(stderr, "GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
   fprintf(stderr, "GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
   fflush(stderr);

   glClearColor(0.3, 0.3, 0.7, 0.0);
}


int
main(int argc, char **argv)
{
   glutInit(&argc, argv);

   if (argc > 1) {
      rot = atof(argv[1]);
      printf("Using rotation %g\n", rot);
      fflush(stdout);
   }

   glutInitWindowPosition(0, 0);
   glutInitWindowSize(250, 250);

   glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE);

   win = glutCreateWindow(*argv);
   if (!win) {
      exit(1);
   }

   Init();

   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Draw);
   glutMainLoop();
   return 0;
}
