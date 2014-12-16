# Copyright © 2011 Intel Corporation
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

dnl Process this file with autoconf to create configure.

AC_PREREQ([2.59])
AC_INIT([glw], [8.0.0],
    [https://bugs.freedesktop.org/enter_bug.cgi?product=Mesa])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([dist-bzip2])

dnl Check for progs
AC_PROG_CPP
AC_PROG_CC
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

# Enable quiet compiles on automake 1.11.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Make sure the pkg-config macros are defined
m4_ifndef([PKG_PROG_PKG_CONFIG],
    [m4_fatal([Could not locate the pkg-config autoconf macros.
  These are usually located in /usr/share/aclocal/pkg.m4. If your macros
  are in a different location, try setting the environment variable
  ACLOCAL="aclocal -I/other/macro/dir" before running autoreconf.])])
PKG_PROG_PKG_CONFIG()

AC_ARG_ENABLE([motif],
    [AS_HELP_STRING([--enable-motif],
        [use Motif widgets @<:@default=disabled@:>@])],
    [enable_motif="$enableval"],
    [enable_motif=no])
AM_CONDITIONAL([ENABLE_MOTIF], [test "$enable_motif" = yes])

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

PKG_CHECK_MODULES([GLW],[x11 xt])

MOTIF_CFLAGS=
if test "x$enable_motif" = xyes; then
    AC_PATH_PROG([MOTIF_CONFIG], [motif-config], [no])
    if test "x$MOTIF_CONFIG" != xno; then
        MOTIF_CFLAGS=`$MOTIF_CONFIG --cflags`
        MOTIF_LIBS=`$MOTIF_CONFIG --libs`
    else
        AC_CHECK_HEADER([Xm/PrimitiveP.h], [],
            [AC_MSG_ERROR([Can't locate Motif headers])])
        AC_CHECK_LIB([Xm], [XmGetPixmap], [MOTIF_LIBS="-lXm"],
            [AC_MSG_ERROR([Can't locate Motif Xm library])])
    fi
    # MOTIF_LIBS is prepended to GLW_LIB_DEPS since Xm needs Xt/X11
    GLW_PC_LIB_PRIV="$MOTIF_LIBS $GLW_PC_LIB_PRIV"
    GLW_PC_CFLAGS="$MOTIF_CFLAGS $GLW_PC_CFLAGS"
fi

AC_SUBST([GLW_PC_LIB_PRIV])
AC_SUBST([GLW_PC_CFLAGS])

CFLAGS="$CFLAGS $GL_CFLAGS"
LIBS="$LIBS $GL_LIBS $MOTIF_LIBS"

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
CFLAGS="$CFLAGS $CWARNFLAGS"

PKG_CHECK_MODULES(X11, [x11 xext], [x11_enabled=yes], [enable_x11=no])

AC_CONFIG_FILES([glw.pc])
AC_OUTPUT([
	Makefile
])
