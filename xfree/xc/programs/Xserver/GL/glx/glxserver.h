#ifndef _GLX_server_h_
#define _GLX_server_h_

/* $XFree86: xc/programs/Xserver/GL/glx/glxserver.h,v 1.2 1999/06/14 07:31:32 dawes Exp $ */
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

#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xmd.h>
#include <misc.h>
#include <dixstruct.h>
#include <pixmapstr.h>
#include <gcstruct.h>
#include <extnsionst.h>
#include <resource.h>
#include <scrnintstr.h>
#include "GL/glx_ansic.h"


/*
** The X header misc.h defines these math functions.
*/
#undef abs
#undef fabs

#include <GL/gl.h>
#include <GL/glxproto.h>
#include <GL/glxint.h>

/* For glxscreens.h */
typedef struct __GLXdrawablePrivateRec __GLXdrawablePrivate;

#include "glxscreens.h"
#include "glxdrawable.h"
#include "glxcontext.h"
#include "glxerror.h"


#define GLX_SERVER_MAJOR_VERSION 1
#define GLX_SERVER_MINOR_VERSION 2

#ifndef True
#define True 1
#endif
#ifndef False
#define False 0
#endif

/*
** GLX resources.
*/
typedef XID GLXContextID;
typedef XID GLXPixmap;
typedef XID GLXDrawable;

typedef struct __GLXcontextRec *GLXContext;
typedef struct __GLXclientStateRec __GLXclientState;

extern __GLXscreenInfo *__glXActiveScreens;
extern GLint __glXNumActiveScreens;

/************************************************************************/

/*
** The last context used (from the server's persective) is cached.
*/
extern __GLXcontext *__glXLastContext;
extern __GLXcontext *__glXForceCurrent(__GLXclientState*, GLXContextTag, int*);

/*
** Macros to set, unset, and retrieve the flag that says whether a context
** has unflushed commands.
*/
#define __GLX_NOTE_UNFLUSHED_CMDS(glxc) glxc->hasUnflushedCommands = GL_TRUE
#define __GLX_NOTE_FLUSHED_CMDS(glxc) glxc->hasUnflushedCommands = GL_FALSE
#define __GLX_HAS_UNFLUSHED_CMDS(glxc) (glxc->hasUnflushedCommands)

/************************************************************************/

/*
** State kept per client.
*/
struct __GLXclientStateRec {
    /*
    ** Whether this structure is currently being used to support a client.
    */
    Bool inUse;

    /*
    ** Buffer for returned data.
    */
    GLbyte *returnBuf;
    GLint returnBufSize;
    
    /*
    ** Keep track of large rendering commands, which span multiple requests.
    */
    GLint largeCmdBytesSoFar;		/* bytes received so far	*/
    GLint largeCmdBytesTotal;		/* total bytes expected		*/
    GLint largeCmdRequestsSoFar;	/* requests received so far	*/
    GLint largeCmdRequestsTotal;	/* total requests expected	*/
    GLbyte *largeCmdBuf;
    GLint largeCmdBufSize;

    /*
    ** Keep a list of all the contexts that are current for this client's
    ** threads.
    */
    __GLXcontext **currentContexts;
    GLint numCurrentContexts;
    
    /* Back pointer to X client record */
    ClientPtr client;

    int GLClientmajorVersion;
    int GLClientminorVersion;
    char *GLClientextensions;
};

extern __GLXclientState *__glXClients[];

/************************************************************************/

/*
** Dispatch tables.
*/
typedef void (*__GLXdispatchRenderProcPtr)(GLbyte *);
typedef int (*__GLXdispatchSingleProcPtr)(__GLXclientState *, GLbyte *);
typedef int (*__GLXdispatchVendorPrivProcPtr)(__GLXclientState *, GLbyte *);
extern __GLXdispatchRenderProcPtr __glXRenderTable[];
extern __GLXdispatchSingleProcPtr __glXSingleTable[];
extern __GLXdispatchVendorPrivProcPtr __glXVendorPrivTable_EXT[];
extern __GLXdispatchRenderProcPtr __glXSwapRenderTable[];
extern __GLXdispatchSingleProcPtr __glXSwapSingleTable[];
extern __GLXdispatchVendorPrivProcPtr __glXSwapVendorPrivTable_EXT[];

/*
 * Dispatch for GLX commands.
 */
typedef int (*__GLXprocPtr)(__GLXclientState *, char *pc);
extern __GLXprocPtr __glXProcTable[];

