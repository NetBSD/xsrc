/* include/dix-config.h.  Generated from dix-config.h.in by configure.  */
/* dix-config.h.in: not at all generated.                      -*- c -*- */

#ifndef _DIX_CONFIG_H_
#define _DIX_CONFIG_H_

/* Support BigRequests extension */
#define BIGREQS 1

/* Builder address */
/* #define BUILDERADDR "xorg@lists.freedesktop.org" */

/* Operating System Name */
/* #define OSNAME "NetBSD 5.99.40 amd64" */

/* Operating System Vendor */
/* #define OSVENDOR "" */

/* Builder string */
#define BUILDERSTRING ""

/* Default font path */
/* #define COMPILEDDEFAULTFONTPATH "/usr/local/lib/X11/fonts/misc/,/usr/local/lib/X11/fonts/TTF/,/usr/local/lib/X11/fonts/OTF,/usr/local/lib/X11/fonts/Type1/,/usr/local/lib/X11/fonts/100dpi/,/usr/local/lib/X11/fonts/75dpi/" */

/* Miscellaneous server configuration files path */
#define SERVER_MISC_CONFIG_PATH "/usr/X11R7/lib/xorg"

/* Support Composite Extension */
#define COMPOSITE 1

/* Support Damage extension */
#define DAMAGE 1

/* Build for darwin with Quartz support */
/* #undef DARWIN_WITH_QUARTZ */

/* Use OsVendorVErrorF */
/* #undef DDXOSVERRORF */

/* Use ddxBeforeReset */
/* #undef DDXBEFORERESET */

/* Build DPMS extension */
#define DPMSExtension 1

/* Build GLX extension */
#define GLXEXT 1

/* Build GLX DRI loader */
/* #undef GLX_DRI */

/* Path to DRI drivers */
#define DRI_DRIVER_PATH "/usr/X11R7/lib/modules/dri"

/* Support XDM-AUTH*-1 */
#define HASXDMAUTH 1

/* Define to 1 if you have the `getdtablesize' function. */
#define HAS_GETDTABLESIZE 1

/* Define to 1 if you have the `getifaddrs' function. */
#define HAS_GETIFADDRS 1

/* Define to 1 if you have the `getpeereid' function. */
#define HAS_GETPEEREID 1

/* Define to 1 if you have the `getpeerucred' function. */
/* #undef HAS_GETPEERUCRED */

/* Define to 1 if you have the `mmap' function. */
#define HAS_MMAP 1

/* Support SHM */
#define HAS_SHM 1

/* Have the 'strlcpy' function */
#define HAS_STRLCPY 1

/* Define to 1 if you have the <asm/mtrr.h> header file. */
/* #undef HAVE_ASM_MTRR_H */

/* Has backtrace support */
/* #undef HAVE_BACKTRACE */

/* Define to 1 if you have the <byteswap.h> header file. */
/* #undef HAVE_BYTESWAP_H */

/* Define to 1 if you have cbrt */
#define HAVE_CBRT 1

/* Define to 1 if you have the <dbm.h> header file. */
/* #undef HAVE_DBM_H */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Have execinfo.h */
/* #undef HAVE_EXECINFO_H */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `geteuid' function. */
#define HAVE_GETEUID 1

/* Define to 1 if you have the `getisax' function. */
/* #undef HAVE_GETISAX */

/* Define to 1 if you have the `getuid' function. */
#define HAVE_GETUID 1

/* Define to 1 if you have the `getzoneid' function. */
/* #undef HAVE_GETZONEID */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Have Quartz */
/* #undef XQUARTZ */

/* Support application updating through sparkle. */
/* #undef XQUARTZ_SPARKLE */

/* Prefix to use for launchd identifiers */
#define LAUNCHD_ID_PREFIX "org.x"

/* Build a standalone xpbproxy */
/* #undef STANDALONE_XPBPROXY */

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the `link' function. */
#define HAVE_LINK 1

/* Define to 1 if you have the <linux/agpgart.h> header file. */
/* #undef HAVE_LINUX_AGPGART_H */

/* Define to 1 if you have the <linux/apm_bios.h> header file. */
/* #undef HAVE_LINUX_APM_BIOS_H */

/* Define to 1 if you have the <linux/fb.h> header file. */
/* #undef HAVE_LINUX_FB_H */

/* Define to 1 if you have the `mkstemp' function. */
#define HAVE_MKSTEMP 1

/* Define to 1 if you have the <ndbm.h> header file. */
#define HAVE_NDBM_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <rpcsvc/dbm.h> header file. */
/* #undef HAVE_RPCSVC_DBM_H */

/* Define to use libc SHA1 functions */
#define HAVE_SHA1_IN_LIBC 1

/* Define to use CommonCrypto SHA1 functions */
/* #undef HAVE_SHA1_IN_COMMONCRYPTO */

/* Define to use libmd SHA1 functions */
/* #undef HAVE_SHA1_IN_LIBMD */

/* Define to use libgcrypt SHA1 functions */
/* #undef HAVE_SHA1_IN_LIBGCRYPT */

/* Define to use libsha1 for SHA1 */
/* #undef HAVE_SHA1_IN_LIBSHA1 */

/* Define to 1 if you have the `shmctl64' function. */
/* #undef HAVE_SHMCTL64 */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strrchr' function. */
#define HAVE_STRRCHR 1

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL 1

/* Define to 1 if SYSV IPC is available */
#define HAVE_SYSV_IPC 1

