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
 * $XConsortium: opndsply.m,v 1.17 94/04/17 21:02:37 rws Exp $
 */
>>TITLE XOpenDisplay CH02
Display *
XOpenDisplay(display_name)
char	*display_name = config.display;
>>MAKE
>>#
>>#
>># Plant some rules in the Makefile to construct
>># stand-alone executable Test1 to allow the setting
>># of environment variables.
>>#
>># Cal 5/8/91
>>#
#
# The following lines are copied from the .m file by mc
# under control of the >>MAKE directive
# to create rules for the executable file Test1.
#
AUXFILES=Test1
AUXCLEAN=Test1.o Test1

all: Test

Test1 : Test1.o $(LIBS) $(TCMCHILD)
	$(CC) $(LDFLAGS) -o $@ Test1.o $(TCMCHILD) $(LIBLOCAL) $(LIBS) $(SYSLIBS)

#
# End of section copied from the .m file.
#
>>EXTERN
#include	"ctype.h"
#include	"Xatom.h"
static char *copystring (src, len) /* Courtesy of Xlib. */
    char *src;
    int len;
{
    char *dst = (char *) malloc (len + 1);

    if (dst) {
	strncpy (dst, src, len);
	dst[len] = '\0';
    }

    return dst;
}

>>ASSERTION Good C
If the system is POSIX compliant and supports DECnet:
When the
.A display_name
argument is a string of the form
.A hostname::number.screen_num ,
then a call to xname returns a pointer to a 
.S display
structure and opens a connection to
display server number
.A number
on
.A hostname
with default screen
.A screen_number .
>>STRATEGY
If the system is POSIX compliant and supports DECnet transport:
  Obtain the display and screen numbers by parsing the XT_DISPLAY config variable.
  Open a connection using xname.
  Verify that the call did not return NULL.
  Issue a NoOperation request using XNoOp.
  Flush the Output buffer using XFlush.
  Close the display using XCloseDisplay.
