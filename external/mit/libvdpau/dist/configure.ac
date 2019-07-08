AC_PREREQ(2.60)

AC_INIT(libvdpau, 1.2, [vdpau@lists.freedesktop.org], libvdpau)
AM_INIT_AUTOMAKE([dist-bzip2 foreign])

AC_CONFIG_HEADERS(config.h)

# Check for secure_getenv
AC_USE_SYSTEM_EXTENSIONS
AC_CHECK_FUNCS([__secure_getenv secure_getenv])

# Disable static libraries by default.  Use --enable-static if you really want
# them.
AC_DISABLE_STATIC

# Check for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

# Checks for dependencies.
PKG_CHECK_MODULES(X11, x11)
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

# Check for optional dependencies.
AC_ARG_ENABLE(dri2, AS_HELP_STRING([--disable-dri2], [Disable driver name query through DRI2 (default: auto)]), [DRI2=$enableval], [DRI2=auto])
PKG_CHECK_MODULES(dri2proto, dri2proto >= 2.2, [HAVE_DRI2PROTO=yes], [HAVE_DRI2PROTO=no])
case "$DRI2,$HAVE_DRI2PROTO" in
    yes,no)
        AC_MSG_ERROR([DRI2 queries require dri2proto >= 2.2])
        ;;
    yes,yes | auto,yes)
        AC_DEFINE(DRI2, 1, [Request driver name from DRI2])
        DRI2=yes
        PKG_CHECK_MODULES(XEXT, xext)
        AC_SUBST([XEXT_CFLAGS])
        AC_SUBST([XEXT_LIBS])
        ;;
esac
AM_CONDITIONAL(DRI2, test "x$DRI2" = xyes)

dnl Check to see if dlopen is in default libraries (like Solaris, which
dnl has it in libc), or if libdl is needed to get it.
AC_CHECK_FUNC([dlopen], [],
        AC_CHECK_LIB([dl], [dlopen], DLOPEN_LIBS="-ldl"))
AC_SUBST([DLOPEN_LIBS])

# Look for POSIX threads
AC_SEARCH_LIBS([pthread_once], [pthread pthreads c_r], [
    AS_IF([test "$ac_cv_search_pthread_once" != "none required"], [
        PTHREAD_LIBS="$ac_cv_search_pthread_once"
    ])
])
AC_SUBST([PTHREAD_LIBS])

# Check for Doxygen.
AC_ARG_ENABLE(documentation, AS_HELP_STRING([--disable-documentation], [Disable Doxygen documentation (default: auto)]), [DOCS=$enableval], [DOCS=auto])
if test "x$DOCS" != xno; then
    AC_CHECK_TOOL([DOXYGEN], [doxygen], [no])
    AC_CHECK_TOOL([DOT], [dot], [no])
else
    DOXYGEN=no
    DOT=no
fi
if test "x$DOCS" = xyes; then
    if test "x$DOXYGEN" = xno; then
        AC_ERROR([Documentation enabled but doxygen was not found in your path])
    fi
    if test "x$DOT" = xno; then
        AC_ERROR([Documentation enabled but dot was not found in your path.  Please install graphviz])
    fi
fi
AM_CONDITIONAL([ENABLE_DOCS], [test "x$DOXYGEN" != xno -a "x$DOT" != xno])
AC_SUBST(DOXYGEN)

# Options
AC_ARG_WITH(module-dir,
            AC_HELP_STRING([--with-module-dir=DIR],
                           [Default module directory [[default=LIBDIR/vdpau]]]),
            [moduledir="$withval"],
            [moduledir="$libdir/vdpau"])
AC_SUBST(moduledir)

XORG_CHANGELOG

AC_OUTPUT([Makefile
           doc/Makefile
           src/Makefile
           test/Makefile
           trace/Makefile
           vdpau.pc])
