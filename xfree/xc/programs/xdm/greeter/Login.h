/* $TOG: Login.h /main/10 1998/02/11 10:00:41 kaleb $ */
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
/* $XFree86: xc/programs/xdm/greeter/Login.h,v 3.4 2000/06/14 00:16:15 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 */


#ifndef _XtLogin_h
#define _XtLogin_h

/***********************************************************************
 *
 * Login Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 border		     BorderColor	pixel		Black
 borderWidth	     BorderWidth	int		1
 foreground	     Foreground		Pixel		Black
 height		     Height		int		120
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 width		     Width		int		120
 x		     Position		int		0
 y		     Position		int		0

*/

# define XtNgreeting		"greeting"
# define XtNunsecureGreeting	"unsecureGreeting"
# define XtNnamePrompt		"namePrompt"
# define XtNpasswdPrompt	"passwdPrompt"
# define XtNfail		"fail"
# define XtNnotifyDone		"notifyDone"
# define XtNpromptColor		"promptColor"
# define XtNgreetColor		"greetColor"
# define XtNfailColor		"failColor"
# define XtNpromptFont		"promptFont"
# define XtNgreetFont		"greetFont"
# define XtNfailFont		"failFont"
# define XtNfailTimeout		"failTimeout"
# define XtNsessionArgument	"sessionArgument"
# define XtNsecureSession	"secureSession"
# define XtNallowAccess		"allowAccess"
# define XtNallowNullPasswd	"allowNullPasswd"
# define XtNallowRootLogin	"allowRootLogin"

#ifdef XPM
/* added by Amit Margalit Oct 1996 */
# define XtNhiColor		"hiColor"
# define XtNshdColor		"shdColor"
# define XtNframeWidth		"frameWidth"
# define XtNinnerFramesWidth	"innerFramesWidth"
# define XtNsepWidth		"sepWidth"

/* caolan begin */
#define XtNlastEventTime "lastEventTime"
#define XtCLastEventTime "LastEventTime"
/* caolan end */

#define XtNuseShape "useShape"
#define XtCUseShape "UseShape"
#define XtNlogoFileName "logoFileName"
#define XtCLogoFileName "LogoFileName"
#define XtNlogoPadding "logoPadding"
#define XtCLogoPadding "LogoPadding"

# define XtCFrameWidth		"FrameWidth"

#endif /* XPM */
# define XtCGreeting		"Greeting"
# define XtCNamePrompt		"NamePrompt"
# define XtCPasswdPrompt	"PasswdPrompt"
# define XtCFail		"Fail"
# define XtCFailTimeout		"FailTimeout"
# define XtCSessionArgument	"SessionArgument"
# define XtCSecureSession	"SecureSession"
# define XtCAllowAccess		"AllowAccess"
# define XtCAllowNullPasswd	"AllowNullPasswd"
# define XtCAllowRootLogin	"AllowRootLogin"

/* notifyDone interface definition */

#define NAME_LEN	32

typedef struct _LoginData { 
	char	name[NAME_LEN], passwd[NAME_LEN];
} LoginData;

# define NOTIFY_OK	0
# define NOTIFY_ABORT	1
# define NOTIFY_RESTART	2
# define NOTIFY_ABORT_DISPLAY	3

typedef struct _LoginRec *LoginWidget;  /* completely defined in LoginPrivate.h */
typedef struct _LoginClassRec *LoginWidgetClass;    /* completely defined in LoginPrivate.h */

extern WidgetClass loginWidgetClass;

#endif /* _XtLogin_h */
/* DON'T ADD STUFF AFTER THIS #endif */
