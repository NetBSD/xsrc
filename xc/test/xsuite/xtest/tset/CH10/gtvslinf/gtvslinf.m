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
 * $XConsortium: gtvslinf.m,v 1.6 94/04/17 21:09:37 rws Exp $
 */
>>TITLE XGetVisualInfo CH10
XVisualInfo *
XGetVisualInfo(display, vinfo_mask, vinfo_template, nitems_return)
Display	*display = Dsp;
long	vinfo_mask;
XVisualInfo	*vinfo_template = &v_tmpl;
int	*nitems_return = &n_ret;
>>EXTERN
int	n_ret;
int	n_all;
XVisualInfo	v_tmpl;

void
init_vp(mask, vp, vptmpl)
long mask;
XVisualInfo     *vp;
XVisualInfo     *vptmpl;
{
	vptmpl->visualid = 0;
	vptmpl->screen = -1;
	vptmpl->depth = 0;
	vptmpl->class = -1;
	vptmpl->red_mask = 0L;
	vptmpl->green_mask = 0L;
	vptmpl->blue_mask = 0L;
	vptmpl->colormap_size = 0;
	vptmpl->bits_per_rgb = 0;

	if ((mask & VisualIDMask))
		vptmpl->visualid = vp->visualid;
	if ((mask & VisualScreenMask))
		vptmpl->screen = vp->screen;
	if ((mask & VisualDepthMask))
		vptmpl->depth = vp->depth;
	if ((mask & VisualClassMask))
		vptmpl->class = vp->class;
	if ((mask & VisualRedMaskMask))
		vptmpl->red_mask = vp->red_mask;
	if ((mask & VisualGreenMaskMask))
		vptmpl->green_mask = vp->green_mask;
	if ((mask & VisualBlueMaskMask))
		vptmpl->blue_mask = vp->blue_mask;
	if ((mask & VisualColormapSizeMask))
		vptmpl->colormap_size = vp->colormap_size;
	if ((mask & VisualBitsPerRGBMask))
		vptmpl->bits_per_rgb = vp->bits_per_rgb;
}

XVisualInfo *
get_visuals(mask, vp, n_all, vptest, n)
long mask;
XVisualInfo     *vp;
int	n_all;
XVisualInfo     *vptest;
int	*n;
{
XVisualInfo     *vptr, *vpret;
int	i;

	*n = 0;

	/* Allocate enough space (all visuals may match vptest) */
	vptr = vpret = (XVisualInfo *)calloc(n_all, sizeof(XVisualInfo));

	/* Obtain sub-list of visuals which match in the mask fields */
	for(i = 0; i < n_all; i++, vp++) {
		/* All visuals have same screen, but check in case code */
		/*   gets used another way one day */
		if ((mask & VisualIDMask) && (vptest->visualid != vp->visualid))
			continue;
		if ((mask & VisualScreenMask) && (vptest->screen != vp->screen))
			continue;
		if ((mask & VisualDepthMask) && (vptest->depth != vp->depth))
			continue;
		if ((mask & VisualClassMask) && (vptest->class != vp->class))
			continue;
		if ((mask & VisualRedMaskMask) && (vptest->red_mask != vp->red_mask))
			continue;
		if ((mask & VisualGreenMaskMask) && (vptest->green_mask != vp->green_mask))
			continue;
		if ((mask & VisualBlueMaskMask) && (vptest->blue_mask != vp->blue_mask))
			continue;
		if ((mask & VisualColormapSizeMask) && (vptest->colormap_size != vp->colormap_size))
			continue;
		if ((mask & VisualBitsPerRGBMask) && (vptest->bits_per_rgb != vp->bits_per_rgb))
			continue;
		*vptr++ = *vp;
		(*n)++;
		trace("get_visuals: Adding visual number %d", *n);
	}

	return(vpret);
}

void
report_visuals(n, vp)
int	n;
XVisualInfo     *vp;
{
int	i;

	for(i = 0; i < n; i++, vp++) {
		report("Visual %d: screen %d; depth %d; class %s.",
			i, vp->screen, vp->depth, displayclassname(vp->class));
	}
}

int
check_visuals(mask, n, expected, observed)
long mask;
int	n;
XVisualInfo     *expected;
XVisualInfo     *observed;
{
XVisualInfo     *vp, *vpo;
int	i, j;
int	matched;
int	pass = 0;

	for (i = 0, vp = expected; i < n; i++, vp++) {
		matched = 0;
		for (j = 0, vpo = observed; j < n; j++, vpo++) {
			if (vp->visualid != vpo->visualid)
				continue;
			if (vp->screen != vpo->screen)
				continue;
			if (vp->depth != vpo->depth)
				continue;
			if (vp->class != vpo->class)
				continue;
			if (vp->red_mask != vpo->red_mask)
				continue;
			if (vp->green_mask != vpo->green_mask)
				continue;
			if (vp->blue_mask != vpo->blue_mask)
				continue;
			if (vp->colormap_size != vpo->colormap_size)
				continue;
			if (vp->bits_per_rgb != vpo->bits_per_rgb)
				continue;
			matched = 1;
			break;
		}
		if (!matched) {
			report("%s did not return matching visual", TestName);
			report("when selecting with mask %d (%s)", mask,
			  visualmaskname(mask));
			report_visuals(1, vp);
			return(1);
		} else
			pass++;
	}

	if (n > 0 && pass == n)
		return(0);
	report("Path counter was %d instead of %d", pass, n);
	return(-1);
}

