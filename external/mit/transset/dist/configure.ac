
# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([transset], [1.0.2],
	[https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], [transset])
RELEASE_DATE="2018-03-09"
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])

# Require xorg-macros 1.8 or later for MAN_SUBSTS set by XORG_MANPAGE_SECTIONS
m4_ifndef([XORG_MACROS_VERSION],
	  [m4_fatal([must install xorg-macros 1.8 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.8)
XORG_DEFAULT_OPTIONS

# Obtain compiler/linker options for the transset dependencies
PKG_CHECK_MODULES(TRANSSET, x11 xproto >= 7.0.17)

AC_DEFINE_UNQUOTED(RELEASE_DATE, ["$RELEASE_DATE"],
                                 [Release date of this package])

AC_CONFIG_FILES([
	Makefile
	man/Makefile])
AC_OUTPUT
