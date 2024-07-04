AC_INIT([libpthread-stubs],
        0.5,
        [xcb@lists.freedesktop.org])
AC_CONFIG_SRCDIR([pthread-stubs.pc.in])
AM_INIT_AUTOMAKE([foreign dist-xz])

dnl Check if the following functions have stubs.
dnl See the README for specifics about the list.
funclist="\
pthread_condattr_destroy \
pthread_condattr_init \
pthread_cond_broadcast \
pthread_cond_destroy \
pthread_cond_init \
pthread_cond_signal \
pthread_cond_timedwait \
pthread_cond_wait \
pthread_equal \
pthread_exit \
pthread_mutex_destroy \
pthread_mutex_init \
pthread_mutex_lock \
pthread_mutex_unlock \
pthread_self"

AC_CHECK_FUNCS($funclist, [], [HAVE_STUBS=no])


if test "x$HAVE_STUBS" != xno; then
    PKG_CONFIG_CFLAGS=
    PKG_CONFIG_LIBS=
else
    dnl See the README why '-pthread' is deemed sufficient.
    PKG_CONFIG_CFLAGS="-pthread"
    PKG_CONFIG_LIBS="-pthread"
fi

AC_SUBST([PKG_CONFIG_CFLAGS])
AC_SUBST([PKG_CONFIG_LIBS])

AC_CONFIG_FILES([Makefile pthread-stubs.pc])
AC_OUTPUT
