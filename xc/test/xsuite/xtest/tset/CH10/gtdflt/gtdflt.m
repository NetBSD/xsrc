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
 * $XConsortium: gtdflt.m,v 1.18 94/04/17 21:09:35 rws Exp $
 */
>>#
>># I have assumed POSIX here in all assertions which make reference to
>># environment variables.
>>#
>># Cal 16/07/91
>>#
>>TITLE XGetDefault CH10
char *
XGetDefault(display, program, option)
Display	*display = Dsp;
char	*program;
char	*option;
>>MAKE
AUXFILES=.Xdefaults Test3 Test4 Test5
AUXCLEAN=Test3 Test4 Test5 Test3.o Test4.o Test5.o .Xdefaults\
	.Xdefaults-$(XTESTHOST)

all: Test

.Xdefaults : Xdefaults HstXdefaults
	$(RM) .Xdefaults ./.Xdefaults-$(XTESTHOST)
	$(CP) Xdefaults .Xdefaults
	$(CP) HstXdefaults ./.Xdefaults-$(XTESTHOST)

Test3 : Test3.o $(LIBS) $(TCMCHILD)
	$(CC) $(LDFLAGS) -o $@ Test3.o $(TCMCHILD) $(LIBLOCAL) $(LIBS) $(SYSLIBS)

Test4 : Test4.o $(LIBS) $(TCMCHILD)
	$(CC) $(LDFLAGS) -o $@ Test4.o $(TCMCHILD) $(LIBLOCAL) $(LIBS) $(SYSLIBS)

Test5 : Test5.o $(LIBS) $(TCMCHILD)
	$(CC) $(LDFLAGS) -o $@ Test5.o $(TCMCHILD) $(LIBLOCAL) $(LIBS) $(SYSLIBS)

