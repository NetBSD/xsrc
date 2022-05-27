dnl
dnl Process this file with autoconf to create configure.

# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([xcursor-themes], [1.0.6],
        [https://gitlab.freedesktop.org/xorg/data/cursors/issues],
        [xcursor-themes])
AC_CONFIG_SRCDIR([Makefile.am])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])

# Require xorg-macros: XORG_DEFAULT_OPTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.3 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.3)
XORG_DEFAULT_OPTIONS

AC_PROG_INSTALL
AC_PROG_LN_S

AC_PATH_PROG(XCURSORGEN, xcursorgen, no)
if test "x$XCURSORGEN" = xno; then
	AC_MSG_ERROR([xcursorgen is required to build these cursors.])
fi
AC_SUBST([XCURSORGEN])

PKG_CHECK_MODULES(ICONDEFS, xcursor)
pkg_cursordir=`$PKG_CONFIG --variable=icondir xcursor`
AC_ARG_WITH(cursordir,
        AC_HELP_STRING([--with-cursordir=<pathname>],
          [specify directory for cursor files (default is autodetected)]),
        [cursordir="$withval"], [cursordir="${pkg_cursordir}"])
AC_SUBST([cursordir])

AM_CONDITIONAL(WHITEGLASS, true)
AM_CONDITIONAL(REDGLASS, true)
AM_CONDITIONAL(HANDHELDS, true)


AC_CONFIG_FILES([Makefile
                handhelds/Makefile
                redglass/Makefile
                whiteglass/Makefile
])
AC_OUTPUT
