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
 * $XConsortium: dsplynm.m,v 1.8 94/04/17 21:07:26 rws Exp $
 */
>>TITLE XDisplayName CH08
char *
XDisplayName(string)
char *string;
>>MAKE
>>#
>>#
>># Plant some rules in the Makefile to construct
>># stand-alone executable Test1 to allow the setting
>># of environment variables.
>>#
>># Cal 22/6/91
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
>>ASSERTION Good B 1
A call to xname returns a pointer to the display name that
.F XOpenDisplay
would attempt to use when passed
.A string
as an argument.
>>STRATEGY
Fork a child process using tet_fork.
In child :
  Exec the file "./Test1" with the environment variable DISPLAY set to 
    the value of XT_DISPLAY config variable.
  In Test1:
    Call xname with string set to TestString:0.0
    Verify that the call returned a pointer to TestString:0.0
    Obtain the value of the DISPLAY environment variable.
    Call xname with string set to NULL.
    Verify that the DISPLAY variable value is identical to the string 
      returned by xname.
>>CODE

        if(config.posix_system == 0) {
                untested("This assertion can only be tested on a POSIX system.");
        } else
		tet_fork(t001exec, TET_NULLFP, 0, 0xFF);

>>EXTERN
extern char **environ;

static void
t001exec()
{
char	*argv[2];
char	*str;
char	*dstr;
char	*mstr = "DISPLAY=%s";
int	pass = 0, fail = 0;

	if((dstr = tet_getvar("XT_DISPLAY")) == (char *) NULL) {
		delete("XT_DISPLAY configuration variable is not defined.");
		return;
	}		

	if((str = (char *) malloc( strlen(dstr) + strlen(mstr) + 1)) == (char *) NULL) {
		delete("malloc() failed.");
		return;
	}

	sprintf(str, mstr, dstr);

	argv[0] = "./Test1";
	argv[1] = (char *) NULL;

	if (xtest_putenv( str ) ) {
		delete("xtest_putenv failed");
		return;
	}

	(void) tet_exec("./Test1", argv, environ);

	delete("Exec of file ./Test1 failed");
	free( (char *) str);
}
