/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

#ifndef _CONFIG_H

# define _CONFIG_H

# include "xorg-server.h"

/* Add support for AtomBIOS tables */
#define ATOM_BIOS 1

/* Add support for AtomBIOS code */
#define ATOM_BIOS_PARSER 1

/* Use Damage extension */
#define DAMAGE 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* have RotateFreeShadow API */
#define HAVE_FREE_SHADOW 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Have pci_device_enable prototype */
#define HAVE_PCI_DEVICE_ENABLE 1

/* Define to 1 if you have <pci/pci.h>. */
/* #undef HAVE_PCI_PCI_H */

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
/* #undef HAVE_XEXTPROTO_71 */

/* Define to 1 if you have <${sdkdir}/xf86_ansic.h>. */
/* #undef HAVE_XF86_ANSIC_H */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* MonRec has member maxPixClock */
#define MONREC_HAS_BANDWIDTH 1

/* MonRec has member reducedblanking */
#define MONREC_HAS_REDUCED 1

/* Name of package */
#define PACKAGE "xf86-video-radeonhd"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://bugs.freedesktop.org/enter_bug.cgi?product=xorg&component=Driver/radeonhd"

/* Define to the full name of this package. */
#define PACKAGE_NAME "xf86-video-radeonhd"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "xf86-video-radeonhd 1.3.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "xf86-video-radeonhd"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.3.0"

/* Major Version */
#define PACKAGE_VERSION_MAJOR 1

/* Minor Version */
#define PACKAGE_VERSION_MINOR 3

/* Patchlevel Version */
#define PACKAGE_VERSION_PATCHLEVEL 0

/* Build RandR 1.2 Support */
#define RANDR_12_SUPPORT 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Build support for DRI */
/* #undef USE_DRI */

/* Build support for Exa */
#define USE_EXA 1

/* Version number of package */
#define VERSION "1.3.0"

/* xf86CrtcFuncsRec has member set_mode_major */
#define XF86CRTCFUNCS_HAS_SETMODEMAJOR 1

#endif
