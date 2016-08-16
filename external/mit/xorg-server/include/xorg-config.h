/* include/xorg-config.h.  Generated from xorg-config.h.in by configure.  */
/* xorg-config.h.in: not at all generated.                      -*- c -*-
 * 
 * This file differs from xorg-server.h.in in that -server is installed
 * with the rest of the SDK for external drivers/modules to use, whereas
 * -config is for internal use only (i.e. building the DDX).
 *
 */

#ifndef _XORG_CONFIG_H_
#define _XORG_CONFIG_H_

#include <dix-config.h>
#include <xkb-config.h>

/* Building Xorg server. */
#define XORGSERVER 1

/* Current X.Org version. */
#define XORG_VERSION_CURRENT (((1) * 10000000) + ((18) * 100000) + ((4) * 1000) + 0)

/* Name of X server. */
#define __XSERVERNAME__ "Xorg"

/* URL to go to for support. */
/* #define __VENDORDWEBSUPPORT__ "http://wiki.x.org" */

/* Built-in output drivers. */
/* #undef DRIVERS */

/* Built-in input drivers. */
/* #undef IDRIVERS */

/* Path to configuration file. */
#define XF86CONFIGFILE "xorg.conf"

/* Path to configuration file. */
#define __XCONFIGFILE__ "xorg.conf"

/* Name of configuration directory. */
#define __XCONFIGDIR__ "xorg.conf.d"

/* Path to loadable modules. */
/* #define DEFAULT_MODULE_PATH "/usr/X11R7/lib/xorg/modules" */

/* Path to installed libraries. */
/* #define DEFAULT_LIBRARY_PATH "/usr/X11R7/lib" */

/* Default log location */
#define DEFAULT_LOGDIR "/var/log"

/* Default logfile prefix */
#define DEFAULT_LOGPREFIX "Xorg."

/* Default XDG_DATA dir under HOME */
#define DEFAULT_XDG_DATA_HOME ".local/share"

/* Default log dir under XDG_DATA_HOME */
#define DEFAULT_XDG_DATA_HOME_LOGDIR "xorg"

/* Building DRI-capable DDX. */
#define XF86DRI 1

/* Build DRI2 extension */
#define DRI2 1

/* Define to 1 if you have the <stropts.h> header file. */
/* #undef HAVE_STROPTS_H */

/* Define to 1 if you have the <sys/kd.h> header file. */
/* #undef HAVE_SYS_KD_H */

/* Define to 1 if you have the <sys/vt.h> header file. */
/* #undef HAVE_SYS_VT_H */

/* Define to 1 if you have the `walkcontext' function (used on Solaris for
   xorg_backtrace in hw/xfree86/common/xf86Events.c */
/* #undef HAVE_WALKCONTEXT */

/* Define to 1 if unsigned long is 64 bits. */
/* #undef _XSERVER64 */

/* Building vgahw module */
#define WITH_VGAHW 1

/* NetBSD PIO alpha IO */
/* #undef USE_ALPHA_PIO */

/* BSD AMD64 iopl */
/* #undef USE_AMD64_IOPL */

/* BSD /dev/io */
/* #undef USE_DEV_IO */

/* BSD i386 iopl */
/* #undef USE_I386_IOPL */

/* System is BSD-like */
#define CSRG_BASED 1

/* System has PC console */
#define PCCONS_SUPPORT 1

/* System has PCVT console */
#define PCVT_SUPPORT 1

/* System has syscons console */
/* #undef SYSCONS_SUPPORT */

/* System has wscons console */
#define WSCONS_SUPPORT 1

/* System has /dev/xf86 aperture driver */
/* #undef HAS_APERTURE_DRV */

/* Has backtrace support */
/* #undef HAVE_BACKTRACE */

/* Name of the period field in struct kbd_repeat */
/* #undef LNX_KBD_PERIOD_NAME */

/* Have execinfo.h */
/* #undef HAVE_EXECINFO_H */

/* Define to 1 if you have the <sys/mkdev.h> header file. */
/* #undef HAVE_SYS_MKDEV_H */

/* Path to text files containing PCI IDs */
#define PCI_TXT_IDS_PATH ""

/* Use SIGIO handlers for input device events by default */
#define USE_SIGIO_BY_DEFAULT TRUE

/* Build with libdrm support */
#define WITH_LIBDRM 1

/* Use libpciaccess */
#define XSERVER_LIBPCIACCESS 1

/* Have setugid */
#define HAVE_ISSETUGID 1

/* Have getresuid */
/* #undef HAVE_GETRESUID */

/* Have X server platform bus support */
/* #undef XSERVER_PLATFORM_BUS */

/* Define to 1 if you have the `seteuid' function. */
#define HAVE_SETEUID 1

/* Support APM/ACPI power management in the server */
/* #undef XF86PM */

#endif /* _XORG_CONFIG_H_ */
