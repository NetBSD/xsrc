/* $Xorg: globals.h,v 1.3 2000/08/17 19:44:29 cpqbld Exp $ */

/*

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

*/
/* $XFree86: xc/lib/SM/globals.h,v 1.3 2001/01/17 19:41:31 dawes Exp $ */

/*
 * Author: Ralph Mor, X Consortium
 */

extern void _SmcDefaultErrorHandler ();
extern void _SmsDefaultErrorHandler ();

extern IcePoAuthStatus _IcePoMagicCookie1Proc ();
extern IcePaAuthStatus _IcePaMagicCookie1Proc ();

extern void _SmcProcessMessage ();
extern void _SmsProcessMessage ();

int 	_SmcOpcode = 0;
int 	_SmsOpcode = 0;

int	_SmVersionCount = 1;

IcePoVersionRec	_SmcVersions[] = {
	  	    {SmProtoMajor, SmProtoMinor, _SmcProcessMessage}};

IcePaVersionRec _SmsVersions[] = {
	  	    {SmProtoMajor, SmProtoMinor, _SmsProcessMessage}};

int		_SmAuthCount = 1;
char		*_SmAuthNames[] = {"MIT-MAGIC-COOKIE-1"};
IcePoAuthProc 	_SmcAuthProcs[] = {_IcePoMagicCookie1Proc};
IcePaAuthProc 	_SmsAuthProcs[] = {_IcePaMagicCookie1Proc};

#ifndef __EMX__
SmsNewClientProc _SmsNewClientProc;
SmPointer        _SmsNewClientData;
#else
SmsNewClientProc _SmsNewClientProc = 0;
SmPointer        _SmsNewClientData = 0;
#endif

SmcErrorHandler _SmcErrorHandler = _SmcDefaultErrorHandler;
SmsErrorHandler _SmsErrorHandler = _SmsDefaultErrorHandler;
