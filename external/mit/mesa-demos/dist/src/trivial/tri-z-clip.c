/** Test Z-axis clipping of a triangle */


#include <stdio.h>
#include <stdlib.h>
#include "glut_wrap.h"


static GLboolean ztest = GL_FALSE;


static void
Reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, 2.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(0, 0, -6.0);
}


static void
Key(unsigned char key, int x, int y)
{
   if (key == 'z') {
      ztest = !ztest;
      printf("ztest = %d\n", ztest);
      fflush(stdout);
   }
   else if (key == 27) {
      exit(1);
   }
   glutPostRedisplay();
}


static void
Draw(void)
{
   glClearColor(0.5, 0.5, 0.5, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

   if (ztest)
      glEnable(GL_DEPTH_TEST);
   else
      glDisable(GL_DEPTH_TEST);

   glBegin(GL_TRIANGLES);
   glColor3f(0, 0, 1); 
   glVertex3f(-0.5, 0.7, -10.0);
   glColor3f(1, 0, 0); 
   glVertex3f(-0.5, -0.7, -10.0);
   glColor3f(0, 1, 0); 
   glVertex3f(1.0,  0.0, 12.0);
   glEnd();

   glutSwapBuffers();
}


int
main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(250, 250);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow(*argv);

    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Key);
    glutDisplayFunc(Draw);
    glutMainLoop();
    return 0;
}
