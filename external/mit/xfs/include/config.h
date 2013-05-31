/* xfs-config.h.  Generated from xfs-config.h.in by configure.  */
/* xfs-config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if `struct sockaddr_in' has a `sin_len' member */
#define BSD44SOCKETS 1

/* comma-separated list of strings for config file paths when not specified */
#ifndef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE "/etc/X11/fs/config"
#endif

/* Define to 1 if you have the `daemon' function. */
#define HAVE_DAEMON 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `ws2_32' library (-lws2_32). */
/* #undef HAVE_LIBWS2_32 */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if the system has the type `socklen_t'. */
#define HAVE_SOCKLEN_T 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Support IPv6 for TCP connections */
#define IPv6 1

/* Support os-specific local connections */
/* #undef LOCALCONN */

/* Name of package */
#define PACKAGE "xfs"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://bugs.freedesktop.org/enter_bug.cgi?product=xorg"

/* Define to the full name of this package. */
#define PACKAGE_NAME "xfs"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "xfs 1.1.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "xfs"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.1.3"

/* Major version of this package */
#define PACKAGE_VERSION_MAJOR 1

/* Minor version of this package */
#define PACKAGE_VERSION_MINOR 1

/* Patch version of this package */
#define PACKAGE_VERSION_PATCHLEVEL 3

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Support TCP socket connections */
#define TCPCONN 1

/* Support UNIX socket connections */
#define UNIXCONN 1

/* Build support for logging via syslog */
#define USE_SYSLOG 1

/* Version number of package */
#define VERSION "1.1.3"

/* Build support for starting from inetd */
#define XFS_INETD 1

/* Define to 1 if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* # undef _ALL_SOURCE */
#endif

/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Enable extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
