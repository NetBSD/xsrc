#  Copyright 2008 Red Hat, Inc.
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
#  THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Process this file with autoconf to produce a configure script

# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([xf86-video-qxl],
        [0.1.6],
        [https://gitlab.freedesktop.org/xorg/driver/xf86-video-qxl/issues],
        [xf86-video-qxl])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CANONICAL_HOST

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-xz])
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([config.h])
AC_SYS_LARGEFILE

# Require xorg-macros: XORG_DEFAULT_OPTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([you must install X.Org util-macros 1.4 or later (or the corresponding package from your distribution) before running autoconf/autogen])])
XORG_MACROS_VERSION(1.4)
XORG_DEFAULT_OPTIONS

# Initialize libtool
AC_DISABLE_STATIC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AH_TOP([#include "xorg-server.h"])

# Define a configure option for an alternate module directory
AC_ARG_WITH(xorg-module-dir, [  --with-xorg-module-dir=DIR ],
                             [ moduledir="$withval" ],
                             [ moduledir="$libdir/xorg/modules" ])
AC_SUBST(moduledir)

# Store the list of server defined optional extensions in REQUIRED_MODULES
XORG_DRIVER_CHECK_EXT(RANDR, randrproto)
XORG_DRIVER_CHECK_EXT(RENDER, renderproto)
XORG_DRIVER_CHECK_EXT(XV, videoproto)
XORG_DRIVER_CHECK_EXT(XFreeXDGA, xf86dgaproto)

# We can use either version 1 or version 2 of libXfont
PKG_CHECK_EXISTS(xfont2,
                 [xfont_pc=xfont2
                  AC_DEFINE(HAVE_XFONT2,1,[Version 2 of the libXfont library])],
                 [xfont_pc=xfont])

# Obtain compiler/linker options for the driver dependencies
PKG_CHECK_MODULES(XORG, [xorg-server >= 1.0.99.901] xproto fontsproto $xfont_pc $REQUIRED_MODULES)

save_CFLAGS="$CFLAGS"
CFLAGS="$XORG_CFLAGS"
AC_CHECK_DECL(XSERVER_LIBPCIACCESS,
	      [XSERVER_LIBPCIACCESS=yes], [XSERVER_LIBPCIACCESS=no],
	      [#include "xorg-server.h"])
CFLAGS="$save_CFLAGS"

AC_ARG_ENABLE(kms,
	      AS_HELP_STRING([--disable-kms],
	                     [Disable KMS support [[default=enabled]]]),
	     [DRM_MODE="$enableval"],
	     [DRM_MODE=yes])

# Checks for libraries.
if test "x$DRM_MODE" = xyes; then
	# require support for kms cursor hotspot
	PKG_CHECK_MODULES(DRM, [libdrm >= 2.4.46])
else
	DRM_MODE=no
fi

if test "x$XSERVER_LIBPCIACCESS" = xyes; then
    PKG_CHECK_MODULES([PCIACCESS], [pciaccess >= 0.10])
fi
AM_CONDITIONAL(XSERVER_LIBPCIACCESS, test "x$XSERVER_LIBPCIACCESS" = xyes)
save_CFLAGS="$CFLAGS"
CFLAGS="$DRM_CFLAGS $CFLAGS"
if test "x$DRM_MODE" = xyes; then
   	AC_CHECK_HEADER(xf86drmMode.h,[DRM_MODE=yes],[DRM_MODE=no],[#include <stdint.h>
#include <stdlib.h>])
	if test "x$DRM_MODE" = xyes; then
	   	AC_DEFINE(XF86DRM_MODE,1,[DRM kernel modesetting])
        else
                DRM_MODE=no
        fi
fi
CFLAGS="$save_CFLAGS"
AM_CONDITIONAL(DRM_MODE, test x$DRM_MODE = xyes)

enable_qxl=yes
AC_ARG_ENABLE(xspice,
[  --enable-xspice[[=(no|yes|only)]]  Build the xspice driver
                   no      (default) Do not build
                   yes     Build in addition to qxl
                   only    Only build xspice],
[  if test "x$enableval" = "xyes"; then
        enable_xspice=yes
   elif test "x$enableval" = "xonly"; then
        enable_xspice=yes
        enable_qxl=no
   else
        enable_xspice=no
   fi
])

if test "x$enable_xspice" = "xyes"; then
    AC_CHECK_HEADERS(sys/inotify.h)
    AC_CHECK_FUNCS(inotify_init1)
    PKG_CHECK_MODULES([SPICE], [spice-server >= 0.6.3],
    [
        AC_SUBST(SPICE_CFLAGS)
        AC_SUBST(SPICE_LIBS)
    ],
    )
else
    enable_xspice=no
fi

AC_ARG_ENABLE([ccid],
            [AS_HELP_STRING([--enable-ccid],
            [Build the spiceccid SmartCard driver (default is no)])],
            [enable_ccid=$enableval],
            [enable_ccid=no])
AC_ARG_WITH(ccid-module-dir,
            [AS_HELP_STRING([--with-ccid-module-dir=DIR ],
            [Specify the install path for spiceccid driver (default is $libdir/pcsc/drivers/serial)])],
            [ cciddir="$withval" ],
            [ cciddir="$libdir/pcsc/drivers/serial" ])
AC_SUBST(cciddir)
if test "x$enable_ccid" != "xno"; then
    PKG_CHECK_MODULES(LIBPCSCLITE, [libpcsclite])
    PKG_CHECK_MODULES(LIBCACARD, [libcacard])

    if test "x$enable_xspice" = "xno"; then
        AC_MSG_ERROR([Building with ccid requires xspice, but xspice is not enabled])
    fi
fi


AM_CONDITIONAL(BUILD_XSPICE, test "x$enable_xspice" = "xyes")
AM_CONDITIONAL(BUILD_QXL, test "x$enable_qxl" = "xyes")
AM_CONDITIONAL(BUILD_SPICECCID, test "x$enable_ccid" = "xyes")

AC_ARG_ENABLE([udev],
		AS_HELP_STRING([--disable-udev], [Disable libudev support [default=auto]]),
		[enable_udev="$enableval"],
		[enable_udev=auto])
if test "x$enable_udev" != "xno"; then
	PKG_CHECK_MODULES(LIBUDEV, [libudev], [LIBUDEV=yes], [LIBUDEV=no])
	if test "x$LIBUDEV" = xyes; then
		AC_DEFINE(HAVE_LIBUDEV, 1,[libudev support])
	elif test "x$enable_udev" != "xauto"; then
		AC_MSG_ERROR([Building with udev requested but libudev not found])
	fi
fi
AM_CONDITIONAL(LIBUDEV, test x$LIBUDEV = xyes)

PKG_CHECK_MODULES([SPICE_PROTOCOL], [spice-protocol >= 0.12.0])

# AC_CHECK_FILE is not supported when cross compiling
if test "$cross_compiling" = "no" ; then
    AC_CHECK_FILE(.git, [
        GIT_VERSION=`git log -1 --format=%h`
        AC_DEFINE_UNQUOTED([GIT_VERSION], ["$GIT_VERSION"], [Defined if building from git])
        ]
    )
fi

AC_CONFIG_FILES([
                Makefile
                src/Makefile
                src/spiceccid/Makefile
                src/uxa/Makefile
                scripts/Makefile
                examples/Makefile
])
AC_OUTPUT

dnl ==========================================================================
echo "

        xf86-video-qxl $VERSION
        =====================

        prefix:                   ${prefix}
        c compiler:               ${CC}

        drm:                      ${DRM_CFLAGS}
        KMS:                      ${DRM_MODE}
        Build qxl:                ${enable_qxl}
        Build xspice:             ${enable_xspice}
        Build spiceccid:          ${enable_ccid}
"
