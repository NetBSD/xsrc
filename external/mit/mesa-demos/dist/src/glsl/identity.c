/**
 * Test very basic glsl functionality (identity vertex and fragment shaders).
 * Brian Paul & Stephane Marchesin
 */


#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"


static GLuint fragShader;
static GLuint vertShader;
static GLuint program;
static GLint win = 0;


static void
Redisplay(void)
{
   glClear(GL_COLOR_BUFFER_BIT);

   glBegin(GL_TRIANGLES);
   glVertex3f(-0.9, -0.9, 0.0);
   glVertex3f( 0.9, -0.9, 0.0);
   glVertex3f( 0.0,  0.9, 0.0);
   glEnd();

   glutSwapBuffers();
}


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
CleanUp(void)
{
   glDeleteShader(fragShader);
   glDeleteShader(vertShader);
   glDeleteProgram(program);
   glutDestroyWindow(win);
}


static void
Key(unsigned char key, int x, int y)
{
   if (key == 27) {
      CleanUp();
      exit(0);
   }
   glutPostRedisplay();
}


static void
Init(void)
{
   static const char *fragShaderText =
      "void main() {\n"
      "   gl_FragColor = vec4(1.0,0.0,0.0,1.0);\n"
      "}\n";
   static const char *vertShaderText =
      "void main() {\n"
      "   gl_Position = gl_Vertex;\n"
      "}\n";

   if (!ShadersSupported())
      exit(1);

   fragShader = CompileShaderText(GL_FRAGMENT_SHADER, fragShaderText);

   vertShader = CompileShaderText(GL_VERTEX_SHADER, vertShaderText);

   program = LinkShaders(vertShader, fragShader);

   glUseProgram(program);

   assert(glGetError() == 0);

   glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

   printf("GL_RENDERER = %s\n",(const char *) glGetString(GL_RENDERER));

   assert(glIsProgram(program));
   assert(glIsShader(fragShader));
   assert(glIsShader(vertShader));

   glColor3f(1, 0, 0);
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(200, 200);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   win = glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Redisplay);
   Init();
   glutMainLoop();
   return 0;
}
