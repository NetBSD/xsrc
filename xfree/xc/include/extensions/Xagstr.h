/*
Copyright 1996, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.
*/
/* $TOG: Xagstr.h /main/4 1998/02/09 11:21:26 kaleb $ */

#ifndef _XAGSTR_H_ /* { */
#define _XAGSTR_H_

#include "Xag.h"

#define XAppGroup CARD32

#define XAGNAME "XC-APPGROUP"

#define XAG_MAJOR_VERSION	1	/* current version numbers */
#define XAG_MINOR_VERSION	0

#define XagWindowTypeX11	0
#define XagWindowTypeMacintosh	1
#define XagWindowTypeWin32	2
#define XagWindowTypeWin16	3

typedef struct _XagQueryVersion {
    CARD8	reqType;	/* always XagReqCode */
    CARD8	xagReqType;	/* always X_XagQueryVersion */
    CARD16	length B16;
    CARD16	client_major_version B16;
    CARD16	client_minor_version B16;
} xXagQueryVersionReq;
#define sz_xXagQueryVersionReq		8

typedef struct {
    BYTE	type;		/* X_Reply */
    BOOL	pad1;
    CARD16	sequence_number B16;
    CARD32	length B32;
    CARD16	server_major_version B16;
    CARD16	server_minor_version B16;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xXagQueryVersionReply;
#define sz_xXagQueryVersionReply	32

/* Set AppGroup Attributes masks */
#define XagSingleScreenMask		1 << XagNsingleScreen
#define XagDefaultRootMask		1 << XagNdefaultRoot
#define XagRootVisualMask		1 << XagNrootVisual
#define XagDefaultColormapMask		1 << XagNdefaultColormap
#define XagBlackPixelMask		1 << XagNblackPixel
#define XagWhitePixelMask		1 << XagNwhitePixel
#define XagAppGroupLeaderMask		1 << XagNappGroupLeader

typedef struct _XagCreate {
    CARD8	reqType;	/* always XagReqCode */
    CARD8	xagReqType;	/* always X_XagCreate */
    CARD16	length B16;
    XAppGroup	app_group B32;
    CARD32	attrib_mask B32; /* LISTofVALUE follows */
} xXagCreateReq;
#define sz_xXagCreateReq		12

typedef struct _XagDestroy {
    CARD8	reqType;	/* always XagReqCode */
    CARD8	xagReqType;	/* always X_XagDestroy */
    CARD16	length B16;
    XAppGroup	app_group  B32;
} xXagDestroyReq;
#define sz_xXagDestroyReq		8

typedef struct _XagGetAttr {
    CARD8	reqType;	/* always XagReqCode */
    CARD8	xagReqType;	/* always X_XagGetAttr */
    CARD16	length B16;
    XAppGroup	app_group B32;
} xXagGetAttrReq;
#define sz_xXagGetAttrReq		8

typedef struct {
    BYTE	type;		/* X_Reply */
    BOOL	pad1;
    CARD16	sequence_number B16;
    CARD32	length B32;
    Window	default_root B32;
    VisualID	root_visual B32;
    Colormap	default_colormap B32;
    CARD32	black_pixel B32;
    CARD32	white_pixel B32;
    BOOL	single_screen;
    BOOL	app_group_leader;
    CARD16	pad2 B16;
} xXagGetAttrReply;
#define sz_xXagGetAttrReply		32

typedef struct _XagQuery {
    CARD8	reqType;	/* always XagReqCode */
    CARD8	xagReqType;	/* always X_XagQuery */
    CARD16	length B16;
    CARD32	resource B32;
} xXagQueryReq;
#define sz_xXagQueryReq			8

typedef struct {
    BYTE	type;		/* X_Reply */
    BOOL	pad1;
    CARD16	sequence_number B16;
    CARD32	length B32;
    XAppGroup	app_group B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xXagQueryReply;
#define sz_xXagQueryReply		32

typedef struct _XagCreateAssoc {
    CARD8	reqType;	/* always XagReqCode */
    CARD8	xagReqType;	/* always X_XagCreateAssoc */
    CARD16	length B16;
    Window	window B32;
    CARD16	window_type B16;
    CARD16	system_window_len B16; /* LISTofCARD8 follows */
} xXagCreateAssocReq;
#define sz_xXagCreateAssocReq		12

typedef struct _XagDestroyAssoc {
    CARD8	reqType;	/* always XagReqCode */
    CARD8	xagReqType;	/* always X_XagDestroyAssoc */
    CARD16	length B16;
    Window	window B32;
} xXagDestroyAssocReq;
#define sz_xXagDestroyAssocReq		8

#undef XAppGroup

#endif /* } _XAGSTR_H_ */

