/* $TOG: info.h /main/2 1998/02/09 14:13:31 kaleb $ */
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
******************************************************************************/
/* $XFree86: xc/programs/xsm/info.h,v 3.2 1999/03/07 11:41:23 dawes Exp $ */

extern void ShowHint(ClientRec *client);
extern void DisplayProps(ClientRec *client);
extern char * GetProgramName(char *fullname);
extern void UpdateClientList(void);
extern void ClientInfoStructureNotifyXtHandler(Widget w, XtPointer closure, 
					       XEvent *event, 
					       Boolean *continue_to_dispatch);
extern void ClientInfoXtProc(Widget w, XtPointer client_data, 
			     XtPointer callData);
extern void create_client_info_popup(void);