int
check_mask_visuals(mask, all, n_all)
long mask;
XVisualInfo     *all;
int	n_all;
{
XVisualInfo     *vp, *expected, *visuals;
int	i;
int	r;
int	n_expected;
int	pass = 0;

	/* For each visual */
	for(i = 0, vp = all; i < n_all; i++, vp++) {
		/* Obtain list of visuals matching vp in the masked fields. */
		/* This is a bit like our own XGetVisualInfo. */
		expected = get_visuals(mask, all, n_all, vp, &n_expected);

		/* 
		 * Need VisualScreenMask, since "all" points to a list
		 * of visuals for the default screen only.
		 */
		init_vp(mask, vp, vinfo_template);
 		vinfo_template->screen = XDefaultScreen(Dsp);
		vinfo_mask = VisualScreenMask | mask;

		/* Call XGetVisualInfo directly to avoid return; in code */
                startcall(display);
                if (isdeleted())
                        return(-1);
		visuals = XGetVisualInfo(display, vinfo_mask, vinfo_template, nitems_return);
		endcall(display);
		if (geterr() != Success) {
			report("Got %s, Expecting Success", errorname(geterr()));
			return(1);
		}

		/* 
		 * Verify that xname does not return NULL.
		 * (should be at least one visual - vp!).
		 */
		if (visuals == (XVisualInfo *)NULL) {
			report("%s returned NULL with visual mask %d", 
					TestName, visualmaskname(mask));
			return(1);
		} else
			pass++;

		/* Verify that xname returns same size list. */
		if (n_ret != n_expected) {
			report("%s returned %d instead of %d visual(s)",
						TestName, n_ret, n_expected);
			report("when selecting with mask %s", 
			  			visualmaskname(mask));
			report_visuals(n_ret, visuals);
			return(1);
		} else {
			pass++;
			trace("%s returned %d visual(s)", TestName, n_ret);
		}

		/* Verify that xname returns same sub-list. */
		if ((r = check_visuals(mask, n_expected, expected, visuals)) < 0)
			return(-1);
		else if (r > 0) 
			return(1);
		else
			pass++;

		/* Free the list of all visuals using XFree. */
		XFree((char*)visuals);
		free((void*)expected);
	}

	if (n_all > 0 && pass == 3*n_all)
		return(0);
	report("Path counter was %d instead of %d", pass, 3*n_all);
	return(-1);
}

>>ASSERTION Good A
A call to xname returns the list of
.S XVisualInfo
structures, which can be freed by
.S XFree ,
which match those components of the
.A vinfo_template
argument specified by the
.A vinfo_mask
argument, and returns the number of such structures in the
.A nitems_return
argument.
>>STRATEGY
Initialise the list of class/depth pairs from parameter XT_VISUAL_CLASSES.
Obtain list of all visuals for the screen.
Verify that xname does not return NULL (should be at least one visual).
Verify that xname returns at least one visual for each class/depth pair
  specified in parameter XT_VISUAL_CLASSES.
Verify that the list returned matches those in XT_VISUAL_CLASSES.
Verify that each visual can be selected using VisualIDMask.
Verify that each visual can be selected using VisualScreenMask.
Verify that each visual can be selected using VisualDepthMask.
Verify that each visual can be selected using VisualClassMask.
Verify that each visual can be selected using
  VisualScreenMask|VisualDepthMask|VisualClassMask.
Verify that each visual can be selected using VisualRedMaskMask.
Verify that each visual can be selected using VisualGreenMaskMask.
Verify that each visual can be selected using VisualBlueMaskMask.
Verify that each visual can be selected using VisualColormapSizeMask.
Verify that each visual can be selected using VisualBitsPerRGBMask.
Verify that each visual can be selected using
  VisualBitsPerRGBMask|VisualColormapSizeMask.
Verify that each visual can be selected using VisualAllMask.
Free the list of all visuals using XFree.
>>CODE
int	j;
int	r;
int	matched;
int	class;
int depth;	/* (depths usually unsigned, but not for nextvclass) */
XVisualInfo	*all, *vp;

/* Initialise the list of class/depth pairs from parameter XT_VISUAL_CLASSES. */
	if(initvclass() < 0) {
		delete("The list of expected visual classes could not be initialised.");
		return;
	}

/* Obtain list of all visuals for the screen. */
 	vinfo_template->screen = XDefaultScreen(Dsp);
	vinfo_mask = VisualNoMask | VisualScreenMask;
	all = XCALL;
	n_all = n_ret;

