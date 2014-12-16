/* -*- mode: c; tab-width: 3; indent-tabs-mode: nil; c-basic-offset: 3; coding: utf-8-unix -*- */
/*
  Copyright (c) 2013 Kristóf Ralovich

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

// Simplified version of the algorithm described in 
// K. Ralovich, M. Magdics: Recursive Ray Tracing in Geometry Shader,
// Proceedings of the Fifth Hungarian Conference on Computer Graphics and
// Geometry, Budapest, Hungary, 26 Jan 2010.

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"
#include <math.h>
#include <stddef.h> // offsetof

// TODO: port to piglit too

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define S__LINE__ STRINGIFY(__LINE__)

static const float INF=9999.9F;

static int Win;
static int WinWidth = 256, WinHeight = 256;
static GLboolean mouseGrabbed = GL_FALSE;

static GLuint vertShader;
static GLuint geomShader;
static GLuint fragShader;
static GLuint program;

static GLuint pgQuery;

static GLuint dst;
static GLuint eyeRaysAsPoints;

int posAttribLoc;
int orig_tAttribLoc;
int dir_idxAttribLoc;
int uv_stateAttribLoc;
int posVaryingLoc;
int orig_tVaryingLoc;
int dir_idxVaryingLoc;
int uv_stateVaryingLoc;
size_t nRayGens=3;

float rot[9] = {1,0,0,  0,1,0,   0,0,1};

static const char* vsSource =
"                                                                  \n"
"#version 150 core                                                 \n"
"#line " S__LINE__ "\n"
"#define SHADOWS                                                   \n"
"#define RECURSION                                                 \n"
"                                                                  \n"
"const float INF=9999.9;                                           \n"
"const float EPSILON = 0.00001;                                    \n"
"                                                                  \n"
"uniform vec3 cameraPos;                                           \n"
"uniform mat3 rot3;                                                \n"
"uniform vec3 lightPos;                                            \n"
"uniform vec4 backgroundColor;                                     \n"
"uniform int emitNoMore;                                           \n"
"                                                                  \n"
"in vec4 pos;                                                      \n"
"in vec4 orig_t;                                                   \n"
"in vec4 dir_idx;                                                  \n"
"in vec4 uv_state;                                                 \n"
"// uv_state.z = state                                             \n"
"// uv_state.w = type (ray generation)                             \n"
"                                                                  \n"
"//int state // inFB                                               \n"
"//    0: generation of ray dirs needed                            \n"
"//    1: do not generate ray dirs, keep in GS, intersect, not in FB\n"
"//    3: cull in next GS, already in FB                           \n"
"//int type  // isShadow                                           \n"
"//   -1: not shadow ray, reflected                                \n"
"//    0: not shadow ray, eye ray                                  \n"
"//    1: shadow ray                                               \n"
"                                                                  \n"
"out vec4 orig_t1;                                                 \n"
"out vec4 dir_idx1;                                                \n"
"out vec4 uv_state1;                                               \n"
"                                                                  \n"
"                                                                  \n"
"//----------------------------------------------------------------\n"
"                                                                  \n"
"struct Ray                                                        \n"
"{                                                                 \n"
"  vec3 orig;                                                      \n"
"  vec3 dir;                                                       \n"
"};                                                                \n"
"                                                                  \n"
"struct Sphere                                                     \n"
"{                                                                 \n"
"  vec3 c;                                                         \n"
"  float r;                                                        \n"
"};                                                                \n"
"                                                                  \n"
"struct Isec                                                       \n"
"{                                                                 \n"
"  float t;                                                        \n"
"  int idx;                                                        \n"
"                                                                  \n"
"  vec3 hit;                                                       \n"
"  vec3 n;                                                         \n"
"};                                                                \n"
"                                                                  \n"
"const Sphere spheres0 = Sphere( vec3(0.0,0.0,-1.0), 0.5 );        \n"
"const Sphere spheres1 = Sphere( vec3(-3.0,0.0,-1.0), 1.5 );       \n"
"const Sphere spheres2 = Sphere( vec3(0.0,3.0,-1.0), 0.5 );        \n"
"const Sphere spheres3 = Sphere( vec3(2.0,0.0,-1.0), 1.0 );        \n"
"const Sphere spheres4 = Sphere( vec3(0.0,-11.0,-1.0), 10.0 );     \n"
"const int nSpheres = 5;                                           \n"
"const Sphere spheres[5]=Sphere[5](spheres0, spheres1, spheres2, spheres3, spheres4);\n"
"                                                                  \n"
"Isec                                                              \n"
"lookupNormal(const in Ray ray, in Isec isec)                      \n"
"{                                                                 \n"
"  Sphere sph=spheres[isec.idx];                                   \n"
"  vec3 c = sph.c;                                                 \n"
"  float r = sph.r;                                                \n"
"  isec.hit  = ray.orig + ray.dir * isec.t;                        \n"
"  isec.n = (isec.hit - c) / r;                                    \n"
"  return isec;                                                    \n"
"}                                                                 \n"
"                                                                  \n"
"void                                                              \n"
"intersect(const in Ray ray,                                       \n"
"          const in Sphere sph,                                    \n"
"          const in int idx,                                       \n"
"          inout Isec isec)                                        \n"
"{                                                                 \n"
"  // Project both o and the sphere to the plane perpendicular to d\n"
"  // and containing c. Let x be the point where the ray intersects\n"
"  // the plane. If |x-c| < r, the ray intersects the sphere.      \n"
"  vec3 o = ray.orig;                                              \n"
"  vec3 d = ray.dir;                                               \n"
"  vec3 n = -d;                                                    \n"
"  vec3 c = sph.c;                                                 \n"
"  float r = sph.r;                                                \n"
"  float t = dot(c-o,n)/dot(n,d);//ray parameter for point x       \n"
"  vec3 x = o+d*t;                                                 \n"
"  float e = length(x-c);                                          \n"
"  if (e > r)                                                      \n"
"  {                                                               \n"
"    // no intersection                                            \n"
"    return;                                                       \n"
"  }                                                               \n"
"                                                                  \n"
"  // Apply Pythagorean theorem on the (intersection,x,c) triangle \n"
"  // to get the distance between c and the intersection.          \n"
"  float f = sqrt(r*r - e*e);                                      \n"
"  float dist = t - f;                                             \n"
"  if (dist < 0.0)                                                 \n"
"  {                                                               \n"
"    // inside the sphere                                          \n"
"    return;                                                       \n"
"  }                                                               \n"
"                                                                  \n"
"  if (dist < EPSILON)                                             \n"
"    return;                                                       \n"
"                                                                  \n"
"  if (dist > isec.t)                                              \n"
"    return;                                                       \n"
"                                                                  \n"
"  isec.t = dist;                                                  \n"
" isec.idx = idx;                                                  \n"
"}                                                                 \n"
"                                                                  \n"
"Isec\n"
"intersect_spheres(const in Ray ray,\n"
"                  const in float max_t /*= INF*/)\n"
"{\n"
"  Isec nearest;\n"
"  nearest.t = max_t;\n"
"  nearest.idx = -1;\n"
"\n"
"  intersect(ray, spheres0, 0, nearest);\n"
"  intersect(ray, spheres1, 1, nearest);\n"
"  intersect(ray, spheres2, 2, nearest);\n"
"  intersect(ray, spheres3, 3, nearest);\n"
"  intersect(ray, spheres4, 4, nearest);                                \n"
"                                                                       \n"
"  return nearest;                                                      \n"
"}                                                                      \n"
"                                                                       \n"
"                                                                       \n"
"                                                                       \n"
"//---------------------------------------------------------------------\n"
"                                                                       \n"
"                                                                       \n"
"                                                                       \n"
"                                                                       \n"
"void                                                                   \n"
"main()                                                                 \n"
"{                                                                      \n"
"  //inVS();                                                            \n"
"  Ray  ray   = Ray(orig_t.xyz, dir_idx.xyz);                                \n"
"  Isec isec  = Isec(orig_t.w, int(dir_idx.w), vec3(0,0,0), vec3(0,0,0));    \n"
"  int  state = int(uv_state.z);                                             \n"
"  int  type  = int(uv_state.w);                                             \n"
"                                                                            \n"
"  if (state == 0)                                                            \n"
"  {                                                                         \n"
"    // generate eye rays\n"
"    ray = Ray(cameraPos, normalize(vec3(pos.x, pos.y, -1.0) * rot3));   \n"
"    isec.t = INF;\n"
"    isec.idx = -1;\n"
"    state = 1;\n"
"    type = 0;                                                           \n"
"    isec = intersect_spheres(ray, isec.t);                              \n"
"  }                                                                     \n"
"#if defined(SHADOWS) || defined(RECURSION)                              \n"
"  else if (state == 1)                                                  \n"
"  {                                                                     \n"
"    isec = intersect_spheres(ray, isec.t);                              \n"
"  }                                                                     \n"
"#endif                                                                  \n"
"  //else state == 3                                                     \n"
"                                                                        \n"
"  //outVS();                                                            \n"
"  gl_Position  = pos;                                                   \n"
"  orig_t1.xyz  = ray.orig;                                              \n"
"  orig_t1.w    = isec.t;                                                \n"
"  dir_idx1.xyz = ray.dir;                                               \n"
"  dir_idx1.w   = float(isec.idx);                                       \n"
"  uv_state1.z  = float(state);                                          \n"
"  uv_state1.w  = float(type);                                           \n"
"}\n";