/*
 * Tables for computing the size of each rendering command.
 */
typedef struct {
    int bytes;
    int (*varsize)(GLbyte *pc, Bool swap);
} __GLXrenderSizeData;
extern __GLXrenderSizeData __glXRenderSizeTable[];
extern __GLXrenderSizeData __glXRenderSizeTable_EXT[];
  
/************************************************************************/

/*
** X resources.
*/
extern RESTYPE __glXContextRes;
extern RESTYPE __glXClientRes;
extern RESTYPE __glXPixmapRes;
extern RESTYPE __glXDrawableRes;

/************************************************************************/

/*
** Prototypes.
*/


extern char *__glXcombine_strings(const char *, const char *);

extern void __glXDisp_DrawArrays(GLbyte*);
extern void __glXDispSwap_DrawArrays(GLbyte*);


/*
** Routines for sending swapped replies.
*/

extern void __glXSwapMakeCurrentReply(ClientPtr client,
				      xGLXMakeCurrentReply *reply);
extern void __glXSwapIsDirectReply(ClientPtr client,
				   xGLXIsDirectReply *reply);
extern void __glXSwapQueryVersionReply(ClientPtr client,
				       xGLXQueryVersionReply *reply);
extern void __glXSwapQueryContextInfoEXTReply(ClientPtr client,
					      xGLXQueryContextInfoEXTReply *reply,
					      int *buf);
extern void glxSwapQueryExtensionsStringReply(ClientPtr client,
				xGLXQueryExtensionsStringReply *reply, char *buf);
extern void glxSwapQueryServerStringReply(ClientPtr client,
				xGLXQueryServerStringReply *reply, char *buf);


/*
 * Routines for computing the size of variably-sized rendering commands.
 */

extern int __glXCallListsReqSize(GLbyte *pc, Bool swap);
extern int __glXBitmapReqSize(GLbyte *pc, Bool swap);
extern int __glXFogfvReqSize(GLbyte *pc, Bool swap);
extern int __glXFogivReqSize(GLbyte *pc, Bool swap);
extern int __glXLightfvReqSize(GLbyte *pc, Bool swap);
extern int __glXLightivReqSize(GLbyte *pc, Bool swap);
extern int __glXLightModelfvReqSize(GLbyte *pc, Bool swap);
extern int __glXLightModelivReqSize(GLbyte *pc, Bool swap);
extern int __glXMaterialfvReqSize(GLbyte *pc, Bool swap);
extern int __glXMaterialivReqSize(GLbyte *pc, Bool swap);
extern int __glXTexParameterfvReqSize(GLbyte *pc, Bool swap);
extern int __glXTexParameterivReqSize(GLbyte *pc, Bool swap);
extern int __glXTexImage1DReqSize(GLbyte *pc, Bool swap);
extern int __glXTexImage2DReqSize(GLbyte *pc, Bool swap);
extern int __glXTexEnvfvReqSize(GLbyte *pc, Bool swap);
extern int __glXTexEnvivReqSize(GLbyte *pc, Bool swap);
extern int __glXTexGendvReqSize(GLbyte *pc, Bool swap);
extern int __glXTexGenfvReqSize(GLbyte *pc, Bool swap);
extern int __glXTexGenivReqSize(GLbyte *pc, Bool swap);
extern int __glXMap1dReqSize(GLbyte *pc, Bool swap);
extern int __glXMap1fReqSize(GLbyte *pc, Bool swap);
extern int __glXMap2dReqSize(GLbyte *pc, Bool swap);
extern int __glXMap2fReqSize(GLbyte *pc, Bool swap);
extern int __glXPixelMapfvReqSize(GLbyte *pc, Bool swap);
extern int __glXPixelMapuivReqSize(GLbyte *pc, Bool swap);
extern int __glXPixelMapusvReqSize(GLbyte *pc, Bool swap);
extern int __glXDrawPixelsReqSize(GLbyte *pc, Bool swap);
extern int __glXTypeSize(GLenum enm);
extern int __glXDrawArraysSize(GLbyte *pc, Bool swap);
extern int __glXPrioritizeTexturesReqSize(GLbyte *pc, Bool swap);
extern int __glXTexSubImage1DReqSize(GLbyte *pc, Bool swap);
extern int __glXTexSubImage2DReqSize(GLbyte *pc, Bool swap);

#endif /* !__GLX_server_h__ */
