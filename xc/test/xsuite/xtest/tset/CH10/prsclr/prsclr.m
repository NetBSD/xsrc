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
 * $XConsortium: prsclr.m,v 1.8 94/04/17 21:09:56 rws Exp $
 */
>>TITLE XParseColor CH10
>>#
>># Note Do{Red,Green,Blue} are set in the XColor structure.
>>#
Status
XParseColor(display, colormap, spec, exact_def_return)
Display		*display = Dsp;
Colormap	colormap = DefaultColormap(Dsp, 0);
char		*spec = config.good_colorname;
XColor		*exact_def_return = &color_ret;
>>EXTERN

#include	<ctype.h>

XColor		color_ret;

Status
checkcolor( red, green, blue, shift, ret_desired)
unsigned int	red, green, blue;
unsigned int	shift;
XColor		*ret_desired;
{
	ret_desired->red = (unsigned short) red<<shift;
	ret_desired->green = (unsigned short) green<<shift;
	ret_desired->blue = (unsigned short) blue<<shift;
	return((ret_desired->red == color_ret.red) && (ret_desired->green == color_ret.green) && (ret_desired->blue == color_ret.blue));
}

rgb_report(desired)
XColor	*desired;
{
	report("%s() returned red 0x%x green 0x%x blue 0x%x instead of red 0x%x green 0x%x blue 0x%x.",  
		TestName,
		(int) color_ret.red, (int) color_ret.green, (int) color_ret.blue,
		(int) desired->red, (int) desired->green, (int) desired->blue);
}

