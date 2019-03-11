/**
 * Test compilation, link, draw time for very large shaders.
 * Command line arguments:
 *  -v    verbose output (print shader code)
 *  -c N  generate shaders of complexity N
 *  -n N  generate and draw with N shader programs
 *
 * Brian Paul
 * 3 Dec 2015
 */


#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif


#define MAX_PROGRAMS 10000

static int Win;
static int WinWidth = 400, WinHeight = 400;
static int verbose = 0;
static int num_shaders = 1;
static int complexity = 5;
static int programs[MAX_PROGRAMS];

static const float u1_val[4] = {.1, .2, .3, .4};
static const float u2_val[4] = {.6, .7, .8, .9};

static const char *VS_code =
   "void main()\n"
   "{\n"
   "  gl_Position = ftransform();\n"
   "}\n";

static const float coords[3][4] = {
   {0, 0.1, 0, 1},
   {0, 0, 0, 1},
   {0, -0.1, 0, 1}
};

#define NUM_POINTS (sizeof(coords) / sizeof(coords[0]))



struct dynamic_string {
   char *buffer;
   unsigned len;
   unsigned buffer_size;
};


static void
append_string(struct dynamic_string *ds, const char *s)
{
   int l = strlen(s);
   if (ds->len + l >= ds->buffer_size) {
      /* grow buffer */
      int newsize = ds->buffer_size + l + 4096;
      char *newbuf = malloc(newsize);
      assert(newbuf);
      if (ds->buffer)
         strcpy(newbuf, ds->buffer);
      free(ds->buffer);
      ds->buffer = newbuf;
      ds->buffer_size = newsize;
   }
   strcpy(ds->buffer + ds->len, s);
   ds->len += l;
   assert(strlen(ds->buffer) == ds->len);
}


/**
 * Index is a term put into the shader code to make each shader a little
 * different.
 */
static char *
gen_large_shader(int num_functions, int index)
{
   int i;
   struct dynamic_string ds = {0};
   char s[100];

   append_string(&ds, "#version 120\n");
   append_string(&ds, "\nuniform vec4 u1, u2;\n\n");

   for (i = 0; i < num_functions; i++) {
      snprintf(s, sizeof(s), "vec4 func%d(vec4 a, float b)\n", i);
      append_string(&ds, s);
      append_string(&ds, "{\n");
      if (i == 0) {
         append_string(&ds, "   return a * b;\n");
      }
      else {
         snprintf(s, sizeof(s),
                  "   vec4 s = a * func%d(a, float(%d)) + vec4(b);\n",
                  i-1, index);
         append_string(&ds, s);

         snprintf(s, sizeof(s),
                  "   vec4 t = a / func%d(a, 3.0) - vec4(b);\n", i-1);
         append_string(&ds, s);

         if (i & 1) {
            append_string(&ds, "   vec4 u = max(s, t);\n");
         }
         else {
            /* use a conditional */
            append_string(&ds, "   vec4 u = min(s, t);\n");
            append_string(&ds, "   if (s.x > t.x) {\n");
            snprintf(s, sizeof(s), "      u = vec4(%d);\n", i);
            append_string(&ds, s);
            append_string(&ds, "   }\n");
         }

         append_string(&ds, "   return u;\n");
      }

      append_string(&ds, "}\n\n");
   }

   append_string(&ds, "void main()\n");
   append_string(&ds, "{\n");
   snprintf(s, sizeof(s), "   gl_FragColor = func%d(u1, u2.x);\n", i-1);
   append_string(&ds, s);
   append_string(&ds, "}\n");

   return ds.buffer;
}


