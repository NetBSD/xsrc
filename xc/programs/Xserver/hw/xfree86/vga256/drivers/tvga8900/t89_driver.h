/* $XConsortium: t89_driver.h /main/3 1996/02/21 18:08:07 kaleb $ */
/*
 * Copyright 1995 by Alan Hourihane, Wigan, England.
 *
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
 * Author:  Alan Hourihane, alanh@fairlite.demon.co.uk
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tvga8900/t89_driver.h,v 3.9.2.11 1999/06/21 09:45:19 hohndel Exp $ */

extern int TVGAchipset;
extern Bool IsCyber;
/*
 * Trident Chipset Definitions
 */

#define TVGA8200LX	0
#define TVGA8800CS	1
#define TVGA8900B	2
#define TVGA8900C	3
#define TVGA8900CL	4
#define TVGA8900D	5
#define TVGA9000	6
#define TVGA9000i	7
#define TVGA9100B	8
#define TVGA9200CXr	9
#define TGUI9400CXi	10
#define TGUI9420	11
#define TGUI9420DGi	12
#define TGUI9430DGi	13
#define TGUI9440AGi	14
#define CYBER9320	15
#define TGUI96xx	16 /* backwards compatibility */
#define TGUI9660	16
#define TGUI9680	17
#define TGUI9682	18
#define TGUI9685	19
#define CYBER9382	20
#define CYBER9385	21
#define CYBER9388	22
#define CYBER9397	23
#define CYBER9520	24
#define CYBER9525 	25
#define IMAGE975	26
#define IMAGE985	27
#define CYBER939A	28
#define BLADE3D		29
#define CYBERBLADE	30

#define IsTGUI9440	(TVGAchipset == TGUI9440AGi)
#define IsTGUI9660	(TVGAchipset == TGUI9660)
#define IsTGUI9680	(TVGAchipset == TGUI9680)
#define IsTGUI9682	(TVGAchipset == TGUI9682)
#define IsTGUI9685	(TVGAchipset == TGUI9685)
#define IsAdvCyber	((TVGAchipset == CYBER9382) || \
			 (TVGAchipset == CYBER9385) || \
			 (TVGAchipset == CYBER9388) || \
			 (TVGAchipset == CYBER9520) || \
			 (TVGAchipset == CYBER9525) || \
			 (TVGAchipset == CYBER939A) || \
			 (TVGAchipset == CYBER9397))
#define Is3Dchip	((TVGAchipset == CYBER9397) || \
			 (TVGAchipset == CYBER939A) || \
			 (TVGAchipset == CYBER9388) || \
			 (TVGAchipset == CYBER9520) || \
			 (TVGAchipset == CYBER9525) || \
			 (TVGAchipset == IMAGE975) || \
			 (TVGAchipset == IMAGE985) || \
			 (TVGAchipset == BLADE3D) || \
			 (TVGAchipset == CYBERBLADE))

#ifdef INITIALIZE_LIMITS
/* Clock Limits */
int tridentClockLimit[] = {
	80000,
	80000,
	80000,
	80000,
	80000,
	80000,
	80000,
	80000,
	80000,
	80000,	
	80000,
	80000,
	80000,
	80000,
	90000,
	90000,
	135000,
	135000,
	135000,
	170000,
	135000,
	135000,
	170000,
	170000,
	230000,
	230000,
	230000,
	230000,
	230000,
	230000,
	230000,
	230000,
};

int tridentClockLimit16bpp[] = {
	40000,
	40000,
	40000,
	40000,
	40000,
	40000,
	40000,
	40000,
	40000,
	40000,
	40000,
	40000,
	40000,
	40000,
	45000,
	45000,
	135000,
	135000,
	135000,
	170000,
	135000,
	135000,
	170000,
	170000,
	230000,
	230000,
	230000,
	230000,
	230000,
	230000,
	230000,
	230000,
}; 

int tridentClockLimit24bpp[] = {
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	70000,
	70000,
	70000,
	85000,
	70000,
	70000,
	85000,
	85000,
	115000,
	115000,
	115000,
	115000,
	115000,
	115000,
	115000,
	115000,
};

int tridentClockLimit32bpp[] = {
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	25180,
	70000,
	70000,
	70000,
	85000,
	70000,
	70000,
	85000,
	85000,
	115000,
	115000,
	115000,
	115000,
	115000,
	115000,
	115000,
	115000,
};

#else

extern int tridentClockLimit[];
extern int tridentClockLimit16bpp[];
extern int tridentClockLimit24bpp[];
extern int tridentClockLimit32bpp[];

#endif


/*
 * Trident DAC's
 */

#define TKD8001		0
#define TGUIDAC		1
