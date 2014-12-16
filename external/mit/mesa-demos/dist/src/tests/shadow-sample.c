#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"

#define windowSize 100
GLfloat tolerance[5];
struct ShaderProgram
{
   const char *name;
   const char *fragShaderString;
   GLfloat expectedColor[4];
} Programs[] = {
   {
      "shadow2D(): 1", "uniform sampler2DShadow texZ; \n"
         "void main() { \n"
         "   vec3 coord = vec3(0.1, 0.1, 0.5); \n"
         "   // shadow map value should be 0.25 \n"
         "   gl_FragColor = shadow2D(texZ, coord) + vec4(0.25); \n"
         "   // 0.5 <= 0.25 ? color = 1 : 0\n" "} \n", {
   0.25, 0.25, 0.25, 1.0},}, {
      "shadow2D(): 2", "uniform sampler2DShadow texZ; \n"
         "void main() { \n"
         "   vec3 coord = vec3(0.1, 0.1, 0.2); \n"
         "   // shadow map value should be 0.25 \n"
         "   gl_FragColor = shadow2D(texZ, coord); \n"
         "   // 0.2 <= 0.25 ? color = 1 : 0\n" "} \n", {
   1.0, 1.0, 1.0, 1.0},}, {
      "shadow2D(): 3", "uniform sampler2DShadow texZ; \n"
         "void main() { \n"
         "   vec3 coord = vec3(0.9, 0.9, 0.95); \n"
         "   // shadow map value should be 0.75 \n"
         "   gl_FragColor = shadow2D(texZ, coord) + vec4(0.25); \n"
         "   // 0.95 <= 0.75 ? color = 1 : 0\n" "} \n", {
   0.25, 0.25, 0.25, 1.0},}, {
      "shadow2D(): 4", "uniform sampler2DShadow texZ; \n"
         "void main() { \n"
         "   vec3 coord = vec3(0.9, 0.9, 0.65); \n"
         "   // shadow map value should be 0.75 \n"
         "   gl_FragColor = shadow2D(texZ, coord); \n"
         "   // 0.65 <= 0.75 ? color = 1 : 0\n" "} \n", {
   1.0, 1.0, 1.0, 1.0}}, {
      NULL, NULL, {
0, 0, 0, 0}}};

static void
setupTextures(void)
{
   GLfloat teximageZ[16][16];
   GLint i, j;
   GLuint objZ;
   glGenTextures(1, &objZ);

   /* 2D GL_DEPTH_COMPONENT texture (for shadow sampler tests) */
   for (i = 0; i < 16; i++) {
      for (j = 0; j < 16; j++) {
         if (j < 8)
            teximageZ[i][j] = 0.25;

         else
            teximageZ[i][j] = 0.75;
      }
   }
   glBindTexture(GL_TEXTURE_2D, objZ);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 16, 16, 0,
                GL_DEPTH_COMPONENT, GL_FLOAT, teximageZ);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB,
                   GL_COMPARE_R_TO_TEXTURE_ARB);
}

