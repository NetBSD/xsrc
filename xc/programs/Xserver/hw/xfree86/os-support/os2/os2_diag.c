/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/os2/os2_diag.c,v 3.1.2.3 1997/07/19 07:00:09 dawes Exp $ */
/*
 * (c) Copyright 1997 by Holger Veit
 *			<Holger.Veit@gmd.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * HOLGER VEIT  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Holger Veit shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Holger Veit.
 *
 */
/* $XConsortium$ */

/* This file checks whether the user has installed the system correctly, 
 * to avoid the numerous questions why this or that does not work
 */

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "scrnintstr.h"

#include "compiler.h"

#define I_NEED_OS2_H
#define INCL_DOSFILEMGR
#define INCL_KBD
#define INCL_VIO
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#include "xf86.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

static BOOL diag_checks = FALSE;

/* from Eberhard to check for the right EMX version */
static void check_emx (void)
{
	ULONG rc;
	HMODULE hmod;
	char name[CCHMAXPATH];
	char fail[9];

	if (_emx_rev < 50) {
		ErrorF("xf86-OS/2: This program requires emx.dll revision 50 (0.9c fix 2) "
			"or later.\n");
		rc = DosLoadModule (fail, sizeof (fail), "emx", &hmod);
		if (rc == 0) {
			rc = DosQueryModuleName (hmod, sizeof (name), name);
			if (rc == 0)
				ErrorF("Please delete or update `%s'.\n", name);
			DosFreeModule (hmod);
		}
		exit (2);
        }
}

static void check_bsl(const char *var)
{
	char *t1 = strrchr(var,'\\');
	if (strchr(var,'/')) {
		ErrorF("xf86-OS/2: \"%s\" must exclusively use backward slashes \"\\\"\n",
			var);
	}
	if (t1 && *(t1+1)=='\0') {
		ErrorF("xf86-OS/2: \"%s\" mustn't end with \"\\\"\n",var);
		*t1 = '\0';
	}
}


static void check_fsl(const char *var)
{
	char *t1 = strrchr(var,'/');
	if (strchr(var,'\\')) {
		ErrorF("xf86-OS/2: \"%s\" must exclusively use forward slashes \"/\"\n",
			var);
	}
}


static void check_long(const char* path)
{
	FILE *f;
	char n[300];

	sprintf(n,"%s\\xf86_test_for_very_long_filename",path);
	f = fopen(n,"w");
	if (f==NULL) {
	ErrorF("xf86-OS/2: \"%s\" does not accept long filenames\nmust reside on HPFS or similar\n",
		path);
	} else {
		fclose(f);
		unlink(n);
	}
}

char *check_env_present(const char *env)
{
	char *e = getenv(env);
	if (!e) {
		ErrorF("xf86-OS/2: You have no \"%s\" environment variable, but need one\n",
			env);
		return 0;
	}
	return e;
}

void os2_checkinstallation(void)
{
	char *emxopt, *tmp, *home, *logname, *termcap;
	char hostname[256], *display, *hostvar, *s, *h;
	struct hostent *hent;
	struct in_addr *in;
	int i;

	if (diag_checks) return;
	diag_checks = TRUE;

	/* test whether the EMX version is okay */
	check_emx();

	/* Check a number of environment variables */
	emxopt = getenv("EMXOPT");
	if (emxopt) {
		for (i=0; i<strlen(emxopt); i++) {
			if (emxopt[i]=='-') {
				switch (emxopt[++i]) {
				case 't':
					ErrorF("xf86-OS/2: Remove -t option from EMXOPT variable!\n");
					break;
				case 'r':
					ErrorF("xf86-OS/2: Remove -r option from EMXOPT variable!\n");
				}
			}
		}
	}

	tmp = check_env_present("TMP");
	if (tmp) {
	        check_bsl(tmp);
	        check_long(tmp);
	}

	home = check_env_present("HOME");
	if (home) {
	        check_bsl(home);
	        check_long(home);
	}

	logname = check_env_present("LOGNAME");
	termcap = check_env_present("TERMCAP");
	if (termcap)
	        check_fsl(termcap);

	if (gethostname(hostname,sizeof(hostname)) != 0) {
		ErrorF("xf86-OS/2: gethostname() failed: Check TCP/IP setup!\n");
	} else {
		ErrorF("xf86-OS/2: gethostname() returns: \"%s\"\n",hostname);
	}

	display = check_env_present("DISPLAY");
	if (display)
		ErrorF("xf86-OS/2: DISPLAY to listen is set to: \"%s\"\n",
			display);

	hostvar = check_env_present("HOSTNAME");
	
	strcpy(hostname,display);
	h = strchr(hostname,':');
	if (!h)
		ErrorF("xf86-OS/2: Invalid DISPLAY name: expected something like XXX:0.0\n");
	else
		*h = 0;
	h = strchr(hostname,'/');
	if (h) 
		h++;
	else
		h = hostname;

	if (stricmp(h,hostvar)) {
		ErrorF("xf86-OS/2: HOSTNAME does not match DISPLAY: Do you really mean this?\n");
		ErrorF("xf86-OS/2:   This means that xinit/startx and client access may not work\n");
		ErrorF("xf86-OS/2:   which is intentional usually only when connection to a XDM server\n");
	}

	hent = gethostbyname(h);
	if (!hent) 
		ErrorF("xf86-OS/2: gethostbyname() failed: Check TCP/IP setup\n");
	else {
		ErrorF("xf86-OS/2: gethostbyname() returns the following data:\n");
		ErrorF("xf86-OS/2:    official host name: \"%s\"\n",hent->h_name);
		while ((s= *(hent->h_aliases)) != NULL) {
			ErrorF("xf86-OS/2:                 alias: \"%s\"\n",s);
			hent->h_aliases++;
		}
		ErrorF("xf86-OS/2:    addr type = %d,  addr length = %d\n",
			hent->h_addrtype, hent->h_length);
		if (hent->h_addrtype == AF_INET) {
			while ((in= (struct in_addr*)*(hent->h_addr_list++)) != NULL) {
				ErrorF("xf86-OS/2:      Internet address: \"%s\"\n",
					inet_ntoa(*in));
			}
		} else {
			ErrorF("xf86-OS/2: addr type should be %d: Check network setup and install TCP/IP support correctly\n",
				AF_INET);
		}
	}
}