static const char* gsSource = 
"#version 150 core                                                        \n"
"#line " S__LINE__ "\n"
"layout(points) in;                                                       \n"
"layout(points, max_vertices = 3) out;                                    \n"
"                                                                         \n"
"#define SHADOWS                                                          \n"
"#define RECURSION                                                        \n"
"                                                                         \n"
"const float INF=9999.9;                                                  \n"
"const float EPSILON = 0.00001;                                           \n"
"                                                                         \n"
"uniform vec3 cameraPos;                                                  \n"
"uniform mat3 rot3;                                                       \n"
"uniform vec3 lightPos;                                                   \n"
"uniform vec4 backgroundColor;                                            \n"
"uniform int emitNoMore;                                                  \n"
"                                                                         \n"
"                                                                         \n"
"//-----------------------------------------------------------------------\n"
"                                                                         \n"
"struct Ray                                                               \n"
"{                                                                        \n"
"  vec3 orig;                                                             \n"
"  vec3 dir;                                                              \n"
"};                                                                       \n"
"                                                                         \n"
"struct Sphere                                                            \n"
"{                                                                        \n"
"  vec3 c;                                                                \n"
"  float r;                                                               \n"
"};                                                                       \n"
"                                                                         \n"
"struct Isec                                                              \n"
"{                                                                        \n"
"  float t;                                                               \n"
"  int idx;                                                               \n"
"                                                                         \n"
"  vec3 hit;                                                              \n"
"  vec3 n;                                                                \n"
"};                                                                       \n"
"                                                                         \n"
"const Sphere spheres0 = Sphere( vec3(0.0,0.0,-1.0), 0.5 );\n"
"const Sphere spheres1 = Sphere( vec3(-3.0,0.0,-1.0), 1.5 );\n"
"const Sphere spheres2 = Sphere( vec3(0.0,3.0,-1.0), 0.5 );\n"
"const Sphere spheres3 = Sphere( vec3(2.0,0.0,-1.0), 1.0 );\n"
"const Sphere spheres4 = Sphere( vec3(0.0,-11.0,-1.0), 10.0 );\n"
"const int nSpheres = 5;\n"
"const Sphere spheres[5]=Sphere[5](spheres0, spheres1, spheres2, spheres3, spheres4);\n"
"                                                                         \n"
"Isec                                                                     \n"
"lookupNormal(const in Ray ray, in Isec isec)                             \n"
"{                                                                        \n"
"  Sphere sph=spheres[isec.idx];                                          \n"
"  vec3 c = sph.c;                                                        \n"
"  float r = sph.r;                                                       \n"
"  isec.hit  = ray.orig + ray.dir * isec.t;                               \n"
"  isec.n = (isec.hit - c) / r;                                           \n"
"  return isec;                                                           \n"
"}                                                                        \n"
"                                                                         \n"
"in vec4 orig_t1[1];                                                      \n"
"in vec4 dir_idx1[1];                                                     \n"
"in vec4 uv_state1[1];                                                    \n"
"                                                                         \n"
"out vec4 orig_t2;                                                        \n"
"out vec4 dir_idx2;                                                       \n"
"out vec4 uv_state2;                                                      \n"
"                                                                         \n"
"                                                                         \n"
"void                                                                     \n"
"main()                                                                   \n"
"{                                                                        \n"
"  //inGS();                                                              \n"
"  Ray  ray   = Ray(orig_t1[0].xyz, dir_idx1[0].xyz);                     \n"
"  Isec isec  = Isec(orig_t1[0].w, int(dir_idx1[0].w), vec3(0,0,0), vec3(0,0,0));  \n"
"  int  state = int(uv_state1[0].z);                                      \n"
"  int  type  = int(uv_state1[0].w);                                      \n"
"                                                                         \n"
"  if (state > 1)                                                         \n"
"    return;                                                              \n"
"                                                                         \n"
"  if (isec.idx == -1)                                                    \n"
"    return;                                                              \n"
"                                                                         \n"
"  // emitPassThrough();                          \n"
"  gl_Position  = gl_in[0].gl_Position;           \n"
"  orig_t2      = orig_t1[0];                     \n"
"  dir_idx2     = dir_idx1[0];                    \n"
"  uv_state2.xyw= uv_state1[0].xyw;               \n"
"  uv_state2.z = 3.0; /*state*/                   \n"
"  EmitVertex();                                  \n"
"  EndPrimitive();                                                        \n"
"                                                                         \n"
"  if (type != 0 || emitNoMore>0)                                         \n"
"    return;                                                              \n"
"                                                                         \n"
"#if defined(SHADOWS) || defined(RECURSION)\n"
"  isec = lookupNormal(ray, isec);\n"
"  vec3 hitN = isec.n;\n"
"  vec3 hitP = ray.orig + ray.dir*isec.t + hitN*EPSILON;\n"
"#endif                                                                \n"
"#ifdef SHADOWS                                                          \n"
"  vec3 toLight = lightPos - hitP;\n"
"  float lightDist = length(toLight);\n"
"  Ray shadowRay = Ray(hitP, toLight/lightDist);\n"
"  Isec shadowHit = Isec(lightDist, -1, vec3(0,0,0), vec3(0,0,0));\n"
"  state = 1;                                       \n"
"  type = 1;                                        \n"
"                                                   \n"
"  //emitShadowRay();                               \n"
"  gl_Position  = gl_in[0].gl_Position;             \n"
"  orig_t2.xyz  = shadowRay.orig;                   \n"
"  orig_t2.w    = shadowHit.t;                      \n"
"  dir_idx2.xyz = shadowRay.dir;                    \n"
"  dir_idx2.w   = float(shadowHit.idx);             \n"
"  uv_state2.z  = float(state);                     \n"
"  uv_state2.w  = float(type);                      \n"
"  EmitVertex();                                    \n"
"  EndPrimitive();                                  \n"
"#endif                                             \n"
"#ifdef RECURSION                                   \n"
"  Ray  reflRay = Ray(hitP, reflect(ray.dir, hitN));\n"
"  Isec reflHit = Isec(INF, -1, vec3(0,0,0), vec3(0,0,0));\n"
"  state = 1;          // intersect in next pass, FS discard in this pass\n"
"  type  = -1;                                   \n"
"                                                \n"
"  //emitReflRay();                              \n"
"  gl_Position  = gl_in[0].gl_Position;          \n"
"  orig_t2.xyz  = reflRay.orig;                  \n"
"  orig_t2.w    = reflHit.t;                     \n"
"  dir_idx2.xyz = reflRay.dir;                   \n"
"  dir_idx2.w   = float(reflHit.idx);            \n"
"  uv_state2.z  = float(state);                  \n"
"  uv_state2.w  = float(type);                   \n"
"  EmitVertex();                                 \n"
"  EndPrimitive();                               \n"
"#endif\n"
"}\n";

