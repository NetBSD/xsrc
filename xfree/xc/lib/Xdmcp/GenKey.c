/*
 * $Xorg: GenKey.c,v 1.3 2000/08/17 19:45:48 cpqbld Exp $
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
 * Author:  Keith Packard, MIT X Consortium
 */

/* $XFree86: xc/lib/Xdmcp/GenKey.c,v 3.6 2001/01/17 19:42:43 dawes Exp $ */

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

static void
getbits (long data, unsigned char *dst)
{
    dst[0] = (data      ) & 0xff;
    dst[1] = (data >>  8) & 0xff;
    dst[2] = (data >> 16) & 0xff;
    dst[3] = (data >> 24) & 0xff;
}

/* EMX is not STDC, but sometimes it is */
#if defined(X_NOT_STDC_ENV) && !defined(__EMX__)
#define Time_t long
extern Time_t time ();
#else
#define Time_t time_t
#endif

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
long random();
#endif

#if defined(SYSV) || defined(SVR4)
#define srandom srand48
#define random lrand48
#endif

void
XdmcpGenerateKey (XdmAuthKeyPtr key)
{
    long    lowbits, highbits;

    srandom ((int)getpid() ^ time((Time_t *)0));
    lowbits = random ();
    highbits = random ();
    getbits (lowbits, key->data);
    getbits (highbits, key->data + 4);
}
