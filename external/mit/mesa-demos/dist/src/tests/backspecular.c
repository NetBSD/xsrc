#include <stdlib.h>
#include <GL/glew.h>
#include "glut_wrap.h"


static const float black[4]    = { 0, 0, 0, 1 };
static const float white[4]    = { 1, 1, 1, 1 };
static const float diffuse[4]  = { 1, 0, 0, 1 };
static const float diffuseb[4] = { 0, 1, 0, 1 };
static const float specular[4] = { 0.5, 0.5, 0.5, 1 };
static const float ambient[4]  = { 0.2, 0.2, 0.2, 1 };
static const float lightpos[4] = { 0, 0, 10, 0 };
static const float shininess = 50;

static double angle = 0.0;

static int autorotate = 1;
static double angle_delta = 1.0;
static int timeout = 10;


static void
display(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glPushMatrix();
   glRotated(angle, 0.0, 1.0, 0.0);

   glBegin(GL_QUADS);
   glColor3f(1, 1, 1);
   glNormal3d(0, 0, 1);
   glVertex3d(-1, -1, 0);
   glVertex3d(+1, -1, 0);
   glVertex3d(+1, +1, 0);
   glVertex3d(-1, +1, 0);
   glEnd();

   glPopMatrix();

   glutSwapBuffers();
}


static void
timer(int value)
{
   angle += angle_delta;
   glutTimerFunc(timeout, timer, 0);
   glutPostRedisplay();
}


static void
key(unsigned char key, int x, int y)
{
   if (key == 27) {
      exit(0);
   }
}


int
main(int argc, char **argv)
{
   /* init glut */
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutCreateWindow("Backface specular test");
   glutDisplayFunc(display);
   glutKeyboardFunc(key);
   if (autorotate)
      glutTimerFunc(timeout, timer, 0);

   /* setup lighting */
   glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
   glMaterialfv(GL_BACK,  GL_DIFFUSE, diffuseb);

   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  black);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  specular);
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   ambient);
   glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shininess);

   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
   glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

   glLightfv(GL_LIGHT0, GL_AMBIENT,  black);
   glLightfv(GL_LIGHT0, GL_DIFFUSE,  white);
   glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
   glLightfv(GL_LIGHT0, GL_SPECULAR, white);

   glDisable(GL_CULL_FACE);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   /* setup camera */
   glMatrixMode(GL_PROJECTION);
   gluPerspective(30.0, 1.0, 1.0, 10.0);
   glMatrixMode(GL_MODELVIEW);
   gluLookAt(0.0, 0.0, 5.0,
             0.0, 0.0, 0.0,
             0.0, 1.0, 0.0);

   /* setup misc */
   glEnable(GL_DEPTH_TEST);
   glClearColor(0, 0, 1, 1);

   /* run */
   glutMainLoop();

   return 0;
}