static const char* fsSource = 
"#version 150 core                                                        \n"
"#line " S__LINE__ "\n"
"                                                                         \n"
"#define SHADOWS                                                          \n"
"#define RECURSION                                                        \n"
"                                                                         \n"
"const float INF=9999.9;                                                  \n"
"const float EPSILON = 0.00001;                                           \n"
"                                                                         \n"
"uniform vec3 cameraPos;                                                  \n"
"uniform mat3 rot3;                                                       \n"
"uniform vec3 lightPos;                                                   \n"
"uniform vec4 backgroundColor;                                            \n"
"uniform int emitNoMore;                                                  \n"
"                                                                         \n"
"out vec4 frag_color;                                                     \n"
"                                                                         \n"
"//-----------------------------------------------------------------------\n"
"                                                                         \n"
"struct Ray\n"
"{\n"
"  vec3 orig;\n"
"  vec3 dir;\n"
"};\n"
"\n"
"struct Sphere\n"
"{\n"
"  vec3 c;\n"
"  float r;\n"
"};\n"
"\n"
"struct Isec\n"
"{\n"
"  float t;\n"
"  int idx;\n"
"\n"
"  vec3 hit;\n"
"  vec3 n;\n"
"};\n"
"\n"
"const Sphere spheres0 = Sphere( vec3(0.0,0.0,-1.0), 0.5 );\n"
"const Sphere spheres1 = Sphere( vec3(-3.0,0.0,-1.0), 1.5 );\n"
"const Sphere spheres2 = Sphere( vec3(0.0,3.0,-1.0), 0.5 );\n"
"const Sphere spheres3 = Sphere( vec3(2.0,0.0,-1.0), 1.0 );\n"
"const Sphere spheres4 = Sphere( vec3(0.0,-11.0,-1.0), 10.0 );\n"
"const int nSpheres = 5;\n"
"const Sphere spheres[5]=Sphere[5](spheres0, spheres1, spheres2, spheres3, spheres4);\n"
"\n"
"Isec\n"
"lookupNormal(const in Ray ray, in Isec isec)\n"
"{\n"
"  Sphere sph=spheres[isec.idx];\n"
"  vec3 c = sph.c;\n"
"  float r = sph.r;\n"
"  isec.hit  = ray.orig + ray.dir * isec.t;\n"
"  isec.n = (isec.hit - c) / r;\n"
"  return isec;\n"
"}\n"
"\n"
"in vec4 orig_t2;\n"
"in vec4 dir_idx2;\n"
"in vec4 uv_state2;\n"
"\n"
"vec3\n"
"idx2color(const in int idx)\n"
"{\n"
"  vec3 diff;\n"
"  if (idx == 0)\n"
"    diff = vec3(1.0, 0.0, 0.0);\n"
"  else if (idx == 1)\n"
"    diff = vec3(0.0, 1.0, 0.0);\n"
"  else if (idx == 2)\n"
"    diff = vec3(0.0, 0.0, 1.0);\n"
"  else if (idx == 3)\n"
"    diff = vec3(1.0, 1.0, 0.0);\n"
"  else if (idx == 4)\n"
"    diff = vec3(0.7, 0.7, 0.7);\n"
"  return diff;\n"
"}\n"
"\n"
"\n"
"void\n"
"main()\n"
"{\n"
"  Ray  ray   = Ray(orig_t2.xyz, dir_idx2.xyz);\n"
"  Isec isec  = Isec(orig_t2.w, int(dir_idx2.w), vec3(0,0,0), vec3(0,0,0));\n"
"  int  state = int(uv_state2.z);\n"
"  int  type  = int(uv_state2.w);\n"
"\n"
"  if (state < 3)\n"
"  {\n"
"    discard;\n"
"  }\n"
"\n"
"\n"
"  if (type == 0)\n"
"  {\n"
"    Ray eyeRay = ray;\n"
"    Isec eyeHit = isec;\n"
"    if (eyeHit.idx == -1)\n"
"    {\n"
"      frag_color = vec4(backgroundColor.rgb, 0.0);\n"
"      return;\n"
"    }\n"
"    vec3 eyeHitPosition = eyeRay.orig + eyeRay.dir * eyeHit.t;\n"
"    vec3 lightVec = lightPos - eyeHitPosition;\n"
"    eyeHit = lookupNormal(eyeRay, eyeHit);\n"
"    vec3  N      = eyeHit.n;\n"
"    vec3  L      = normalize(lightVec);                                         \n"
"    float NdotL  = max(dot(N, L), 0.0);                                         \n"
"    vec3 diffuse = idx2color(eyeHit.idx); // material color of the visible point\n"
"    frag_color = vec4(diffuse * NdotL, 1.0);                                  \n"
"    return;                                                                \n"
"  }                                                                        \n"
"#ifdef SHADOWS                                                             \n"
"  if (type > 0)                                                            \n"
"  {                                                               \n"
"    Isec shadowHit = isec;                                        \n"
"    if (shadowHit.idx == -1)                                      \n"
"    {                                                             \n"
"      discard;                                                    \n"
"    }                                                             \n"
"    frag_color = vec4(-1,-1,-1, 0.0);                           \n"
"    return;                                                       \n"
"  }                                                               \n"
"#endif                                                            \n"
"#ifdef RECURSION                                                  \n"
"  // else type < 0                                                \n"
"  {                                                               \n"
"    Ray reflRay = ray;                                            \n"
"    Isec reflHit = isec;                                          \n"
"    if (reflHit.idx == -1)                                        \n"
"    {                                                             \n"
"      discard;                                                    \n"
"    }                                                             \n"
"    vec3 reflHitPosition = reflRay.orig + reflRay.dir * reflHit.t;\n"
"    vec3 lightVec = lightPos - reflHitPosition;                   \n"
"    reflHit = lookupNormal(reflRay, reflHit);                     \n"
"    vec3  N      = reflHit.n;                                     \n"
"    vec3  L      = normalize(lightVec);                           \n"
"    float NdotL  = max(dot(N, L), 0.0);                           \n"
"    vec3 diffuse = idx2color(reflHit.idx);                        \n"
"    frag_color = vec4(diffuse * NdotL * 0.25, 1.0); // material color of the visible point\n"
"    return;                                                       \n"
"  }                                                               \n"
"#endif                                                            \n"
"}                                                                 \n";

