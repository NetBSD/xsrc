/*
 
Copyright (c) 1990, 1991  X Consortium

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
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: vblowup_c.c,v 1.14 94/04/17 21:01:43 rws Exp $
 */
/*************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts,

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
/**
* FACILITY: Regression Test Library
*
* ABSTRACT:
*
*	This module contains code for the blowup (pixel expander) program.
*	There are two routines, blowup and makebig.  makebig serves to
*	get the image from the root window, expand it and draw the expanded
*	picture on the blowup window.  blowup serves to setup the initial
*	environment, handle all mouse and button events and call makebig.
*       Vblowup differs from blowup in that it compares the window pixmap
*       against a known good pixmap and displays the erroneously lit bits
*       by putting an 'X' through them.  This serves as a first pass at
*       attempting to provide interactive screen validation tools.
*
* 	Written by Erik Morse, November 1987 
*
* MODIFIED BY:
*
*	X-12	kieron		Kieron Drake		27-Feb-1991
*		Almost completely revamped - not enough time to rewrite
*		this horrible stuff completely, unfortunately - but
*		removed dependencies on root window. Instead use TWO images,
*		one "known good image" (kgi) and one "probably bad image" (pbi)
*		- which is sometimes referred to as "ximage", in vmakebig() in
*		particular, for historical reasons. Add support for an Expose
*		handler for other window, with its own zoom and image select
*		(parameters are expose_handler, egc, winzoomp, while "which"
*		is passed to expose_handler to select which to show:
*			0x1 = bad, 0x2 = good, 0x3 = both ).
*		Lots of tidying up, refresh, h_light, exposure handling....
*		Jeez! This is now huge! Oh for a rewrite with Xt/Xaw/Motif
*		or whatever.
*
*				kieron	(c) UniSoft Ltd. 27/02/91
*
*
*       X-11    EJM008		Erik Morse	     	23-Nov-1988
*		Add include for portable.h for opaque structure
*		access macros.
* 
*       X-10    TAT001		Todd A. Trimble		12-May-1988
*               Add ability to warp pointer to first erroneous pixel. 
*               Fixed button picks inside blowup window.
*                
*	X-9	PCV001		Peter C. Vinsel		19-Apr-1988
*		fixed position of target window relative to root. allowed
*		button picks inside blowup window.
*
*       X-8     TAT001		Todd A. Trimble		10-Apr-1988
*               Modify to force color displays to show
*               monochrome pixels during monochrome tests.
*                
*       X-7     EJM007		Erik Morse	     	01-Mar-1988
*               Allow additional parameters: good_image, and the
*               corresponding x and y offsets describing where the
*               good_image is to be located on the window.
* 
*	X-6	EJM006		Erik J. Morse		17-Feb-1988
*		Change XSetFunction calls to XSetState to set up a
*		plane mask so that GXinvert really inverts on multi
*		plane systems.
*
*	X-5	EJM005		Erik J. Morse		09-Feb-1988
*		Remove #define lines, add #include regrdef.h
*
*	X-4	EJM004		Erik J. Morse		11-Jan-1988
*		Include a colormap parameter so that we can blow up
*		color images
*
*	X-3	EJM003		Erik J. Morse		08-Jan-1988
*		Use XDefaultGC for window foreground and backgrounds
*
*	X-2	EJM002		Erik J. Morse	  	03-DEC-1987
*		Fix calls to XMapSubwindows       
*
*	X-1	EJM001		Erik J. Morse		01-DEC-1987
*		Add code to deallocate resources.
*
* CALLING SEQUENCE:
*
*	Vblowup(display, window, init_x, init_y, size, granularity,good_image,x_offset,y_offset,background_color)
*
* FORMAL PARAMETERS:
*
*	display
*	    Pointer to current display connection structure
*
*	window
*	    target window for blowups - blowup uses this window to
*	    adjust the current x, y coordinate readings
*
*	init_x
*	    x coord of upper left hand corner of blowup window relative to the
*	    root window origin
*
*	init_y
*	    y coord of upper left hand corner of blowup window relative to the
*	    root window origin
*
* 	size
*	    size of the blowup window expansion area in pixels
*
*	granularity
*	    coarseness of the zoom - this number indicates by how many pixels
*	    an expanded pixel will grow or shrink when the zoom in or zoom out
*	    option is chosen	                                                        
*	
*	cmap
*	    colormap installed on target window or None                                        
*
*       good_image
*           image for comparison against window contents.
*
*       x_offset,y_offset
*           coordinate offset of good_image's upper left corner from the upper left
*           corner of the root_window in which the test window resides.
*
*       background_color
*           color of background used during create_window operation.
*
* IMPLICIT INPUTS:
*
*	NONE
*
* IMPLICIT OUTPUTS:
*
*	NONE
*
* COMPLETION STATUS: (or ROUTINE VALUE:)
*
*	NONE
*
* SIDE EFFECTS:
*                   
*	NONE
*
**/              
#include <stdio.h>
#include "Xlib.h"  		/* xlib defs */
#include "cursorfont.h"		/* cursor info */
/* #include "drawutil.h"		// color thingys */

