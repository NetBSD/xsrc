/* include/dix-config.h.  Generated from dix-config.h.in by configure.  */
/* dix-config.h.in: not at all generated.                      -*- c -*- */

#ifndef _DIX_CONFIG_H_
#define _DIX_CONFIG_H_

/* Support BigRequests extension */
#define BIGREQS 1

/* Builder address */
/* #define BUILDERADDR "tech-x11@NetBSD.org" */

/* Operating System Name */
/* #define OSNAME "NetBSD-7.99.35-i386" */

/* Operating System Vendor */
/* #define OSVENDOR "The NetBSD Foundation" */

/* Builder string */
#define BUILDERSTRING ""

/* Default font path */
/* #define COMPILEDDEFAULTFONTPATH "/usr/X11R7/share/fonts/X11/misc,/usr/X11R7/share/fonts/X11/TTF,/usr/X11R7/share/fonts/X11/OTF,/usr/X11R7/share/fonts/X11/Type1,/usr/X11R7/share/fonts/X11/100dpi,/usr/X11R7/share/fonts/X11/75dpi,/usr/X11R7/share/fonts/X11/cyrillic,/usr/X11R7/lib/X11/fonts/misc,/usr/X11R7/lib/X11/fonts/TTF,/usr/X11R7/lib/X11/fonts/OTF,/usr/X11R7/lib/X11/fonts/Type1,/usr/X11R7/lib/X11/fonts/100dpi,/usr/X11R7/lib/X11/fonts/75dpi,/usr/X11R7/lib/X11/fonts/cyrillic" */

/* Miscellaneous server configuration files path */
#define SERVER_MISC_CONFIG_PATH "/usr/X11R7/lib/xorg"

/* Support Composite Extension */
#define COMPOSITE 1

/* Support Damage extension */
#define DAMAGE 1

/* Use OsVendorVErrorF */
/* #undef DDXOSVERRORF */

/* Use ddxBeforeReset */
/* #undef DDXBEFORERESET */

/* Build DPMS extension */
#define DPMSExtension 1

#if 0 /* notyet */
/* Build DRI3 extension */
#define DRI3 1
#endif

/* Build GLX extension */
#define GLXEXT 1

/* Build GLX DRI loader */
/* #undef GLX_DRI */

/* Path to DRI drivers */
#define DRI_DRIVER_PATH "/usr/X11R7/lib/modules/dri"

/* Support XDM-AUTH*-1 */
#define HASXDMAUTH 1

/* Support SHM */
#define HAS_SHM 1

/* Has backtrace support */
/* #undef HAVE_BACKTRACE */

/* Has libunwind support */
/* #undef HAVE_LIBUNWIND */

/* Define to 1 if you have the <byteswap.h> header file. */
/* #undef HAVE_BYTESWAP_H */

/* Define to 1 if you have the `cbrt' function. */
#define HAVE_CBRT 1

/* Define to 1 if you have the <dbm.h> header file. */
/* #undef HAVE_DBM_H */

/* Define to 1 if you have the declaration of `program_invocation_short_name', and
   to 0 if you don't. */
#define HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME 0

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Have execinfo.h */
/* #undef HAVE_EXECINFO_H */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `ffs' function. */
#define HAVE_FFS 1

/* Define to 1 if you have the `getdtablesize' function. */
#define HAVE_GETDTABLESIZE 1

/* Define to 1 if you have the `getifaddrs' function. */
#define HAVE_GETIFADDRS 1

/* Define to 1 if you have the `getpeereid' function. */
#define HAVE_GETPEEREID 1

/* Define to 1 if you have the `getpeerucred' function. */
/* #undef HAVE_GETPEERUCRED */

/* Define to 1 if you have the `getprogname' function. */
#define HAVE_GETPROGNAME 1

/* Define to 1 if you have the `getzoneid' function. */
/* #undef HAVE_GETZONEID */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Have Quartz */
/* #undef XQUARTZ */

/* Support application updating through sparkle. */
/* #undef XQUARTZ_SPARKLE */

/* Prefix to use for bundle identifiers */
#define BUNDLE_ID_PREFIX ""

/* Build a standalone xpbproxy */
/* #undef STANDALONE_XPBPROXY */

/* Define to 1 if you have the `bsd' library (-lbsd). */
/* #undef HAVE_LIBBSD */

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the libdispatch (GCD) available */
/* #undef HAVE_LIBDISPATCH */

