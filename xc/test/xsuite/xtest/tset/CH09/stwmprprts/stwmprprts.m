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
 * $XConsortium: stwmprprts.m,v 1.19 94/04/17 21:09:11 rws Exp $
 */
>>TITLE XSetWMProperties CH09
void
XSetWMProperties(display, w, window_name, icon_name, argv, argc, normal_hints, wm_hints, class_hints)
Display		*display = Dsp;
Window		w = DRW(Dsp);
XTextProperty	*window_name = (XTextProperty *) NULL;
XTextProperty	*icon_name =  (XTextProperty *) NULL;
char		**argv = (char **) NULL;
int		argc = 0;
XSizeHints	*normal_hints = (XSizeHints *) NULL;
XWMHints	*wm_hints = (XWMHints *) NULL;
XClassHint	*class_hints = (XClassHint *) NULL;
>>MAKE
>>#
>>#
>># Plant some rules in the Makefile to construct
>># stand-alone executables Test1 and Test2 to allow the setting
>># of environment variables.
>>#
>># Cal 14/6/91
>>#
#
# The following lines are copied from the .m file by mc
# under control of the >>MAKE directive
# to create rules for the executable files Test1 and Test2.
#
AUXFILES=Test1 Test2
AUXCLEAN=Test1.o Test1 Test2.o Test2

all: Test

Test1 : Test1.o $(LIBS) $(TCMCHILD)
	$(CC) $(LDFLAGS) -o $@ Test1.o $(TCMCHILD) $(LIBLOCAL) $(LIBS) $(SYSLIBS)
 
Test2 : Test2.o $(LIBS) $(TCMCHILD)
	$(CC) $(LDFLAGS) -o $@ Test2.o $(TCMCHILD) $(LIBLOCAL) $(LIBS) $(SYSLIBS)
 
#
# End of section copied from the .m file.
#
>>#
>>#
>>#
>>EXTERN
#define		NewNumPropSizeElements 18       /* ICCCM v. 1 */
#define		NumPropWMHintsElements 9
#include	"Xatom.h"
static XSizeHints	sizehints = { PAllHints,1,2,3,4,5,6,7,8,9,10, {11,12}, {13,14}, 15, 16, 17};
static XSizeHints	sizehints_1 = { PAllHints,1,2,3,4,5,6,7,8,9,10, {11,12}, {13,14}, 15, 16, 17};
>>ASSERTION Good A
When the
.A window_name
argument is non-NULL, then a call to xname
sets the WM_NAME property for the window
.A w
to be of data, type, format and number of items as specified by the
.M value
field, the
.M encoding
field, the
.M format
field, and the
.M nitems
field of the
.S XTextProperty 
structure named by the
.A window_name
argument.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_NAME property with XSetWMProperties.
Obtain the WM_NAME property using XGetTextProperty.
Verify that the format, encoding and value are correct.
>>CODE
Window	win;
char	*str1 = "Xtest test string1";
char	*str2 = "Xtest test string2";
char	*str[2];
Status	status;
char	**list_return;
int	count_return;
XTextProperty	tp, rtp;
XVisualInfo	*vp;

	str[0] = str1;
	str[1] = str2;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	if(XStringListToTextProperty(str, 2, &tp) == False) {
		delete("XStringListToTextProperty() Failed.");
		return;
	} else
		CHECK;

	w = win;
	window_name = &tp;
	XCALL;

	if(XGetTextProperty(display, win, &rtp, XA_WM_NAME) == False) {
		delete("XGetTextProperty() returned False.");
		return;
	} else
		CHECK;
	
	if(tp.encoding != rtp.encoding) {
		report("The encoding component of the XTextProperty was incorrect.");
		FAIL;
	} else
		CHECK;

	if(tp.format != rtp.format) {
		report("The format component of the XTextProperty was %d instead of %d.", rtp.format, tp.format );
		FAIL;
	} else
		CHECK;

	if(tp.nitems != rtp.nitems) {
		report("The nitems component of the XTextProperty was %lu instead of %lu.", rtp.nitems, tp.nitems);
		FAIL;
	} else
		CHECK;

	if(XTextPropertyToStringList( &rtp, &list_return, &count_return) == False) {
		delete("XTextPropertyToStringList() returned False.");
		return;
	} else
		CHECK;

	if (count_return != 2) {
		delete("XTextPropertyToStringList() count_return was %d instead of 2.", count_return);
		return;
	} else
		CHECK;

	if( (strncmp(str1, list_return[0], strlen(str1)) != 0) || (strncmp(str2, list_return[1], strlen(str2)) != 0) ) {
		report("Value strings were:");
		report("\"%s\" and \"%s\"", list_return[0], list_return[1]);
		report("Instead of:");
		report("\"%s\" and \"%s\"", str1, str2);
		FAIL;
	} else
		CHECK;

	XFree((char*)tp.value);
	XFree((char*)rtp.value);
	XFreeStringList(list_return);

	CHECKPASS(8);

