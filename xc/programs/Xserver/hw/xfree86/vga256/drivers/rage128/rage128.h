/*
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * ATI Rage128 driver for 3.3.x
 *
 * Author:  Alan Hourihane, alanh@fairlite.demon..co.uk
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rage128/rage128.h,v 1.1.2.2 1999/10/12 18:33:28 hohndel Exp $
 */

#define CLOCK_CNTL_INDEX	0x8
#define CLOCK_CNTL_DATA		0xC
#define GEN_INT_CNTL		0x40
#define CRTC_GEN_CNTL		0x50
#define CRTC_EXT_CNTL		0x54
#define DAC_CNTL		0x58
#define I2C_CNTL_1		0x94
#define MEM_CNTL		0x140
#define MPP_TB_CONFIG		0x1C0
#define MPP_GP_CONFIG		0x1C8
#define VIPH_CONTROL		0x1D0
#define	CRTC_H_TOTAL_DISP	0x200
#define CRTC_H_SYNC_STRT_WID	0x204
#define CRTC_V_TOTAL_DISP	0x208
#define CRTC_V_SYNC_STRT_WID	0x20C
#define CRTC_OFFSET		0x224
#define CRTC_OFFSET_CNTL	0x228
#define CRTC_PITCH		0x22C
#define OVR_CLR			0x230
#define	OVR_WID_LEFT_RIGHT	0x234
#define OVR_WID_TOP_BOTTOM	0x238
#define DDA_CONFIG		0x2E0
#define DDA_ON_OFF		0x2E4
#define OV0_SCALE_CNTL		0x420
#define SUBPIC_CNTL		0x540
#define CAP0_TRIG_CNTL		0x950
#define CAP1_TRIG_CNTL		0x9C0

#define ATI_READ(r) \
	{ \
		outl(IOBase, r); \
		save->ATIReg[r] = inl(IOBase + 0x04); \
	}

#define ATI_WRITE(r) \
	{ \
		outl(IOBase, r); \
		outl(IOBase + 0x04, restore->ATIReg[r]); \
	}
