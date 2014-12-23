# Copyright © 2009 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Authors:
#    Eric Anholt <eric@anholt.net>

dnl Process this file with autoconf to create configure.

AC_PREREQ([2.59])
AC_INIT([mesa-demos], [8.2.0],
    [https://bugs.freedesktop.org/enter_bug.cgi?product=Mesa&component=Demos])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([dist-bzip2])

dnl Check for progs
AC_PROG_CPP
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

AC_CONFIG_MACRO_DIR([m4])

# Enable quiet compiles on automake 1.11.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Make sure the pkg-config macros are defined
m4_ifndef([PKG_PROG_PKG_CONFIG],
    [m4_fatal([Could not locate the pkg-config autoconf macros.
  These are usually located in /usr/share/aclocal/pkg.m4. If your macros
  are in a different location, try setting the environment variable
  ACLOCAL="aclocal -I/other/macro/dir" before running autoreconf.])])
PKG_PROG_PKG_CONFIG()

dnl Get the pkg-config definitions for libGL.  We include a fallback
dnl path for GL implementation that don't provide a .pc file
PKG_CHECK_MODULES(GL, [gl], [], [
		  AC_CHECK_HEADER([GL/gl.h],
		  		  [],
		  		  AC_MSG_ERROR([GL not found]))
		  AC_CHECK_LIB([GL],
			       [glBegin],
			       [GL_LIBS=-lGL],
			       AC_MSG_ERROR([GL required]))
		  ])

DEMO_CFLAGS="$DEMO_CFLAGS $GL_CFLAGS"
DEMO_LIBS="$DEMO_LIBS $GL_LIBS"

dnl Check for GLUT
GLUT_CFLAGS=""
GLUT_LIBS=-lglut
glut_enabled=yes
AC_ARG_WITH([glut],
	[AS_HELP_STRING([--with-glut=DIR],
			[glut install directory])],
	[GLUT_CFLAGS="-I$withval/include"
	 GLUT_LIBS="-L$withval/lib -lglut"])
AC_CHECK_HEADER([GL/glut.h],
		[],
		[glut_enabled=no])
AC_CHECK_LIB([glut],
		[glutInit],
		[],
		[glut_enabled=no])

dnl Check for FreeGLUT 2.6 or later
AC_EGREP_HEADER([glutInitContextProfile],
		[GL/freeglut.h],
		[AC_DEFINE(HAVE_FREEGLUT)],
		[])

dnl Check for GLEW
PKG_CHECK_MODULES(GLEW, [glew >= 1.5.4])
DEMO_CFLAGS="$DEMO_CFLAGS $GLEW_CFLAGS"
DEMO_LIBS="$DEMO_LIBS $GLEW_LIBS"

# LIBS was set by AC_CHECK_LIB above
LIBS=""

PKG_CHECK_MODULES(GLU, [glu], [],
		  [AC_CHECK_HEADER([GL/glu.h],
				   [],
		  		   AC_MSG_ERROR([GLU not found]))
		   AC_CHECK_LIB([GLU],
				[gluBeginCurve],
				[GLU_LIBS=-lGLU],
				AC_MSG_ERROR([GLU required])) ])

DEMO_CFLAGS="$DEMO_CFLAGS $GLU_CFLAGS"
DEMO_LIBS="$DEMO_LIBS $GLU_LIBS"

AC_ARG_ENABLE([egl],
    [AS_HELP_STRING([--enable-egl],
        [enable EGL library @<:@default=auto@:>@])],
    [egl_enabled="$enableval"],
    [egl_enabled=auto])
if test "x$egl_enabled" != "xno"; then
    PKG_CHECK_MODULES(EGL, [egl], [egl_enabled=yes], [egl_enabled=no])
fi
AC_ARG_ENABLE([gles1],
    [AS_HELP_STRING([--enable-gles1],
        [enable support for OpenGL ES 1.x API @<:@default=auto@:>@])],
    [glesv1_enabled="$enableval"],
    [glesv1_enabled=auto])
if test "x$glesv1_enabled" != "xno"; then
    PKG_CHECK_MODULES(GLESV1, [glesv1_cm], [glesv1_enabled=yes], [glesv1_enabled=no])
fi
AC_ARG_ENABLE([gles2],
    [AS_HELP_STRING([--enable-gles2],
        [enable support for OpenGL ES 2.x API @<:@default=auto@:>@])],
    [glesv2_enabled="$enableval"],
    [glesv2_enabled=auto])
if test "x$glesv2_enabled" != "xno"; then
    PKG_CHECK_MODULES(GLESV2, [glesv2], [glesv2_enabled=yes], [glesv2_enabled=no])
fi
AC_ARG_ENABLE([vg],
    [AS_HELP_STRING([--enable-vg],
        [enable support for OpenVG API @<:@default=auto@:>@])],
    [vg_enabled="$enableval"],
    [vg_enabled=auto])
if test "x$vg_enabled" != "xno"; then
    PKG_CHECK_MODULES(VG, [vg], [vg_enabled=yes], [vg_enabled=no])
fi
AC_ARG_ENABLE([osmesa],
    [AS_HELP_STRING([--enable-osmesa],
        [enable OSMesa library @<:@default=auto@:>@])],
    [osmesa_enabled="$enableval"],
    [osmesa_enabled=auto])
if test "x$osmesa_enabled" != "xno"; then
    PKG_CHECK_MODULES(OSMESA, [osmesa], [osmesa_enabled=yes], [osmesa_enabled=no])
fi
AC_ARG_ENABLE([libdrm],
    [AS_HELP_STRING([--enable-libdrm],
        [enable support for libdrm @<:@default=auto@:>@])],
    [drm_enabled="$enableval"],
    [drm_enabled=auto])
if test "x$drm_enabled" != "xno"; then
    PKG_CHECK_MODULES(DRM, [libdrm], [drm_enabled=yes], [drm_enabled=no])
fi
dnl The OSMesa .pc uses OSMesa32, while we want to build with other versions
dnl too.
OSMESA32_LIBS=$OSMESA_LIBS
OSMESA16_LIBS=`echo $OSMESA_LIBS | sed 's|32|16|g'`
OSMESA_LIBS=`echo $OSMESA_LIBS | sed 's|32||g'`

dnl Compiler macros
case "$host_os" in
linux*|*-gnu*|gnu*)
    DEMO_CFLAGS="$DEMO_CFLAGS -D_GNU_SOURCE -DPTHREADS"
    ;;
solaris*)
    DEMO_CFLAGS="$DEFINES -DPTHREADS -DSVR4"
    ;;
