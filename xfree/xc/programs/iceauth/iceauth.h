/*
 * $TOG: iceauth.h /main/4 1998/02/09 13:44:06 kaleb $
 *
 * 
Copyright 1989, 1998  The Open Group

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
 * *
 * Author:  Jim Fulton, MIT X Consortium
 */

/* $XFree86: xc/programs/iceauth/iceauth.h,v 3.2 1999/02/19 21:27:10 hohndel Exp $ */

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICEutil.h>
#include <X11/Xfuncs.h>

#ifndef True
typedef int Bool;
#define False 0
#define True 1
#endif

extern char *ProgramName;

#ifdef X_NOT_STDC_ENV
extern char *malloc(), *realloc();
#else
#include <stdlib.h>
#endif

extern int auth_initialize ( char *authfilename );
extern int auth_finalize ( void );
extern int process_command ( char *inputfilename, int lineno, int argc, char **argv );
extern int print_help ( FILE *fp, char *cmd );

extern int verbose;
extern Bool ignore_locks;
extern Bool break_locks;