>>ASSERTION Good A
When the
.A icon_name
argument is non-NULL, then
a call to xname sets the WM_ICON_NAME
property for the window
.A w
to be of data, type, format and number of items as specified by the
.M value
field, the
.M encoding
field, the
.M format
field, and the
.M nitems
field of the
.S XTextProperty 
structure named by the
.A icon_name
argument.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_ICON_NAME property with XSetWMProperties.
Obtain the WM_ICON_NAME property using XGetTextProperty.
Verify that the property format was correct.
Verify that the property type was correct.
Verify that the propery value was correct.
Free allocated property with XFree.
>>CODE
Window	win;
char	*str1 = "Xtest test string1";
char	*str2 = "Xtest test string2";
char	*str[2];
Status	status;
char	**list_return;
int	count_return;
XTextProperty	tp, rtp;
XVisualInfo	*vp;

	str[0] = str1;
	str[1] = str2;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	if(XStringListToTextProperty(str, 2, &tp) == False) {
		delete("XStringListToTextProperty() Failed.");
		return;
	} else
		CHECK;
	w = win;
	icon_name = &tp;
	XCALL;

	if(XGetWMIconName(display, win, &rtp) == False) {
		delete("XGetWMIconName() returned False.");
		return;
	} else
		CHECK;

	if(XGetTextProperty(display, win, &rtp, XA_WM_ICON_NAME) == False) {
		delete("XGetTextProperty() returned False.");
		return;
	} else
		CHECK;

	if(tp.encoding != rtp.encoding) {
		report("The encoding component of the XTextProperty was incorrect.");
		FAIL;
	} else
		CHECK;

	if(tp.format != rtp.format) {
		report("The format component of the XTextProperty was %d instead of %d.", rtp.format, tp.format);
		FAIL;
	} else
		CHECK;

	if(tp.nitems != rtp.nitems) {
		report("The nitems component of the XTextProperty was %lu instead of %lu.", rtp.nitems, tp.nitems);
		FAIL;
	} else
		CHECK;

	if(XTextPropertyToStringList( &rtp, &list_return, &count_return) == False) {
		delete("XTextPropertyToStringList() returned False.");
		return;
	} else
		CHECK;

	if (count_return != 2) {
		delete("XTextPropertyToStringList() count_return was %d instead of 2.", count_return);
		return;
	} else
		CHECK;

	if( (strncmp(str1, list_return[0], strlen(str1)) != 0) || (strncmp(str2, list_return[1], strlen(str2)) != 0) ) {
		report("Value strings were:");
		report("\"%s\" and \"%s\"", list_return[0], list_return[1]);
		report("Instead of:");
		report("\"%s\" and \"%s\"", str1, str2);
		FAIL;
	} else
		CHECK;

	XFree((char*)tp.value);
	XFree((char*)rtp.value);
	XFreeStringList(list_return);

	CHECKPASS(9);

