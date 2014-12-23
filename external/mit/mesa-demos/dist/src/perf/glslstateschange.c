/**
 * Test states change when using shaders & textures.
 *
 * Copyright (C) 2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2011  Red Hat      All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "readtex.h"
#include "shaderutil.h"
#include "glmain.h"
#include "common.h"

static const char *VertFile1 = "glslstateschange1.vert";
static const char *FragFile1 = "glslstateschange1.frag";
static const char *VertFile2 = "glslstateschange2.vert";
static const char *FragFile2 = "glslstateschange2.frag";
static struct uniform_info Uniforms1[] = {
   { "tex1",  1, GL_SAMPLER_2D, { 0, 0, 0, 0 }, -1 },
   { "tex2",  1, GL_SAMPLER_2D, { 1, 0, 0, 0 }, -1 },
   { "UniV1", 1, GL_FLOAT_VEC4, { 0.8, 0.2, 0.2, 0 }, -1 },
   { "UniV2", 1, GL_FLOAT_VEC4, { 0.6, 0.6, 0.6, 0 }, -1 },
   END_OF_UNIFORMS
};
static struct uniform_info Uniforms2[] = {
   { "tex1",  1, GL_SAMPLER_2D, { 0, 0, 0, 0 }, -1 },
   { "tex2",  1, GL_SAMPLER_2D, { 1, 0, 0, 0 }, -1 },
   { "UniV1", 1, GL_FLOAT_VEC4, { 0.8, 0.2, 0.2, 0 }, -1 },
   { "UniV2", 1, GL_FLOAT_VEC4, { 0.6, 0.6, 0.6, 0 }, -1 },
   END_OF_UNIFORMS
};
static GLuint Program1;
static GLuint Program2;
static GLint P1VertCoord_attr = -1;
static GLint P1TexCoord0_attr = -1, P1TexCoord1_attr = -1;
static GLint P2VertCoord_attr = -1;
static GLint P2TexCoord0_attr = -1, P2TexCoord1_attr = -1;

static const char *TexFiles[4] =
   {
      DEMOS_DATA_DIR "tile.rgb",
      DEMOS_DATA_DIR "tree2.rgba",
      DEMOS_DATA_DIR "tile.rgb",
      DEMOS_DATA_DIR "tree2.rgba"
   };
GLuint texObj[4];


int WinWidth = 500, WinHeight = 500;

static GLfloat Xrot = 0.0, Yrot = .0, Zrot = 0.0;
static GLfloat EyeDist = 10;



static const GLfloat Tex0Coords[4][2] = {
   { 0.0, 0.0 }, { 2.0, 0.0 }, { 2.0, 2.0 }, { 0.0, 2.0 }
};

static const GLfloat Tex1Coords[4][2] = {
   { 0.0, 0.0 }, { 1.0, 0.0 }, { 1.0, 1.0 }, { 0.0, 1.0 }
};

static const GLfloat VertCoords[4][2] = {
   { -3.0, -3.0 }, { 3.0, -3.0 }, { 3.0, 3.0 }, { -3.0, 3.0 }
};

static void
DrawPolygonArray(GLint VertCoord_attr,
                 GLint TexCoord0_attr,
                 GLint TexCoord1_attr)
{
   void *vertPtr, *tex0Ptr, *tex1Ptr;

   vertPtr = VertCoords;
   tex0Ptr = Tex0Coords;
   tex1Ptr = Tex1Coords;

   if (VertCoord_attr >= 0) {
      glVertexAttribPointer(VertCoord_attr, 2, GL_FLOAT, GL_FALSE, 0, vertPtr);
      glEnableVertexAttribArray(VertCoord_attr);
   } else {
      glVertexPointer(2, GL_FLOAT, 0, vertPtr);
      glEnableClientState(GL_VERTEX_ARRAY);
   }

   glVertexAttribPointer(TexCoord0_attr, 2, GL_FLOAT, GL_FALSE, 0, tex0Ptr);
   glEnableVertexAttribArray(TexCoord0_attr);

   glVertexAttribPointer(TexCoord1_attr, 2, GL_FLOAT, GL_FALSE, 0, tex1Ptr);
   glEnableVertexAttribArray(TexCoord1_attr);

   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

static void
Draw(unsigned count)
{
   unsigned i;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   for (i = 0; i < count; i++) {
      Yrot = 0.05 * i;
      glPushMatrix(); /* modelview matrix */
      glTranslatef(0.0, 0.0, -EyeDist);
      glRotatef(Zrot, 0, 0, 1);
      glRotatef(Yrot, 0, 1, 0);
      glRotatef(Xrot, 1, 0, 0);

      glUseProgram(Program1);
      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, texObj[0]);
      glActiveTexture(GL_TEXTURE0 + 1);
      glBindTexture(GL_TEXTURE_2D, texObj[1]);
      glUniform4fARB(Uniforms1[2].location, Xrot, Yrot, Zrot, 1.000000);
      glUniform4fARB(Uniforms1[3].location, Xrot, Yrot, Zrot, 1.000000);
      DrawPolygonArray(P1VertCoord_attr, P1TexCoord0_attr, P1TexCoord1_attr);

      glUseProgram(Program2);
      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, texObj[2]);
      glActiveTexture(GL_TEXTURE0 + 1);
      glBindTexture(GL_TEXTURE_2D, texObj[3]);
      glUniform4fARB(Uniforms2[2].location, Xrot, Yrot, Zrot, 1.000000);
      glUniform4fARB(Uniforms2[3].location, Xrot, Yrot, Zrot, 1.000000);
      DrawPolygonArray(P2VertCoord_attr, P2TexCoord0_attr, P2TexCoord1_attr);

      glPopMatrix();

   }
   glutSwapBuffers();
}