>>CODE
Display	*display;
char	*cptr, *sptr, *dnop, *snop;
int	dno;
int	sno;
char	ssno[9], sdno[9];

	if(config.posix_system == 0) {
		unsupported("This assertion can only be tested on a POSIX system.");
		return;
	} else
		CHECK;

	if(config.decnet == 0) {
		unsupported("This assertion can only be tested on a system with support for DECnet.");
		return;
	} else
		CHECK;

	for(cptr = config.display; (*cptr) && (*cptr != ':'); cptr++)
		;

	if(*cptr == '\0') {
		delete("XT_DISPLAY does not specify a display.");
		return;
	} else {
		CHECK;
		if(*++cptr != ':') {
			delete("XT_DISPLAY does not contain a valid display name.");
			return;
		} else
			CHECK;

		for(sptr = ++cptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
			;
		if( (sptr == cptr) || (*sptr != '\0' && *sptr != '.') ) {
			delete("Bad display number in XT_DISPLAY.");
			return;
		} else
			CHECK;

		if( (dnop = copystring(cptr, sptr - cptr)) == (char *) 0) {
			delete("malloc failed,");
			return;
		} else
			CHECK;

		dno = atoi(dnop);

		if(*sptr) {
			for(cptr = ++sptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
				;
			if(*sptr) {
				delete("Bad screen number in XT_DISPLAY.");
				return;
			} else
				CHECK;

			if((snop = copystring(cptr, sptr - cptr)) == (char *) NULL) {
				delete("malloc failed.");
				return;
			} else
				CHECK;

			sno = atoi(snop);
			free(snop);

		} else {
			CHECK; CHECK;
			sno = 0; /* No screen number in XT_DISPLAY, assume zero. */
		}
	}

	sprintf(ssno, "%d", sno);
	sprintf(sdno, "%d", dno);

	if((display_name = (char *) malloc(1 + 3 + strlen(ssno) + strlen(sdno) + strlen(config.displayhost))) == (char *) 0) {
		delete("malloc failed.");
		return;
	} else
		CHECK;

	sprintf(display_name, "%s::%s.%s", config.displayhost, sdno, ssno);

	display = XOpenDisplay(display_name);
	if(display == (Display *) NULL) {
		report("%s() returned NULL with argument \"%s\".", TestName, display_name);
		FAIL;
	} else {
		CHECK;
		XNoOp(display);
		XFlush(display);
		XCloseDisplay(display);
	}

	free(display_name);
	CHECKPASS(10);

>>ASSERTION Good C
If the system is POSIX compliant and supports DECnet:
When the
.A display_name
argument is a string of the form
.A hostname::number ,
then a call to xname returns a pointer to a
.S display
structure and opens a connection to
display server number
.A number
on
.A hostname
with default screen
.A 0 .
>>STRATEGY
If the system is POSIX compliant and supports DECnet transport:
  Obtain the display numbers by parsing the XT_DISPLAY config variable.
  Open a connection of the form host::number using xname.
  Verify that the call did not return NULL.
  Obtain the screen number using XDefaultScreen.
  Verify that the screen number is 0.
  Close the display using XCloseDisplay.
>>CODE
Display	*display;
char	*cptr, *sptr, *dnop, *snop;
int	rsno;
int	dno;
char	sdno[9];

	if(config.posix_system == 0) {
		unsupported("This assertion can only be tested on a POSIX system.");
		return;
	} else
		CHECK;

	if(config.decnet == 0) {
		unsupported("This assertion can only be tested on a system with support for DECnet.");
		return;
	} else
		CHECK;

	for(cptr = config.display; (*cptr) && (*cptr != ':'); cptr++)
		;

	if(*cptr == '\0') {
		delete("XT_DISPLAY does not specify a display.");
		return;
	} else {
		CHECK;
		if(*++cptr != ':') {
			delete("XT_DISPLAY does not contain a valid display name.");
			return;
		} else
			CHECK;

		for(sptr = ++cptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
			;
		if( (sptr == cptr) || (*sptr != '\0' && *sptr != '.') ) {
			delete("Bad display number in XT_DISPLAY.");
			return;
		} else
			CHECK;

		if( (dnop = copystring(cptr, sptr - cptr)) == (char *) 0) {
			delete("malloc failed,");
			return;
		} else
			CHECK;

		dno = atoi(dnop);
		free(dnop);

		if(*sptr) {
			for(cptr = ++sptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
				;
			if(*sptr) {
				delete("Bad screen number in XT_DISPLAY.");
				return;
			} else
				CHECK;

			if((snop = copystring(cptr, sptr - cptr)) == (char *) NULL) {
				delete("malloc failed.");
				return;
			} else
				CHECK;

		} else {
			CHECK; CHECK;
		}
	}

	sprintf(sdno, "%d", dno);

	if((display_name = (char *) malloc(1 + 2 + strlen(sdno) + strlen(config.displayhost))) == (char *) 0) {
		delete("malloc failed.");
		return;
	} else
		CHECK;

	sprintf(display_name, "%s::%s", config.displayhost, sdno);
	free(snop);
	free(dnop);

	display = XOpenDisplay(display_name);
	if(display == (Display *) NULL) {
		report("%s() returned NULL with argument \"%s\".", TestName, display_name);
		FAIL;
	} else {
		CHECK;
		if( (rsno=XDefaultScreen(display)) != 0 ) {
			report("%s() with argument \"%s\" opened screen %d instead of screen 0.", TestName, display_name, rsno);
			FAIL;
		} else
			CHECK;

		XNoOp(display);
		XFlush(display);
		XCloseDisplay(display);
	}

	free(display_name);
	CHECKPASS(11);

>>ASSERTION Good C
If the system is POSIX compliant and supports TCP:
When the
.A display_name
argument is a string of the form
.A hostname:number.screen_num ,
then a call to xname returns a pointer to a
.S display
structure and opens a connection to
display server number
.A number
on
.A hostname
with default screen
.A screen_number .
>>STRATEGY
If the system is POSIX compliant and supports TCP transport:
  Obtain the display and screen numbers by parsing the XT_DISPLAY config variable.
  Open a connection using xname.
  Verify that the call did not return NULL.
  Issue a NoOperation request using XNoOp.
  Flush the Output buffer using XFlush.
  Close the display using XCloseDisplay.
>>CODE
Display	*display;
char	*cptr, *sptr, *dnop, *snop;
int	dno;
int	sno;
char	ssno[9], sdno[9];

	if(config.posix_system == 0) {
		unsupported("This assertion can only be tested on a POSIX system.");
		return;
	} else
		CHECK;

	if(config.tcp == 0) {
		unsupported("This assertion can only be tested on a system with support for TCP.");
		return;
	} else
		CHECK;

	for(cptr = config.display; (*cptr) && (*cptr != ':'); cptr++)
		;

	if(*cptr == '\0') {
		delete("XT_DISPLAY does not specify a display.");
		return;
	} else {
		CHECK;
		for(sptr = ++cptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
			;
		if( (sptr == cptr) || (*sptr != '\0' && *sptr != '.') ) {
			delete("Bad display number in XT_DISPLAY.");
			return;
		} else
			CHECK;

		if( (dnop = copystring(cptr, sptr - cptr)) == (char *) 0) {
			delete("malloc failed,");
			return;
		} else
			CHECK;

		dno = atoi(dnop);
		free(dnop);

		if(*sptr) {
			for(cptr = ++sptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
				;
			if(*sptr) {
				delete("Bad screen number in XT_DISPLAY.");
				return;
			} else
				CHECK;

			if((snop = copystring(cptr, sptr - cptr)) == (char *) NULL) {
				delete("malloc failed.");
				return;
			} else
				CHECK;

			sno = atoi(snop);
			free(snop);

		} else {
			CHECK; CHECK;
			sno = 0; /* No screen number in XT_DISPLAY, assume zero. */
		}
	}

	sprintf(ssno, "%d", sno);
	sprintf(sdno, "%d", dno);

	if((display_name = (char *) malloc(1 + 2 + strlen(ssno) + strlen(sdno) + strlen(config.displayhost))) == (char *) 0) {
		delete("malloc failed.");
		return;
	} else
		CHECK;

	sprintf(display_name, "%s:%s.%s", config.displayhost, sdno, ssno);

	display = XOpenDisplay(display_name);
	if(display == (Display *) NULL) {
		report("%s() returned NULL with argument \"%s\".", TestName, display_name);
		FAIL;
	} else {
		CHECK;
		XNoOp(display);
		XFlush(display);
		XCloseDisplay(display);
	}

	free(display_name);
	CHECKPASS(9);

>>ASSERTION Good C
If the system is POSIX compliant and supports TCP:
When the
.A display_name
argument is a string of the form
.A hostname:number ,
then a call to xname returns a pointer to a
.S display
structure and opens a connection to
display server number
.A number
on
.A hostname
with default screen
.A 0 .
>>STRATEGY
If the system is POSIX compliant and supports TCP transport:
  Obtain the display numbers by parsing the XT_DISPLAY config variable.
  Open a connection of the form host:number using xname.
  Verify that the call did not return NULL.
  Obtain the screen number using XDefaultScreen.
  Verify that the screen number is 0.
  Close the display using XCloseDisplay.
>>CODE
Display	*display;
char	*cptr, *sptr, *dnop, *snop;
int	rsno;
int	dno;
char	sdno[9];

	if(config.posix_system == 0) {
		unsupported("This assertion can only be tested on a POSIX system.");
		return;
	} else
		CHECK;

	if(config.tcp == 0) {
		unsupported("This assertion can only be tested on a system with support for TCP.");
		return;
	} else
		CHECK;

	for(cptr = config.display; (*cptr) && (*cptr != ':'); cptr++)
		;

	if(*cptr == '\0') {
		delete("XT_DISPLAY does not specify a display.");
		return;
	} else {
		CHECK;

		for(sptr = ++cptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
			;
		if( (sptr == cptr) || (*sptr != '\0' && *sptr != '.') ) {
			delete("Bad display number in XT_DISPLAY.");
			return;
		} else
			CHECK;

		if( (dnop = copystring(cptr, sptr - cptr)) == (char *) 0) {
			delete("malloc failed,");
			return;
		} else
			CHECK;

		dno = atoi(dnop);
		free(dnop);

		if(*sptr) {
			for(cptr = ++sptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
				;
			if(*sptr) {
				delete("Bad screen number in XT_DISPLAY.");
				return;
			} else
				CHECK;

			if((snop = copystring(cptr, sptr - cptr)) == (char *) NULL) {
				delete("malloc failed.");
				return;
			} else
				CHECK;

			free(snop);

		} else {
			CHECK; CHECK;
		}
	}

	sprintf(sdno, "%d", dno);

	if((display_name = (char *) malloc(1 + 1 + strlen(sdno) + strlen(config.displayhost))) == (char *) 0) {
		delete("malloc failed.");
		return;
	} else
		CHECK;

	sprintf(display_name, "%s:%s", config.displayhost, sdno);

	display = XOpenDisplay(display_name);
	if(display == (Display *) NULL) {
		report("%s() returned NULL with argument \"%s\".", TestName, display_name);
		FAIL;
	} else {
		CHECK;
		if( (rsno=XDefaultScreen(display)) != 0 ) {
			report("%s() with argument \"%s\" opened screen %d instead of screen 0.", TestName, display_name, rsno);
			FAIL;
		} else
			CHECK;

		XNoOp(display);
		XFlush(display);
		XCloseDisplay(display);
	}

	free(display_name);
	CHECKPASS(10);

>>ASSERTION Good C
If the system is POSIX compliant:
When the
.A display_name
argument is NULL, then a call to xname opens a connection specified by 
the value of the DISPLAY environment variable.
>>STRATEGY
Fork a child process using tet_fork.
In child:
  Exec the file "./Test1" with the environment variable DISPLAY set to the value of XT_DISPLAY config variable.
  Open the display NULL using xname.
  Obtain the actual display string used using XDisplayString.
  Obtain the value of the DISPLAY environment variable using getvar.
  Verify that these values are the same.
>>CODE

	if(config.posix_system == 0) {
		unsupported("This assertion can only be tested on a POSIX system.");
	} else
		(void) tet_fork(t005exec, TET_NULLFP, 0 , ~0);

>>EXTERN
extern char **environ;

static void
t005exec()
{
char	*argv[2];
char	*str;
char	*mstr = "DISPLAY=%s";

	if((str = (char *) malloc( strlen(config.display) + strlen(mstr) - 1)) == (char *) NULL) {
		delete("malloc() failed.");
		return;
	}

	sprintf(str, mstr, config.display);

	argv[0] = "./Test1";
	argv[1] = (char *) NULL;

	if (xtest_putenv(str)) {
		delete("xtest_putenv failed");
		return;
	}

	(void) tet_exec("./Test1", argv, environ);

	delete("Exec of file ./Test1 failed");
	free( (char *) str);
}

>>ASSERTION Good D 1
If the system is POSIX compliant and supports DECnet and a local display server:
When the
.A display_name
argument is a string of the form
.A ::number.screen_num
or
.A ::number ,
then a call to xname opens the most efficient transport connection
available to the specified display server and default screen on the client machine.
>>STRATEGY
If the system is POSIX compliant and supports DECnet and a local display server:
  Obtain the display and screen numbers from XT_DISPLAY.
  Open a connection of the form ::number.display.
  Verify that the call did not return NULL.
  Issue a NoOperation request using XNoOp.
  Flush the Output buffer using XFlush.
  Close the display using XCloseDisplay.
  Open a connection of the form ::number .
  Verify that the call did not return NULL.
  Issue a NoOperation request using XNoOp.
  Flush the Output buffer using XFlush.
  Close the display using XCloseDisplay.
>>CODE
Display	*display;
char	*cptr, *sptr, *dnop, *snop;
int	dno;
int	sno;
char	ssno[9], sdno[9];

	if(config.posix_system == 0) {
		unsupported("This assertion can only be tested on a POSIX system.");
		return;
	} else
		CHECK;

	if(config.decnet == 0) {
		unsupported("This assertion can only be tested on a system with support for DECnet.");
		return;
	} else
		CHECK;

	if(config.local == 0) {
		unsupported("This assertion can only be tested on a system with a local display server.");
		return;
	} else
		CHECK;

	for(cptr = config.display; (*cptr) && (*cptr != ':'); cptr++)
		;

	if(*cptr == '\0') {
		delete("XT_DISPLAY does not specify a display.");
		return;
	} else {
		CHECK;
		if(*++cptr != ':') {
			delete("XT_DISPLAY does not contain a valid display name.");
			return;
		} else
			CHECK;

		for(sptr = ++cptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
			;
		if( (sptr == cptr) || (*sptr != '\0' && *sptr != '.') ) {
			delete("Bad display number in XT_DISPLAY.");
			return;
		} else
			CHECK;

		if( (dnop = copystring(cptr, sptr - cptr)) == (char *) 0) {
			delete("malloc failed,");
			return;
		} else
			CHECK;

		dno = atoi(dnop);
		free(dnop);

		if(*sptr) {
			for(cptr = ++sptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
				;
			if(*sptr) {
				delete("Bad screen number in XT_DISPLAY.");
				return;
			} else
				CHECK;

			if((snop = copystring(cptr, sptr - cptr)) == (char *) NULL) {
				delete("malloc failed.");
				return;
			} else
				CHECK;

			sno = atoi(snop);
			free(snop);

		} else {
			CHECK; CHECK;
			sno = 0; /* No screen number in XT_DISPLAY, assume zero. */
		}
	}

	sprintf(ssno, "%d", sno);
	sprintf(sdno, "%d", dno);

	if((display_name = (char *) malloc(1 + 3 + strlen(ssno) + strlen(sdno))) == (char *) 0) {
		delete("malloc failed.");
		return;
	} else
		CHECK;

	sprintf(display_name, "::%s.%s", sdno, ssno);

	display = XOpenDisplay(display_name);
	if(display == (Display *) NULL) {
		report("%s() returned NULL with argument \"%s\".", TestName, display_name);
		FAIL;
	} else {
		CHECK;
		XNoOp(display);
		XFlush(display);
		XCloseDisplay(display);
	}

	free(display_name);
/* added :: */

	if((display_name = (char *) malloc(1 + 2 + strlen(sdno))) == (char *) 0) {
		delete("malloc failed.");
		return;
	} else
		CHECK;

	sprintf(display_name, "::%s", sdno);

	display = XOpenDisplay(display_name);
	if(display == (Display *) NULL) {
		report("%s() returned NULL with argument \"%s\".", TestName, display_name);
		FAIL;
	} else {
		CHECK;
		XNoOp(display);
		XFlush(display);
		XCloseDisplay(display);
	}

	free(display_name);

	CHECKUNTESTED(13);


>>ASSERTION Good D 1
If the system is POSIX compliant and supports TCP and a local display server:
When the
.A display_name
argument is a string of the form
.A :number.screen_num
or
.A :number ,
then a call to xname opens the most efficient transport connection
available to the specified display server and default screen on the client machine.
>>STRATEGY
If the system is POSIX compliant and supports TCP and a local display server:
  Obtain the display and screen numbers from XT_DISPLAY.
  Open a connection of the form :number.display.
  Verify that the call did not return NULL.
  Issue a NoOperation request using XNoOp.
  Flush the Output buffer using XFlush.
  Close the display using XCloseDisplay.
  Open a connection of the form :number .
  Verify that the call did not return NULL.
  Issue a NoOperation request using XNoOp.
  Flush the Output buffer using XFlush.
  Close the display using XCloseDisplay.
>>CODE
Display	*display;
char	*cptr, *sptr, *dnop, *snop;
int	dno;
int	sno;
char	ssno[9], sdno[9];

	if(config.posix_system == 0) {
		unsupported("This assertion can only be tested on a POSIX system.");
		return;
	} else
		CHECK;

	if(config.tcp == 0) {
		unsupported("This assertion can only be tested on a system with support for TCP.");
		return;
	} else
		CHECK;

	if(config.local == 0) {
		unsupported("This assertion can only be tested on a system with a local display server.");
		return;
	} else
		CHECK;

	for(cptr = config.display; (*cptr) && (*cptr != ':'); cptr++)
		;

	if(*cptr == '\0') {
		delete("XT_DISPLAY does not specify a display.");
		return;
	} else {
		CHECK;

		for(sptr = ++cptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
			;
		if( (sptr == cptr) || (*sptr != '\0' && *sptr != '.') ) {
			delete("Bad display number in XT_DISPLAY.");
			return;
		} else
			CHECK;

		if( (dnop = copystring(cptr, sptr - cptr)) == (char *) 0) {
			delete("malloc failed,");
			return;
		} else
			CHECK;

		dno = atoi(dnop);
		free(dnop);

		if(*sptr) {
			for(cptr = ++sptr; *sptr && isascii(*sptr) && isdigit(*sptr); sptr++)
				;
			if(*sptr) {
				delete("Bad screen number in XT_DISPLAY.");
				return;
			} else
				CHECK;

			if((snop = copystring(cptr, sptr - cptr)) == (char *) NULL) {
				delete("malloc failed.");
				return;
			} else
				CHECK;

			sno = atoi(snop);
			free(snop);

		} else {
			CHECK; CHECK;
			sno = 0; /* No screen number in XT_DISPLAY, assume zero. */
		}
	}

	sprintf(ssno, "%d", sno);
	sprintf(sdno, "%d", dno);

	if((display_name = (char *) malloc(1 + 2 + strlen(ssno) + strlen(sdno))) == (char *) 0) {
		delete("malloc failed.");
		return;
	} else
		CHECK;

	sprintf(display_name, ":%s.%s", sdno, ssno);

	display = XOpenDisplay(display_name);
	if(display == (Display *) NULL) {
		report("%s() returned NULL with argument \"%s\".", TestName, display_name);
		FAIL;
	} else {
		CHECK;
		XNoOp(display);
		XFlush(display);
		XCloseDisplay(display);
	}

	free(display_name);
/* added : */

	if((display_name = (char *) malloc(1 + 1 + strlen(sdno))) == (char *) 0) {
		delete("malloc failed.");
		return;
	} else
		CHECK;

	sprintf(display_name, ":%s", sdno);

	display = XOpenDisplay(display_name);
	if(display == (Display *) NULL) {
		report("%s() returned NULL with argument \"%s\".", TestName, display_name);
		FAIL;
	} else {
		CHECK;
		XNoOp(display);
		XFlush(display);
		XCloseDisplay(display);
	}

	free(display_name);

	CHECKUNTESTED(12);


>>ASSERTION Good A
When a call to xname is successful, then
all of the screens in the display can be used by the client.
>>STRATEGY
For each screen:
	Obtain the root window ID of the alternate screen.
	Obtain the window attributes of that window using XGetWindowAttributes.
	Verify that the call returned non zero.
>>CODE
int			scount;
int			i;
Window			aroot;
XWindowAttributes	atts;

	scount = ScreenCount(Dsp);

	for(i=0; i< scount; i++) {

		aroot = RootWindow(Dsp, i);
	
		if(XGetWindowAttributes(Dsp, aroot, &atts) == 0) {
			report("Unable to access the attributes of the root window of screen %d.", i);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(scount);

>>ASSERTION Good A
On a call to xname the RESOURCE_MANAGER property of the root window
of screen 0 is read and stored.
>>STRATEGY
Get the RESOURCE_MANAGER property of the root window on screen 0 
	using XGetWindowProperty.
Open a display using xname.
Verify that the call did not return NULL.
Obtain the value of the RESOURCE_MANAGER property using XResourceManagerString.
Verify that the value of the property is correct.
>>CODE
Window	root0;
unsigned char	*rmstr;
char	*rstr;
Display	*display;
Atom	type_ret;
int	format_ret;
unsigned long	nitems_ret;
unsigned long	bytes_after_ret;

	root0 = XRootWindow(Dsp, 0);

	if(XGetWindowProperty(Dsp, root0, XA_RESOURCE_MANAGER, 0L, -1L, False, 
			XA_STRING, &type_ret, &format_ret, &nitems_ret,
				&bytes_after_ret, &rmstr) != Success) {
		delete("XGetWindowProperty did not return Success");
		return;
	}
	if(type_ret != XA_STRING || format_ret != 8 ||
	  nitems_ret == 0 || rmstr == (unsigned char *)NULL) {
		trace("nitems was %ld", nitems_ret);
		trace("type was  %d", type_ret);
		trace("format was  %d", format_ret);
		rmstr = (unsigned char *)NULL;
	}
	trace("Config.display is %s", config.display);
	display = XOpenDisplay(config.display);
	
	if(display == (Display *) NULL) {
		report("%s() returned NULL with argument %s.", TestName, config.display);
		FAIL;
		return;
	} else 
		CHECK;

	rstr = XResourceManagerString(display);

	if(rmstr == (unsigned char *)NULL) {
		if(rstr != (char *)NULL && *rstr != '\0') {
			report("After calling %s", TestName);
			report("XResourceManagerString returned a value :");
			report("  \"%s\",", rstr);
			report("although the RESOURCE_MANAGER property of the");
			report("root window of screen zero was unset.");
			FAIL;
		} else
			CHECK;
	} else {
		if((rstr == (char *) NULL) || (strcmp(rstr, rmstr) != 0)) {
			report("After calling %s", TestName);
			report("XResourceManagerString returned a value :");
			report("  \"%s\",", rstr == (char *) NULL ? "<NULL_POINTER>" : rstr);
			report("instead of:");
			report("  \"%s\".", rmstr);
			FAIL;
		} else
			CHECK;
	}

	XCloseDisplay(display);

	CHECKPASS(2);	

>>ASSERTION Good A
When a call to xname is not successful, then it returns NULL.
>>STRATEGY
Open a display with hostname as the argument using xname.
Verify that the call returned NULL.
>>CODE
Display *display;

	display_name = config.displayhost;
	display = XCALL;

	if(display != (Display *) NULL) {
		report("%s() did not return NULL with argument \"%s\".", TestName, display_name);
		FAIL;
	} else
		PASS;		
