#  Copyright 2005 Adam Jackson.
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  on the rights to use, copy, modify, merge, publish, distribute, sub
#  license, and/or sell copies of the Software, and to permit persons to whom
#  the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice (including the next
#  paragraph) shall be included in all copies or substantial portions of the
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
#  ADAM JACKSON BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Process this file with autoconf to produce a configure script

# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([xf86-video-nouveau],
        [1.0.18],
        [https://gitlab.freedesktop.org/xorg/driver/xf86-video-nouveau/-/issues/],
        [xf86-video-nouveau])

AC_DEFINE_UNQUOTED([NV_DRIVER_DATE],
                   [$(echo -n \";git log |head -3|tail -1|tr -d '\n';echo -n \")],
                   [Driver date])

AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR(.)

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-xz])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Require X.Org macros 1.8 or later for MAN_SUBSTS set by XORG_MANPAGE_SECTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.8 or later before running autoconf/autogen
           Hint: either install from source, https://gitlab.freedesktop.org/xorg/util/macros.git or,
           depending on you distribution, try package 'xutils-dev' or 'xorg-x11-util-macros'])])

XORG_MACROS_VERSION(1.8)
XORG_DEFAULT_OPTIONS

# Require X.Org server macros (i.e. XORG_DRIVER_CHECK_EXT) to check for required modules
m4_ifndef([XORG_DRIVER_CHECK_EXT],
          [m4_fatal([must install xorg-server macros before running autoconf/autogen
           Hint: either install from source, https://gitlab.freedesktop.org/xorg/xserver.git or,
           depending on your distribution, try package 'xserver-xorg-dev' or 'xorg-x11-server-devel'])])

# Initialize libtool
LT_INIT([disable-static])

AH_TOP([#include "xorg-server.h"])

# Define a configure option for an alternate module directory
AC_ARG_WITH(xorg-module-dir,
            AS_HELP_STRING([--with-xorg-module-dir=DIR],
                           [Default xorg module directory [[default=$libdir/xorg/modules]]]),
            [moduledir="$withval"],
            [moduledir="$libdir/xorg/modules"])

# Store the list of server defined optional extensions in REQUIRED_MODULES
XORG_DRIVER_CHECK_EXT(RANDR, randrproto)
XORG_DRIVER_CHECK_EXT(RENDER, renderproto)
XORG_DRIVER_CHECK_EXT(XV, videoproto)
XORG_DRIVER_CHECK_EXT(DPMSExtension, xextproto)
XORG_DRIVER_CHECK_EXT(DRI2, [dri2proto >= 2.6])

# Checks for libraries
PKG_CHECK_MODULES(LIBDRM, [libdrm >= 2.4.60])
PKG_CHECK_MODULES(LIBDRM_NOUVEAU, [libdrm_nouveau >= 2.4.25])

# Obtain compiler/linker options for the driver dependencies
PKG_CHECK_MODULES(XORG, [xorg-server >= 1.18] xproto fontsproto $REQUIRED_MODULES)
PKG_CHECK_MODULES(XEXT, [xextproto >= 7.0.99.1],
		  HAVE_XEXTPROTO_71="yes"; AC_DEFINE(HAVE_XEXTPROTO_71, 1, [xextproto 7.1 available]),
		  HAVE_XEXTPROTO_71="no")
AM_CONDITIONAL(HAVE_XEXTPROTO_71, [ test "$HAVE_XEXTPROTO_71" = "yes" ])

PKG_CHECK_MODULES([PCIACCESS], [pciaccess >= 0.10])

PKG_CHECK_MODULES(LIBUDEV, [libudev], [LIBUDEV=yes], [LIBUDEV=no])
if test "x$LIBUDEV" = xyes; then
	AC_DEFINE(HAVE_LIBUDEV, 1, [libudev support])
fi
AM_CONDITIONAL(LIBUDEV, [ test "x$LIBUDEV" = "xyes" ] )

# Use -Wall all the time
CFLAGS="$CFLAGS -Wall"

# Check for -minline-all-stringops availability
OLD_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -minline-all-stringops"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #include <stdio.h> ]], [[ ]])],
[ CFLAGS="$OLD_CFLAGS -minline-all-stringops"],[CFLAGS="$OLD_CFLAGS"])

# needed for the next test
CFLAGS="$CFLAGS $XORG_CFLAGS"

AC_SUBST([CFLAGS])
AC_SUBST([moduledir])

DRIVER_NAME=nouveau
AC_SUBST([DRIVER_NAME])

AC_CHECK_DECL(xf86CursorResetCursor,
              [AC_DEFINE(HAVE_XF86_CURSOR_RESET_CURSOR, 1,
              [Have xf86CursorResetCursor API])], [],
              [#include <xorg-server.h>
               #include <xf86Cursor.h>])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	man/Makefile
])
AC_OUTPUT

dnl
dnl Output some configuration info for the user
dnl
echo ""
echo "        prefix:              $prefix"
echo "        exec_prefix:         $exec_prefix"
echo "        libdir:              $libdir"
echo "        includedir:          $includedir"

echo ""
echo "        CFLAGS:              $CFLAGS"
echo "        CXXFLAGS:            $CXXFLAGS"
echo "        Macros:              $DEFINES"

echo ""
echo "        Run '${MAKE-make}' to build xf86-video-nouveau"
echo ""
