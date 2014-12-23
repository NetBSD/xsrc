/**
 * Test conversion of a perspective-interpolated value to be a linearly
 * interpolated value.
 * Brian Paul
 * 22 March 2011
 */


#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"


static GLuint VertShader1, VertShader2;
static GLuint FragShader1, FragShader2;
static GLuint Program1, Program2;
static GLint Win = 0;
static int Width = 600, Height = 300;


static void
Redisplay(void)
{
   GLuint i;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* draw left and right perspective projected quads */
   for (i = 0; i < 2; i++) {
      glPushMatrix();

         if (i == 0) {
            glUseProgram(Program1);
            glViewport(0, 0, 300, 300);
         }
         else {
            glUseProgram(Program2);
            glViewport(300, 0, 300, 300);
         }

         glScalef(0.9, 0.9, 1.0);

         glBegin(GL_POLYGON);
         glTexCoord2f(0, 0);   glVertex3f(-1, -1, 0);
         glTexCoord2f(1, 0);   glVertex3f( 1, -1, 0);
         glTexCoord2f(1, 1);   glVertex3f( 1,  1, -12);
         glTexCoord2f(0, 1);   glVertex3f(-1,  1, -12);
         glEnd();

      glPopMatrix();
   }

   /* draw grayscale quad in middle */
   glUseProgram(0);
   glViewport(0, 0, Width, Height);
   glPushMatrix();
      glTranslatef(0.0, -0.38, 0);
      glScalef(0.05, 0.52, 1.0);
      glBegin(GL_POLYGON);
      glColor3f(0, 0, 0);   glVertex2f(-1, -1);
      glColor3f(0, 0, 0);   glVertex2f( 1, -1);
      glColor3f(1, 1, 1);   glVertex2f( 1,  1);
      glColor3f(1, 1, 1);   glVertex2f(-1,  1);
      glEnd();
   glPopMatrix();

   if (0) {
      GLfloat buf1[300*4], buf2[300*4];
      GLuint i;
      glReadPixels(Width * 1 / 4, 0, 1, Height, GL_RGBA, GL_FLOAT, buf1);
      glReadPixels(Width * 3 / 4, 0, 1, Height, GL_RGBA, GL_FLOAT, buf2);
      for (i = 1; i < Height; i++) {
         printf("row %d:  %f (delta %f)  %f (delta %f)\n",
                i,
                buf1[i*4], buf1[i*4] - buf1[i*4-4],
                buf2[i*4], buf2[i*4] - buf2[i*4-4]);
      }
   }

   glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-1, 1, -1, 1, 2, 200);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, -2);

   Width = width;
   Height = height;
}


static void
CleanUp(void)
{
   glDeleteShader(FragShader1);
   glDeleteShader(FragShader2);
   glDeleteShader(VertShader1);
   glDeleteShader(VertShader2);
   glDeleteProgram(Program1);
   glDeleteProgram(Program2);
   glutDestroyWindow(Win);
}


static void
Key(unsigned char key, int x, int y)
{
  (void) x;
  (void) y;

   switch(key) {
   case 27:
      CleanUp();
      exit(0);
      break;
   }
   glutPostRedisplay();
}


static void
Init(void)
{
   /**
    * Regular perspective interpolation
    */
   static const char *VertShaderText1 =
      "void main() {\n"
      "   vec4 pos = ftransform();\n"
      "   gl_TexCoord[0] = gl_MultiTexCoord0; \n"
      "   gl_Position = pos;\n"
      "}\n";

   static const char *FragShaderText1 =
      "void main() {\n"
      "   float gray = gl_TexCoord[0].y; \n"
      "   gl_FragColor = vec4(gray); \n"
      "}\n";

   /**
    * Perspective interpolation, converted to linear interpolation
    * (put window position W in texcoord.w)
    */
   static const char *VertShaderText2 =
      "void main() {\n"
      "   vec4 pos = ftransform();\n"
      "   gl_TexCoord[0] = gl_MultiTexCoord0 * pos.w; \n"
      "   gl_TexCoord[0].w = pos.w; \n"
      "   gl_Position = pos;\n"
      "}\n";

   static const char *FragShaderText2 =
      "void main() {\n"
      "   float gray = gl_TexCoord[0].y / gl_TexCoord[0].w; \n"
      "   gl_FragColor = vec4(gray); \n"
      "}\n";

   if (!ShadersSupported())
      exit(1);

   VertShader1 = CompileShaderText(GL_VERTEX_SHADER, VertShaderText1);
   VertShader2 = CompileShaderText(GL_VERTEX_SHADER, VertShaderText2);

   FragShader1 = CompileShaderText(GL_FRAGMENT_SHADER, FragShaderText1);
   FragShader2 = CompileShaderText(GL_FRAGMENT_SHADER, FragShaderText2);

   Program1 = LinkShaders(VertShader1, FragShader1);
   Program2 = LinkShaders(VertShader2, FragShader2);

   glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
   glEnable(GL_DEPTH_TEST);

   printf("GL_RENDERER = %s\n",(const char *) glGetString(GL_RENDERER));

   assert(glIsProgram(Program1));
   assert(glIsProgram(Program2));
   assert(glIsShader(VertShader1));
   assert(glIsShader(VertShader2));
   assert(glIsShader(FragShader1));
   assert(glIsShader(FragShader2));

   glColor3f(1, 0, 0);
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(Width, Height);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   Win = glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Redisplay);
   Init();
   glutMainLoop();
   return 0;
}
