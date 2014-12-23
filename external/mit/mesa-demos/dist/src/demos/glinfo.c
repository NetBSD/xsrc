
/*
 * Print GL, GLU and GLUT version and extension info
 *
 * Brian Paul  This file in public domain.
 * October 3, 1997
 */


#include <stdio.h>
#include "glut_wrap.h"


int main( int argc, char *argv[] )
{
   glutInit( &argc, argv );
   glutInitDisplayMode( GLUT_RGB );
   glutCreateWindow(argv[0]);

   printf("GL_VERSION: %s\n", (char *) glGetString(GL_VERSION));
   printf("GL_RENDERER: %s\n", (char *) glGetString(GL_RENDERER));
   printf("GL_VENDOR: %s\n", (char *) glGetString(GL_VENDOR));
   printf("GL_EXTENSIONS: %s\n", (char *) glGetString(GL_EXTENSIONS));

#ifdef GL_VERSION_2_0
   {
      const GLubyte *v = glGetString(GL_VERSION);
      if (v[0] * 10 + v[2] >= 20) {
         const GLubyte *slv = glGetString(GL_SHADING_LANGUAGE_VERSION);
         printf("GL_SHADING_LANGUAGE_VERSION = %s\n", slv);
      }
   }
#endif

   printf("GLU_VERSION: %s\n", (char *) gluGetString(GLU_VERSION));
   printf("GLU_EXTENSIONS: %s\n", (char *) gluGetString(GLU_EXTENSIONS));
   printf("GLUT_API_VERSION: %d\n", GLUT_API_VERSION);
#ifdef GLUT_XLIB_IMPLEMENTATION
   printf("GLUT_XLIB_IMPLEMENTATION: %d\n", GLUT_XLIB_IMPLEMENTATION);
#endif

   return 0;
}