/* Verify that xname does not return NULL (should be at least one visual). */
	if (all == (XVisualInfo *)NULL) {
		report("%s returned NULL with visual mask %d", 
					TestName, visualmaskname(vinfo_mask));
		FAIL;
		return;
	} else
		CHECK;

/* Verify that xname returns at least one visual for each class/depth pair */
/*   specified in parameter XT_VISUAL_CLASSES. */
	if (n_all < nvclass()) {
		report("%s returned %d instead of at least %d visual(s)",
						TestName, n_all, nvclass());
		report_visuals(n_all, all);
		FAIL;
		return;
	} else {
		CHECK;
		trace("%s returned %d visual(s)", TestName, n_all);
	}

/* Verify that the list returned matches those in XT_VISUAL_CLASSES. */
	for(j = 0, vp = all; j < n_all; j++, vp++) {
		if (vp->screen != XDefaultScreen(Dsp)) {	
			report("%s returned visual for wrong screen", TestName);
			report("Expected screen: %d; Observed screen %d.",
				XDefaultScreen(Dsp), vp->screen);
			report_visuals(1, vp);
			FAIL;
		} else
			CHECK;

		matched = 0;
        	for (resetvclass(); nextvclass(&class, &depth); ) {
			if (vp->depth == (unsigned)depth &&
			    vp->class == class) {
				matched = 1;
				break;
			}
		}
		if (!matched) {
			report("%s returned visual", TestName);
			report("which is not specified in XT_VISUAL_CLASSES.");
			report_visuals(1, vp);
			FAIL;
		} else
			CHECK;
	}

/* Verify that each visual can be selected using VisualIDMask. */
	if((r = check_mask_visuals((long)VisualIDMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using VisualScreenMask. */
	if((r = check_mask_visuals((long)VisualScreenMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using VisualDepthMask. */
	if((r = check_mask_visuals((long)VisualDepthMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using VisualClassMask. */
	if((r = check_mask_visuals((long)VisualClassMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using */
/*   VisualScreenMask|VisualDepthMask|VisualClassMask. */
	if((r = check_mask_visuals((long)(VisualScreenMask|
					VisualDepthMask|
					VisualClassMask), all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using VisualRedMaskMask. */
	if((r = check_mask_visuals((long)VisualRedMaskMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using VisualGreenMaskMask. */
	if((r = check_mask_visuals((long)VisualGreenMaskMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using VisualBlueMaskMask. */
	if((r = check_mask_visuals((long)VisualBlueMaskMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using VisualColormapSizeMask. */
	if((r = check_mask_visuals((long)VisualColormapSizeMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using VisualBitsPerRGBMask. */
	if((r = check_mask_visuals((long)VisualBitsPerRGBMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using */
/*   VisualBitsPerRGBMask|VisualColormapSizeMask. */
	if((r = check_mask_visuals((long)(VisualBitsPerRGBMask|
			      VisualColormapSizeMask), all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;
/* Verify that each visual can be selected using VisualAllMask. */
	if((r = check_mask_visuals((long)VisualAllMask, all, n_all)) < 0) {
		delete("Path check error in check_mask_visuals");
		return;
	} else if (r > 0)
		FAIL;
	else
		CHECK;

/* Free the list of all visuals using XFree. */
	XFree((char*)all);

	CHECKPASS(14 + 2*n_all);

>>ASSERTION Bad A
When no visuals match the template specified by the
.A vinfo_mask
and
.A vinfo_template
arguments, 
then a call to xname returns NULL.
>>STRATEGY
Obtain list of all visuals for the screen.
Initialise visual template to first visual returned.
Call xname with VisualDepthMask and depth 0.
Verify that number of matching visuals is set to 0.
Verify that xname returns NULL.
>>CODE
XVisualInfo	*visuals;
XVisualInfo	*all;

/* Obtain list of all visuals for the screen. */
        vinfo_template->screen = XDefaultScreen(Dsp);
        vinfo_mask = VisualNoMask | VisualScreenMask;
        all = XCALL;

/* Initialise visual template to first visual returned. */
	init_vp((long)VisualAllMask, all, vinfo_template);
	if(all != (XVisualInfo *)NULL)
		XFree((char*)all);

/* Call xname with VisualDepthMask and depth 0. */
	vinfo_template->depth = 0;
	vinfo_mask = VisualNoMask | VisualDepthMask;
	visuals = XCALL;

/* Verify that number of matching visuals is set to 0. */
	if (n_ret != 0) {
		report("%s set nitems_return to %d instead of 0", 
						TestName, n_ret);
		FAIL;
	} else
		CHECK;

/* Verify that xname returns NULL. */
	if (visuals != (XVisualInfo *)NULL) {
		XFree((char*)visuals);
		report("%s did not return NULL", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad B 1
When sufficient storage cannot be allocated,
then a call to xname returns NULL.
