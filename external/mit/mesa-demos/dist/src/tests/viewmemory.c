/**
 * Try to see other parts of the user's video memory by creating textures
 * with uninitialized contents.  Display those textures in the window.
 * Insprired by stories of WebGL security issues.
 *
 * The OpenGL driver should probably initialize all memory allocations
 * to zero (textures, renderbuffers, buffer objects, etc).
 *
 * Brian Paul
 * June 2011
 */

#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"

static int Win;
static int WinWidth = 1024, WinHeight = 512;
static GLboolean Anim = GL_FALSE;

static GLuint *Textures;
static GLuint TexWidth = 1024, TexHeight = 512;
static GLuint NumTextures = 50, CurTexture;


static void
Idle(void)
{
   static int prevTime = 0;
   int curTime = glutGet(GLUT_ELAPSED_TIME);

   if (!prevTime) {
      prevTime = curTime;
   }
   else {
      if (curTime - prevTime > 250) {
         prevTime = curTime;
         CurTexture = (CurTexture + 1) % NumTextures;
         glutPostRedisplay();
      }
   }
}


static void
MakeTextures(void)
{
   GLuint i;

   Textures = (GLuint *) malloc(NumTextures * sizeof(GLuint));

   glGenTextures(NumTextures, Textures);
   for (i = 0; i < NumTextures; i++) {
      glBindTexture(GL_TEXTURE_2D, Textures[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TexWidth, TexHeight, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   }
}


static void
PrintString(const char *s)
{
   while (*s) {
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, (int) *s);
      s++;
   }
}


static void
Draw(void)
{
   char s[100];

   sprintf(s, "Texture %u", CurTexture);

   glClear(GL_COLOR_BUFFER_BIT);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, CurTexture);

   glBegin(GL_POLYGON);
   glTexCoord2f(0, 0);  glVertex2f(0, 0);
   glTexCoord2f(1, 0);  glVertex2f(WinWidth, 0);
   glTexCoord2f(1, 1);  glVertex2f(WinWidth, WinHeight);
   glTexCoord2f(0, 1);  glVertex2f(0, WinHeight);
   glEnd();

   glDisable(GL_TEXTURE_2D);
   glColor3f(0, 1, 0);
   glWindowPos2iARB(10, 10);
   PrintString(s);

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
   glOrtho(0, WinWidth, 0, WinHeight, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


static void
Key(unsigned char key, int x, int y)
{
   switch (key) {
   case 'a':
   case 'A':
      Anim = !Anim;
      if (Anim)
         glutIdleFunc(Idle);
      else
         glutIdleFunc(NULL);
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
   (void) x;
   (void) y;
   switch (key) {
   case GLUT_KEY_LEFT:
      if (CurTexture > 0)
         CurTexture--;
      break;
   case GLUT_KEY_RIGHT:
      if (CurTexture + 1 < NumTextures)
         CurTexture++;
      break;
   }
   glutPostRedisplay();
}


static void
Init(void)
{
   printf("GL_RENDERER: %s\n", (char *) glGetString(GL_RENDERER));
   if (glutExtensionSupported("GL_ARB_robustness"))
      printf("GL_ARB_robustness supported\n");
   else
      printf("GL_ARB_robustness not supported\n");

   MakeTextures();
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   printf("Keys:\n");
   printf("  a: toggle animation\n");
   printf("  Left/Right: back / next frame\n");
   printf("  Esc: exit\n");
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);

   if (argc > 1) {
      NumTextures = atoi(argv[1]);
      if (NumTextures < 1) {
         printf("Invalid argument (number of textures)\n");
         return 1;
      }
   }
   printf("Creating %u textures\n", NumTextures);

   glutInitWindowSize(WinWidth, WinHeight);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   Win = glutCreateWindow(argv[0]);
   glewInit();
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