/* Define to 1 if you have the <sys/agpio.h> header file. */
#define HAVE_SYS_AGPIO_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/io.h> header file. */
/* #undef HAVE_SYS_IO_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/utsname.h> header file. */
#define HAVE_SYS_UTSNAME_H 1

/* Define to 1 if you have the <sys/vm86.h> header file. */
/* #undef HAVE_SYS_VM86_H */

/* Define to 1 if you have the <tslib.h> header file. */
/* #undef HAVE_TSLIB_H */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <fnmatch.h> header file. */
#define HAVE_FNMATCH_H 1

/* Have /dev/urandom */
/* #undef HAVE_URANDOM */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the `vasprintf' function. */
#define HAVE_VASPRINTF 1

/* Support IPv6 for TCP connections */
#define IPv6 1

/* Support os-specific local connections */
/* #undef LOCALCONN */

/* Support MIT-SHM Extension */
#define MITSHM 1

/* Enable some debugging code */
/* #undef DEBUG */

/* Name of package */
#ifndef PACKAGE
#define PACKAGE "xorg-server"
#endif

/* Internal define for Xinerama */
#define PANORAMIX 1

/* Overall prefix */
/* #define PROJECTROOT "/usr/local" */

/* Support RANDR extension */
#define RANDR 1

/* Support Record extension */
#define XRECORD 1

/* Support RENDER extension */
#define RENDER 1

/* Support X resource extension */
#define RES 1

/* Support MIT-SCREEN-SAVER extension */
#define SCREENSAVER 1

/* Support Secure RPC ("SUN-DES-1") authentication for X11 clients */
/* #undef SECURE_RPC */

/* Support SHAPE extension */
#define SHAPE 1

/* Define to 1 on systems derived from System V Release 4 */
/* #undef SVR4 */

/* Support TCP socket connections */
#define TCPCONN 1

/* Enable touchscreen support */
/* #undef TOUCHSCREEN */

/* Support tslib touchscreen abstraction library */
/* #undef TSLIB */

/* Support UNIX socket connections */
#define UNIXCONN 1

/* Define to use byteswap macros from <sys/endian.h> */
#define USE_SYS_ENDIAN_H 1

/* unaligned word accesses behave as expected */
/* #undef WORKING_UNALIGNED_INT */

/* Build X string registry */
#define XREGISTRY 1

/* Build X-ACE extension */
#define XACE 1

/* Build SELinux extension */
/* #undef XSELINUX */

/* Support XCMisc extension */
#define XCMISC 1

/* Build Security extension */
/* #undef XCSECURITY */

/* Support Xdmcp */
#define XDMCP 1

/* Build XFree86 BigFont extension */
/* #undef XF86BIGFONT */

/* Support XFree86 Video Mode extension */
#define XF86VIDMODE 1

/* Support XFixes extension */
#define XFIXES 1

/* Build XDGA support */
#define XFreeXDGA 1

/* Support Xinerama extension */
#define XINERAMA 1

/* Vendor release */
/* #undef XORG_RELEASE */

/* Current Xorg version */
#define XORG_VERSION_CURRENT (((1) * 10000000) + ((10) * 100000) + ((6) * 1000) + 0)

/* Xorg release date */
#define XORG_DATE "2011-07-08"

/* Build Xv Extension */
#define XvExtension 1

/* Build XvMC Extension */
#define XvMCExtension 1

/* Support XSync extension */
#define XSYNC 1

/* Support XTest extension */
#define XTEST 1

/* Support Xv extension */
#define XV 1

/* Support DRI extension */
#define XF86DRI 1

/* Build DRI2 extension */
#define DRI2 1

/* Build DBE support */
#define DBE 1

/* Vendor name */
#define XVENDORNAME "The X.Org Foundation"

/* Enable GNU and other extensions to the C environment for GLIBC */
/* #undef _GNU_SOURCE */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Build Rootless code */
/* #undef ROOTLESS */

/* Define to 1 if unsigned long is 64 bits. */
#ifdef _LP64
#define _XSERVER64 1
#endif

/* System is BSD-like */
#define CSRG_BASED 1

/* Define to 1 if `struct sockaddr_in' has a `sin_len' member */
#define BSD44SOCKETS 1

/* Support D-Bus */
/* #undef HAVE_DBUS */

/* Use libudev for input hotplug */
/* #undef CONFIG_UDEV */

/* Use D-Bus for input hotplug */
/* #undef CONFIG_NEED_DBUS */

/* Support the D-Bus hotplug API */
/* #undef CONFIG_DBUS_API */

/* Support HAL for hotplug */
/* #undef CONFIG_HAL */

/* Have a monotonic clock from clock_gettime() */
#define MONOTONIC_CLOCK 1

/* Define to 1 if the DTrace Xserver provider probes should be built in */
/* #undef XSERVER_DTRACE */

/* Define to 16-bit byteswap macro */
#define bswap_16 bswap16

/* Define to 32-bit byteswap macro */
#define bswap_32 bswap32

/* Define to 64-bit byteswap macro */
#define bswap_64 bswap64

/* Need the strcasecmp function. */
/* #undef NEED_STRCASECMP */

/* Need the strncasecmp function. */
/* #undef NEED_STRNCASECMP */

/* Need the strcasestr function. */
/* #undef NEED_STRCASESTR */

/* Define to 1 if you have the `ffs' function. */
#define HAVE_FFS 1

/* If the compiler supports a TLS storage class define it to that here */
/* #undef TLS */

/* Correctly set _XSERVER64 for OSX fat binaries */
#ifdef __APPLE__
#include "dix-config-apple-verbatim.h"
#endif

#endif /* _DIX_CONFIG_H_ */
