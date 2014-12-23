/**
 * Test some hacks to approximate wide lines, AA lines and stippled lines
 * by drawing the lines multiple times at offsets and stippling with a fragment
 * shader.
 *
 * Brian Paul
 * June 2011
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"


static int Win;
static int WinWidth = 1000, WinHeight = 500;
static GLboolean Anti = GL_FALSE, Stipple = GL_FALSE;
static GLfloat LineWidth = 1.0, MaxLineWidth = 3.0;
static GLuint StippleProgram;


/**
 * Generate a 2D texture stipple pattern from 1D line stipple pattern.
 * Note: the stipple repeat factor could be implemented by scaling the
 * texcoords in the frag shader.
 */
static GLuint
LineStippleToTexture(GLushort pattern)
{
   GLuint tex, i, j;
   GLubyte pattern2d[16][16];

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);

   for (i = 0; i < 16; i++) {
      for (j = 0; j < 16; j++) {
         GLuint ibit = (pattern >> i) & 1;
         GLuint jbit = (pattern >> j) & 1;
         pattern2d[i][j] = (ibit | jbit) * 255;
      }
   }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 16, 16, 0,
                GL_LUMINANCE, GL_UNSIGNED_BYTE, pattern2d);

   return tex;
}


/** Draw some lines */
static void
DrawLines(void)
{
   const float r0 = 0.2, r1 = 0.9;
   float a;

   glBegin(GL_LINES);
   for (a = 0.0; a < 2.0*M_PI; a += 0.1) {
      float x0 = r0 * cos(a), y0 = r0 * sin(a);
      float x1 = r1 * cos(a), y1 = r1 * sin(a);
      glVertex2f(x0, y0);
      glVertex2f(x1, y1);
   }
   glEnd();
   glBegin(GL_LINE_LOOP);
   for (a = 0.0; a <= 2.0*M_PI; a += 0.1) {
      float x1 = r1 * cos(a), y1 = r1 * sin(a);
      glVertex2f(x1, y1);
   }
   glEnd();
   glBegin(GL_LINE_LOOP);
   for (a = 0.0; a <= 2.0*M_PI; a += 0.1) {
      float x0 = r0 * cos(a), y0 = r0 * sin(a);
      glVertex2f(x0, y0);
   }
   glEnd();
   glBegin(GL_LINE_LOOP);
   glVertex2f(-r1, -r1);
   glVertex2f(+r1, -r1);
   glVertex2f(+r1, +r1);
   glVertex2f(-r1, +r1);
   glEnd();
}


static void
Draw(void)
{
   GLint vpWidth = WinWidth / 2;
   GLint vpHeight = WinHeight;

   glClear(GL_COLOR_BUFFER_BIT);

   /* draw regular lines */
   {
      if (Anti) {
         glEnable(GL_LINE_SMOOTH);
         glEnable(GL_BLEND);
      }
      if (Stipple)
         glEnable(GL_LINE_STIPPLE);
      glLineWidth(LineWidth);

      glViewport(0, 0, vpWidth, vpHeight);
      glColor3f(1, 1, 1);
      DrawLines();

      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_LINE_STIPPLE);
      glDisable(GL_BLEND);
      glLineWidth(1.0);
   }

   /* draw hacked lines */
   {
#define ALPHA 0.5
      static const float offsets_alpha[5][3] = {
         { 0.0, 0.0, 1.0 },
#if 0
         { 1.0, 0.5, ALPHA },
         { -1.0, -0.5, ALPHA },
         { -0.5, 1.0, ALPHA },
         { 0.5, -1.0, ALPHA }
#else
         { 1.0, 0.0, ALPHA },
         { -1.0, 0.0, ALPHA },
         { 0.0, 1.0, ALPHA },
         { 0.0, -1.0, ALPHA }
#endif
      };

      int passes, pass;
      float s;

      glViewport(vpWidth, 0, vpWidth, vpHeight);

      if (Anti || LineWidth > 1.5) {
         passes = 5;
      }
      else {
         passes = 1;
      }
      if (Anti) 
         s = 2.0 * LineWidth / MaxLineWidth;
      else
         s = 1.5 * LineWidth / MaxLineWidth;

      if (Stipple) {
         glUseProgram(StippleProgram);
      }
      if (Anti) {
         glEnable(GL_BLEND);
      }

      for (pass = 0; pass < passes; pass++) {
         /* Translate all vertices by small x/y offset */
         float tx = offsets_alpha[pass][0] / vpWidth * s;
         float ty = offsets_alpha[pass][1] / vpHeight * s;
         /* adjust fragment alpha (this could be done in many ways) */
         glColor4f(1, 1, 1, offsets_alpha[pass][2]);

         glPushMatrix();
         glTranslatef(tx, ty, 0.0);
            DrawLines();
         glPopMatrix();
      }

      if (1) {
         /* debug: show the 2D texture / stipple pattern */
         GLfloat w = 32.0 / (vpWidth);
         GLfloat h = 32.0 / (vpHeight);

         glColor3f(1, 1, 1);
         glBegin(GL_QUADS);
         glVertex2f(0.0, 0.0);
         glVertex2f(w, 0.0);
         glVertex2f(w, h);
         glVertex2f(0.0, h);
         glEnd();
      }

      glUseProgram(0);
      glDisable(GL_BLEND);

   }

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
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


