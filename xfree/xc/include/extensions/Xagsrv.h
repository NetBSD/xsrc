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
/* $Xorg: Xagsrv.h,v 1.3 2000/08/18 04:05:45 coskrey Exp $ */

#ifndef _XAGSRV_H_
#define _XAGSRV_H_

extern void XagExtensionInit(
#if NeedFunctionPrototypes
    void
#endif
);

extern void XagConnectionInfo(
#if NeedFunctionPrototypes
    ClientPtr			/* client */,
    xConnSetupPrefix**		/* conn_prefix */,
    char**			/* conn_info */,
    int*			/* num_screens */
#endif
);

extern VisualID XagRootVisual(
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern Colormap XagDefaultColormap(
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern ClientPtr XagLeader(
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern void XagCallClientStateChange(
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern Bool XagIsControlledRoot (
#if NeedFunctionPrototypes
    ClientPtr			/* client */,
    WindowPtr			/* pParent */
#endif
);

extern XID XagId (
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern void XagGetDeltaInfo (
#if NeedFunctionPrototypes
    ClientPtr			/* client */,
    CARD32*			/* buf */
#endif
);

#endif /* _XAGSRV_H_ */