/******** module globals **********/

static short pixel_wrong;
static unsigned long expected_pixel;
static int windx,windy,wind_width,wind_height,scrn_width,scrn_height;
static int key_win_width = 68, x_win_height = 17; /* depends on font used */
static validate_color = 1;	/* 0 = position only */

#define DEPTHMASK(n)	(((n) >= 32) ? 0xffffffff : ((0x1 << (n)) - 1))

/*
 * get the image and blow it up
 */                          
/*******
	This is called within the event loop of VBlowup, which may have, nay
	will have, pushed back events onto the event-Q. The only type expected
	to survive across a call of vmakebig() are Expose events, though, so
	as long as we don't eat (all of) these we are safe.	- kieron
*******/

void vmakebig(display, window, blowup_win, x, y, zoom_factor,pixels_across,
		size, format, gc, gcback, gcfore, kgi, ximage, ix, iy, 
		background, view_color, test_color)
Display *display;
Window window;
Window blowup_win;
int x, y, zoom_factor;
int pixels_across;               
int format;
GC gc;                  
unsigned long gcback,gcfore;
XImage *kgi;			/* known good image */
XImage *ximage;			/* possibly bad image */
int ix, iy;		  	/* origin of known good image relative to the screen */
unsigned long background;       /* background color */
int view_color,test_color;
{
	int across,down,e,f, azf;    
	unsigned long pixel, old_pixel, testpixel, goodpixel, mask;               
	XSegment h_lines[200];                                                  
	XSegment v_lines[200];
        int half_pixels_across = pixels_across>>1;
	XEvent junk, save;

#ifdef DEBUG
(void)printf("makebig: x=%d, y=%d, z_f=%d, p_a=%d, size=%d, ix=%d, iy=%d\n",
		x, y, zoom_factor, pixels_across, size, ix, iy);
(void)printf("windx=%d,windy=%d,wind_width=%d,wind_height=%d,scrn_width=%d,scrn_height=%d\n",
		windx,windy,wind_width,wind_height,scrn_width,scrn_height);
#endif /* DEBUG */

	    mask = DEPTHMASK(ximage->depth) & ( (kgi==None) ? DEPTHMASK(32) :
						DEPTHMASK(kgi->depth) );
            pixel_wrong = 0;

	    old_pixel = XWhitePixel(display, XDefaultScreen(display));

            XSetForeground(display, gc, XBlackPixel(display,XDefaultScreen(display)));  /* put in for monochrome case */
	    XSetBackground(display, gc, XWhitePixel(display,XDefaultScreen(display)));

	    for (across = 0; across < pixels_across; across++) {            
		azf = (across * zoom_factor) + key_win_width + 2; 
		for (down = 0; down < pixels_across; down++) { 
     		    int xx = across + x - ix - windx;
		    int yy = down + y - iy - windy;

#ifdef DEBUG
(void)printf("makebig: try for BAD at (%d, %d) - in [0..%d, 0..%d]?\n",
     			xx, yy, ximage->width, ximage->height);
#endif /* DEBUG */
		    if (xx < ximage->width && yy < ximage->height)
                        pixel = XGetPixel(ximage,xx,yy);
		    else
			pixel = background;
 		    if (pixel != background) {
                        if (view_color)
        	            XSetForeground(display, gc, pixel);
                        else
                            XSetForeground(display, gc, gcfore);
  		        XFillRectangle(display, blowup_win, gc, azf, 
			               ((down * zoom_factor) + x_win_height + 1), zoom_factor, zoom_factor);
		    }
/*************/ 
                    if (kgi != None) {
                        testpixel = pixel;
#ifdef DEBUG
(void)printf("makebig: try for GOOD at (%d, %d) - in [0..%d, 0..%d]?\n",
			xx, yy, kgi->width, kgi->height);
#endif /* DEBUG */
     			if ((xx < kgi->width) && (yy < kgi->height) &&
			    (xx >= 0) && (yy >= 0)) {
                            goodpixel = XGetPixel(kgi,xx,yy);
			    if ((testpixel&mask) != (goodpixel&mask)) {
                                if ((across == half_pixels_across)&&(down == half_pixels_across))
                                {
                                    pixel_wrong = 1;
                                    expected_pixel = goodpixel;
                                }
			    	XSetForeground(display, gc, validate_color?1:XWhitePixel(display, XDefaultScreen(display)));
			    	XDrawLine(display, blowup_win, gc, azf, ((down * zoom_factor) + x_win_height + 1),
				      azf + zoom_factor, ((down * zoom_factor) + x_win_height + 1) + zoom_factor);  
			    	XDrawLine(display, blowup_win, gc, azf + zoom_factor + 1, ((down * zoom_factor) + x_win_height + 1),
				      azf + 1, ((down * zoom_factor) + x_win_height + 1) + zoom_factor);  
			    	XSetForeground(display, gc, validate_color?0:XBlackPixel(display, XDefaultScreen(display)));
			    	XDrawLine(display, blowup_win, gc, azf + zoom_factor, ((down * zoom_factor) + x_win_height + 1),
				      azf, ((down * zoom_factor) + x_win_height + 1) + zoom_factor);  
			    	XDrawLine(display, blowup_win, gc, azf + 1, ((down * zoom_factor) + x_win_height + 1),
				      azf + zoom_factor + 1, ((down * zoom_factor) + x_win_height + 1) + zoom_factor);
			    }
			}                                                                                          
		    } /* if (kgi != None) .... */
/*************/ 
                } /* down loop */
	    } /* across loop */
/*
 * setup and draw the pixel boundaries
 */                                                                      
	    for (e = zoom_factor,f = 0; f < pixels_across; e += zoom_factor, f++) {
	     	v_lines[f].x1 = e + key_win_width + 2;
		v_lines[f].y1 =	x_win_height + 1;
		v_lines[f].x2 =	e + key_win_width + 2;
		v_lines[f].y2 =	size + x_win_height + 1;                
		h_lines[f].x1 = key_win_width + 2;
		h_lines[f].y1 = e + x_win_height + 1;
		h_lines[f].x2 = size + key_win_width + 2;
		h_lines[f].y2 = e + x_win_height + 1;
	    }   
                                
            /* add highlights to middle pair of lines */

            f = pixels_across;
            v_lines[f].x1 = (pixels_across>>1)*zoom_factor + key_win_width + 2 + 1;
	    v_lines[f].y1 = x_win_height + 1;
	    v_lines[f].x2 = v_lines[f].x1;
	    v_lines[f].y2 = size + x_win_height + 1;                
	    h_lines[f].x1 = key_win_width + 2;
	    h_lines[f].y1 = (pixels_across>>1)*zoom_factor + x_win_height + 1 + 1;
	    h_lines[f].x2 = size + key_win_width + 2;
	    h_lines[f].y2 = h_lines[f].y1;
            f++;
     	    v_lines[f].x1 = ((pixels_across>>1)+1)*zoom_factor + key_win_width + 2 + 1;
	    v_lines[f].y1 = x_win_height + 1;
	    v_lines[f].x2 = v_lines[f].x1;
	    v_lines[f].y2 = size + x_win_height + 1;                
	    h_lines[f].x1 = key_win_width + 2;
	    h_lines[f].y1 = ((pixels_across>>1)+1)*zoom_factor + x_win_height + 1 + 1;
	    h_lines[f].x2 = size + key_win_width + 2;
	    h_lines[f].y2 = h_lines[f].y1;
	    
	    /* set the plane mask to the low order bit to really invert black and white on multi plane systems */
	    XSetState(display, gc, XWhitePixel(display, XDefaultScreen(display)), XBlackPixel(display, XDefaultScreen(display)),
			GXinvert, (unsigned long)1);
	    XDrawSegments(display, blowup_win, gc, v_lines, pixels_across+2);
	    XDrawSegments(display, blowup_win, gc, h_lines, pixels_across+2);
	    XSetState(display, gc, XWhitePixel(display, XDefaultScreen(display)), XBlackPixel(display, XDefaultScreen(display)),
			GXcopy, (unsigned long)XAllPlanes());
	    /* munge events - see comment at head of routine and at call point */
	    while (XCheckMaskEvent(display, ButtonPressMask | PointerMotionMask,
			&junk))
		; /* donothing */  	                                        
	    save.type = ButtonPress; /* any non Expose will do */
	    while (XCheckWindowEvent(display, blowup_win, ExposureMask, &junk))
		if (junk.xexpose.count == 0)
			save = junk;
	    if (save.type == Expose)
		XPutBackEvent(display, &save); /* restore one of many */
}

