dnl  Copyright 2007 Peter Hutterer <peter@cs.unisa.edu.au>
dnl Process this file with autoconf to create configure.

AC_PREREQ([2.57])
AC_INIT(xinput,[1.4.0], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],xinput)
AM_INIT_AUTOMAKE([dist-bzip2])
AM_MAINTAINER_MODE

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_INSTALL

# Checks for pkg-config packages
PKG_CHECK_MODULES(XINPUT, x11 xext [xi >= 1.2] [inputproto >= 1.5])

LIBS="$XINPUT_LIBS $LIBS"
AC_CHECK_FUNC(XSetClientPointer,AC_DEFINE(HAVE_XI2,1,[XInput Version 2 Enabled]))
AM_CONDITIONAL(HAVE_XI2, [test "x$ac_cv_func_XSetClientPointer" = "xyes"] )

AC_SUBST(XINPUT_CFLAGS)
AC_SUBST(XINPUT_LIBS)
AC_SUBST(HAVE_XI2)

AC_SUBST(VERSION)

XORG_MANPAGE_SECTIONS
XORG_RELEASE_VERSION

AC_OUTPUT([Makefile
           src/Makefile
           man/Makefile])