>>ASSERTION Good A
When the
.A argv
argument is non-NULL, then a call to xname
sets the WM_COMMAND property for the window
.A w
to the
.A argv
argument.
>>STRATEGY
Create a window using XCreateWindow.
Set the WM_COMMAND property using XSetWMProperties.
Obtain the value of the WM_COMMAND property using XGetTextProperty.
Verify that the number and value of the returned strings is correct.
Release the allocated memory using XFreeStringList.
>>CODE
XVisualInfo	*vp;
char	*nullstr = "<NULL>";
char	**strpp, *strp;
char	*str1 = "XTest string 1";
char	*str2 = "XTest string 2";
char	*prop[2];
char	**rstrings = (char**) NULL;
int	rcount = 0;
int	i;
XTextProperty	rtp;
Status	status;
char	**list_return;
int	count_return;
int	len;

	prop[0] = str1;
	prop[1] = str2;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);
	argv = prop;
	argc = 2;
	XCALL;

	if(XGetTextProperty(display, w, &rtp, XA_WM_COMMAND) == False) {
		delete("XGetTextProperty() returned False.");
		return;
	} else
		CHECK;

	if(rtp.encoding != XA_STRING ) {
		report("The encoding component of the XTextProperty was %lu instead of STRING (%lu).",
			(unsigned long)rtp.encoding, (unsigned long)XA_STRING);
		FAIL;
	} else
		CHECK;

	if(rtp.format != 8) {
		report("The format component of the XTextProperty was %d instead of %d.", rtp.format, 8 );
		FAIL;
	} else
		CHECK;

	len = strlen(str1) + 1 + strlen(str2) + 1;

	if(rtp.nitems != len) {
		report("The nitems component of the XTextProperty was %lu instead of %lu.", rtp.nitems, len);
		FAIL;
	} else
		CHECK;

    /*
     * Ignore final <NUL> if present since UNIX WM_COMMAND is nul-terminated, unlike
     * the nul-separated text properties.
     * Cal - 7/6/91
     */
	if (rtp.value[rtp.nitems - 1] == '\0') rtp.nitems--;


	if(XTextPropertyToStringList( &rtp, &list_return, &count_return) == False) {
		delete("XTextPropertyToStringList() returned False.");
		return;
	} else
		CHECK;
	
	if (count_return != argc) {
		delete("XTextPropertyToStringList() count_return was %d instead of %d.", count_return, argc);
		return;
	} else
		CHECK;

	if( (strcmp(str1, list_return[0]) != 0) || (strcmp(str2, list_return[1]) != 0) ) {
		report("Value strings were:");
		report("\"%s\" and \"%s\"", list_return[0], list_return[1]);
		report("Instead of:");
		report("\"%s\" and \"%s\"", str1, str2);
		FAIL;
	} else
		CHECK;

	XFree((char*)rtp.value);
	XFreeStringList(list_return);

	CHECKPASS(7);