/* Define to 1 if you have the <linux/agpgart.h> header file. */
/* #undef HAVE_LINUX_AGPGART_H */

/* Define to 1 if you have the <linux/apm_bios.h> header file. */
/* #undef HAVE_LINUX_APM_BIOS_H */

/* Define to 1 if you have the <linux/fb.h> header file. */
/* #undef HAVE_LINUX_FB_H */

/* Define to 1 if you have the `mmap' function. */
#define HAVE_MMAP 1

/* Define to 1 if you have the <ndbm.h> header file. */
#define HAVE_NDBM_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the `reallocarray' function. */
/* #undef HAVE_REALLOCARRAY */

/* Define to 1 if you have the <rpcsvc/dbm.h> header file. */
/* #undef HAVE_RPCSVC_DBM_H */

/* Define to 1 if you have the `arc4random_buf' function. */
#define HAVE_ARC4RANDOM_BUF 1

/* Define to use libc SHA1 functions */
#define HAVE_SHA1_IN_LIBC 1

/* Define to use CommonCrypto SHA1 functions */
/* #undef HAVE_SHA1_IN_COMMONCRYPTO */

/* Define to use CryptoAPI SHA1 functions */
/* #undef HAVE_SHA1_IN_CRYPTOAPI */

/* Define to use libmd SHA1 functions */
/* #undef HAVE_SHA1_IN_LIBMD */

/* Define to use libgcrypt SHA1 functions */
/* #undef HAVE_SHA1_IN_LIBGCRYPT */

/* Define to use libnettle SHA1 functions */
/* #undef HAVE_SHA1_IN_LIBNETTLE */

/* Define to use libsha1 for SHA1 */
/* #undef HAVE_SHA1_IN_LIBSHA1 */

/* Define to 1 if you have the `shmctl64' function. */
/* #undef HAVE_SHMCTL64 */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strcasestr' function. */
#define HAVE_STRCASESTR 1

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strlcat' function. */
#define HAVE_STRLCAT 1

/* Define to 1 if you have the `strlcpy' function. */
#define HAVE_STRLCPY 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strndup' function. */
#define HAVE_STRNDUP 1

/* Define to 1 if libsystemd-daemon is available */
/* #undef HAVE_SYSTEMD_DAEMON */

/* Define to 1 if SYSV IPC is available */
#define HAVE_SYSV_IPC 1

/* Define to 1 if you have the <sys/agpio.h> header file. */
#define HAVE_SYS_AGPIO_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/utsname.h> header file. */
#define HAVE_SYS_UTSNAME_H 1

/* Define to 1 if you have the `timingsafe_memcmp' function. */
/* #undef HAVE_TIMINGSAFE_MEMCMP */

/* Define to 1 if you have the <tslib.h> header file. */
/* #undef HAVE_TSLIB_H */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <fnmatch.h> header file. */
#define HAVE_FNMATCH_H 1

/* Have /dev/urandom */
/* #undef HAVE_URANDOM */

/* Define to 1 if you have the `vasprintf' function. */
#define HAVE_VASPRINTF 1

/* Support IPv6 for TCP connections */
#ifndef __NetBSD__	/* Defined by the build */
# define IPv6 1
#endif

/* Support os-specific local connections */
/* #undef LOCALCONN */

/* Support MIT-SHM Extension */
#define MITSHM 1

/* Enable some debugging code */
/* #undef DEBUG */

/* Name of package */
#define PACKAGE "xorg-server"

/* Internal define for Xinerama */
#define PANORAMIX 1

#if 0
/* Support Present extension */
#define PRESENT 1
#endif

/* Overall prefix */
/* #define PROJECTROOT "/usr/X11R7" */

/* Support RANDR extension */
#define RANDR 1

#if 0
/* Support Record extension */
#define XRECORD 1
#endif

/* Support RENDER extension */
#define RENDER 1

/* Support X resource extension */
#define RES 1

/* Support client ID tracking in X resource extension */
#define CLIENTIDS 1

/* Support MIT-SCREEN-SAVER extension */
#define SCREENSAVER 1

/* Support Secure RPC ("SUN-DES-1") authentication for X11 clients */
/* #undef SECURE_RPC */

