#include "framework.h"

static void Display (void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   RenderScene ();
   glutSwapBuffers ();
}

static void Idle (void)
{
   glutPostRedisplay ();
}

void InitFramework (int *argc, char *argv[])
{
   glutInit (argc, argv);
   glutInitWindowPosition (0, 0);
   glutInitWindowSize (200, 200);
   glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutCreateWindow (argv[0]);
   glewInit();

   printf ("VENDOR: %s\n", glGetString (GL_VENDOR));
   printf ("RENDERER: %s\n", glGetString (GL_RENDERER));

   InitScene ();

   glutDisplayFunc (Display);
   glutIdleFunc (Idle);
   glutMainLoop ();
}

GLboolean CheckObjectStatus (GLhandleARB handle)
{
   GLint type, status, length;
   GLcharARB *infolog;

   glGetObjectParameterivARB (handle, GL_OBJECT_TYPE_ARB, &type);
   if (type == GL_SHADER_OBJECT_ARB)
      glGetObjectParameterivARB (handle, GL_OBJECT_COMPILE_STATUS_ARB, &status);
   else if (type == GL_PROGRAM_OBJECT_ARB)
      glGetObjectParameterivARB (handle, GL_OBJECT_LINK_STATUS_ARB, &status);
   else {
      assert (0);
      return GL_FALSE;
   }

   if (status)
      return GL_TRUE;

   printf ("\n%s FAILED. INFO LOG FOLLOWS:\n",
           type == GL_SHADER_OBJECT_ARB ? "SHADER COMPILE" : "PROGRAM LINK");

   glGetObjectParameterivARB (handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
   infolog = (GLcharARB *) (malloc (length));
   if (infolog != NULL) {
      glGetInfoLogARB (handle, length, NULL, infolog);
      printf ("%s", infolog);
      free (infolog);
   }

   printf ("\n");

   return GL_FALSE;
}