struct vec4
{
   union {
      float _[4];
      struct { float x,y,z,w; };
   };
   vec4(float a, float b, float c, float d) : x(a), y(b), z(c), w(d) {}
};

struct GSRay
{
   vec4 pos;
   vec4 orig_t;
   vec4 dir_idx;
   vec4 uv_state;
};

static float
deg2rad(const float degree)
{
   return( degree * 0.017453292519943295769236907684886F);
}

static void
rotate_xy(float* mat3, const float degreesAroundX, const float degreesAroundY)
{
   const float radX = deg2rad(degreesAroundX);
   const float c1 = cosf(radX);
   const float s1 = sinf(radX);
   const float radY = deg2rad(degreesAroundY);
   const float c2 = cosf(radY);
   const float s2 = sinf(radY);
   mat3[0] = c2;    mat3[3] = 0.0F; mat3[6] = s2;
   mat3[1] = s1*s2; mat3[4] = c1;   mat3[7] = -s1*c2;
   mat3[2] = -c1*s2;mat3[5] = s1;   mat3[8] = c1*c2;
}

static void
identity(float* mat3)
{
   mat3[0] = 1.0F; mat3[3] = 0.0F; mat3[6] = 0.0F;
   mat3[1] = 0.0F; mat3[4] = 1.0F; mat3[7] = 0.0F;
   mat3[2] = 0.0F; mat3[5] = 0.0F; mat3[8] = 1.0F;
}

