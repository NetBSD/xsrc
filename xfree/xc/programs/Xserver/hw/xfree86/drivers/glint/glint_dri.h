/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/glint_dri.h,v 1.5 2001/01/31 16:14:55 alanh Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,

TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Author:
 *   Jens Owen <jens@precisioninsight.com>
 *
 */

/* 
 * Glint specific record passed back to client driver 
 * via DRIGetDeviceInfo request
 */
typedef struct {
    drmHandle		hControlRegs0;
    drmHandle		hControlRegs1;
    drmHandle		hControlRegs2;
    drmHandle		hControlRegs3;
    drmSize		sizeControlRegs0;
    drmSize		sizeControlRegs1;
    drmSize		sizeControlRegs2;
    drmSize		sizeControlRegs3;
    drmMapFlags 	flagsControlRegs0;
    drmMapFlags 	flagsControlRegs1;
    drmMapFlags 	flagsControlRegs2;
    drmMapFlags		flagsControlRegs3;
    int			numMultiDevices;
    int			pprod;
} GLINTDRIRec, *GLINTDRIPtr;

#define GLINT_DRI_BUF_COUNT 20
#define GLINT_DRI_BUF_SIZE  0x1000