>>ASSERTION Good B 1
When the first character of the
.A spec
argument is not "#", then a call to xname returns in the 
.A exact_def_return
argument
the rgb values for
the colour named by the 
.A spec
argument on the screen associated with the
.A colormap
argument 
and returns non-zero.
>>STRATEGY
Parse the color XT_GOOD_COLORNAME using xname.
Verify that the call returns non-zero.
>>CODE
Status result;

	result = XCALL;
	if(result == 0) {
		report("%s() returned zero.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);

>>ASSERTION Good A
When the first character of the
.A spec
argument is a "#", and the remainder of the string comprises 3 hexadecimal digits,
then a call to xname sets the
.A exact_def_return
argument to have
.M red
component equal to the value of the first digit << 12, to have
.M green
component equal to the value of the second digit << 12, to have
.M blue
component equal to the value of the third digit << 12 and returns non-zero.
>>STRATEGY
Parse the string "#18f" using xname.
Verify that the returned XColor structure had red 0x1 green 0x8 blue 0xf.
>>CODE
Status	ret;
XColor	dcol;

	spec = "#18f";
	ret = XCALL;
	if( ret == 0 ) {
		report("%s() with spec \"%s\" returned 0.", TestName, spec);
		FAIL;
	} else
		CHECK;

	if( checkcolor(0x1, 0x8, 0xf, 12, &dcol) == 0) {
		rgb_report(&dcol);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
When the first character of the
.A spec
argument is a "#", and the remainder of the string comprises 6 hexadecimal digits,
then a call to xname sets the
.A exact_def_return
argument to have
.M red
component equal to the value of the first two digits << 8, to have
.M green
component equal to the value of the second two digits << 8, to have
.M blue
component equal to the value of the third two digits << 8 and returns non-zero.
>>STRATEGY
Parse the string "#f1f8ff" using xname.
Verify that the returned XColor structure had red 0xf1 green 0xf8 blue 0xff.
>>CODE
Status	ret;
XColor	dcol;

	spec = "#f1f8ff";
	ret = XCALL;
	if( ret == 0 ) {
		report("%s() with spec \"%s\" returned 0.", TestName, spec);
		FAIL;
	} else
		CHECK;

	if( checkcolor(0xf1, 0xf8, 0xff, 8, &dcol) == 0) {
		rgb_report(&dcol);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
When the first character of the
.A spec
argument is a "#", and the remainder of the string comprises 9 hexadecimal digits,
then a call to xname sets the
.A exact_def_return
argument to have
.M red
component equal to the value of the first three digits << 4, to have
.M green
component equal to the value of the second three digits << 4, to have
.M blue
component equal to the value of the third three digits << 4 and returns non-zero.
>>STRATEGY
Parse the string "#af1bf8cff" using xname.
Verify that the returned XColor structure had red 0xaf1 green 0xbf8 blue 0xcff.
>>CODE
Status	ret;
XColor	dcol;

	spec = "#af1bf8cff";
	ret = XCALL;
	if( ret == 0 ) {
		report("%s() with spec \"%s\" returned 0.", TestName, spec);
		FAIL;
	} else
		CHECK;

	if( checkcolor(0xaf1, 0xbf8, 0xcff, 4, &dcol) == 0) {
		rgb_report(&dcol);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
When the first character of the
.A spec
argument is a "#", and the remainder of the string comprises 12 hexadecimal digits,
then a call to xname sets the
.A exact_def_return
argument to have
.M red
component equal to the value of the first four digits, to have
.M green
component equal to the value of the second four digits, to have
.M blue
component equal to the value of the third four digits and returns non-zero.
>>STRATEGY
Parse the string "#faf01bf81cff" using xname.
Verify that the returned XColor structure had red 0xfaf0 green 0x1bf8 blue 0x1cff.
>>CODE
Status	ret;
XColor	dcol;

	spec = "#faf01bf81cff";
	ret = XCALL;
	if( ret == 0 ) {
		report("%s() with spec \"%s\" returned 0.", TestName, spec);
		FAIL;
	} else
		CHECK;

	if( checkcolor(0xfaf0, 0x1bf8, 0x1cff, 0, &dcol) == 0) {
		rgb_report(&dcol);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
Upper and lower case characters in the
.A spec
argument refer to the same colour.
>>STRATEGY
Parse the string XT_GOOD_COLORNAME using xname.
Verify that the call did not return zero.
Parse the string obtained by inverting each character of XT_GOOD_COLORNAME using xname.
Verify that the call did not return zero.
Verify that the returned red, green and blue values are the same.
Parse the string #feAAc1 using xname.
Verify that the call did not return zero.
Parse the string #FEAaC1 using xname.
Verify that the call did not return zero.
Verify that the returned red, green and blue values are the same.
>>CODE
char		*ptr;
char		*str;
Status		result;
XColor		dcol;
unsigned short	red, green, blue;

	result = XCALL;
	if(result == 0) {
		report("%s() with spec \"%s\" returned zero.", TestName, config.good_colorname);
		FAIL;
	} else
		CHECK;

	red = color_ret.red;
	green = color_ret.green;
	blue = color_ret.blue;

	str = xt_strdup(config.good_colorname);

	if(str == (char *) NULL) {
		delete("xt_strdup() returned NULL.");
		return;
	} else
		CHECK;

	for(ptr = str; *ptr; ptr++)
		if(isupper(*ptr)) 
			*ptr = tolower(*ptr);
		else
			*ptr = toupper(*ptr);
	
	spec = str;
	result = XCALL;
	if(result == 0) {
		report("%s() with spec \"%s\" returned zero.", TestName, str);
		FAIL;
	} else
		CHECK;

	if((red != color_ret.red) || (green != color_ret.green) || (blue != color_ret.blue)) {
		report("%s() did not map color names %s and %s to the same rgb values.", TestName, config.good_colorname, str);
		FAIL;		
	} else
		CHECK;

	spec = "#feAAc1";
	result = XCALL;
	if( result == 0 ) {
		report("%s() with spec \"%s\" returned 0.", TestName, spec);
		FAIL;
	} else
		CHECK;

	if( checkcolor(0xfe, 0xaa, 0xc1, 8, &dcol) == 0) {
		rgb_report(&dcol);
		FAIL;
	} else
		CHECK;

	spec = "#FEAaC1";
	result = XCALL;
	if( result == 0 ) {
		report("%s() with spec \"%s\" returned 0.", TestName, spec);
		FAIL;
	} else
		CHECK;

	if( checkcolor(0xfe, 0xaa, 0xc1, 8, &dcol) == 0) {
		rgb_report(&dcol);
		FAIL;
	} else
		CHECK;

	CHECKPASS(8);


>>ASSERTION Good A
A call to xname sets the
.M flags
component of the
.A exact_def_return
argument to
.S "DoRed|DoGreen|DoBlue" .
>>STRATEGY
Parse the string "#000" using xname.
Verify that the flags component of the returned XColor structure was DoRed|DoGreen|DoBlue.
>>CODE
Status	ret;
char	flags;
XColor	dcol;

	spec = "#000";
	ret = XCALL;
	if( ret == 0 ) {
		report("%s() with spec \"%s\" returned 0.", TestName, spec);
		FAIL;
	} else
		CHECK;

	if(color_ret.flags != (flags = DoRed | DoGreen | DoBlue)) {
		report("%s() set the flags component of the retured XColor structure to %d instead of DoRed|DoGreen|DoBlue (%d).",
			TestName, (int) color_ret.flags, (int) flags);

		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
When the first character of the
.A spec
argument is a "#" 
and the remainder of the string does not comprise 
3, 6, 9 or 12 hexadecimal digits,
then a call to xname returns zero.
>>STRATEGY
Parse the string "#f0" using xname.
Verify that the call returns zero.
Parse the string "#1010" using xname.
Verify that the call returns zero.
Parse the string "##ffeeffeeffe" using xname.
Verify that the call returns zero.
Parse the string "##0011223344556" using xname.
Verify that the call returns zero.
>>CODE
Status ret;

	spec = "#f0";
	ret = XCALL;
	if( ret != 0 ) {
		report("%s() with spec \"%s\" did not return zero.", TestName, spec);
		FAIL;
	} else
		CHECK;

	spec = "#1010";
	ret = XCALL;
	if( ret != 0 ) {
		report("%s() with spec \"%s\" did not return zero.", TestName, spec);
		FAIL;
	} else
		CHECK;

	spec = "#ffeeffeeffe"; /* 11 digits */
	ret = XCALL;
	if( ret != 0 ) {
		report("%s() with spec \"%s\" did not return zero.", TestName, spec);
		FAIL;
	} else
		CHECK;

	spec = "#0011223344556"; /* 13 digits */
	ret = XCALL;
	if( ret != 0 ) {
		report("%s() with spec \"%s\" did not return zero.", TestName, spec);
		FAIL;
	} else
		CHECK;

	CHECKPASS(4);

>>ASSERTION Good A
When the first character of the
.A spec
argument is not a "#" 
and the colour named by the 
.A spec 
argument is not in the colour database,
then a call to xname returns zero.
>>STRATEGY
Parse the string XT_BAD_COLORNAME using xname.
Verify that the call returns zero.
>>CODE
char		*ptr;
char		*str;
Status		result;
XColor		dcol;
unsigned short	red, green, blue;

	spec = config.bad_colorname;
	result = XCALL;
	if(result != 0) {
		report("%s() with spec \"%s\" did not return zero.", TestName, config.bad_colorname);
		FAIL;
	} else
		PASS;

>>ASSERTION Bad A
.ER BadColor