static void
Draw(void)
{
   glClearColor( 0.2, 0.5, 0.3, 0.0 );
   glClearDepth(0.11F);
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glDisable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   glUseProgram(program);

   glUniformMatrix3fv(glGetUniformLocation(program, "rot3"), 1, 0, rot);

   //gs.gs->getVertexAttribLocation("pos", gs.posAttribLoc);
   //gs.gs->getVertexAttribLocation("orig_t", gs.orig_tAttribLoc);
   //gs.gs->getVertexAttribLocation("dir_idx", gs.dir_idxAttribLoc);
   //gs.gs->getVertexAttribLocation("uv_state", gs.uv_stateAttribLoc);
   posAttribLoc = glGetAttribLocation(program, "pos");
   orig_tAttribLoc = glGetAttribLocation(program, "orig_t");
   dir_idxAttribLoc = glGetAttribLocation(program, "dir_idx");
   uv_stateAttribLoc = glGetAttribLocation(program, "uv_state");

   glBindFragDataLocation(program, 0, "frag_color");

   ////printf("%d\n", i);
   //gs.fpwQuery->beginQuery();
   //gs.pgQuery->beginQuery();
   glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dst);
   glBeginQuery(GL_PRIMITIVES_GENERATED, pgQuery);
   glBeginTransformFeedback(GL_POINTS);
   //gs.eyeRaysAsPoints->bindAs(ARRAY);
   glBindBuffer(GL_ARRAY_BUFFER, eyeRaysAsPoints);
   {
      glEnableVertexAttribArray(posAttribLoc);
      glVertexAttribPointer(posAttribLoc, 4, GL_FLOAT, GL_FALSE,
                            sizeof(GSRay), (void*)offsetof(GSRay, pos));

      glEnableVertexAttribArray(orig_tAttribLoc);
      glVertexAttribPointer(orig_tAttribLoc, 4, GL_FLOAT, GL_FALSE,
                            sizeof(GSRay), (void*)offsetof(GSRay, orig_t));

      glEnableVertexAttribArray(dir_idxAttribLoc);
      glVertexAttribPointer(dir_idxAttribLoc, 4, GL_FLOAT, GL_FALSE,
                            sizeof(GSRay), (void*)offsetof(GSRay, dir_idx));

      glEnableVertexAttribArray(uv_stateAttribLoc);
      glVertexAttribPointer(uv_stateAttribLoc, 4, GL_FLOAT, GL_FALSE,
                            sizeof(GSRay), (void*)offsetof(GSRay, uv_state));

      //if (getShadows() || getMaxRecursion() > 0)
      //gs.gs->set_uniform("emitNoMore", 1, 0);
      glUniform1i(glGetUniformLocation(program, "emitNoMore"), 0);

      //glEnable(GL_RASTERIZER_DISCARD);
      glDrawArrays(GL_POINTS, 0, WinWidth*WinHeight);
      //glDisable(GL_RASTERIZER_DISCARD);

      glDisableVertexAttribArray(uv_stateAttribLoc);

      glDisableVertexAttribArray(dir_idxAttribLoc);

      glDisableVertexAttribArray(orig_tAttribLoc);

      glDisableVertexAttribArray(posAttribLoc);
   }
   //gs.eyeRaysAsPoints->unbindAs(ARRAY);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glEndTransformFeedback();
   //gs.pgQuery->endQuery();
   glEndQuery(GL_PRIMITIVES_GENERATED);
   //gs.fpwQuery->endQuery();

   ////psoLog(LOG_RAW) << "1st: " << gs.fpwQuery->getQueryResult() << ", " << gs.pgQuery->getQueryResult() << "\n";


   ////swap(src, dst);
   glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

   ////clear();

   ////fpwQuery->beginQuery();
   ////pgQuery->beginQuery();
   //// With GL_ARB_color_buffer_float we can use negative color values
   //// and disable clamping with ClampColorARB. This might be better for
   //// compositing the pixels in shadow.
   glEnable(GL_BLEND);
   glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX); // modeRGB, modeA
   glBlendFuncSeparate(GL_ONE, GL_SRC_ALPHA,     // srcRGB, dstRGB
                       GL_ONE, GL_ONE);          // arcA,   dstA
   //gs.dst->bindAs(ARRAY);
   glBindBuffer(GL_ARRAY_BUFFER, dst);
   {
      glEnableVertexAttribArray(posAttribLoc);
      glVertexAttribPointer(posAttribLoc, 4, GL_FLOAT, GL_FALSE,
                            sizeof(GSRay), (void*)offsetof(GSRay, pos));

      glEnableVertexAttribArray(orig_tAttribLoc);
      glVertexAttribPointer(orig_tAttribLoc, 4, GL_FLOAT, GL_FALSE,
                            sizeof(GSRay), (void*)offsetof(GSRay, orig_t));

      glEnableVertexAttribArray(dir_idxAttribLoc);
      glVertexAttribPointer(dir_idxAttribLoc, 4, GL_FLOAT, GL_FALSE,
                            sizeof(GSRay), (void*)offsetof(GSRay, dir_idx));

      glEnableVertexAttribArray(uv_stateAttribLoc);
      glVertexAttribPointer(uv_stateAttribLoc, 4, GL_FLOAT, GL_FALSE,
                            sizeof(GSRay), (void*)offsetof(GSRay, uv_state));

      //if (getShadows() || getMaxRecursion() > 0)
      //gs.gs->set_uniform("emitNoMore", 1, 1);
      glUniform1i(glGetUniformLocation(program, "emitNoMore"), 1);
      //GLint fpw = gs.fpwQuery->getQueryResult();
      //GLint pg = gs.pgQuery->getQueryResult();
      GLint pg;
      glGetQueryObjectiv(pgQuery, GL_QUERY_RESULT, &pg);
      //pso_runtime_check(fpw == pg);
      glDrawArrays(GL_POINTS, 0, pg);

      glDisableVertexAttribArray(uv_stateAttribLoc);

      glDisableVertexAttribArray(dir_idxAttribLoc);

      glDisableVertexAttribArray(orig_tAttribLoc);

      glDisableVertexAttribArray(posAttribLoc);
   }
   //gs.dst->unbindAs(ARRAY);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glDisable(GL_BLEND);
   ////pgQuery->endQuery();
   ////fpwQuery->endQuery();

   ////psoLog(LOG_RAW) << "2nd: " << fpwQuery->getQueryResult() << ", " << pgQuery->getQueryResult() << "\n\n";
   ////pso_runtime_check(fpwQuery->getQueryResult() == pgQuery->getQueryResult());

   ////swap(src, dst);
   ////for(;;);

   glUseProgram(0);

   glDisable(GL_CULL_FACE);

