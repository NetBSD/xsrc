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
/* $XFree86: xc/programs/Xserver/hw/tinyx/vesa/vesa.h,v 1.1 2004/06/02 22:43:03 dawes Exp $ */
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

#ifndef _VESA_H_
#define _VESA_H_

#include "tinyx.h"
#include "layer.h"
#include "vm86.h"
#ifdef RANDR
#include "randrstr.h"
#endif

#define VESA_TEXT_SAVE	(64*1024)

#define MODE_SUPPORTED	0x01
#define MODE_COLOUR	0x08
#define MODE_GRAPHICS	0x10
#define MODE_VGA	0x20
#define MODE_LINEAR	0x80

#define MODE_DIRECT	0x1

#define MEMORY_TEXT	0
#define MEMORY_CGA	1
#define MEMORY_HERCULES	2
#define MEMORY_PLANAR	3
#define MEMORY_PSEUDO	4
#define MEMORY_NONCHAIN	5
#define MEMORY_DIRECT	6
#define MEMORY_YUV	7

typedef struct _VesaMode {
    int		mode;			/* mode number */
    int		vbe;			/* a VBE mode */
    int		ModeAttributes;		/* mode attributes */
    int		NumberOfPlanes;		/* number of memory planes */
    int		BitsPerPixel;		/* bits per pixel */
    int		MemoryModel;		/* memory model type */
    int		RedMaskSize;            /* size of direct color red mask in bits */
    int		RedFieldPosition;       /* bit position of lsb of red mask */
    int		GreenMaskSize;          /* size of direct color green mask in bits */
    int		GreenFieldPosition;     /* bit position of lsb of green mask */
    int		BlueMaskSize;           /* size of direct color blue mask in bits */
    int		BlueFieldPosition;      /* bit position of lsb of blue mask */
    int		RsvdMaskSize;           /* size of direct color reserved mask bits*/
    int		RsvdFieldPosition;      /* bit position of lsb of reserved mask */
    int		DirectColorModeInfo;    /* direct color mode attributes */
    int		XResolution;            /* horizontal resolution */
    int		YResolution;            /* vertical resolution */
    int		BytesPerScanLine;       /* bytes per scan line */
} VesaModeRec, *VesaModePtr;

#include "vbe.h"
#include "vga.h"

typedef struct _VesaCardPriv {
    int		vbe;
    VesaModePtr modes;
    int		nmode;
    Vm86InfoPtr	vi;
    int		vga_palette;
    int		old_vbe_mode;
    int		old_vga_mode;
    VbeInfoPtr	vbeInfo;
    char	text[VESA_TEXT_SAVE];
    CARD8	cmap[256*4];
} VesaCardPrivRec, *VesaCardPrivPtr;

#define VESA_LINEAR	0
#define VESA_WINDOWED	1
#define VESA_PLANAR	2
#define VESA_MONO	3

typedef struct _VesaScreenPriv {
    VesaModeRec	mode;
    Bool	shadow;
    Rotation	randr;
    int		mapping;
    int		origDepth;
    int		layerKind;
    void	*fb;
    int		fb_size;
    CARD32	fb_phys;
    LayerPtr	pLayer;
} VesaScreenPrivRec, *VesaScreenPrivPtr;

extern int vesa_video_mode;
extern Bool vesa_force_mode;

void
vesaListModes(void);

Bool
vesaCardInit(KdCardInfo *card);

Bool
vesaInitialize (KdCardInfo *card, VesaCardPrivPtr priv);

Bool
vesaScreenInitialize (KdScreenInfo *screen, VesaScreenPrivPtr pscr);

Bool
vesaScreenInit(KdScreenInfo *screen);    

LayerPtr
vesaLayerCreate (ScreenPtr pScreen);

Bool
vesaInitScreen(ScreenPtr pScreen);

Bool
vesaFinishInitScreen(ScreenPtr pScreen);

Bool
vesaEnable(ScreenPtr pScreen);

Bool
vesaDPMS (ScreenPtr pScreen, int mode);

void
vesaDisable(ScreenPtr pScreen);

void
vesaPreserve(KdCardInfo *card);

void
vesaRestore(KdCardInfo *card);

void
vesaCardFini(KdCardInfo *card);

void
vesaScreenFini(KdScreenInfo *screen);

void
vesaPutColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs);

void
vesaGetColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs);

int
vesaProcessArgument (int argc, char **argv, int i);

void
vesaUseMsg (void);

Bool
vesaRandRSetConfig (ScreenPtr pScreen, Rotation randr, int rate,
		    RRScreenSizePtr pSize);


#endif /* _VESA_H_ */
