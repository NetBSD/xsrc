/************************************************************************
Copyright 1991, 1998  The Open Group

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


Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Xorg: ws.h,v 1.3 2000/08/17 19:48:20 cpqbld Exp $ */

#ifndef NO_DEC_VALUE_ADDED
#define SCREENW6        640
#define SCREENW8        800
#define SCREENW10       1024
#define SCREENW12       1280

#define SCREENH6        480
#define SCREENH8        600
#define SCREENH10       768
#define SCREENH12       1024


#define DEF_SCREEN_WIDTH  1024;
#define DEF_SCREEN_HEIGHT  768;
#endif

#define NOMAPYET        (ColormapPtr) 1

#define  ARG_DPIX	(1 << 0)
#define  ARG_DPIY	(1 << 1)
#define  ARG_DPI	(1 << 2)
#define  ARG_BLACKVALUE	(1 << 3)
#define  ARG_WHITEVALUE	(1 << 4)
#define	 ARG_CLASS	(1 << 5)
#define	 ARG_EDGE_L	(1 << 6)
#define	 ARG_EDGE_R	(1 << 7)
#define	 ARG_EDGE_T	(1 << 8)
#define	 ARG_EDGE_B	(1 << 9)
#define  ARG_MONITOR	(1 << 10)
#define	 ARG_DEPTH	(1 << 11)
#ifndef NO_DEC_VALUE_ADDED
#define  ARG_TXXORFIX	(1 << 12)	/* for TX */
#define  ARG_TXBANKSW	(1 << 13)	/* for TX */
#define  ARG_TXRVISUAL	(1 << 14)	/* for TX */
#define  ARG_SCREEN	(1 << 15)       /* */
#define  ARG_VSYNC	(1 << 16)	/* */
#endif

typedef struct  {
	int flags;
	int dpix;
	int dpiy;
	int dpi;
	int class;
	char  *blackValue;
	char  *whiteValue;
	int edge_left;
	int edge_right;
	int edge_top;
	int edge_bottom;
	ws_monitor monitor;
	int depth;
#ifndef NO_DEC_VALUE_ADDED
	int txXorFix;		/* for TX */
	int txBankSwitch;	/* for TX */
	int txRootDepth;	/* for TX */
	int txRootClass;	/* for TX */
	int screenHeight;	/*   */
	int screenWidth;	/*  */
	int screenVsync;
#endif
} ScreenArgsRec;

typedef struct {
	unsigned int		currentmask; 	/* saved plane mask */
	BoxPtr			cursorConstraint;
	ws_screen_descriptor	*screenDesc;
	ColormapPtr		pInstalledMap;
	ScreenArgsRec 		*args;
	Bool			(*CloseScreen)();
#ifndef NO_DEC_VALUE_ADDED
	void			(*CursorControl)();
#endif
} wsScreenPrivate;

typedef struct {
	char *moduleID;	/* graphic module ID */
	Bool (*createProc)();	/* create procedure for this hardware type */
} wsAcceleratorTypes;

extern void wsStoreColors();
extern void wsInstallColormap();
extern void wsUninstallColormap();
extern int wsListInstalledColormaps();
extern int wsScreenPrivateIndex;
extern Bool wsSaveScreen();
extern int dpix, dpiy, dpi;
#ifndef NO_DEC_VALUE_ADDED
extern void wsMakeScreenOnly();
extern void wsInputOutputFinish();
extern int  wsRemapPhysToLogScreens;
extern void wsEnableScreen();
extern void wsDisableScreen();
extern int  wsPhysScreenNum();
extern void wsRegisterAbortProc();
extern void wsRegisterGiveUpProc();
extern wsScreenPrivate * wsAllocScreenInfo();
extern int  wsDisabledScreens[];
extern int  wsOnlyScreen;
extern int  wsPhysToLogScreens[];
extern int  screenHeight, screenWidth;
#endif

extern ScreenArgsRec screenArgs[];

extern ScreenPtr wsScreens[];
extern int class;
extern int forceDepth;
extern int fdPM;   /* this is the file descriptor for screen so
		    can do IOCTL to colormap */
extern int ws_cpu;

#ifndef NO_DEC_VALUE_ADDED
#define WSP_PTR(pScr) \
    ((wsScreenPrivate*)(pScr)->devPrivates[wsScreenPrivateIndex].ptr)
#define WS_SCREEN(pScr) (WSP_PTR(pScr)->screenDesc->screen)
#define wspCursorControl(psn, control) \
    if (psn >= 0) (*WSP_PTR(wsScreens[psn])->CursorControl)(psn, control)
#endif