/* delta used as vertical, rather than horiz, here! */
/* font misused as well; indicates kgi required to draw! */
XTextItem labels[] = {
	{ " X         ", 11, 11, None},
	{ " Y         ", 11, 11, None},
	{ " B/G/both  ", 11, 11, None},  
	{ " color/mono", 11, 11, None},  
	{ " next error", 11, 11, 1},  
	{ " sub-zoom +", 11, 11, None},
	{ " sub-zoom -", 11, 11, None},
	{ " quit      ", 11, 11, None},
	{ " big-zoom +", 11, 11, None},
	{ " big-zoom -", 11, 11, None},
	{ " next      ", 11, 11, None}
};

h_light(display, wins, nwins, gc, labs, target, kgi)
Display	*display;
Window	*wins;
int	nwins;
GC	gc;
XTextItem	*labs;
Window	target;
XImage	*kgi;
{
	int i;

	for(i=0; i<nwins; i++)
	    if ( (target == None || target == wins[i]) &&
			(kgi != None || labs[i].font == None) )
		XDrawImageString(display, wins[i], gc,
		    0, labs[i].delta, labs[i].chars, labs[i].nchars);
		    /* delta used as vertical, rather than horiz, here! */
}

refresh(display, wins, nwins, gc, labs, kgi)
Display	*display;
Window	*wins;
int	nwins;
GC	gc;
XTextItem	*labs;
XImage	*kgi;
{
	h_light(display, wins, nwins, gc, labs, None, kgi);
}

