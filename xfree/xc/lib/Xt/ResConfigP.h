/* $TOG: ResConfigP.h /main/2 1998/02/06 13:25:14 kaleb $ */
/*

Copyright 1987, 1988, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/*
 * 1.2 src/gos/2d/XTOP/lib/Xt/custom_proto.h, xtoolkit, gos42G 8/12/93 10:35:50 
 *
 * COMPONENT_NAME:  XTOOLKIT
 *
 * FUNCTIONS:
 *
 * ORIGINS:  27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _RESCONFIGP_H
#define _RESCONFIGP_H

/*
 * Atom names for resource configuration management customization tool.
 */
#define RCM_DATA "Custom Data"
#define RCM_INIT "Custom Init"

extern void _XtResourceConfigurationEH(
#if NeedFunctionPrototypes
	Widget 		/* w */, 
	XtPointer 	/* client_data */, 
	XEvent * 	/* event */
#endif
);

#endif
