/* 
Copyright (c) 2000 by Juliusz Chroboczek
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions: 
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
/* $XFree86: xc/programs/Xserver/hw/tinyx/vesa/vesainit.c,v 1.1 2004/06/02 22:43:03 dawes Exp $ */
/*
 * Copyright (c) 2004 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vesa.h"

const KdCardFuncs vesaFuncs = {
    vesaCardInit,               /* cardinit */
    vesaScreenInit,             /* scrinit */
    vesaInitScreen,             /* initScreen */
    vesaPreserve,               /* preserve */
    vesaEnable,                 /* enable */
    vesaDPMS,                   /* dpms */
    vesaDisable,                /* disable */
    vesaRestore,                /* restore */
    vesaScreenFini,             /* scrfini */
    vesaCardFini,               /* cardfini */
    
    0,                          /* initCursor */
    0,                          /* enableCursor */
    0,                          /* disableCursor */
    0,                          /* finiCursor */
    0,                          /* recolorCursor */
    
    0,                          /* initAccel */
    0,                          /* enableAccel */
    0,                          /* syncAccel */
    0,                          /* disableAccel */
    0,                          /* finiAccel */
    
    vesaGetColors,              /* getColors */
    vesaPutColors,              /* putColors */

    vesaFinishInitScreen,	/* finishInitScreen */
};

void
InitCard(char *name)
{
    KdCardAttr attr;
    KdCardInfoAdd((KdCardFuncs *) &vesaFuncs, &attr, 0);
}

void
InitOutput (ScreenInfo *pScreenInfo, int argc, char **argv)
{
    KdInitOutput (pScreenInfo, argc, argv);
}

void
InitInput (int argc, char **argv)
{
    KdInitInput(&LinuxMouseFuncs, &LinuxKeyboardFuncs);
}

int
ddxProcessArgument (int argc, char **argv, int i)
{
    int	ret;
    
    if (!(ret = vesaProcessArgument (argc, argv, i)))
	ret = KdProcessArgument(argc, argv, i);
    return ret;
}

void
ddxUseMsg()
{
  KdUseMsg();
  vesaUseMsg();
}