>>ASSERTION Good A
When the
.A normal_hints
argument is non-NULL, then a call to xname
sets the WM_NORMAL_HINTS property for the window
.A w
to be of type
.S WM_SIZE_HINTS ,
format 32 and to have value set
to the
.A normal_hints
argument.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_NORMAL_HINTS property with XSetWMProperties.
Obtain the value, type and format of the WM_NORMAL_HINTS property using XGetWindowProperty.
Verify that the type is WM_SIZE_HINTS.
Verify that the format is 32.
Verify that the value is identical to that value set by XSetWMNormalHints.
>>CODE
Window		win;
XVisualInfo	*vp;
Atom		rtype;
int		rformat;
unsigned long	ritems, rbytes, *uls = NULL;
XSizeHints	pp;
int		i;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	w = win;	
	normal_hints = &sizehints_1;
	XCALL;

	if( XGetWindowProperty(display, win, XA_WM_NORMAL_HINTS, 0L,
			(long) NewNumPropSizeElements, False,
			AnyPropertyType, &rtype, &rformat, &ritems, &rbytes,
			(unsigned char **) &uls) != Success ) {
		delete("XGetWindowProperty() did not return Success.");
		return;
	} else
		CHECK;

	if( rtype !=  XA_WM_SIZE_HINTS ) {
		report("WM_NORMAL_HINTS property was type %lu instead of XA_WM_SIZE_HINTS (%lu)",
			(unsigned long) rtype, (unsigned long) XA_WM_SIZE_HINTS);
		FAIL;
	} else
		CHECK;

	if( rformat !=  32 ) {
		report("WM_NORMAL_HINTS property was format %d instead of 32.", rformat);
		FAIL;
	} else
		CHECK;

	/* unpack from the array of unsigned longs into pp */
	pp.flags = uls[i=0];

	pp.x = (int)uls[++i]; /* obsolete for new window mgrs, but clients */
	pp.y = (int)uls[++i];
	pp.width = (int)uls[++i];
	pp.height = (int)uls[++i]; /* should set so old wm's don't mess up */

	pp.min_width = (int)uls[++i]; pp.min_height = (int)uls[++i];
	pp.max_width = (int)uls[++i]; pp.max_height = (int)uls[++i];
    	pp.width_inc = (int)uls[++i]; pp.height_inc = (int)uls[++i];
	pp.min_aspect.x = (int)uls[++i]; pp.min_aspect.y = (int)uls[++i];
	pp.max_aspect.x = (int)uls[++i]; pp.max_aspect.y = (int)uls[++i];

	pp.base_width = (int)uls[++i];	/* added by ICCCM version 1 */
	pp.base_height = (int)uls[++i];	/* added by ICCCM version 1 */
	pp.win_gravity = (int)uls[++i];	/* added by ICCCM version 1 */

	if(pp.flags != PAllHints) {
		report("The flags component of the XSizeHints structure was %lu instead of PAllHints (%ld).", pp.flags, PAllHints);
		FAIL;
	} else
		CHECK;

	if(pp.x != 1) {
		report("The x component of the XSizeHints structure was %d instead of 1.", pp.x);
		FAIL;
	} else
		CHECK;

	if(pp.y != 2) {
		report("The y component of the XSizeHints structure was %d instead of 2.", pp.y);
		FAIL;
	} else
		CHECK;

	if(pp.width != 3) {
		report("The width component of the XSizeHints structure was %d instead of 3.", pp.width);
		FAIL;
	} else
		CHECK;

	if(pp.height != 4) {
		report("The height component of the XSizeHints structure was %d instead of 4.", pp.height);
		FAIL;
	} else
		CHECK;

	if(pp.min_width != 5) {
		report("The min_width component of the XSizeHints structure was %d instead of 5.", pp.min_width);
		FAIL;
	} else
		CHECK;

	if(pp.min_height != 6) {
		report("The min_height component of the XSizeHints structure was %d instead of 6.", pp.min_height);
		FAIL;
	} else
		CHECK;

	if(pp.max_width != 7) {
		report("The max_width component of the XSizeHints structure was %d instead of 7.", pp.max_width);
		FAIL;
	} else
		CHECK;

	if(pp.max_height != 8) {
		report("The max_height component of the XSizeHints structure was %d instead of 8.", pp.max_height);
		FAIL;
	} else
		CHECK;

	if(pp.width_inc != 9) {
		report("The width_inc component of the XSizeHints structure was %d instead of 9.", pp.width_inc);
		FAIL;
	} else
		CHECK;

	if(pp.height_inc != 10) {
		report("The height_inc component of the XSizeHints structure was %d instead of 10.", pp.height_inc);
		FAIL;
	} else
		CHECK;

	if((pp.min_aspect.x != 11) || (pp.min_aspect.y != 12)){
		report("The min_aspect components of the XSizeHints structure were %d, %d instead of 11, 12.",
			pp.min_aspect.x, pp.min_aspect.y);
		FAIL;
	} else
		CHECK;

	if((pp.max_aspect.x != 13) || (pp.max_aspect.y != 14)){
		report("The max_aspect components of the XSizeHints structure were %d, %d instead of 13, 14.",
			pp.max_aspect.x, pp.max_aspect.y);
		FAIL;
	} else
		CHECK;

	if(pp.base_width != 15) {
		report("The base_width component of the XSizeHints structure was %d instead of 15.", pp.base_width);
		FAIL;
	} else
		CHECK;

	if(pp.base_height != 16) {
		report("The base_height component of the XSizeHints structure was %d instead of 16.", pp.base_height);
		FAIL;
	} else
		CHECK;

	if(pp.win_gravity != 17) {
		report("The win_gravity component of the XSizeHints structure was %d instead of 17", pp.win_gravity);
		FAIL;
	} else
		CHECK;

	XFree((char*)uls);
	CHECKPASS(19);
	