/* Support SHAPE extension */
#define SHAPE 1

/* Where to install Xorg.bin and Xorg.wrap */
/* #undef SUID_WRAPPER_DIR */

/* Define to 1 on systems derived from System V Release 4 */
/* #undef SVR4 */

/* sysconfdir */
#define SYSCONFDIR "/usr/X11R7/etc"

/* Support TCP socket connections */
#define TCPCONN 1

/* Support tslib touchscreen abstraction library */
/* #undef TSLIB */

/* Support UNIX socket connections */
#define UNIXCONN 1

/* Define to use byteswap macros from <sys/endian.h> */
#define USE_SYS_ENDIAN_H 1

/* unaligned word accesses behave as expected */
/* #undef WORKING_UNALIGNED_INT */

/* Build X string registry */
/* #undef XREGISTRY */

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

#if 0
/* Support XFree86 Video Mode extension */
#define XF86VIDMODE 1
#endif

/* Support XFixes extension */
#define XFIXES 1

#if 0
/* Build XDGA support */
#define XFreeXDGA 1
#endif

/* Support Xinerama extension */
#define XINERAMA 1

/* Vendor release */
/* #undef XORG_RELEASE */

/* Current Xorg version */
#define XORG_VERSION_CURRENT (((1) * 10000000) + ((18) * 100000) + ((4) * 1000) + 0)

/* Xorg release date */
#define XORG_DATE "2016-07-19"

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

#if 0
/* Build DBE support */
#define DBE 1
#endif

/* Vendor name */
#define XVENDORNAME "The X.Org Foundation"

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Enable GNU and other extensions to the C environment for GLIBC */
#define _GNU_SOURCE 1

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

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

/* Use libudev for kms enumeration */
/* #undef CONFIG_UDEV_KMS */

/* Use udev_monitor_filter_add_match_tag() */
/* #undef HAVE_UDEV_MONITOR_FILTER_ADD_MATCH_TAG */

/* Use udev_enumerate_add_match_tag() */
/* #undef HAVE_UDEV_ENUMERATE_ADD_MATCH_TAG */

/* Enable D-Bus core */
/* #undef NEED_DBUS */

/* Support HAL for hotplug */
/* #undef CONFIG_HAL */

/* Enable systemd-logind integration */
/* #undef SYSTEMD_LOGIND */

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

/* Define to 1 if typeof works with your compiler. */
#define HAVE_TYPEOF 1

/* Define to __typeof__ if your compiler spells it that way. */
/* #undef typeof */

/* Correctly set _XSERVER64 for OSX fat binaries */
#ifdef __APPLE__
#include "dix-config-apple-verbatim.h"
#endif

/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/* Defined if needed to expose struct msghdr.msg_control */
/* #undef _XOPEN_SOURCE */

/* Have support for X shared memory fence library (xshmfence) */
#define HAVE_XSHMFENCE 1

/* Use XTrans FD passing support */
#define XTRANS_SEND_FDS 1

/* Wrap SIGBUS to catch MIT-SHM faults */
#define BUSFAULT 1

/* Directory for shared memory temp files */
#define SHMDIR "/var/shm"

/* Don't let Xdefs.h define 'pointer' */
/* #define _XTYPEDEF_POINTER       1 */

/* Don't let XIproto define 'Pointer' */
/* #define _XITYPEDEF_POINTER      1 */

/* Ask fontsproto to make font path element names const */
#define FONT_PATH_ELEMENT_NAME_CONST    1

#if 0 /* not yet! */
/* Build GLAMOR */
#define GLAMOR 1

/* Build glamor's GBM-based EGL support */
#define GLAMOR_HAS_GBM 1

/* Build glamor/gbm has linear support */
#define GLAMOR_HAS_GBM_LINEAR 1
#endif

#if 0
/* byte order */
#define X_BYTE_ORDER X_LITTLE_ENDIAN
#endif

/* Listen on TCP socket */
/* #undef LISTEN_TCP */

/* Listen on Unix socket */
#define LISTEN_UNIX 1

/* Listen on local socket */
#define LISTEN_LOCAL 1

/* Define if no local socket credentials interface exists */
/* #undef NO_LOCAL_CLIENT_CRED */

/* Have posix_fallocate() */
#define HAVE_POSIX_FALLOCATE 1

#endif /* _DIX_CONFIG_H_ */
