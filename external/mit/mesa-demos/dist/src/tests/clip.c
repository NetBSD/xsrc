
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "glut_wrap.h"

static int Win;
static int WinWidth = 400, WinHeight = 400;
static GLfloat Xrot = 0, Yrot = 0, Zrot = 0;
static GLboolean Anim = GL_FALSE;

static int CurVert = 0, MoveAll = 0;
static GLfloat Verts[3][2];


static void
Idle(void)
{
   Xrot += 3.0;
   Yrot += 4.0;
   Zrot += 2.0;
   glutPostRedisplay();
}


static void
Draw(void)
{
   glClear(GL_COLOR_BUFFER_BIT);

   glBegin(GL_POLYGON);
   glVertex2fv(Verts[0]);
   glVertex2fv(Verts[1]);
   glVertex2fv(Verts[2]);
   glEnd();

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
RotVerts(void)
{
   float tx = Verts[0][0], ty = Verts[0][1];
   Verts[0][0] = Verts[1][0];   Verts[0][1] = Verts[1][1];
   Verts[1][0] = Verts[2][0];   Verts[1][1] = Verts[2][1];
   Verts[2][0] = tx;            Verts[2][1] = ty;
}

static void
Save(void)
{
   int i;
   FILE *f = fopen("verts.txt", "w");
   for (i = 0; i < 3; i++)
      fprintf(f, "%f %f\n", Verts[i][0], Verts[i][1]);
   fclose(f);
   printf("Saved data\n");
}

static void
Restore(void)
{
   int i;
   FILE *f = fopen("verts.txt", "r");
   if (f) {
      printf("Restoring data\n");
      for (i = 0; i < 3; i++) {
         int n =fscanf(f, "%f %f\n", &Verts[i][0], &Verts[i][1]);
         (void) n;
      }
      fclose(f);
   }
}

static void
Key(unsigned char key, int x, int y)
{
   (void) x;
   (void) y;
   switch (key) {
   case 'r':
      RotVerts();
      break;
   case 'a':
      MoveAll = 1;
      break;
   case '0':
      MoveAll = 0;
      CurVert = 0;
      break;
   case '1':
      MoveAll = 0;
      CurVert = 1;
      break;
   case '2':
      MoveAll = 0;
      CurVert = 2;
      break;
   case 's':
      Save();
      break;
   case 27:
      glutDestroyWindow(Win);
      exit(0);
      break;
   }
   glutPostRedisplay();
}

static void
move(float dx, float dy)
{
   int i;

   dx *= 0.05;
   dy *= 0.05;
   if (MoveAll) {
      for (i = 0; i < 3; i++) {
         Verts[i][0] += dx;
         Verts[i][1] += dy;
      }
   }
   else {
      Verts[CurVert][0] += dx;
      Verts[CurVert][1] += dy;
   }

   printf("\n");
   for (i = 0; i < 3; i++) {
      printf("%f %f\n", Verts[i][0], Verts[i][1]);
   }
}


static void
SpecialKey(int key, int x, int y)
{
   switch (key) {
   case GLUT_KEY_UP:
      move(0, +1);
      break;
   case GLUT_KEY_DOWN:
      move(0, -1);
      break;
   case GLUT_KEY_LEFT:
      move(-1, 0);
      break;
   case GLUT_KEY_RIGHT:
      move(1, 0);
      break;
   }
   glutPostRedisplay();
}


static void
Init(void)
{
   Verts[0][0] = 0.0; Verts[0][1] = 0.0;
   Verts[1][0] = 1.2; Verts[1][1] = -0.5;
   Verts[2][0] = 0.6; Verts[2][1] = -1.5;

   Restore();

   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}


int
main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(WinWidth, WinHeight);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   Win = glutCreateWindow(argv[0]);
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutSpecialFunc(SpecialKey);
   glutDisplayFunc(Draw);
   if (Anim)
      glutIdleFunc(Idle);
   Init();
   glutMainLoop();
   return 0;
}
