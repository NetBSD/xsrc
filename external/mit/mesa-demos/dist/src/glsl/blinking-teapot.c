/**
 * blinking-teapot demo. It displays a teapot whose color go from blue to pink.
 * The color variation is handled by uniform buffer object.
 * Sources mostly from http://www.jotschi.de/?p=427 which uses UBO SPEC example
 *
 * Vincent Lejeune 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"


static int mouse_old_x, mouse_old_y;
static int mouse_buttons = 0;
static float rotate_x = 0.0, rotate_y = 0.0;
static float translate_z = -2.0;

static float delta = 0.01;
static GLfloat wf, hf;


static const GLchar *names[] = { "SurfaceColor", "WarmColor", "CoolColor",
  "DiffuseWarm", "DiffuseCool"
};

static GLuint buffer_id, uniformBlockIndex, uindex, vshad_id, fshad_id,
  prog_id;

static GLsizei uniformBlockSize;
static GLint singleSize;
static GLint offset;

static const GLfloat colors[] =
  { 0.45, 0.45, 1, 1, 0.45, 0.45, 1, 1, 0.75, 0.75, 0.75, 1,
  0.0, 0.0, 1.0, 1, 0.0, 1.0, 0.0, 1,
};

static GLfloat teapot_color = 1;

static void
reshape (int w, int h)
{
  wf = (GLfloat) w;
  hf = (GLfloat) h;
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (60.0, wf / hf, 0.1, 100.0);
}

static void
init_opengl (void)
{

  if (!ShadersSupported ())
    exit (1);

  if (!glutExtensionSupported("GL_ARB_uniform_buffer_object")) {
     printf("GL_ARB_uniform_buffer_object is required.\n");
     exit(1);
  }     

  vshad_id = CompileShaderFile (GL_VERTEX_SHADER, "blinking-teapot.vert");
  fshad_id = CompileShaderFile (GL_FRAGMENT_SHADER, "blinking-teapot.frag");
  prog_id = LinkShaders (vshad_id, fshad_id);

  UseProgram (prog_id);

  reshape (680, 400);

  glGenBuffers (1, &buffer_id);

  glBindBuffer (GL_UNIFORM_BUFFER, buffer_id);
  glBufferData (GL_UNIFORM_BUFFER, uniformBlockSize, NULL, GL_DYNAMIC_DRAW);

  glBindBufferBase (GL_UNIFORM_BUFFER, 0, buffer_id);
  glUniformBlockBinding (prog_id, uniformBlockIndex, 0);

  glGetUniformIndices (prog_id, 1, &names[2], &uindex);

  glGetActiveUniformsiv (prog_id, 1, &uindex, GL_UNIFORM_OFFSET, &offset);
  glGetActiveUniformsiv (prog_id, 1, &uindex, GL_UNIFORM_SIZE, &singleSize);

  glViewport (0, 0, 680, 400);
  glBufferData (GL_UNIFORM_BUFFER, 80, colors, GL_DYNAMIC_DRAW);
}

static void
render (void)
{
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glUseProgram (prog_id);
  glEnable (GL_DEPTH_TEST);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (0.0, 0.0, translate_z);
  glRotatef (rotate_x, 1.0, 0.0, 0.0);
  glRotatef (rotate_y, 0.0, 1.0, 0.0);
  glColor3f (1.0, 1.0, 1.0);

  glBindBuffer (GL_UNIFORM_BUFFER, buffer_id);
  glBufferSubData (GL_UNIFORM_BUFFER, offset, 4 * singleSize, &teapot_color);

  glFrontFace (GL_CW);
  glutSolidTeapot (0.6);
  glFrontFace (GL_CCW);
  glutSwapBuffers ();
  glutPostRedisplay ();

  teapot_color += delta;

  if (teapot_color > 1.0)
    {
      delta = -0.01;
    }

  if (teapot_color < 0.0)
    {
      delta = +0.01;
    }

}

static void
mouse (int button, int state, int x, int y)
{
  if (state == GLUT_DOWN)
    {
      mouse_buttons |= 1 << button;
    }
  else if (state == GLUT_UP)
    {
      mouse_buttons = 0;
    }

  mouse_old_x = x;
  mouse_old_y = y;
  glutPostRedisplay ();
}

static void
motion (int x, int y)
{
  float dx, dy;
  dx = x - mouse_old_x;
  dy = y - mouse_old_y;

  if (mouse_buttons & 1)
    {
      rotate_x += dy * 0.2;
      rotate_y += dx * 0.2;
    }
  else if (mouse_buttons & 4)
    {
      translate_z += dy * 0.01;
    }

  mouse_old_x = x;
  mouse_old_y = y;
}

int
main (int argc, char **argv)
{
  glutInit (&argc, argv);
  glutInitWindowSize (400, 400);
  glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow ("UBO Example");

  glutDisplayFunc (render);
  glutMouseFunc (mouse);
  glutMotionFunc (motion);
  glewInit ();
  init_opengl ();
  glutMainLoop ();
  return 0;
}
