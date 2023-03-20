dnl
dnl Process this file with autoconf to create configure.

# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([xorg-cf-files], [1.0.8],
        [https://gitlab.freedesktop.org/xorg/util/cf/-/issues],
        [xorg-cf-files])
AC_CONFIG_SRCDIR([Makefile.am])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-xz])

# Require xorg-macros 1.20 or later: XORG_DEFAULT_NOCODE_OPTIONS
m4_ifndef([XORG_MACROS_VERSION],
	  [m4_fatal([must install xorg-macros 1.20 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.20)
XORG_DEFAULT_NOCODE_OPTIONS

# Require X.Org's font util macros 1.1 or later for XORG_FONTROOTDIR
m4_ifndef([XORG_FONT_MACROS_VERSION],
          [m4_fatal([must install X.Org font-util 1.1 or later before running autoconf/autogen])])
XORG_FONT_MACROS_VERSION(1.1)
XORG_FONTROOTDIR
XORG_FONTSUBDIR([ENCODINGSDIR],[encodingsdir],[encodings])
# Convert ${FONTROOTDIR} to something Imake can handle
AC_PROG_SED
ENCODINGSDIR="`echo ${ENCODINGSDIR} | ${SED} 's/${FONTROOTDIR}/FontDir/'`"

m4_ifdef([AS_HELP_STRING], , [m4_define([AS_HELP_STRING], m4_defn([AC_HELP_STRING]))])

DEFAULT_XCONFDIR="${libdir}/X11/config"
AC_ARG_WITH(config-dir,
	AS_HELP_STRING([--with-config-dir=<path>], [Path to config dir (default: ${libdir}/X11/config)]),
	[XCONFDIR="$withval"],
	[XCONFDIR="$DEFAULT_XCONFDIR"])
AC_SUBST(XCONFDIR)

AC_CONFIG_FILES([
	Makefile
	site.def
])
AC_OUTPUT