>>ASSERTION Good A
When the
.A wm_hints
argument is non-NULL, then a call to xname
sets the WM_HINTS property for the window
.A w
to be of type
.S WM_HINTS ,
format 32 and to have value set
to the
.A wm_hints
argument.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_HINTS property for the window with XSetWMProperties.
Verify type and format are XA_WM_HINTS and 32, respectively.
Verify that the property value was correctly set with XGetWindowProperty.
>>CODE
Window		win;
XVisualInfo	*vp;
XWMHints	hints;
long		*hints_ret;
unsigned long	leftover, nitems;
int		actual_format;
Atom		actual_type;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	hints.flags = AllHints;
	hints.input = True;
	hints.initial_state = IconicState;
	hints.icon_pixmap =  154376L;
	hints.icon_window = 197236L;
	hints.icon_x = 13;
	hints.icon_y = 7;
	hints.icon_mask = 146890L;
	hints.window_group = 137235L;

	w = win;
	wm_hints = &hints;
	XCALL;

	if (XGetWindowProperty(display, win, XA_WM_HINTS, 0L, (long)NumPropWMHintsElements,
	    False, AnyPropertyType, &actual_type, &actual_format,  &nitems, &leftover,
	    (unsigned char **)&hints_ret) != Success) {
		delete("XGetWindowProperty() did not return Success.");
		return;
	} else
		CHECK;

	if(leftover != 0) {
		report("The leftover elements numbered %lu instead of 0", leftover);
		FAIL;
	} else
		CHECK;

	if(actual_format != 32) {
		report("The format of the WM_HINTS property was %lu instead of 32", actual_format);
		FAIL;
	} else
		CHECK;

	if(actual_type != XA_WM_HINTS) {
		report("The type of the WM_HINTS property was %lu instead of XA_WM_HINTS (%lu)", actual_type, XA_WM_HINTS);
		FAIL;
	} else
		CHECK;

	if(nitems != NumPropWMHintsElements) {
		report("The number of elements comprising the WM_HINTS property was %lu instead of %lu.",
				actual_type, (unsigned long) NumPropWMHintsElements);
		FAIL;
	} else
		CHECK;

	if(hints_ret[0] != hints.flags) {
		report("The flags component was %lu instead of %lu.", hints_ret[0], hints.flags);
		FAIL;
	} else
		CHECK;

	if(hints_ret[1] != hints.input) {
		report("The hints_ret component of the XWMHints structure was %lu instead of %d.", hints_ret[1], hints.input);
		FAIL;
	} else
		CHECK;

	if(hints_ret[2] != hints.initial_state) {
		report("The initial_state component of the XWMHints structure was %lu instead of %d.", hints_ret[2], hints.initial_state);
		FAIL;
	} else
		CHECK;

	if(hints_ret[3] != hints.icon_pixmap) {
		report("The icon_pixmap component of the XWMHints structure was %lu instead of %lu.", hints_ret[3], hints.icon_pixmap);
		FAIL;
	} else
		CHECK;

	if(hints_ret[4] != hints.icon_window) {
		report("The icon_window component of the XWMHints structure was %lu instead of %lu.", hints_ret[4], hints.icon_window);
		FAIL;
	} else
		CHECK;

	if(hints_ret[5] != hints.icon_x) {
		report("The icon_x component of the XWMHints structure was %ld instead of %d.", hints_ret[5], hints.icon_x);
		FAIL;
	} else
		CHECK;

	if(hints_ret[6] != hints.icon_y) {
		report("The icon_y component of the XWMHints structure was %ld instead of %d.", hints_ret[6], hints.icon_y);
		FAIL;
	} else
		CHECK;

	if(hints_ret[7] != hints.icon_mask) {
		report("The icon_mask component of the XWMHints structure was %lu instead of %lu.", hints_ret[7], hints.icon_mask);
		FAIL;
	} else
		CHECK;

	if(hints_ret[8] != hints.window_group) {
		report("The window_group component of the XWMHints structure was %lu instead of %lu.", hints_ret[8], hints.window_group);
		FAIL;
	} else
		CHECK;

	XFree((char*)hints_ret);

	CHECKPASS(14);


>>ASSERTION Good A
When the
.A class_hints
argument is non-NULL, then a call to xname
sets the WM_CLASS property for the window
.A w
to the
.A class_hints
argument.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_CLASS property for the window using XSetWMProperites.
Obtain the value type and format  of the WM_CLASS property using XGetWindowProperty.
Verify that the format is 8.
Verify that the type is STRING.
Verify that the value is correct.
>>CODE
Window		win;
XVisualInfo	*vp;
XClassHint	classhint, retchint;
int		reslen;
char		*propp = NULL, *s;
unsigned long	leftover, nitems, len;
int		actual_format;
Atom		actual_type;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	classhint.res_name = "XTestResName";
	classhint.res_class = "XTestClassName";

	reslen = strlen(classhint.res_name);
	len = reslen + 1 + strlen(classhint.res_class) + 1;

	w = win;
	class_hints = &classhint;
	XCALL;

	if (XGetWindowProperty(display, win, XA_WM_CLASS, 0L, len, False,
	      AnyPropertyType, &actual_type, &actual_format, &nitems, &leftover,
		  (unsigned char **)&propp) != Success) {
		delete("XGetWindowProperty() did not return Success.");
		return;
	} else
		CHECK;

	if(leftover != 0) {
		report("The leftover elements numbered %lu instead of 0", leftover);
		FAIL;
	} else
		CHECK;

	if(actual_format != 8) {
		report("The format of the WM_HINTS property was %lu instead of 8", actual_format);
		FAIL;
	} else
		CHECK;

	if(actual_type != XA_STRING) {
		report("The type of the WM_CLASS property was %lu instead of STRING (%lu).", actual_type, (unsigned long) XA_STRING);
		FAIL;
	} else
		CHECK;

	if(propp == NULL) {

		report("No value was set for the WM_CLASS property.");
		FAIL;		

	} else {

		if(strcmp(propp, classhint.res_name) != 0) {
			report("The res_name component of the XClassHint structure was \"%s\" instead of \"%s\"", propp, classhint.res_name);
			FAIL;
		} else
			CHECK;

		if(strcmp(s = propp+1+reslen, classhint.res_class) != 0) {
			report("The res_class component of the XClassHint structure was \"%s\" instead of \"%s\".", s, classhint.res_class);
			FAIL;
		} else
			CHECK;
		XFree(propp);
	}

	CHECKPASS(6);


