#ifndef _glxext_h_
#define _glxext_h_

/* $XFree86: xc/programs/Xserver/GL/glx/glxext.h,v 1.3 1999/06/14 07:31:27 dawes Exp $ */
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

typedef struct {
    int type;
    void (*resetExtension)(void);
    Bool (*initVisuals)(
	VisualPtr *       visualp,
	DepthPtr *        depthp,
	int *             nvisualp,
	int *             ndepthp,
	int *             rootDepthp,
	VisualID *        defaultVisp,
	unsigned long     sizes,
	int               bitsPerRGB
	);
    void (*setVisualConfigs)(
	int                nconfigs,
	__GLXvisualConfig *configs,
	void              **privates
	);
} __GLXextensionInfo;

extern GLboolean __glXFreeContext(__GLXcontext *glxc);
extern void __glXFlushContextCache(void);

extern void __glXNoSuchRenderOpcode(GLbyte*);
extern int __glXNoSuchSingleOpcode(__GLXclientState*, GLbyte*);
extern void __glXErrorCallBack(__GLinterface *gc, GLenum code);
extern void __glXClearErrorOccured(void);
extern GLboolean __glXErrorOccured(void);
extern void __glXResetLargeCommandStatus(__GLXclientState*);

extern int __glXQueryContextInfoEXT(__GLXclientState *cl, GLbyte *pc);
extern int __glXSwapQueryContextInfoEXT(__GLXclientState *cl, char *pc);

extern Bool __glXCoreType(void);
extern void GlxExtensionInit(void);
extern void GlxSetVisualConfigs(
    int nconfigs,
    __GLXvisualConfig *configs,
    void **privates
);
extern int GlxInitVisuals(
    VisualPtr *       visualp,
    DepthPtr *        depthp,
    int *             nvisualp,
    int *             ndepthp,
    int *             rootDepthp,
    VisualID *        defaultVisp,
    unsigned long     sizes,
    int               bitsPerRGB,
    int               preferredVis
);

#endif /* _glxext_h_ */