char *whiches[] = {
	"   ERROR   ",	/* 0x0	=>	shouldn't happen */
	"Server Data",	/* 0x1	=>	"bad" image file */
	"Pixval Data",	/* 0x2	=>	"good" image file */
	"Comparison "	/* 0x3	=>	both of the above */
};

                                                  
                                                

/*
 * do initialization and handle events                                                                
 */


VBlowup(display, window, egc, init_x, init_y, w, h, size, granularity, cmap,
		pbi, kgi, ix, iy,
		background, warp_pointer_x, warp_pointer_y,show_banner,
		compare_color, expose_handler, winzoomp, font_name)
Display *display;
Window window; /* window in bg with >=1:1 version of pbi in it */
GC	egc;
int init_x, init_y;
int size, w, h;
int granularity;
Colormap cmap;
XImage *pbi,*kgi;
int ix, iy;
unsigned long background;
int warp_pointer_x, warp_pointer_y;
int show_banner;
int compare_color;
void (*expose_handler)();
int	*winzoomp;
char	*font_name;
{                                                      
	extern void free();
	extern char *malloc();
        int	show_color = compare_color;
                                                                                     
	Font	fid;
        Window highlight = 0, head_win, pix_win, exp_win, blowup_win, tar_win, key_win, key[11];
	GC textgc, gc, dgc, rect_gc;
	XGCValues xgcv;
	XEvent xev, fake;
	int zoom_factor = 10, size2;                                                    
	int pixels_across = 20; 
	int start_x = 0, start_y = 0;
        int old_start_x = -9999, old_start_y = -9999, old_pixels_across = -9999;
	int max_x, max_y;
	char xypair[80];                    
	Cursor curse;
  	int format;
	unsigned long gcfore, gcback;                                           
	Window		tracewind;
        XWindowAttributes win_attr;
	unsigned int which;
	int x,y,i;
	XImage *real_kgi = kgi, *real_pbi = pbi;
	int retval = 0;
/*****
	Window window;

	window = XCreateSimpleWindow(display, window, init_x, init_y, w, h,
		validate_color ? 0 : XBlackPixel(display, XDefaultScreen(display)),
		background);
	XMapWindow(display, window);
*****/

	if (kgi == NULL)
		which = 0x1; /* "bad" only */
	else
		which = 0x3; /* both. 0x2 is "good" only */
	XStoreName(display, window, whiches[which]);

					/* XXX */
	validate_color = compare_color; /* should have come from image file? */

#ifdef DEBUG
(void)printf(
	"VBlowup: init_x=%d,init_y=%d,w=%d,h=%d,size=%d,granularity=%d,ix=%d,\
iy=%d,warp_pointer_x=%d,warp_pointer_y=%d\n",
	init_x, init_y, w, h, size, granularity, ix, iy,
	warp_pointer_x, warp_pointer_y);
#endif /* DEBUG */
        /* initialize rectangle pixmap overlay gc */        

        rect_gc = XCreateGC(display,window,(unsigned long)0,(XGCValues *)NULL);
        XSetForeground(display,rect_gc,1L); /* this works well with xor function */
        XSetBackground(display,rect_gc,0L);
        XSetFunction(display,rect_gc,GXxor);                                    
        XSetLineAttributes(display,rect_gc,2,LineSolid,CapButt,JoinBevel); /* specify line width */

        /* find window offset from root window */

	tracewind = window;
	windx = 0;
      	windy = 0;

        scrn_width = wind_width = w;
        scrn_height = wind_height = h;

	    {
	    XWindowAttributes	Xwatt;                                     

	    XGetWindowAttributes(display,tracewind,&Xwatt);

            wind_width = Xwatt.width;
            wind_height = Xwatt.height;
	    }
                                  
	if (XDefaultDepth(display, XDefaultScreen(display)) == 1) 	/* check for color */
	    format = XYPixmap;
	else 
	    format = ZPixmap;
/*                   
	dgc = XDefaultGC(display, XDefaultScreen(display));
	gcfore = GetGCVforeground(dgc);
	gcback = GetGCVbackground(dgc);
*/

        gcfore = XBlackPixel(display,XDefaultScreen(display));
        gcback = XWhitePixel(display,XDefaultScreen(display));

	size2 = size / 2;
        blowup_win = XCreateSimpleWindow(display,(Window)XRootWindow(display,XDefaultScreen(display)),init_x,init_y,
					 (size + 4 + key_win_width),(size + 2 + x_win_height),1, gcfore, gcback);
	XSelectInput(display, blowup_win, ExposureMask);
	XMapWindow(display,blowup_win);
	XSync(display, 0);
	XWindowEvent(display, blowup_win, ExposureMask, &xev);

	if (cmap != None)
            XSetWindowColormap(display, blowup_win, cmap);
        head_win = XCreateSimpleWindow(display,blowup_win,0,0,
		x=(size + 2 + key_win_width), y=(x_win_height - 2),1, gcfore, gcback);
        key_win = XCreateSimpleWindow(display,blowup_win,0,x_win_height,
		key_win_width, size, 1, gcfore, gcback);
	XMapSubwindows(display, blowup_win); 

	for(i=0;i<=10;i++)
            key[i] = XCreateSimpleWindow(display,key_win,0,i*(y+1),
					 key_win_width,y,0,gcfore, gcback);

	XMapSubwindows(display, key_win); 
        pix_win = XCreateSimpleWindow(display,head_win,0,0,
					 x/2,y,0,gcfore, gcback);
        exp_win = XCreateSimpleWindow(display,head_win,size2,0,
					 x/2,y,0,gcfore, gcback);
	XMapSubwindows(display, head_win); 
	XSync(display, 0);                            
             
	xgcv.function = GXcopy;
	xgcv.background = gcback; 
	xgcv.foreground = gcfore; 
	fid = xgcv.font = XLoadFont(display,
				font_name != NULL ? font_name : "6x10");
					/* lots of "6"s in code
						that depend on this! Aaagh! */
	xgcv.graphics_exposures = False;
	textgc = XCreateGC(display, window,
		GCFunction | GCForeground | GCBackground | GCFont, &xgcv); 
	gc = XCreateGC(display, (Window)XRootWindow(display, XDefaultScreen(display)), 
		GCFunction | GCForeground | GCBackground | GCGraphicsExposures, &xgcv);    
	XSetGraphicsExposures(display, egc, False);
	/* avoid filling event queue with unwanted garbage */

	max_x = XDisplayWidth(display, XDefaultScreen(display)) - 1;
	max_y = XDisplayHeight(display, XDefaultScreen(display)) - 1;

	curse = XCreateFontCursor(display, XC_sb_right_arrow);
        XSync(display,0);
	XDefineCursor(display, window, curse);
        XSync(display,0);
	XStoreName(display, blowup_win, "Blowup");              
        XSync(display,0);                                         

/*
 * select the event inputs.  Note we will do highlight events with pointer motion not enter and leave events,
 * this is because the user interface across the net appears much smoother and only slightly slower with
 * this approach
 */
	XSelectInput(display, window, ButtonPressMask | ExposureMask); 
	XSelectInput(display, blowup_win, ButtonPressMask | ExposureMask);
	for(i=1;i<=3;i++)
	    XSelectInput(display, key[i], ExposureMask | ButtonPressMask | PointerMotionMask);  
        if (real_kgi)
    	    XSelectInput(display, key[4], ExposureMask | ButtonPressMask | PointerMotionMask);  
	for(i=5;i<=10;i++)
	    XSelectInput(display, key[i], ExposureMask | ButtonPressMask | PointerMotionMask);  
	XSync(display, 0);    

        XGetWindowAttributes(display,window,&win_attr);

/***************/
	/* draw window before entering loop or putting things on event-Q so
	   caller supplied exposure_handler need not respect Q. - kieron
	*/
	if (expose_handler != NULL) {
		/* now eat any other Expose events left */
		while (XCheckWindowEvent(display, window,
				ExposureMask, &fake))
			; /* donothing */
		expose_handler(display, window, egc, pbi, kgi,
			compare_color, &xev, which);
		old_start_x = -9999; /* assume old rectangle drawn on */
	}
	/* Expose events will safely survive across calls of vmakebig(). See
	   the remarks about event ordering below. - kieron
	*/
	fake.type = Expose; 
	fake.xexpose.count = 0;
	fake.xexpose.window = blowup_win;
	XPutBackEvent(display, &fake);
#ifdef DEBUG
printf("XPending() = %d\n", XPending(display));
#endif /* DEBUG */

        if (kgi != None)
	{
	    /* fake up a "next error" request, was a move to warp_pointer_x/y */
            fake.xbutton.display = display;
            fake.xbutton.window = key[4]; /* was window */
            fake.xbutton.root = win_attr.root;
            fake.xbutton.time = CurrentTime;
            fake.xbutton.x = warp_pointer_x;
            fake.xbutton.y = warp_pointer_y;
            fake.xbutton.x_root = warp_pointer_x+windx;
            fake.xbutton.y_root = warp_pointer_y+windy;
	    fake.type = ButtonPress; 
            fake.xbutton.state = Button1Mask;
	    XPutBackEvent(display, &fake);

	    /* now have 2 events, ordering is:
			Expose(blowup_win); ButtonPress(key[4]);
		Last Read -----^	First Read -----^

		The ButtonPress MUST be before Expose(b_w) as it won't survive
		across a call on vmakebig().	- kieron
	    */

#ifdef DEBUG
printf("XPending() = %d\n", XPending(display));
#endif /* DEBUG */
	    XSync(display,0);
	    if (fake.xbutton.window == window) { /* need a warp... */
		XWarpPointer(display,None,
			XRootWindow(display,XDefaultScreen(display)),0,0,0,0,
                        warp_pointer_x+windx,warp_pointer_y+windy);
		XSync(display,0);
		XWarpPointer(display,
			XRootWindow(display,XDefaultScreen(display)),
			window,warp_pointer_x+windx,warp_pointer_y+windy,
                        XDisplayWidth(display,0),XDisplayHeight(display,0),
			warp_pointer_x,warp_pointer_y);
		XSync(display,0);
	    }
#ifdef DEBUG
printf("XPending() after flushes = %d\n", XPending(display));
#endif /* DEBUG */
	}
/***************/
                                            
/*                           
 * now get events until the user requests a quit
 */
	while (1) {
	    XNextEvent(display, &xev);                                      

	    if (xev.type == NoExpose || xev.type == GraphicsExpose)
		continue; /* junk these */
	    if (xev.type == Expose) {
		if (xev.xexpose.count != 0)
			continue; /* round for next event */
		if (xev.xexpose.window == window && expose_handler != NULL) {
			/* now eat any other Expose events left */
			while (XCheckWindowEvent(display, window,
					ExposureMask, &fake))
				; /* donothing */
			expose_handler(display, window, egc, pbi, kgi,
				compare_color, &xev, which);
			old_start_x = -9999; /* assume old rectangle drawn on */
			continue; /* round for next event, respect event-Q */
		}
		else /*if (xev.xexpose.window == blowup_win)*/ {
#ifdef DEBUG
printf("expose in blowup_win\n");
#endif /* DEBUG */
			/* labels etc... */
			if (kgi == None) {
			    if (show_banner)
				XDrawString(display, head_win, textgc, ((size2 +(key_win_width/2))-25*6/2), 11, " Blowup - Pixmap Correct ",25);
			    else
				XDrawString(display, head_win, textgc, ((size2 +(key_win_width/2))-10*6/2), 11, " Blowup   ",10);
			}
			else 
			    XDrawString(display, head_win, textgc, ((size2 +(key_win_width/2))-25*6/2), 11, "Blowup - Pixmap Incorrect", 25);
			XDrawString(display, pix_win, textgc, 0, 11, " Pixel  ", 7);
			XSetForeground(display, textgc, gcfore); 
			XSetBackground(display, textgc, gcback); 
			refresh(display, key, 11, textgc, labels, kgi);
			XFlush(display);                            
			while (XCheckMaskEvent(display, ExposureMask, &fake))
				; /* do nothing */
		}
		xev.xbutton.window = None;
		xev.type = ButtonPress; /* fall through to redisplay stuff */
	    }

   	    if (xev.type != MotionNotify) { 	/* not a highlight event */
		if (xev.xbutton.window == key[7]) 		/* quit */
		    break; /* retval = 0 => bye */
		else if (xev.xbutton.window == key[10])		/* next */
		{
		    retval = 1;	/* process next file chunk */
		    break;
		}
		else if (xev.xbutton.window == key[6]) {        /* zoom out */
		    if (zoom_factor > (granularity + 1)) {
			zoom_factor -= granularity;   
			start_x += (pixels_across / 2);
			start_y += (pixels_across / 2);
	    	        pixels_across = size / zoom_factor;
			start_x -= (pixels_across / 2);
			start_y -= (pixels_across / 2);
		    }
		}
		else if (xev.xbutton.window == key[5]) {   	/* zoom in */
		    if (zoom_factor < (size - granularity)) {
		   	zoom_factor += granularity;	             
			start_x += (pixels_across / 2);
			start_y += (pixels_across / 2);
	    	        pixels_across = size / zoom_factor;
			start_x -= (pixels_across / 2);
			start_y -= (pixels_across / 2);
		    }
		}
		else if (xev.xbutton.window == key[8])	/* zoom window in */
		{
		    *winzoomp += granularity;

		    fake.type = Expose; 
		    fake.xexpose.count = 0;
		    fake.xexpose.window = window;
		    XPutBackEvent(display, &fake);
		    continue;	/* round again to pick up faked event(s) */
		}
		else if (xev.xbutton.window == key[9])	/* zoom window out */
		{
		    if (*winzoomp > granularity)
			*winzoomp -= granularity;
		    fake.type = Expose; 
		    fake.xexpose.count = 0;
		    fake.xexpose.window = window;
		    XPutBackEvent(display, &fake);
		    continue;	/* round again to pick up faked event(s) */
		}
     		else if (xev.xbutton.window == key[2])		/* which */
		{
		    /* good/bad toggling? both....? set "which" */
		    /* ...... here ....., which =0x1,0x2,0x3.... */
		    if (real_kgi == None)
			continue;	/* nothing doing */
		    which++; which &= 0x3;
		    switch (which) {
		    case 0x0: which++; /* fall through to 1 */
		    case 0x1:	kgi = None; pbi = real_pbi; break;
		    case 0x2:	kgi = None; pbi = real_kgi; break;
		    case 0x3:	kgi = real_kgi; pbi = real_pbi; break;
		    }
		    XStoreName(display, window, whiches[which]);
		    fake.type = Expose; 
		    fake.xexpose.count = 0;
		    fake.xexpose.window = blowup_win;
		    XPutBackEvent(display, &fake);
		    fake.xexpose.window = window;
		    XPutBackEvent(display, &fake); /* order is inportant */
		    continue;	/* round again to pick up faked event(s) */
		}
     		else if (xev.xbutton.window == key[3])		/* color/mono */
		    show_color = (!show_color);                           
     		else if ((kgi) &&(xev.xbutton.window == key[4]))	/* move to next error */
                {
                    int x_scan,y_scan;
                    unsigned long test_pixel, good_pixel;
                    int begin_x;
                    short found_mismatch;
                    int old_x_scan,old_y_scan;      
                    int lox,loy,hix,hiy;
                    short done;         
		    unsigned long mask = (DEPTHMASK(pbi->depth) & DEPTHMASK(kgi->depth));
                    lox = ix + windx;
                    if (lox < 0)
                        lox = 0;

                    hix = ix + windx + kgi->width;
                    if (hix > scrn_width)
                        hix = scrn_width;

                    loy = iy + windy;
                    if (loy < 0)
                        loy = 0;

                    hiy = iy + windy + kgi->height;
                    if (hiy > scrn_height)
                        hiy = scrn_height;
/******
                    if (old_start_x != -9999) {
			x = (old_start_x-windx) * (*winzoomp);
			y = (old_start_y-windy) * (*winzoomp);
			i = old_pixels_across * (*winzoomp);
                        XDrawRectangle(display,window,rect_gc, x,y,i,i);
		    }
                    old_start_x = -9999;
******/
                    x_scan = start_x + (pixels_across>>1) + 1;
                    y_scan = start_y + (pixels_across>>1);
                    if (y_scan < loy) {
                        y_scan = loy;
                        x_scan = lox;
                    }
                    else if (y_scan > hiy) {
                        y_scan = loy;
                        x_scan = lox;
                    }
                  
                    if (x_scan > hix) {
                        x_scan = lox;
                        y_scan++;
                    }             
                    else if (x_scan < lox)
                        x_scan = lox;
                    
                    old_x_scan = x_scan;
                    old_y_scan = y_scan;

                    found_mismatch = done = 0;
                    while (!done && (y_scan < hiy)) {
                        begin_x = x_scan;
                        while (!done && (x_scan < hix)) {
#ifdef DEBUG
(void)printf("VBlowup: try for both at (%d,%d) - in [0..%d,0..%d]?\n",
		x_scan-ix-windx, y_scan-iy-windy, pbi->width, pbi->height);
#endif /* DEBUG */
                            /* test_pixel = XGetPixel(ximage,x_scan-begin_x,0); */
                            test_pixel = XGetPixel(pbi,x_scan-ix-windx,y_scan-iy-windy);
                            good_pixel = XGetPixel(kgi,x_scan-ix-windx,y_scan-iy-windy);
                            if ((good_pixel&mask) != (test_pixel&mask))
                                found_mismatch = done = 1;
                            else {
                                x_scan++;
                                if ((x_scan == old_x_scan)&&(y_scan == old_y_scan))
                                    done = 1;
                            }
                        }
                        if (!done) {
                            x_scan = lox;
                            y_scan++;
                            if (y_scan >= hiy)
                                y_scan = loy;
                        }
                    }
                    if (found_mismatch) {
                        start_x = x_scan - (pixels_across>>1);
			start_x = (start_x < 0) ? 0 : start_x;
                        start_y = y_scan - (pixels_across>>1);               
			start_y = (start_y < 0) ? 0 : start_y;
                    }
                }
		else if (xev.xbutton.window == blowup_win)
		{
		    start_x += (xev.xbutton.x - key_win_width - 2) / zoom_factor - (pixels_across>>1);
		    start_y += (xev.xbutton.y - x_win_height - 1) / zoom_factor - (pixels_across>>1);
		}
		else if (xev.xbutton.window == window)  {
		    tar_win = 1; /* current window is target window */
	    	    pixels_across = size / zoom_factor;
		    x = (xev.xbutton.x/*_root*/) / (*winzoomp);
		    y = (xev.xbutton.y/*_root*/) / (*winzoomp);
		    start_x = x - (pixels_across / 2);
		    start_y = y - (pixels_across / 2);
		}

		XSetForeground(display, textgc, gcfore); 
		XSetBackground(display, textgc, gcback); 
                if (show_color) 
                    XSetWindowBackground(display,blowup_win,background);
                else
                    XSetWindowBackground(display,blowup_win,gcback);
	    	XClearWindow(display, blowup_win);
	    	XClearWindow(display, pix_win);
                XClearWindow(display, exp_win);
	    	XClearWindow(display, key[0]);             
                XClearWindow(display, key[1]);
	    	if (start_x > (max_x - pixels_across)) start_x = max_x - pixels_across;
	    	if (start_x < 0) start_x = 0;
	    	if (start_y > (max_y - pixels_across)) start_y = max_y - pixels_across;
	    	if (start_y < 0) start_y = 0;

#ifdef DEBUG
(void)printf("Picked BAD pixel at (%d,%d) - in [0..%d, 0..%d]?\n",
		start_x + (pixels_across>>1), start_y + (pixels_across>>1),
		pbi->width, pbi->height);
#endif /* DEBUG */
      	        (void)sprintf(xypair, " Pixel = %lx        ",
			XGetPixel(pbi,
                            start_x + (pixels_across>>1),
			    start_y + (pixels_across>>1)));
	    	XDrawString(display, pix_win, textgc, 2, 10, xypair, strlen(xypair));
		if (tar_win)
		    {
	    	    (void)sprintf(xypair, " X = %dW      ",start_x - windx + (pixels_across>>1)); 
	    	    XDrawString(display, key[0], textgc, 2, 10, xypair, strlen(xypair));
	    	    (void)sprintf(xypair, " Y = %dW      ",start_y - windy + (pixels_across>>1));
	    	    XDrawString(display, key[1], textgc, 2, 10, xypair, strlen(xypair));
		    }
		else
		    {
	    	    (void)sprintf(xypair, " X = %dR      ",start_x + (pixels_across>>1)); 
	    	    XDrawString(display, key[0], textgc, 2, 10, xypair, strlen(xypair));
	    	    (void)sprintf(xypair, " Y = %dR      ",start_y + (pixels_across>>1));
	    	    XDrawString(display, key[1], textgc, 2, 10, xypair, strlen(xypair));
		    }
		/* undraw old rectangle on window, if there was one */
                if (old_start_x != -9999)
                {    
		    x = (old_start_x-windx) * (*winzoomp);
		    y = (old_start_y-windy) * (*winzoomp);
		    i = old_pixels_across * (*winzoomp);
		    XDrawRectangle(display,window,rect_gc, x,y,i,i);
                }

		/* If we've got any events put-back on Q then vmakebig() must
		   not loose them. In practice this only matters with Expose
		   events. All others are on the head of the Q before entering
		   the main loop. Don't put any non-expose events onto the Q
		   in any of the "if (xev.xbutton.window == ..." clauses
		   above, as they (nearly) all drop through to here and would
		   get munched by vmakebig().	- kieron
		*/
	    	vmakebig(display, window, blowup_win, start_x, start_y,
		   	zoom_factor, pixels_across, size, format, gc, gcback, gcfore, kgi, pbi, ix, iy, background,
                        show_color,compare_color);

		/* draw new rectangle on window, remebering it for subsequent
		   undrawing.
		*/
                old_start_x = start_x;
                old_start_y = start_y;
                old_pixels_across = pixels_across;
		x = (start_x-windx) * (*winzoomp);
		y = (start_y-windy) * (*winzoomp);
		i = pixels_across * (*winzoomp);
                XDrawRectangle(display,window,rect_gc,x,y,i,i);

                if ((pixel_wrong)&&(show_color))
                {                                                
      	            (void)sprintf(xypair, " Expect = %X        ",expected_pixel);
	    	    XDrawImageString(display, exp_win, textgc, 2, 10, xypair, 18);
                }
                else
                {
                    sprintf(xypair,"                     ");
	    	    XDrawImageString(display, exp_win, textgc, 2, 10, xypair, strlen(xypair));
                }
  	    }
	    else {	/* highlight movement option */
		if (highlight != xev.xmotion.window) {
			if (highlight != None) {            /* if someone is currently highlighted, unhighlight them */
			XSetForeground(display, textgc, gcfore); 
			XSetBackground(display, textgc, gcback); 
			h_light(display, key+2, 9, textgc, labels+2, highlight, kgi);
			}                              	/* now highlight the requestor */
			XSetBackground(display, textgc, gcfore); 
			XSetForeground(display, textgc, gcback); 
			highlight = xev.xmotion.window; 
			h_light(display, key+2, 9, textgc, labels+2, highlight, kgi);
			/* munch any highlight events left around... see caveat
			   before call on vmakebig() above.
			*/
			while (XCheckMaskEvent(display, PointerMotionMask, &fake))
				; /* donothing */
	 	} /* end not current selection */
	    } /* end highlight */
	}  /* end while(1) */                                
	x = (start_x-windx) * (*winzoomp);
	y = (start_y-windy) * (*winzoomp);
	i = pixels_across * (*winzoomp);
        XDrawRectangle(display,window,rect_gc,x,y,i,i);
	XDestroyWindow(display, blowup_win);
	XFreeCursor(display, curse);
	XUnloadFont(display, fid);
	XFreeGC(display, textgc);
	XFreeGC(display, gc);
	XSync(display, 0);
	return retval;
}
