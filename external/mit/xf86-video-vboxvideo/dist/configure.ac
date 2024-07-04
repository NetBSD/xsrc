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
AC_INIT([xf86-video-vboxvideo],
        [1.0.1],
        [https://gitlab.freedesktop.org/xorg/driver/xf86-video-vbox/-/issues],
        [xf86-video-vboxvideo])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR(.)

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-xz])

# Require X.Org macros 1.8 or later for MAN_SUBSTS set by XORG_MANPAGE_SECTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.8 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.8)
XORG_DEFAULT_OPTIONS

# Disable some of the warnings xorg-macros.m4 sets by default that cause
# too much noise here
XORG_TESTSET_CFLAG([BASE_CFLAGS], [-Wno-declaration-after-statement])
XORG_TESTSET_CFLAG([BASE_CFLAGS], [-Wno-unused-but-set-variable])
XORG_TESTSET_CFLAG([BASE_CFLAGS], [-Wno-unused-value])

# Initialize libtool
LT_INIT([disable-static])

AH_TOP([#include "xorg-server.h"])

# Define a configure option for an alternate module directory
AC_ARG_WITH(xorg-module-dir, [  --with-xorg-module-dir=DIR ],
                             [ moduledir="$withval" ],
                             [ moduledir="$libdir/xorg/modules" ])
AC_SUBST(moduledir)

# Store the list of server defined optional extensions in REQUIRED_MODULES
XORG_DRIVER_CHECK_EXT(RANDR, randrproto)
XORG_DRIVER_CHECK_EXT(RENDER, renderproto)
XORG_DRIVER_CHECK_EXT(DPMSExtension, xextproto)

# Obtain compiler/linker options for the driver dependencies
PKG_CHECK_MODULES(XORG, xorg-server >= 1.6 xproto fontsproto $REQUIRED_MODULES)
PKG_CHECK_MODULES(XEXT, [xextproto >= 7.0.99.1],
                  HAVE_XEXTPROTO_71="yes", HAVE_XEXTPROTO_71="no")

if test "x$HAVE_XEXTPROTO_71" = xyes; then
    AC_DEFINE(HAVE_XEXTPROTO_71, 1, [xextproto 7.1 available])
fi

save_CFLAGS="$CFLAGS"
CFLAGS="$XORG_CFLAGS"
AC_CHECK_DECL(xf86ConfigIsaEntity,
	      [AC_DEFINE(HAVE_ISA, 1, [Have ISA support])],
	      [],
	      [#include "xf86.h"])
CFLAGS="$save_CFLAGS"

save_CFLAGS="$CFLAGS"
CFLAGS="$XORG_CFLAGS"
AC_CHECK_DECL(XSERVER_LIBPCIACCESS,
              [XSERVER_LIBPCIACCESS=yes], [XSERVER_LIBPCIACCESS=no],
              [#include "xorg-server.h"])
CFLAGS="$save_CFLAGS"

if test "x$XSERVER_LIBPCIACCESS" = xyes; then
    PKG_CHECK_MODULES([PCIACCESS], [pciaccess >= 0.12.901])
fi

# Checks for library functions
# We don't use strlcat, strlcpy, or timingsafe_memcmp, but check to quiet
# -Wredundant-decls warning from xorg/os.h which will otherwise redefine them.
AC_CHECK_FUNCS([strlcat strlcpy timingsafe_memcmp])

DRIVER_NAME=vboxvideo
AC_SUBST([DRIVER_NAME])

AC_CONFIG_FILES([
                Makefile
                src/Makefile
                man/Makefile
])
AC_OUTPUT
