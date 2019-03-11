/**
 * Draw stencil region
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"

static int Win;
static int WinWidth = 600, WinHeight = 200;
static GLfloat Xrot = 0.0, Yrot = 0.0;

#define WIDTH 256
#define HEIGHT 150

static GLubyte Image[HEIGHT][WIDTH];


static void
DrawStencilPixels(GLint x, GLint y, GLsizei w, GLsizei h,
                  const GLubyte *stencil)
{
   /* This program is run eight times, once for each stencil bit.
    * The stencil values to draw are found in an 8-bit alpha texture.
    * We read the texture/stencil value and test if bit 'b' is set.
    * If the bit is set, result.A will be non-zero.  Finally, use
    * alpha test and stencil test to update the stencil buffer.
    *
    * The basic algorithm for checking if a bit is set is:
    *   if (is_odd(value / (1 << bit)))
    *      result is one (or non-zero).
    *   else
    *      result is zero.
    * The program parameter contains two values:
    *   parm.x = 255 / (1 << bit)
    *   parm.y = 0.5
    */
   static const char *program =
      "!!ARBfp1.0\n"
      "PARAM parm = program.local[0]; \n"
      "TEMP t; \n"
      "TEX t, fragment.texcoord[0], texture[0], RECT; \n"
      "# t = t * 255 / bit \n"
      "MUL t.x, t.a, parm.x; \n"
      "# t = (int) t \n"
      "FRC t.y, t.x; \n"
      "SUB t.x, t.x, t.y; \n"
      "# t = 5 * 0.5 \n"
      "MUL t.x, t.x, parm.y; \n"
      "# alpha = frac(t) \n"
      "FRC result.color, t.x; \n"
      "END \n";
   GLuint prog;
   GLint bit;

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_ALPHA, w, h, 0,
                GL_ALPHA, GL_UNSIGNED_BYTE, stencil);
   glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   glGenProgramsARB(1, &prog);
   glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, prog);
   glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                      strlen(program), (const GLubyte *) program);
   glEnable(GL_FRAGMENT_PROGRAM_ARB);

   glPushMatrix();
   glTranslatef(x, y, 0);

   glEnable(GL_ALPHA_TEST);
   glAlphaFunc(GL_GREATER, 0.0);

   glEnable(GL_STENCIL_TEST);
   glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

   for (bit = 0; bit < 8; bit++) {
      GLuint mask = 1 << bit;
      glStencilFunc(GL_ALWAYS, 255/*mask*/, mask);
      glStencilMask(mask);

      glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0,
                                   // 1.0 / bit, 0.5, 0.0, 0.0);
                                   255.0 / mask, 0.5, 0.0, 0.0);


      glBegin(GL_TRIANGLE_FAN);
      glTexCoord2f(0, 0);  glVertex2f(0, 0);
      glTexCoord2f(w, 0);  glVertex2f(w, 0);
      glTexCoord2f(w, h);  glVertex2f(w, h);
      glTexCoord2f(0, h);  glVertex2f(0, h);
      glEnd();
   }

   glPopMatrix();

   glDisable(GL_FRAGMENT_PROGRAM_ARB);
   glDisable(GL_ALPHA_TEST);
   glDisable(GL_STENCIL_TEST);
}



static void
Draw(void)
{
   GLint x0 = 5, y0= 5, x1 = 10 + WIDTH, y1 = 5;
   GLubyte tmp[HEIGHT][WIDTH];
   GLint max, i, j;

   glClearColor(0.2, 0.2, 0.8, 0.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   if (0) {
      DrawStencilPixels(x0, y0, WIDTH, HEIGHT, (GLubyte*) Image);
   } else {
      glWindowPos2i(x0, y0);
      glDrawPixels(WIDTH, HEIGHT, GL_STENCIL_INDEX,
                   GL_UNSIGNED_BYTE, (GLubyte*) Image);
   }

   glReadPixels(x0, y0, WIDTH, HEIGHT, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, tmp);

   max = 0;
   for (i = 0; i < HEIGHT; i++) {
      for (j = 0; j < WIDTH; j++) {
         if (tmp[i][j] > max)
            max = tmp[i][j];
      }
   }
   printf("max = %d\n", max);

   glWindowPos2i(x1, y1);
   glDrawPixels(WIDTH, HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, tmp);

   glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
   WinWidth = width;
   WinHeight = height;
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
   case GLUT_KEY_LEFT:
      Yrot -= step;
      break;
   case GLUT_KEY_RIGHT:
      Yrot += step;
      break;
   }
   glutPostRedisplay();
}


static void
Init(void)
{
   int i, j;
   for (i = 0; i < HEIGHT; i++) {
      for (j = 0; j < WIDTH; j++) {
         Image[i][j] = j;
      }
   }
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(WinWidth, WinHeight);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_STENCIL);
   Win = glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutSpecialFunc(SpecialKey);
   glutDisplayFunc(Draw);
   Init();
   glutMainLoop();
   return 0;
}