>># end of included makefile section
>>EXTERN
#include	"Xatom.h"
>>ASSERTION Good A
A call to xname reads and returns from the resource manager database the entry
specified by the
.A program
and
.A option
arguments.
>>STRATEGY
Set the RESOURCE_MANAGER property on the default root window of screen 0 to contain the string XT.OPT.VAL
Open a display using XOpenDisplay.
Obtain the value of the XT.OPT resource using XGetDefault.
Verify that the returned value is "OPT".
>>CODE
unsigned char 	*pval = (unsigned char *) "VAL*VAl\nXT.LEO:CAL\nXT.OPT:VAL\nXT.Bezoomny:Cal\n";
char		*valstr;
char		*res = (char *) NULL;
Display		*client;

	valstr = "VAL";

	XChangeProperty (Dsp, RootWindow(Dsp, 0), XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, pval, 1+strlen(pval));
	XSync(Dsp, False);

	display = opendisplay();
	program = "XT";
	option  = "OPT";
	res = XCALL;	

	if(res == (char *) NULL) {
		report("%s() returned NULL.", TestName);
		FAIL;
	} else {
		CHECK;

		if(strcmp(res, valstr) != 0) {
			report("%s() retured \"%s\" instead of \"%s\"", TestName, res, valstr);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2);

>>ASSERTION Good A
When the option specified by the
.A program
and
.A option
arguments does not exist in the resource manager database,
then a call to xname returns NULL.
>>STRATEGY
Set the RESOURCE_MANAGER property on the default root window to contain some valid strings not containing XT.Cal*
Open a display using XOpenDisplay.
Obtain the value of the XT.Cal resource using xname.
Verify that the call returned NULL.
>>CODE
unsigned char 	*pval = (unsigned char *) "XT.LEO:CAL\nXT.OPT:VAL\nXT.Bezoomny:Cal\n";
char		*nullstr = "<NULL>";
char		*res = (char *) NULL;
Display		*client;


	XChangeProperty (Dsp, RootWindow(Dsp, 0), XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, pval, 1+strlen(pval));
	XSync(Dsp, False);

	display = opendisplay();
	program = "XT";
	option  = "Cal";
	res = XCALL;	

	if(res != (char *) NULL) {
		report("%s() retured \"%s\" instead of \"%s\"", TestName, res, nullstr);
		FAIL;
	} else
		PASS;

>>ASSERTION Good C
If the system is POSIX compliant:
When the RESOURCE_MANAGER property was defined at the time
.A display
was opened, then the default resource manager database is taken from the value
of that property, otherwise the default resource manager database is taken from the
file $HOME/.Xdefaults in the client's home directory.
>>STRATEGY
Fork a new  process using tet_fork.
In the child process:
   Execute the file ./Test3 using tet_exec with the HOME environment variable set to ".".
   In Test3:
      Delete the RESOURCE_MANAGER property using XDeleteProperty.
      Open a new display using XOpenDisplay.
      Obtain the value of the XTest.testval32 resource using xname.
      Verify that the returned value is "VAL_b".
      Set the RESOURCE_MANAGER property using XChangeProperty.
      Open a new display using XOpenDisplay.
      Obtain the value of the XTest.testval32 resource using xname.
      Verify that the returned value is "pval_b".
>>CODE

	if(config.posix_system == 0)
		unsupported("This assertion can only be tested on a POSIX system.");
	else
		tet_fork(t003exec, TET_NULLFP, 0, ~0);

>>EXTERN
extern char **environ;

static void
t003exec()
{
char	*argv[2];
char	*envp;

	argv[0] = "./Test3";
	argv[1] = (char *) NULL;
	envp = "HOME=./";
	if (xtest_putenv( envp )) {
		delete("xtest_putenv failed to add HOME to the environment");
		return;
	}
	tet_exec("./Test3", argv, environ);
	delete("tet_exec() of \"./Test3\" failed.");
}
>>#
>># Altered this assertion - there is no way for Xlib to determine if the 
>># environment variable contains the full path name of a resource file,
>># as was implied - the onus is on the user to set it up properly.
>>#>>ASSERTION
>>#If the system is POSIX compliant:
>>#When the XENVIRONMENT environment variable is defined to be the 
>>#full path name of an existing resource file, 
>>#then this file is merged to update the default resource database.

>>ASSERTION Good C
If the system is POSIX compliant:
When the XENVIRONMENT environment variable is defined, then the
file named by the XENVIRONMENT environment variable is merged to 
update the default resource database.
>>STRATEGY
Fork a new process using tet_fork.
In the child process:
   Execute the file ./Test4 using tet_exec with the XENVIRONMENT variable set to ./EnvXdefaults.
   In Test4:
      Set the RESOURCE_MANAGER property using XChangeProperty.
      Open a display using XOpenDisplay.
      Obtain the value of resources set by the ChangeProperty request and by the XENVIRONMENT file using xname.
      Verify that the resources are set from each source.
      Verify that the XENVIRONMENT resources updated those of the ChangeProperty request.
>>CODE

	if(config.posix_system == 0)
		unsupported("This assertion can only be tested on a POSIX system.");
	else
		tet_fork(t004exec, TET_NULLFP, 0, ~0);

>>EXTERN
static void
t004exec()
{
char	*argv[2];
char	*envp;

	argv[0] = "./Test4";
	argv[1] = (char *) NULL;
	envp = "XENVIRONMENT=./EnvXdefaults";
	if (xtest_putenv(envp)) {
		delete("xtest_putenv failed to add XENVIRONMENT to the environment");
	}

	tet_exec("./Test4", argv, environ);
	delete("tet_exec() of \"./Test4\" failed.");
}


>>ASSERTION Good C
>># 
>># Altered wording of the assertion to explain what `hostname` is.
>># DPJ Cater 21/1/92.
>># 
If the system is POSIX compliant:
When the XENVIRONMENT environment variable is not defined, then the
file $HOME/.Xdefaults-<name> is merged to 
update the default resource database, where <name> specifies the name 
of the machine on which the application is running.
>>STRATEGY
Fork a new process using tet_fork.
In the child process:
   Remove XENVIRONMENT from the environment
   Execute the file ./Test5 using tet_exec with the HOME variable set to ".".
   In Test5:
      Set the RESOURCE_MANAGER property using XChangeProperty.
      Open a display using XOpenDisplay.
      Obtain the value of resources set by the ChangeProperty request and by the file $HOME/.Xdefaults-<name> using xname.
      Verify that the resources are set from each source.
      Verify that the .Xdefaults-<name> resources updated those of the ChangeProperty request.
>>CODE

	if(config.posix_system == 0)
		unsupported("This assertion can only be tested on a POSIX system.");
	else
		tet_fork(t005exec, TET_NULLFP, 0, ~0);

>>EXTERN
static void
t005exec()
{
char	*argv[2];
char	*envp;

	argv[0] = "./Test5";
	argv[1] = (char *) NULL;
	envp = "HOME=.";
	if (xtest_putenv(envp)) {
		delete("xtest_putenv failed to add HOME to the environment");
		return;
	}

	if (getenv("XENVIRONMENT") != (char *)NULL) {
		char **newenv = environ; /* Remove XENVIRONMENT */
	
		trace("Removing XENVIRONMENT from environment");

		while( strncmp("XENVIRONMENT=", *newenv, 13)
			&& *newenv != NULL )
			newenv++;

		if (*newenv == NULL) {
			report("could not remove XENVIRONMENT from the environment");
			UNRESOLVED;
			return;
		}

		do {
			*newenv = *(newenv+1);
			newenv++;
		} while ( *newenv != NULL );
	}
	tet_exec("./Test5", argv, environ);
	delete("tet_exec() of \"./Test5\" failed.");
}