static void
Init(void)
{

   /* check GLSL version */
   GLenum err;
   int bufferBits[5];
   const char *glslVersion =
      (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
   if (!glslVersion || glslVersion[0] != '1') {
      fprintf(stderr, "GLSL 1.x not supported\n");
      return;
   }
   setupTextures();
   err = glGetError();
   assert(!err);                /* should be OK */

   /* setup vertex transform (we'll draw a quad in middle of window) */
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-4.0, 4.0, -4.0, 4.0, 0.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glDrawBuffer(GL_FRONT);
   glReadBuffer(GL_FRONT);

   /* compute error tolerances (may need fine-tuning) */
   glGetIntegerv(GL_RED_BITS, &bufferBits[0]);
   glGetIntegerv(GL_GREEN_BITS, &bufferBits[1]);
   glGetIntegerv(GL_BLUE_BITS, &bufferBits[2]);
   glGetIntegerv(GL_ALPHA_BITS, &bufferBits[3]);
   glGetIntegerv(GL_DEPTH_BITS, &bufferBits[4]);
   tolerance[0] = 2.0 / (1 << bufferBits[0]);
   tolerance[1] = 2.0 / (1 << bufferBits[1]);
   tolerance[2] = 2.0 / (1 << bufferBits[2]);
   if (bufferBits[3])
      tolerance[3] = 2.0 / (1 << bufferBits[3]);

   else
      tolerance[3] = 1.0;
   if (bufferBits[4])
      tolerance[4] = 16.0 / (1 << bufferBits[4]);

   else
      tolerance[4] = 1.0;
}

static void
reportFailure(const char *programName, const GLfloat expectedColor[4],
              const GLfloat actualColor[4])
{
   fprintf(stdout, "FAILURE:\n");
   fprintf(stdout, "  Shader test: %s\n", programName);
   fprintf(stdout, "  Expected color: [%1.3f, %1.3f, %1.3f, %1.3f]\n",
           expectedColor[0], expectedColor[1], expectedColor[2],
           expectedColor[3]);
   fprintf(stdout, "  Observed color: [%1.3f, %1.3f, %1.3f, %1.3f]\n",
           actualColor[0], actualColor[1], actualColor[2], actualColor[3]);
} static GLboolean

equalColors(const GLfloat act[4], const GLfloat exp[4])
{
   const GLfloat *tol = tolerance;
   if ((fabsf(act[0] - exp[0]) > tol[0]) ||
       (fabsf(act[1] - exp[1]) > tol[1]) ||
       (fabsf(act[2] - exp[2]) > tol[2]) || (fabsf(act[3] - exp[3]) > tol[3]))
      return GL_FALSE;

   else
      return GL_TRUE;
}

static GLuint
loadAndCompileShader(GLenum target, const char *str)
{
   GLuint shader;
   shader = glCreateShader(target);
   glShaderSource(shader, 1, (const GLchar **) &str, NULL);
   glCompileShader(shader);
   return shader;
}

static GLboolean
checkCompileStatus(GLenum target, GLuint shader, struct ShaderProgram p)
{
   GLint stat;
   GLchar infoLog[1000];
   GLsizei len;
   glGetShaderiv(shader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      glGetShaderInfoLog(shader, 1000, &len, infoLog);
      fprintf(stderr, "FAILURE:\n");
      fprintf(stderr, "  Shader test: %s\n", p.name);
      if (target == GL_FRAGMENT_SHADER)
         fprintf(stderr, "Fragment shader did not compile:\n");

      else
         fprintf(stderr, "Vertex shader did not compile:\n");;
      fprintf(stderr, "%s\n", infoLog);
      return GL_FALSE;
   }
   return GL_TRUE;
}

static GLboolean
testProgram(struct ShaderProgram p)
{
   const GLfloat r = 0.62;      /* XXX draw 16x16 pixel quad */
   GLuint fragShader = 0, vertShader = 0, program = 0;
   GLint utexZ;
   GLboolean retVal = GL_FALSE;
   GLfloat pixel[4];
   if (p.fragShaderString) {
      fragShader =
         loadAndCompileShader(GL_FRAGMENT_SHADER, p.fragShaderString);
      if (!checkCompileStatus(GL_FRAGMENT_SHADER, fragShader, p)) {
         retVal = GL_FALSE;
         goto cleanup;
      }
   }
   if (!fragShader && !vertShader) {

      /* must have had a compilation errror */
      retVal = GL_FALSE;
      goto cleanup;
   }
   program = glCreateProgram();
   if (fragShader)
      glAttachShader(program, fragShader);
   if (vertShader)
      glAttachShader(program, vertShader);
   glLinkProgram(program);

   /* check link */
   {
      GLint stat;
      glGetProgramiv(program, GL_LINK_STATUS, &stat);
      if (!stat) {
         GLchar log[1000];
         GLsizei len;
         glGetProgramInfoLog(program, 1000, &len, log);
         fprintf(stderr, "FAILURE:\n");
         fprintf(stderr, "  Shader test: %s\n", p.name);;
         fprintf(stderr, "  Link error: ");;
         fprintf(stderr, "%s\n", log);
         retVal = GL_FALSE;
         goto cleanup;
      }
   }
   glUseProgram(program);

   /* load uniform vars */
   utexZ = glGetUniformLocation(program, "texZ");
   assert(utexZ >= 0);
   glUniform1i(utexZ, 0);       /* bind to tex unit 0 */

   /* to avoid potential issue with undefined result.depth.z */
   glDisable(GL_DEPTH_TEST);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* Counter Clockwise */
   glBegin(GL_POLYGON);
   glTexCoord2f(0, 0);
   glVertex2f(-r, -r);
   glTexCoord2f(1, 0);
   glVertex2f(r, -r);
   glTexCoord2f(1, 1);
   glVertex2f(r, r);
   glTexCoord2f(0, 1);
   glVertex2f(-r, r);
   glEnd();

   /* read a pixel from lower-left corder of rendered quad */
   glReadPixels(windowSize / 2 - 2, windowSize / 2 - 2, 1, 1, GL_RGBA,
                GL_FLOAT, pixel);
   if (!equalColors(pixel, p.expectedColor)) {
      reportFailure(p.name, p.expectedColor, pixel);
      retVal = GL_FALSE;
      goto cleanup;
   }

   /* passed! */
   retVal = GL_TRUE;

 cleanup:
   if (fragShader)
      glDeleteShader(fragShader);
   if (vertShader)
      glDeleteShader(vertShader);
   glDeleteProgram(program);
   return retVal;
}

static void
Display(void)
{
   int i, numPassed = 0, numFailed = 0;
   for (i = 0; Programs[i].name; i++) {
      if (testProgram(Programs[i])) {
         numPassed++;
      }

      else {
         numFailed++;
      }
      glFinish();
   }
   fprintf(stderr, "Total = %d. Passed = %d. Failed = %d\n",
           numPassed + numFailed, numPassed, numFailed);
}

static void
Reshape(int width, int height)
{
} static void

Key(unsigned char key, int x, int y)
{
   (void) x;
   (void) y;
   switch (key) {
   case 27:
      exit(0);
      break;
   }
   glutPostRedisplay();
}

int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowPosition(0, 0);
   glutInitWindowSize(windowSize, windowSize);
   glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_SINGLE | GLUT_ALPHA);
   glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Display);
   Init();
   glutMainLoop();
   return 0;
}
