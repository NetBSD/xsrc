/* $XFree86: xc/programs/Xserver/PEX5/include/pexos.h,v 1.2 1999/01/31 12:21:33 dawes Exp $ */

/*
 * This file contains all external OS/ANSI header includes, prototypes and
 * definitions.
 */

#ifndef _PEXOS_H_
#define _PEXOS_H_

#ifndef XFree86LOADER
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifdef NEED_OS_LIMITS
#ifndef X_NOT_POSIX
#ifdef _POSIX_SOURCE
#include <limits.h>
#else
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif
#endif /* X_NOT_POSIX */
#ifndef PATH_MAX
#ifdef WIN32
#define PATH_MAX 512
#else
#include <sys/param.h>
#endif
#ifndef PATH_MAX
#ifdef MAXPATHLEN  
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#endif /* PATH_MAX */
#endif /* NEED_OS_LIMITS */

#ifdef NEED_GETENV

#include <X11/Xos.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h> 
#else
extern char *getenv();
#endif
#endif /* NEED_GETENV */

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#else
#include "xf86_ansic.h"
#endif

#endif /* _PEXOS_H_ */