cygwin*)
    DEMO_CFLAGS="$DEFINES -DPTHREADS"
    ;;
esac

dnl Set up C warning flags.  Copy of XORG_CWARNFLAGS.
if  test "x$GCC" = xyes ; then
    CWARNFLAGS="-Wall -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes \
-Wmissing-declarations -Wnested-externs -fno-strict-aliasing \
-Wbad-function-cast"
    case `$CC -dumpversion` in
    3.4.* | 4.*)
	CWARNFLAGS="$CWARNFLAGS -Wold-style-definition -Wdeclaration-after-statement"
	;;
    esac
else
    AC_CHECK_DECL([__SUNPRO_C], [SUNCC="yes"], [SUNCC="no"])
    if test "x$SUNCC" = "xyes"; then
	CWARNFLAGS="-v"
    fi
fi
DEMO_CFLAGS="$DEMO_CFLAGS $CWARNFLAGS"

AC_ARG_ENABLE([x11],
    [AS_HELP_STRING([--enable-x11],
        [enable support for X11 @<:@default=auto@:>@])],
    [x11_enabled="$enableval"],
    [x11_enabled=auto])
if test "x$x11_enabled" != "xno"; then
    PKG_CHECK_MODULES(X11, [x11 xext], [x11_enabled=yes], [x11_enabled=no])
fi

AC_ARG_ENABLE([wayland],
    [AS_HELP_STRING([--enable-wayland],
        [enable support for wayland @<:@default=no@:>@])],
    [wayland_enabled="$enableval"],
    [wayland_enabled=no])

if test "x$wayland_enabled" != "xno"; then
    PKG_CHECK_MODULES(WAYLAND, [wayland-client wayland-egl], [wayland_enabled=yes], [wayland_enabled=no])
fi

dnl GBM is needed for EGL on KMS
AC_ARG_ENABLE([gbm],
    [AS_HELP_STRING([--enable-gbm],
        [enable support for GBM @<:@default=auto@:>@])],
    [gbm_enabled="$enableval"],
    [gbm_enabled=auto])
if test "x$gbm_enabled" != "xno"; then
    PKG_CHECK_MODULES(GBM, [gbm], [gbm_enabled=yes], [gbm_enabled=no])
fi

dnl FreeType2 is needed by an OpenVG demo
AC_ARG_ENABLE([freetype2],
    [AS_HELP_STRING([--enable-freetype2],
        [enable support for FreeType2 @<:@default=auto@:>@])],
    [freetype2_enabled="$enableval"],
    [freetype2_enabled=auto])
if test "x$freetype2_enabled" != "xno"; then
    PKG_CHECK_MODULES(FREETYPE2, [freetype2], [freetype2_enabled=yes], [freetype2_enabled=no])
fi

mesa_source_enabled=no
AC_ARG_WITH([mesa-source],
	[AS_HELP_STRING([--with-mesa-source=DIR],
			[Mesa source directory for finding gl_API.xml])],
	[mesa_source_enabled="yes"
	 mesa_source_dir="$withval"],
	[mesa_source_enabled="no"])
