/* Copyright (c) 2011 Dave Airlie
   based on vbo-drawarrays.c, which should be MIT licensed */

/* Basic VBO testing ARB_vertex_type_2_10_10_10_rev */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include "glut_wrap.h"

GLboolean bgra = GL_FALSE;
#define i32to10(x) ((x) >= 0 ? (x & 0x1ff) : 1024-(abs((x))& 0x1ff))
#define i32to2(x) ((x) >= 0 ? (x & 0x1) : 3-abs((x)))

static unsigned iconv(int x, int y, int z, int w)
{
	unsigned val;

	val = i32to10(x);
	val |= i32to10(y) << 10;
	val |= i32to10(z) << 20;
	val |= i32to2(w) << 30;
	return val;
}
#define conv(x,y,z,w) (((x) & 0x3ff) | ((y) & 0x3ff) << 10 | ((z) & 0x3ff)<< 20 | ((w) & 0x3) << 30)

struct {
   GLuint pos;
   GLuint color;
} verts[3];

#define XYVAL 90
#define ZVAL -30

#define COLVAL 820

static void SetupVerts(void)
{
  verts[0].pos = iconv(-XYVAL, -XYVAL, ZVAL, 1);
  verts[0].color = conv(COLVAL, 0, 0, 0);

  verts[1].pos = iconv(XYVAL, -XYVAL, ZVAL, 1);
  verts[1].color = conv(0, COLVAL, 0, 0);

  verts[2].pos = iconv(0, XYVAL, ZVAL, 1);
  verts[2].color = conv(0, 0, COLVAL, 0);
}

GLuint arrayObj, elementObj;

static void Init( void )
{
   GLint errno;
   GLuint prognum;
   int color_size = 4;

   static const char *prog1 =
      "!!ARBvp1.0\n"
      "PARAM mvp[4] = {state.matrix.mvp};\n"
      "DP4 result.position.x, vertex.position, mvp[0]; \n"
      "DP4 result.position.y, vertex.position, mvp[1]; \n"
      "DP4 result.position.z, vertex.position, mvp[2]; \n"
      "DP4 result.position.w, vertex.position, mvp[3]; \n"
      "MOV  result.color, vertex.color;\n"
      "END\n";

#ifndef GL_ARB_vertex_type_2_10_10_10_rev
   fprintf(stderr,"built without ARB_vertex_type_2_10_10_10_rev\n");
   exit(1);
#endif

  if (!glutExtensionSupported("GL_ARB_vertex_type_2_10_10_10_rev")){
     fprintf(stderr,"requires ARB_vertex_type_2_10_10_10_rev\n");
     exit(1);
   }

   glGenProgramsARB(1, &prognum);
   glBindProgramARB(GL_VERTEX_PROGRAM_ARB, prognum);
   glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
		      strlen(prog1), (const GLubyte *) prog1);

   assert(glIsProgramARB(prognum));
   errno = glGetError();

   if (errno != GL_NO_ERROR)
   {
      GLint errorpos;

      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorpos);
      printf("errorpos: %d\n", errorpos);
      printf("%s\n", (char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
   }


   glEnableClientState( GL_VERTEX_ARRAY );
   glEnableClientState( GL_COLOR_ARRAY );

   SetupVerts();

   glGenBuffersARB(1, &arrayObj);
   glBindBufferARB(GL_ARRAY_BUFFER_ARB, arrayObj);
   glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(verts), verts, GL_STATIC_DRAW_ARB);

   if (bgra)
     color_size = GL_BGRA;

#ifdef GL_ARB_vertex_type_2_10_10_10_rev
   glVertexPointer( 4, GL_INT_2_10_10_10_REV, sizeof(verts[0]), 0 );
   glColorPointer( color_size, GL_UNSIGNED_INT_2_10_10_10_REV, sizeof(verts[0]), (void *)(sizeof(unsigned int)) );
#endif
}



static void Display( void )
{
   glClearColor(0.3, 0.3, 0.3, 1);
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glEnable(GL_VERTEX_PROGRAM_ARB);

   glDrawArrays( GL_TRIANGLES, 0, 3 );

   glFlush();
}


static void Reshape( int width, int height )
{
   glViewport( 0, 0, width, height );
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   glOrtho(-100.0, 100.0, -100.0, 100.0, -0.5, 1000.0);
   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();
}


static void Key( unsigned char key, int x, int y )
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

static GLenum Args(int argc, char **argv)
{
   GLint i;

   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-bgra") == 0) {
 	 bgra = GL_TRUE;
      } else {
         fprintf(stderr, "%s (Bad option).\n", argv[i]);
         return GL_FALSE;
      }
   }
   return GL_TRUE;
}

int main( int argc, char *argv[] )
{
   glutInit( &argc, argv );

   if (Args(argc, argv) == GL_FALSE) {
      exit(1);
   }

   glutInitWindowPosition( 0, 0 );
   glutInitWindowSize( 250, 250 );
   glutInitDisplayMode( GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH );
   glutCreateWindow(argv[0]);
   glewInit();
   glutReshapeFunc( Reshape );
   glutKeyboardFunc( Key );
   glutDisplayFunc( Display );

   glewInit();
   Init();
   glutMainLoop();
   return 0;
}