static void
Draw(void)
{
   int t0, t1;
   int i;
   int fixed_func_time = 0, glsl_time_1 = 0, glsl_time_2 = 0;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   for (i = 0; i < num_shaders; i++) {
      UseProgram(0);
      t0 = glutGet(GLUT_ELAPSED_TIME);
      glDrawArrays(GL_POINTS, 0, NUM_POINTS);
      glFinish();
      t1 = glutGet(GLUT_ELAPSED_TIME);
      fixed_func_time += t1 - t0;

      UseProgram(programs[i]);
      t0 = glutGet(GLUT_ELAPSED_TIME);
      glDrawArrays(GL_POINTS, 0, NUM_POINTS);
      glFinish();
      t1 = glutGet(GLUT_ELAPSED_TIME);
      glsl_time_1 += t1 - t0;

      t0 = glutGet(GLUT_ELAPSED_TIME);
      glDrawArrays(GL_POINTS, 0, NUM_POINTS);
      glFinish();
      t1 = glutGet(GLUT_ELAPSED_TIME);
      glsl_time_2 += t1 - t0;
   }

   printf("Time to draw fixed-function points: %d ms\n", fixed_func_time);
   printf("Time to draw 1st GLSL shader points: %d ms\n", glsl_time_1);
   printf("Time to draw 2st GLSL shader points: %d ms\n", glsl_time_2);

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
   glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 25.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -15.0);
}


static void
Key(unsigned char key, int x, int y)
{
   if (key == 27) {
      glutDestroyWindow(Win);
      exit(0);
   }
   glutPostRedisplay();
}


static GLuint
create_shader_program(const char *fs_code,
                      int *compile_time, int *link_time)
{
   GLuint fragShader;
   GLuint vertShader;
   GLuint program;
   GLint u1_loc, u2_loc;
   GLint t0, t1, t2;

   vertShader = CompileShaderText(GL_VERTEX_SHADER, VS_code);

   t0 = glutGet(GLUT_ELAPSED_TIME);
   fragShader = CompileShaderText(GL_FRAGMENT_SHADER, fs_code);
   t1 = glutGet(GLUT_ELAPSED_TIME);
   program = LinkShaders(vertShader, fragShader);
   t2 = glutGet(GLUT_ELAPSED_TIME);

   UseProgram(program);
   u1_loc = glGetUniformLocation(program, "u1");
   u2_loc = glGetUniformLocation(program, "u2");
   glUniform4fv(u1_loc, 1, u1_val);
   glUniform4fv(u2_loc, 1, u2_val);
   UseProgram(0);

   *compile_time = t1 - t0;
   *link_time = t2 - t1;

   assert(glGetError() == GL_NO_ERROR);

   return program;
}


static void
Init(void)
{
   GLuint vbo;
   int i, compile_time, link_time, total_compile_time, total_link_time;

   if (!ShadersSupported())
      exit(1);

   printf("Shader complexity: %d\n", complexity);
   printf("Num shaders: %d\n", num_shaders);

   total_compile_time = total_link_time = 0;

   /* create the shader programs */
   for (i = 0; i < num_shaders; i++) {
      char *fs_code = gen_large_shader(complexity, i);

      if (verbose && i==0) {
         printf("Shader[0] code:\n%s\n", fs_code);
      }

      programs[i] = create_shader_program(fs_code, &compile_time, &link_time);
      total_compile_time += compile_time;
      total_link_time += link_time;

      free(fs_code);
   }

   printf("Total glCompileShader() time: %d ms\n", total_compile_time);
   printf("Total glLinkProgram() time: %d ms\n", total_link_time);

   glGenBuffers(1, &vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW);
   glVertexPointer(4, GL_FLOAT, 0, NULL);
   glEnable(GL_VERTEX_ARRAY);
}


int
main(int argc, char *argv[])
{
   int i;

   glutInit(&argc, argv);
   glutInitWindowSize(WinWidth, WinHeight);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   Win = glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Draw);

   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-v") == 0) {
         verbose = 1;
      }
      else if (strcmp(argv[i], "-c") == 0) {
         i++;
         complexity = atoi(argv[i]);
      }
      else if (strcmp(argv[i], "-n") == 0) {
         i++;
         num_shaders = atoi(argv[i]);
      }
      else {
         printf("unexpected option: %s\n", argv[i]);
         exit(1);
      }
   }

   Init();

   glutMainLoop();
   return 0;
}