MESA_GLAPI="$mesa_source_dir/src/mapi/glapi/gen"

AC_ARG_ENABLE([rbug],
    [AS_HELP_STRING([--enable-rbug],
        [enable support for rbug @<:@default=no@:>@])],
    [rbug_enabled="yes"],
    [rbug_enabled="no"])

use_system_data=no
AC_ARG_WITH([system-data-files],
	    [AS_HELP_STRING([--with-system-data-files],
			    [Use system data files for demos (default: no)])],
	    [use_system_data=yes], [])
if test "x$use_system_data" = xyes; then
    demos_data_dir="${datadir}/${PACKAGE}/"
else
    demos_data_dir="../data/"
fi
AC_DEFINE_DIR(DEMOS_DATA_DIR, demos_data_dir, [Data dir for demos])
AC_DEFINE_UNQUOTED([DEMOS_DATA_DIR], "$DEMOS_DATA_DIR",
		   [Directory for demos data files])

dnl
dnl Program library dependencies
dnl    Only libm is added here if necessary as the libraries should
dnl    be pulled in by the linker
dnl
case "$host_os" in
solaris*)
    DEMO_LIBS="$DEMO_LIBS -lX11 -lsocket -lnsl -lm"
    ;;
cygwin*)
    DEMO_LIBS="$DEMO_LIBS -lX11"
    ;;
*)
    DEMO_LIBS="$DEMO_LIBS -lm"
    ;;
esac

AC_SUBST([DEMO_CFLAGS])
AC_SUBST([DEMO_LIBS])
AC_SUBST([EGL_CFLAGS])
AC_SUBST([EGL_LIBS])
AC_SUBST([GLESV1_CFLAGS])
AC_SUBST([GLESV1_LIBS])
AC_SUBST([GLESV2_CFLAGS])
AC_SUBST([GLESV2_LIBS])
AC_SUBST([GLUT_CFLAGS])
AC_SUBST([GLUT_LIBS])
AC_SUBST([X11_CFLAGS])
AC_SUBST([X11_LIBS])
AC_SUBST([OSMESA_CFLAGS])
AC_SUBST([OSMESA_LIBS])
AC_SUBST([OSMESA16_LIBS])
AC_SUBST([OSMESA32_LIBS])
AC_SUBST([MESA_GLAPI])
AC_SUBST([WAYLAND_CFLAGS])
AC_SUBST([WAYLAND_LIBS])


AM_CONDITIONAL(HAVE_EGL, test "x$egl_enabled" = "xyes")
AM_CONDITIONAL(HAVE_GLESV1, test "x$glesv1_enabled" = "xyes")
AM_CONDITIONAL(HAVE_GLESV2, test "x$glesv2_enabled" = "xyes")
AM_CONDITIONAL(HAVE_VG, test "x$vg_enabled" = "xyes")
AM_CONDITIONAL(HAVE_GLUT, test "x$glut_enabled" = "xyes")
AM_CONDITIONAL(HAVE_X11, test "x$x11_enabled" = "xyes")
AM_CONDITIONAL(HAVE_GBM, test "x$gbm_enabled" = "xyes")
AM_CONDITIONAL(HAVE_FREETYPE2, test "x$freetype2_enabled" = "xyes")
AM_CONDITIONAL(HAVE_OSMESA, test "x$osmesa_enabled" = "xyes")
AM_CONDITIONAL(HAVE_DRM, test "x$drm_enabled" = "xyes")
AM_CONDITIONAL(HAVE_MESA_SOURCE, test "x$mesa_source_enabled" = "xyes")
AM_CONDITIONAL(HAVE_WAYLAND, test "x$wayland_enabled" = "xyes")
AM_CONDITIONAL(HAVE_RBUG, test "x$rbug_enabled" = "xyes")

AC_OUTPUT([
	Makefile
	src/Makefile
	src/demos/Makefile
	src/egl/Makefile
	src/egl/eglut/Makefile
	src/egl/opengl/Makefile
	src/egl/opengles1/Makefile
	src/egl/opengles2/Makefile
	src/egl/openvg/Makefile
	src/egl/openvg/trivial/Makefile
	src/egl/oes_vg/Makefile
	src/fp/Makefile
	src/fpglsl/Makefile
	src/glsl/Makefile
	src/gs/Makefile
	src/data/Makefile
	src/objviewer/Makefile
	src/osdemos/Makefile
	src/perf/Makefile
	src/rbug/Makefile
	src/redbook/Makefile
	src/samples/Makefile
	src/slang/Makefile
	src/tests/Makefile
	src/tools/Makefile
	src/trivial/Makefile
	src/util/Makefile
	src/vp/Makefile
	src/vpglsl/Makefile
	src/wgl/Makefile
	src/xdemos/Makefile
])
