/* $Xorg: error.c,v 1.3 2000/08/17 19:54:14 cpqbld Exp $ */
/*

Copyright 1988, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/error.c,v 1.3 2001/01/17 23:45:21 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * error.c
 *
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 */

# include <stdio.h>
# include <stdarg.h>

# include "dm.h"
# include "dm_error.h"

void LogInfo(char * fmt, ...)
{
    fprintf (stderr, "xdm info (pid %d): ", getpid());
    {
	va_list args;
	va_start(args, fmt);
	vfprintf (stderr, fmt, args);
	va_end(args);
    }
    fflush (stderr);
}

void LogError (
    char * fmt, ...)
{
    fprintf (stderr, "xdm error (pid %d): ", getpid());
    {
	va_list args;
	va_start(args, fmt);
	vfprintf (stderr, fmt, args);
	va_end(args);
    }
    fflush (stderr);
}

void LogPanic (char * fmt, ...)
{
    fprintf (stderr, "xdm panic (pid %d): ", getpid());
    {
	va_list args;
	va_start(args, fmt);
	vfprintf (stderr, fmt, args);
	va_end(args);
    }
    fflush (stderr);
    exit (1);
}

void LogOutOfMem (char * fmt, ...)
{
    fprintf (stderr, "xdm: out of memory in routine ");
    {
	va_list args;
	va_start(args, fmt);
	vfprintf (stderr, fmt, args);
	va_end(args);
    }
    fflush (stderr);
}

void Panic (char *mesg)
{
    int	i;

    i = creat ("/dev/console", 0666);
    write (i, "panic: ", 7);
    write (i, mesg, strlen (mesg));
    exit (1);
}


void Debug (char * fmt, ...)
{
    if (debugLevel > 0)
    {
	va_list args;
	va_start(args, fmt);
	vprintf (fmt, args);
	va_end(args);
	fflush (stdout);
    }
}

void InitErrorLog (void)
{
	int	i;
	if (errorLogFile[0]) {
		i = creat (errorLogFile, 0666);
		if (i != -1) {
			if (i != 2) {
				dup2 (i, 2);
				close (i);
			}
		} else
			LogError ("Cannot open errorLogFile %s\n", errorLogFile);
	}
}