>>ASSERTION Good A
When the
.M res_name
component of the
.S XClassHint
structure named by the
.A class_hints
argument is NULL and the RESOURCE_NAME environment variable
is set, then a call to xname sets the WM_CLASS property for
the window
.A w
after substituting the value of the RESOURCE_NAME environment variable
for the NULL
.M res_name
component.
>>STRATEGY
Fork a child process using tet_fork.
Execute the File "./Test1" using tet_exec with envp set to "RESOUCE_NAME=XTest_res_name"
In child:
  Verify the environment variable RESOURCE_NAME is defined in the environment using getenv.
  Set the WM_CLASS property using XSetWMProperties with the class_hints structure having NULL as the res_name component.
  Obtain the value of the WM_CLASS property using XGetClassHint.
  Verify that the value is "XTest_res_name".
>>CODE

	tet_fork(t007exec, TET_NULLFP, 0, 0xFF);

>>EXTERN

extern char **environ;

static void
t007exec()
{
char	*argv[4];
char	*envp;

	argv[0] = "Test1";
	argv[1] = NULL;

	envp = "RESOURCE_NAME=XTest_res_name";
	if (xtest_putenv( envp )) {
		delete("xtest_putenv failed");
		return;
	}

	tet_exec("./Test1", argv, environ);
	delete("tet_exec() of \"./Test1\" failed.");
}
>>ASSERTION Bad Good A
When the
.M res_name
component of the
.S XClassHint
structure named by the
.A class_hints
argument is NULL and the RESOURCE_NAME environment variable
is not set and
.A argv
and
.A argv[0]
are non-NULL, then a call to xname sets the WM_CLASS property for the window
.A w
after substituting
.A argv[0] ,
stripped of any directory prefixes, for the NULL
.M res_name
component. 
>>STRATEGY
Fork a child process using tet_fork.
Execute the File "./Test2" using tet_exec with environ without RESOURCE_NAME.
In child:
  Verify the environment variable RESOURCE_NAME is not defined in the environment using getenv..
  Set the WM_CLASS property using XSetWMProperties with the class_hints structure having NULL as the res_name component.
  Obtain the value of the WM_CLASS property using XGetClassHint.
  Verify that the value is "Test2".
>>CODE

	tet_fork(t008exec, TET_NULLFP, 0, 0xFF);

>>EXTERN
static void
t008exec()
{
char	*argv[4];

	argv[0] = "./Test2";
	argv[1] = NULL;
	if (getenv("RESOURCE_NAME") != (char *)NULL) {
		char **newenv = environ; /* Remove RESOURCE_NAME */

		trace("Removing RESOURCE_NAME from the environment");
		
		while( strncmp("RESOURCE_NAME=", *newenv, 14)
			&& *newenv != NULL )
			newenv++;

		if (*newenv == NULL) {
			report("could not remove RESOURCE_NAME from the environment");
			UNRESOLVED;
			return;
		}

		do {
			*newenv = *(newenv+1);
			newenv++;
		} while( *newenv != NULL) ;
	}

	tet_exec("./Test2", argv, environ);
	delete("tet_exec() of \"./Test2\" failed.");

}
>>ASSERTION Bad A
.ER BadAlloc 
>>ASSERTION Bad A
.ER BadWindow 
>># Kieron	Completed		Review
