#ifndef _glx_ansic_h_
#define _glx_ansic_h_

/* $XFree86: xc/programs/Xserver/GL/include/GL/glx_ansic.h,v 1.3 1999/07/11 08:49:18 dawes Exp $ */
/*
** The contents of this file are subject to the GLX Public License Version 1.0
** (the "License"). You may not use this file except in compliance with the
** License. You may obtain a copy of the License at Silicon Graphics, Inc.,
** attn: Legal Services, 2011 N. Shoreline Blvd., Mountain View, CA 94043
** or at http://www.sgi.com/software/opensource/glx/license.html.
**
** Software distributed under the License is distributed on an "AS IS"
** basis. ALL WARRANTIES ARE DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY
** IMPLIED WARRANTIES OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
** PURPOSE OR OF NON- INFRINGEMENT. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Software is GLX version 1.2 source code, released February,
** 1999. The developer of the Original Software is Silicon Graphics, Inc.
** Those portions of the Subject Software created by Silicon Graphics, Inc.
** are Copyright (c) 1991-9 Silicon Graphics, Inc. All Rights Reserved.
**
** $SGI$
*/

/*
** this needs to check whether we're using XFree86 at all, and then
** which version we're using. Use these macros if version is 3.9+, else
** use normal commands below.
*/

/*
** turns out this include file only exists for XFree86 3.9+ 
** I notice that not having it is not an error and does not stop the build,
** but having it will allow opengl and glx to be built for 3.9+. We no longer
** need an explicit define in the Makefile, just point to the correct X source
** tree and all should be taken care of.
*/

#ifdef XFree86Server
#include <xf86Version.h>
#define XF_Minimum_Version ( (3 * 10) + (9) )
#define XF_Version ( (XF86_VERSION_MAJOR * 10) + (XF86_VERSION_MINOR) )
#endif

#if defined(XFree86LOADER) && defined(XFree86Server) && ( XF_Version >= XF_Minimum_Version)
#include "xf86_ansic.h"
#else
#endif

#if 0 && defined(XFree86LOADER) && defined(XFree86Server) && ( XF_Version >= XF_Minimum_Version)

#include "xf86Module.h"

/*
** I'm not sure if this is entirely correct. xf86_ansic needs to be told
** whether to include these definitions or not. Near as I can tell, a
** necessary define is included when XFree86 is built (as part of the build).
** When we build the glx server, this define isn't present. I don't recall
** having to do this before, but this seems to work. Perhaps I just need to
** include the proper header file, but I can't find it. Anyways, if need be
** this can be fixed later, but works for now...
** I'm specifically refering to NEED_XF86_TYPES being declared here.
*/

#define NEED_XF86_TYPES
#include "xf86_ansic.h"

#define GLX_STDOUT			xf86stdout
#define GLX_STDERR			xf86stderr
#define __glXPrintf			xf86printf
#define __glXFprintf			xf86fprintf
#define __glXSprintf			xf86sprintf
#define __glXVfprintf			xf86vfprintf
#define __glXVsprintf			xf86vsprintf
#define __glXFopen			xf86fopen
#define __glXFclose			xf86fclose
#define __glXCos(x)			xf86cos(x)
#define __glXSin(x)			xf86sin(x)
#define __glXAtan(x)			xf86atan(x)
#define __glXAbs(x)			xf86abs(x)
#define __glXLog(x)			xf86log(x)
#define __glXCeil(x)			xf86ceil(x)
#define __glXFloor(x)			xf86floor(x)
#define __glXSqrt(x)			xf86sqrt(x)
#define __glXPow(x, y)			xf86pow(x, y)
#define __glXMemmove(dest, src, n)	xf86memmove(dest, src, n)
#define __glXMemcpy(dest, src, n)	xf86memcpy(dest, src, n)
#define __glXMemset(s, c, n)		xf86memset(s, c, n)
#define __glXStrdup(str)		xf86strdup(str)
#define __glXStrcpy(dest, src)		xf86strcpy(dest, src)
#define __glXStrncpy(dest, src, n)	xf86strncpy(dest, src, n)
#define __glXStrcat(dest, src)		xf86strcat(dest, src)
#define __glXStrncat(dest, src, n)	xf86strncat(dest, src, n)
#define __glXStrcmp(s1, s2)		xf86strcmp(s1, s2)
#define __glXStrncmp(s1, s2, n)		xf86strncmp(s1, s2, n)
#define __glXStrlen(str)		xf86strlen(str)
#define __glXAbort()			xf86abort()
#define __glXStrtok(s, delim)		xf86strtok(s, delim)
#define __glXStrcspn(s, reject)		xf86strcspn(s, reject)
#define __glXGetenv(a)			xf86getenv(a)

#ifndef assert
#define assert(a)
#endif

#else

/*
** Either not a loadable module, or pre X3.9
*/

/* assert() is named __assert() in the LynxOS libc. We might get
 * unresolved externals if we #undef assert and include assert.h
 * if assert.h was already included on our way here....
 */
#if defined(Lynx) && defined(__assert_h)
#undef __assert_h
#endif

#ifdef assert
#undef assert
#endif

#include <assert.h>

#define GLX_STDOUT			stdout
#define GLX_STDERR			stderr
#define __glXPrintf			printf
#define __glXFprintf			fprintf
#define __glXSprintf			sprintf
#define __glXVfprintf			vfprintf
#define __glXVsprintf			vsprintf
#define __glXFopen			fopen
#define __glXFclose			fclose
#define __glXCos(x)			cos(x)
#define __glXSin(x)			sin(x)
#define __glXAtan(x)			atan(x)
#define __glXAbs(x)			abs(x)
#define __glXLog(x)			log(x)
#define __glXCeil(x)			ceil(x)
#define __glXFloor(x)			floor(x)
#define __glXSqrt(x)			sqrt(x)
#define __glXPow(x, y)			pow(x, y)
#define __glXMemmove(dest, src, n)	memmove(dest, src, n)
#define __glXMemcpy(dest, src, n)	memcpy(dest, src, n)
#define __glXMemset(s, c, n)		memset(s, c, n)
#define __glXStrdup(str)		strdup(str)
#define __glXStrcpy(dest, src)		strcpy(dest, src)
#define __glXStrncpy(dest, src, n)	strncpy(dest, src, n)
#define __glXStrcat(dest, src)		strcat(dest, src)
#define __glXStrncat(dest, src, n)	strncat(dest, src, n)
#define __glXStrcmp(s1, s2)		strcmp(s1, s2)
#define __glXStrncmp(s1, s2, n)		strncmp(s1, s2, n)
#define __glXStrlen(str)		strlen(str)
#define __glXAbort()			abort()
#define __glXStrtok(s, delim)		strtok(s, delim)
#define __glXStrcspn(s, reject)		strcspn(s, reject)
#define __glXGetenv(a)			getenv(a)

#endif

#endif /* _glx_ansic_h_ */

