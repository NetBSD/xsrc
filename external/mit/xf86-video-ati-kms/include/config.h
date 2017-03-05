/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

#include "xorg-server.h"

/* Define to 1 if you have the <byteswap.h> header file. */
/* #undef HAVE_BYTESWAP_H */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <dri3.h> header file. */
#undef HAVE_DRI3_H

/* Have fbGlyphs API */
#undef HAVE_FBGLYPHS

/* Have glamor_egl_destroy_textured_pixmap API */
#undef HAVE_GLAMOR_EGL_DESTROY_TEXTURED_PIXMAP

/* Have glamor_glyphs_init API */
#undef HAVE_GLAMOR_GLYPHS_INIT

/* Define to 1 if you have the <glamor.h> header file. */
/* #undef HAVE_GLAMOR_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* libudev support */
/* #undef HAVE_LIBUDEV */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <misyncshm.h> header file. */
/* #define HAVE_MISYNCSHM_H 1 */

/* Define to 1 if you have the <present.h> header file. */
/* #define HAVE_PRESENT_H 1 */

/* Have RegionDuplicate API */
#define HAVE_REGIONDUPLICATE 1

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

/* xextproto 7.1 available */
#define HAVE_XEXTPROTO_71 1

// XXXMRG
/* Have xf86CursorResetCursor API */
#undef HAVE_XF86_CURSOR_RESET_CURSOR

/* Have xorg_list API */
/* #undef HAVE_XORG_LIST */

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Name of package */
#define PACKAGE "xf86-video-ati"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://bugs.freedesktop.org/enter_bug.cgi?product=xorg"

/* Define to the full name of this package. */
#define PACKAGE_NAME "xf86-video-ati"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "xf86-video-ati 7.7.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "xf86-video-ati"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "7.7.0"

/* Major version of this package */
#define PACKAGE_VERSION_MAJOR 7

/* Minor version of this package */
#define PACKAGE_VERSION_MINOR 7

/* Patch version of this package */
#define PACKAGE_VERSION_PATCHLEVEL 0

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Enable glamor acceleration */
/* #undef USE_GLAMOR */

/* Define to use byteswap macros from <sys/endian.h> */
#define USE_SYS_ENDIAN_H 1

/* Version number of package */
#define VERSION "7.7.0"

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 16-bit byteswap macro */
#define bswap_16 bswap16

/* Define to 32-bit byteswap macro */
#define bswap_32 bswap32

/* Define to 64-bit byteswap macro */
#define bswap_64 bswap64
