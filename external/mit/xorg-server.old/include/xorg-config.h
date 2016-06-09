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
#define XORG_VERSION_CURRENT (((1) * 10000000) + ((10) * 100000) + ((6) * 1000) + 0)

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
#define DEFAULT_MODULE_PATH "/usr/X11R7/lib/modules"

/* Path to installed libraries. */
#define DEFAULT_LIBRARY_PATH "/usr/X11R7/lib"

/* Path to server log file. */
#define DEFAULT_LOGPREFIX "/var/log/Xorg."

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

#if defined(_LP64)
/* Define to 1 if unsigned long is 64 bits. */
#define _XSERVER64 1
#endif

/* Building vgahw module */
#define WITH_VGAHW 1

/* Define to 1 if NetBSD built-in MTRR support is available */
#define HAS_MTRR_BUILTIN 1

/* Define to 1 if BSD MTRR support is available */
/* #undef HAS_MTRR_SUPPORT */

/* NetBSD PIO alpha IO */
/* #undef USE_ALPHA_PIO */

/* BSD AMD64 iopl */
/* #undef USE_AMD64_IOPL */

/* BSD /dev/io */
/* #undef USE_DEV_IO */

#if defined(__i386__) || defined(__x86_64__)
/* BSD i386 iopl */
#define USE_I386_IOPL 1
#endif

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

/* Have pci_system_init_dev_mem() */
#define HAVE_PCI_SYSTEM_INIT_DEV_MEM 1

/* Define to 1 if you have the `pci_device_is_boot_vga' function. */
#define HAVE_PCI_DEVICE_IS_BOOT_VGA 1

/* Have pci_enable_device */
#define HAVE_PCI_DEVICE_ENABLE 1

/* Define to 1 if you have the `pci_device_vgaarb_init' function. */
#define HAVE_PCI_DEVICE_VGAARB_INIT 1

/* Path to text files containing PCI IDs */
#define PCI_TXT_IDS_PATH ""

/* Use SIGIO handlers for input device events by default */
#define USE_SIGIO_BY_DEFAULT TRUE

/* Support PC98 */
/* #undef SUPPORT_PC98 */

/* Build with libdrm support */
#define WITH_LIBDRM 1

#endif /* _XORG_CONFIG_H_ */