static void
Key(unsigned char key, int x, int y)
{
   (void) x;
   (void) y;
   switch (key) {
   case 'a':
      Anti = !Anti;
      break;
   case 's':
      Stipple = !Stipple;
      break;
   case 'w':
      LineWidth -= 0.25;
      if (LineWidth < 1.0)
         LineWidth = 1.0;
      break;
   case 'W':
      LineWidth += 0.25;
      if (LineWidth > MaxLineWidth)
         LineWidth = MaxLineWidth;
      break;
   case 27:
      glutDestroyWindow(Win);
      exit(0);
      break;
   }
   printf("Stipple: %d  Antialias: %d  LineWidth: %f\n",
          Stipple, Anti, LineWidth);

   glutPostRedisplay();
}


static void
SpecialKey(int key, int x, int y)
{
   glutPostRedisplay();
}


static GLuint
MakeFragmentShader(void)
{
   static const char *fragShaderText =
      "uniform sampler2D stippleTex; \n"
      "uniform vec2 viewportSize; \n"
      "float scale = 1.0 / 16.0; \n"
      "void main() \n"
      "{ \n"
      "   vec2 t = gl_FragCoord.xy * scale; \n"
      "   gl_FragColor = gl_Color * texture2D(stippleTex, t); \n"
      "} \n";

   GLuint fragShader, program;
   GLint stippleTex, viewportSize;

   fragShader = CompileShaderText(GL_FRAGMENT_SHADER, fragShaderText);
   program = LinkShaders(0, fragShader);
   glUseProgram(program);

   stippleTex = glGetUniformLocation(program, "stippleTex");
   glUniform1i(stippleTex, 0); /* unit 0 */

   viewportSize = glGetUniformLocation(program, "viewportSize");
   glUniform2f(viewportSize, WinWidth / 2.0, WinHeight);

   glUseProgram(0);

   return program;
}


static void
Usage(void)
{
   printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
   printf("Keys:\n");
   printf("  a - toggle antialiasing\n");
   printf("  s - toogle stippling\n");
   printf("  w/W - decrease/increase line width\n");
}


static void
Init(void)
{
   GLushort pattern = 0xf2;
   GLuint tex;

   if (!ShadersSupported())
      exit(1);

   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glLineStipple(1, pattern);

   StippleProgram = MakeFragmentShader();

   tex = LineStippleToTexture(pattern);
   glBindTexture(GL_TEXTURE_2D, tex);
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(WinWidth, WinHeight);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   Win = glutCreateWindow(argv[0]);
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutSpecialFunc(SpecialKey);
   glutDisplayFunc(Draw);
   glewInit();
   Usage();
   Init();
   glutMainLoop();
   return 0;
}
