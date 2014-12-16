/*
 * Copyright (c) 1991, 1992, 1993 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF
 * ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "glut_wrap.h"


#define CI_OFFSET_1 16
#define CI_OFFSET_2 32


GLenum doubleBuffer = 1;
int win;

static void Init(void)
{
   fprintf(stderr, "GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
   fprintf(stderr, "GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
   fprintf(stderr, "GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
   fflush(stderr);

   glClearColor(0.3, 0.1, 0.3, 0.0);
}

static void Reshape(int width, int height)
{
   glViewport(0, 0, (GLint)width, (GLint)height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
   glMatrixMode(GL_MODELVIEW);
}

static void Clear(void)
{
   fprintf(stderr, "::clearing\n");
   fflush(stderr);

   glClear(GL_COLOR_BUFFER_BIT); 
}

static void DrawTri(void)
{
   fprintf(stderr, "::drawing - tri\n");
   fflush(stderr);

   glBegin(GL_TRIANGLES);
   glColor3f(.8,0,0); 
   glVertex3f(-0.9, -0.9, -30.0);
   glColor3f(0,.9,0); 
   glVertex3f( 0.9, -0.9, -30.0);
   glColor3f(0,0,.7); 
   glVertex3f( 0.0,  0.9, -30.0);
   glEnd();
}

static void DrawClipTri(void)
{
   fprintf(stderr, "::drawing - tri clipped\n");
   fflush(stderr);

   glBegin(GL_TRIANGLES);
   glColor3f(.8,0,0); 
   glVertex3f(-0.9,  0.9, -30.0);
   glColor3f(0,.9,0); 
   glVertex3f( 0.9,  0.9, -30.0);
   glColor3f(0,0,.7); 
   glVertex3f( 0.0, -1.9, -30.0);
   glEnd();
}

static void Swap(void)
{
   fprintf(stderr, "::swapping\n");
   fflush(stderr);

   glutSwapBuffers();
}

static void Flush(void)
{
   fprintf(stderr, "::flush\n");
   fflush(stderr);

   glFlush();
}

static void Key(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
         glutDestroyWindow(win);
         exit(0);
      case 't':
         DrawTri();
         break;
      case 'T':
         DrawClipTri();
         break;
      case 'f':
         Flush();
         break;
      case 's':
         Swap();
         Clear();
         break;
      default:
         return;
   }
}

static void Draw(void)
{
   Clear();
   DrawTri();
   DrawClipTri();
   Swap();
   Clear();
}

static void
Usage(void)
{
   printf("usage:\n");
   printf("  t/T    draw a triangle (caps for clipped)\n");
   printf("  f      flush\n");
   printf("  s      swap\n");
}

int main(int argc, char **argv)
{
   GLenum type;

   glutInit(&argc, argv);

   glutInitWindowPosition(0, 0);
   glutInitWindowSize(250, 250);

   type = GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE;
   glutInitDisplayMode(type);

   win = glutCreateWindow(*argv);
   if (!win) {
      exit(1);
   }

   Init();

   Usage();

   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Draw);
   glutMainLoop();
   return 0;
}