void
PerfDraw(void)
{
   double rate;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   perf_printf("GLSL texture/program change rate\n");

   rate = PerfMeasureRate(Draw);
   perf_printf("  Immediate mode: %s change/sec\n", PerfHumanFloat(rate));

   exit(0);
}

void
PerfNextRound(void)
{
}

static void
InitTextures(void)
{
   GLenum filter = GL_LINEAR;
   int i;

   /* allocate 4 texture objects */
   glGenTextures(4, texObj);

   for (i = 0; i < 4; i++) {
      GLint imgWidth, imgHeight;
      GLenum imgFormat;
      GLubyte *image = NULL;

      image = LoadRGBImage(TexFiles[i], &imgWidth, &imgHeight, &imgFormat);
      if (!image) {
         printf("Couldn't read %s\n", TexFiles[i]);
         exit(0);
      }

      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, texObj[i]);
      gluBuild2DMipmaps(GL_TEXTURE_2D, 4, imgWidth, imgHeight,
                        imgFormat, GL_UNSIGNED_BYTE, image);
      free(image);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
   }
}

static GLuint
CreateAProgram(const char *vertProgFile, const char *fragProgFile,
               struct uniform_info *uniforms, GLint *VertCoord_attr,
               GLint *TexCoord0_attr, GLint *TexCoord1_attr)
{
   GLuint fragShader, vertShader, program;

   vertShader = CompileShaderFile(GL_VERTEX_SHADER, vertProgFile);
   fragShader = CompileShaderFile(GL_FRAGMENT_SHADER, fragProgFile);
   assert(vertShader);
   program = LinkShaders(vertShader, fragShader);

   glUseProgram(program);

   SetUniformValues(program, uniforms);

   assert(ValidateShaderProgram(program));

   *VertCoord_attr = glGetAttribLocation(program, "VertCoord");
   if (*VertCoord_attr > 0) {
      /* We want the VertCoord attrib to have position zero so that
       * the call to glVertexAttrib(0, xyz) triggers vertex processing.
       * Otherwise, if TexCoord0 or TexCoord1 gets position 0 we'd have
       * to set that attribute last (which is a PITA to manage).
       */
      glBindAttribLocation(program, 0, "VertCoord");
      /* re-link */
      glLinkProgram(program);
      /* VertCoord_attr should be zero now */
      *VertCoord_attr = glGetAttribLocation(program, "VertCoord");
      assert(*VertCoord_attr == 0);
   }

   *TexCoord0_attr = glGetAttribLocation(program, "TexCoord0");
   *TexCoord1_attr = glGetAttribLocation(program, "TexCoord1");

   return program;
}

static void
InitPrograms(void)
{
   Program1 = CreateAProgram(VertFile1, FragFile1, Uniforms1,
                             &P1VertCoord_attr,
                             &P1TexCoord0_attr, &P1TexCoord1_attr);
   Program2 = CreateAProgram(VertFile2, FragFile2, Uniforms2,
                             &P2VertCoord_attr,
                             &P2TexCoord0_attr, &P2TexCoord1_attr);
}

void
PerfInit(void)
{
   if (!ShadersSupported())
      exit(1);

   InitTextures();
   InitPrograms();

   glEnable(GL_DEPTH_TEST);

   glClearColor(.6, .6, .9, 0);
   glColor3f(1.0, 1.0, 1.0);
}