//////////////////////////////////////////////////////////////////////

   glutSwapBuffers();

   {
      static int frames = 0;
      static int t0 = 0;
      static int t1 = 0;
      float dt;
      frames++;
      t1 = glutGet(GLUT_ELAPSED_TIME);
      dt = (float)(t1-t0)/1000.0F;
      if (dt >= 5.0F)
      {
         float fps = (float)frames / dt;
         printf("%f FPS (%d frames in %f seconds)\n", fps, frames, dt);
         frames = 0;
         t0 = t1;
      }
   }
}


static void
Reshape(int width, int height)
{
   WinWidth = width;
   WinHeight = height;
   glViewport(0, 0, width, height);

   {
      size_t nElem = WinWidth*WinHeight*nRayGens;
      glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, dst);
      glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, nElem*sizeof(GSRay), 0, GL_STREAM_DRAW);
      GSRay* d = (GSRay*)glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_WRITE);
      for (size_t i = 0; i < nElem; i++)
      {
         d[i].dir_idx = vec4(0.0F, 0.0F, 0.0F, -1.0F);
      }
      glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
      glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
      //printf("Ping-pong VBO size 2x%d Kbytes.\n", (int)nElem*sizeof(GSRay)/1024);
   }

   {
      glBindBuffer(GL_ARRAY_BUFFER, eyeRaysAsPoints);
      glBufferData(GL_ARRAY_BUFFER, WinWidth*WinHeight*sizeof(GSRay), 0, GL_STATIC_DRAW);
      GSRay* d = (GSRay*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
      const float w = 0.5F * WinWidth;
      const float h = 0.5F * WinHeight;
      for (int y = 0; y < WinHeight; y++)
      {
         for (int x = 0; x < WinWidth; x++)
         {
            unsigned int i = y*WinWidth+x;
            const float posx = x / w - 1.0F;
            const float posy = y / h - 1.0F;
            d[i].pos     = vec4(posx, posy, 0.5F, 1.0F);
            d[i].orig_t  = vec4(0.0F, 0.0F, 0.0F, INF);
            d[i].dir_idx = vec4(0.0F, 0.0F, 0.0F, -0.0F);
            d[i].uv_state = vec4(0, 0, 0, 0);
         }
      }
      glUnmapBuffer(GL_ARRAY_BUFFER);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }
}


