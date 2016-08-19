/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

#include "xorg-server.h"

/* Use Damage extension */
/* #undef DAMAGE */

/* Enable debug support */
/* #undef HAVE_DEBUG */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* DRI is available */
#define HAVE_DRI 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* libudev support */
/* #undef HAVE_LIBUDEV */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* libpciacces available */
#define HAVE_PCIACCESS 1

/* Define to 1 if you have the <pthread.h> header file. */
#define HAVE_PTHREAD_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* xextproto 7.1 available */
#define HAVE_XEXTPROTO_71 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "xf86-video-openchrome"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://bugs.freedesktop.org/enter_bug.cgi?product=xorg&component=Driver/openchrome"

/* Define to the full name of this package. */
#define PACKAGE_NAME "xf86-video-openchrome"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "xf86-video-openchrome 0.5.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "xf86-video-openchrome"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.5.0"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Enable build of registers dumper tool */
/* #undef TOOLS */

/* Version number of package */
#define VERSION "0.5.0"

/* Major version */
#define VIA_MAJOR_VERSION 0

/* Minor version */
#define VIA_MINOR_VERSION 5

/* Patch version */
#define VIA_PATCHLEVEL 0

/* Enable DRI driver support */
#define XF86DRI 1

/* Enable XVideo debug support */
/* #undef XV_DEBUG */

/* Compatibility define for older Xen */
#define X_HAVE_XAAGETROP 1

/* Compatibility define for older Xen */
#define X_NEED_I2CSTART 1

/* Compatibility define for older Xen */
#define X_USE_LINEARFB 1

/* Compatibility define for older Xen */
#define X_USE_REGION_NULL 1

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */
