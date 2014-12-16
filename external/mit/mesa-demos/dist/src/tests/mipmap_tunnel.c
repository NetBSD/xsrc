/**
 * Display trilinear mipmap filtering quality.
 * We look down a long tunnel shape which has a mipmapped texture
 * applied to it.  Ideally, the transition from one mipmap level to
 * another should be nice and regular/circular.
 * This sort of test is frequently seen in online articles about GPU
 * texture filtering.
 *
 * Brian Paul
 * 13 Oct 2010
 */


#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include "glut_wrap.h"


static GLfloat LodBias = 0.0;
static GLboolean NearestFilter = GL_FALSE;
static GLfloat Zpos = -10.0, Zrot = 0.0;
static GLuint TexObj;
static GLboolean HaveAniso;
static GLfloat AnisoMax = 1.0, MaxAnisoMax = 8.0;

#define TEX_SIZE 1024


/** Make a solid-colored texture image */
static void
MakeImage(int level, int width, int height, const GLubyte color[4])
{
   const int makeStripes = 0;
   GLubyte img[TEX_SIZE * TEX_SIZE * 3];
   int i, j;
   for (i = 0; i < height; i++) {
      for (j = 0; j < width; j++) {
         int k = (i * width + j) * 3;
         int p = (i / 8) & makeStripes;
         if (p == 0) {
            img[k + 0] = color[0];
            img[k + 1] = color[1];
            img[k + 2] = color[2];
         }
         else {
            img[k + 0] = 0;
            img[k + 1] = 0;
            img[k + 2] = 0;
         }
      }
   }

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, width, height, 0,
                GL_RGB, GL_UNSIGNED_BYTE, img);
}


/** Make a mipmap in which each level is a different, solid color */
static void
MakeMipmap(void)
{
   static const GLubyte colors[12][3] = {
      {255, 0, 0},
      {0, 255, 0},
      {0, 0, 255},
      {0, 255, 255},
      {255, 0, 255},
      {255, 255, 0},
      {255, 0, 0},
      {0, 255, 0},
      {0, 0, 255},
      {0, 255, 255},
      {255, 0, 255},
      {255, 255, 0},
   };
   int i, sz = TEX_SIZE;

   for (i = 0; sz > 0; i++) {
      MakeImage(i, sz, sz, colors[i]);
      printf("Level %d size: %d x %d\n", i, sz, sz);
      sz /= 2;
   }
}


static void
Init(void)
{
   glClearColor(.5, .5, .5, .5);

   glGenTextures(1, &TexObj);
   glBindTexture(GL_TEXTURE_2D, TexObj);
   MakeMipmap();

   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
   printf("GL_VERSION = %s\n", (char *) glGetString(GL_VERSION));

   HaveAniso = glutExtensionSupported("GL_EXT_texture_filter_anisotropic");
   if (HaveAniso) {
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisoMax);
      printf("GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = %f\n", MaxAnisoMax);
   }
}


static void
DrawTunnel(void)
{
   const float radius = 10.0, height = 500.0;
   const int slices = 24, stacks = 52;
   const float bias = 0.995;
   GLUquadric *q = gluNewQuadric();

   glPushMatrix();
      glRotatef(180, 1, 0, 0);
      glEnable(GL_TEXTURE_2D);
      gluQuadricTexture(q, GL_TRUE);
      gluCylinder(q, radius, radius, height, slices, stacks);

      glDisable(GL_TEXTURE_2D);
      glColor3f(0, 0, 0);
      gluQuadricDrawStyle(q, GLU_LINE);
      gluCylinder(q, bias*radius, bias*radius, height/4, slices, stacks/4);
   glPopMatrix();

   gluDeleteQuadric(q);
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
Display(void)
{
   char str[100];

   glBindTexture(GL_TEXTURE_2D, TexObj);

   if (NearestFilter) {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_NEAREST_MIPMAP_NEAREST);
   }
   else {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
   }

   if (HaveAniso) {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, AnisoMax);
   }

   glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, LodBias);

   glClear(GL_COLOR_BUFFER_BIT);

   glPushMatrix();
      glTranslatef(0.0, 0.0, Zpos);
      glRotatef(Zrot, 0, 0, 1);
      DrawTunnel();
   glPopMatrix();

   glColor3f(1, 1, 1);
   glWindowPos2i(10, 10);
   if (HaveAniso)
      sprintf(str, "LOD bias (b/B): %.3f  MaxAnisotropy: %.3f", LodBias, AnisoMax);
   else
      sprintf(str, "LOD bias (b/B): %.3f", LodBias);
   PrintString(str);

   glutSwapBuffers();
}


static void
Reshape(int w, int h)
{
   glViewport(0, 0, w, h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(80.0, 1.0 * (GLfloat) w / (GLfloat) h, 1.0, 3000.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


static void
Key(unsigned char k, int x, int y)
{
   (void) x;
   (void) y;
   switch (k) {
   case 'a':
      AnisoMax -= 0.25;
      if (AnisoMax <= 1.0)
         AnisoMax = 1.0;
      break;
   case 'A':
      AnisoMax += 0.25;
      if (AnisoMax > MaxAnisoMax)
         AnisoMax = MaxAnisoMax;
      break;
   case 'b':
      LodBias -= 0.125;
      break;
   case 'B':
      LodBias += 0.125;
      break;
   case 'f':
      NearestFilter = !NearestFilter;
      break;
   case 'r':
      Zrot--;
      break;
   case 'R':
      Zrot++;
      break;
   case 'z':
      Zpos--;
      break;
   case 'Z':
      Zpos++;
      break;
   case 27:
      exit(0);
      break;
   default:
      return;
   }
   glutPostRedisplay();
}


static void
Usage(void)
{
   printf("Keys:\n");
   printf("  b/B    decrease/increase GL_TEXTURE_LOD_BIAS\n");
   printf("  f      toggle nearest/linear filtering\n");
   printf("  r/R    rotate tunnel\n");
}


int
main(int argc, char **argv)
{
   glutInitWindowSize(600, 600);
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc(Reshape);
   glutDisplayFunc(Display);
   glutKeyboardFunc(Key);
   Init();
   Usage();
   glutMainLoop();
   return 0;
}