static void
Key(unsigned char key, int x, int y)
{
   if (key == 27)
   {
      glutDestroyWindow(Win);
      exit(0);
   }
   glutPostRedisplay();
}


static void
drag(int x, int y)
{
   float scale = 1.5F;
   if (mouseGrabbed)
   {
      static GLfloat xRot = 0, yRot = 0;
      xRot = (float)(x - WinWidth/2) / scale;
      yRot = (float)(y - WinHeight/2) / scale;
      identity(rot);
      rotate_xy(rot, yRot, xRot);
      glutPostRedisplay();
   }
}


static void
mouse(int button, int state, int x, int y)
{
   mouseGrabbed = (state == GLUT_DOWN);
}


static void
Init(void)
{
   glDisable(GL_DEPTH_TEST);

   if (!ShadersSupported())
   {
      fprintf(stderr, "Shaders are not supported!\n");
      exit(-1);
   }

   if (!GLEW_VERSION_3_2)
   {
      fprintf(stderr, "OpenGL 3.2 (needed for transform feedback and "
              "geometry shaders) not supported!\n");
      exit(-1);
   }

   vertShader = CompileShaderText(GL_VERTEX_SHADER, vsSource);
   geomShader = CompileShaderText(GL_GEOMETRY_SHADER_ARB, gsSource);
   fragShader = CompileShaderText(GL_FRAGMENT_SHADER, fsSource);
   program = LinkShaders3(vertShader, geomShader, fragShader);

   const char *varyings[] = {
      "gl_Position",
      "orig_t2",
      "dir_idx2",
      "uv_state2"
   };
   // I think it will be a performance win to use multiple buffer objects to write to
   // instead of using the interleaved mode.
   glTransformFeedbackVaryings(program, 4, varyings, GL_INTERLEAVED_ATTRIBS);
   glLinkProgram(program);

   if (glGetError() != 0)
   {
      fprintf(stderr, "Shaders were not loaded!\n");
      exit(-1);
   }

   if (!glIsShader(vertShader))
   {
      fprintf(stderr, "Vertex shader failed!\n");
      exit(-1);
   }

   if (!glIsShader(geomShader))
   {
      fprintf(stderr, "Geometry shader failed!\n");
      exit(-1);
   }

   if (!glIsShader(fragShader))
   {
      fprintf(stderr, "Fragment shader failed!\n");
      exit(-1);
   }

   if (!glIsProgram(program))
   {
      fprintf(stderr, "Shader program failed!\n");
      exit(-1);
   }

   glUseProgram(program);
   glUniform3f(glGetUniformLocation(program, "cameraPos"), 0,3,5);
   glUniform4f(glGetUniformLocation(program, "backgroundColor"), 0,0,0,1);
   glUniform1i(glGetUniformLocation(program, "emitNoMore"), 1);
   glUniform3f(glGetUniformLocation(program, "lightPos"), 0,8,1);
   glUseProgram(0);

   printf("GL_RENDERER = %s\n",(const char *) glGetString(GL_RENDERER));

   glGenQueries(1, &pgQuery);
   glGenBuffers(1, &dst);
   glGenBuffers(1, &eyeRaysAsPoints);

   GLuint vao;
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);

   printf("\nESC                 = exit demo\nleft mouse + drag   = rotate camera\n\n");
}


int
main(int argc, char *argv[])
{
   glutInitWindowSize(WinWidth, WinHeight);
   glutInit(&argc, argv);

#ifdef HAVE_FREEGLUT
   glutInitContextVersion(3, 2);
   glutInitContextProfile(GLUT_CORE_PROFILE);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
#elif defined __APPLE__
   glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
#else
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
#endif
   Win = glutCreateWindow(argv[0]);

   // glewInit requires glewExperimentel set to true for core profiles.
   // Depending on the glew version it also generates GL_INVALID_ENUM.
   glewExperimental = GL_TRUE;
   glewInit();
   glGetError();

   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutDisplayFunc(Draw);
   glutIdleFunc(Draw);
   glutMouseFunc(mouse);
   glutMotionFunc(drag);
   Init();
   Reshape(WinWidth, WinHeight ); // fix crash under nvidia driver, as Reshape() not being called before rendering, and thus the BO-s were not created
   glutMainLoop();
   glutDestroyWindow(Win);
   return 0;
}

