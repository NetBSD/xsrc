/* test texturing + stippling */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "glut_wrap.h"


static GLubyte fly[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x80, 0x01, 0xC0, 0x06, 0xC0, 0x03, 0x60, 0x04, 0x60, 0x06, 0x20,
0x04, 0x30, 0x0C, 0x20, 0x04, 0x18, 0x18, 0x20, 0x04, 0x0C, 0x30, 0x20,
0x04, 0x06, 0x60, 0x20, 0x44, 0x03, 0xC0, 0x22, 0x44, 0x01, 0x80, 0x22,
0x44, 0x01, 0x80, 0x22, 0x44, 0x01, 0x80, 0x22, 0x44, 0x01, 0x80, 0x22,
0x44, 0x01, 0x80, 0x22, 0x44, 0x01, 0x80, 0x22, 0x66, 0x01, 0x80, 0x66,
0x33, 0x01, 0x80, 0xCC, 0x19, 0x81, 0x81, 0x98, 0x0C, 0xC1, 0x83, 0x30,
0x07, 0xe1, 0x87, 0xe0, 0x03, 0x3f, 0xfc, 0xc0, 0x03, 0x31, 0x8c, 0xc0,
0x03, 0x33, 0xcc, 0xc0, 0x06, 0x64, 0x26, 0x60, 0x0c, 0xcc, 0x33, 0x30,
0x18, 0xcc, 0x33, 0x18, 0x10, 0xc4, 0x23, 0x08, 0x10, 0x63, 0xC6, 0x08,
0x10, 0x30, 0x0c, 0x08, 0x10, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00, 0x08};


static void
Init(void)
{
   fprintf(stderr, "GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
   fprintf(stderr, "GL_VERSION  = %s\n", (char *) glGetString(GL_VERSION));
   fprintf(stderr, "GL_VENDOR   = %s\n", (char *) glGetString(GL_VENDOR));
   fflush(stderr);

   glClearColor(0.0, 0.0, 1.0, 1.0);

#define SIZE 32
   {
      GLubyte tex2d[SIZE][SIZE][3];
      GLint s, t;

      for (s = 0; s < SIZE; s++) {
	 for (t = 0; t < SIZE; t++) {
            tex2d[t][s][0] = s*255/(SIZE-1);
            tex2d[t][s][1] = t*255/(SIZE-1);
            tex2d[t][s][2] = 0;
	 }
      }

      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, 3, SIZE, SIZE, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, tex2d);
   }

   glEnable (GL_POLYGON_STIPPLE);
   glPolygonStipple (fly);
}


static void
Reshape(int width, int height)
{
   glViewport(0, 0, (GLint)width, (GLint)height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
}


static void
Key(unsigned char key, int x, int y)
{
   if (key == 27)
      exit(1);

   glutPostRedisplay();
}


static void
Draw(void)
{
   glClear(GL_COLOR_BUFFER_BIT);

   glBegin(GL_TRIANGLES);
   glTexCoord2f(1,-1);
   glVertex3f(0.9, -0.9, -0.0);
   glTexCoord2f(1,1);
   glVertex3f(0.9,  0.9, -0.0);
   glTexCoord2f(-1,0);
   glVertex3f(-0.9,  0.0, -0.0);
   glEnd();

   glutSwapBuffers();
}


int
main(int argc, char **argv)
{
   glutInit(&argc, argv);
   glutInitWindowPosition(0, 0);
   glutInitWindowSize(250, 250);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

   if (glutCreateWindow(*argv) == GL_FALSE) {
      exit(1);
   }

   Init();

   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Draw);
   glutMainLoop();

   return 0;
}
