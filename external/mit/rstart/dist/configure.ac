
dnl  Copyright 2005 Red Hat, Inc.
dnl 
dnl  Permission to use, copy, modify, distribute, and sell this software and its
dnl  documentation for any purpose is hereby granted without fee, provided that
dnl  the above copyright notice appear in all copies and that both that
dnl  copyright notice and this permission notice appear in supporting
dnl  documentation, and that the name of Red Hat not be used in
dnl  advertising or publicity pertaining to distribution of the software without
dnl  specific, written prior permission.  Red Hat makes no
dnl  representations about the suitability of this software for any purpose.  It
dnl  is provided "as is" without express or implied warranty.
dnl 
dnl  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
dnl  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
dnl  EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
dnl  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
dnl  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
dnl  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
dnl  PERFORMANCE OF THIS SOFTWARE.
dnl
dnl Process this file with autoconf to create configure.

# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([rstart], [1.0.5],
        [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], [rstart])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS(config.h)

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

# Require X.Org macros 1.8 or later for AC_PROG_SED in XORG_MANPAGE_SECTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.8 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.8)
XORG_DEFAULT_OPTIONS

# Checks for programs.
AC_CHECK_PROGS(RSH,[remsh rsh rcmd])
AC_PROG_LN_S

# Define the root directory for client/server
AC_SUBST([rstartdir],[$libdir/X11/rstart])

# Define the root directory for config
AC_SUBST([configdir],[$(sysconfdir)/X11/rstart])

# Obtain compiler options from depedencies
PKG_CHECK_MODULES(RSTART, xproto)

# Define a configure option for an alternate user path
AC_ARG_WITH(default-user-path,  [  --with-default-user-path=PATH ],
	[ DEFAULT_USER_PATH="$withval" ],
        [ DEFAULT_USER_PATH="/bin:/usr/bin:${prefix}/bin:/usr/ucb:/usr/gnu/bin:/usr/local/bin" ])
AC_SUBST(DEFAULT_USER_PATH)

# Define a configure option for an alternate man pages path
AC_ARG_WITH(default-man-path,  [  --with-default-man-path=PATH ],
	[ DEFAULT_X_MANPATH="$withval" ],
        [ DEFAULT_X_MANPATH="${prefix}/man:/usr/share/man:/usr/man" ])
AC_SUBST(DEFAULT_X_MANPATH)

AC_CONFIG_FILES([
	Makefile
	man/Makefile
	commands/Makefile
	contexts/Makefile
	samples/Makefile
	scripts/Makefile
	specs/Makefile])
AC_OUTPUT
