/* $TOG: globals.h /main/15 1998/02/06 13:56:12 kaleb $ */
/******************************************************************************


Copyright 1993, 1998  The Open Group

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

Author: Ralph Mor, X Consortium
******************************************************************************/
/* $XFree86: xc/lib/ICE/globals.h,v 1.2 1998/12/20 11:56:49 dawes Exp $ */

extern void _IceDefaultErrorHandler ();
extern void _IceDefaultIOErrorHandler ();

extern IcePoAuthStatus _IcePoMagicCookie1Proc ();
extern IcePaAuthStatus _IcePaMagicCookie1Proc ();

extern void _IceProcessCoreMessage ();

#ifndef __EMX__
IceConn     	_IceConnectionObjs[256];
char	    	*_IceConnectionStrings[256];
#else
IceConn     	_IceConnectionObjs[256] = {0};
char	    	*_IceConnectionStrings[256] = {0};
#endif
int     	_IceConnectionCount = 0;

#ifndef __EMX__
_IceProtocol 	_IceProtocols[255];
#else
_IceProtocol 	_IceProtocols[255] = {0};
#endif
int         	_IceLastMajorOpcode = 0;

int		_IceAuthCount = 1;
char		*_IceAuthNames[] = {"MIT-MAGIC-COOKIE-1"};
IcePoAuthProc	_IcePoAuthProcs[] = {_IcePoMagicCookie1Proc};
IcePaAuthProc	_IcePaAuthProcs[] = {_IcePaMagicCookie1Proc};

int		_IceVersionCount = 1;
_IceVersion	_IceVersions[] = {
	  	    {IceProtoMajor, IceProtoMinor, _IceProcessCoreMessage}};

_IceWatchProc	*_IceWatchProcs = NULL;

IceErrorHandler   _IceErrorHandler   = _IceDefaultErrorHandler;
IceIOErrorHandler _IceIOErrorHandler = _IceDefaultIOErrorHandler;
