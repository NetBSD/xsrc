/* $TOG: save.h /main/5 1998/02/09 14:15:33 kaleb $ */
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
/* $XFree86: xc/programs/xsm/save.h,v 1.3 1999/03/07 14:23:43 dawes Exp $ */

extern void DoSave(int saveType, int interactStyle, Bool fast);
extern void LetClientInteract(List *cl);
extern void StartPhase2(void);
extern void FinishUpSave(void);
extern void SetSaveSensitivity(Bool on);
extern void SavePopupStructureNotifyXtHandler(Widget w, XtPointer closure, 
					      XEvent *event, 
					      Boolean *continue_to_dispatch);
extern void create_save_popup(void);
extern void PopupSaveDialog(void);
extern void CheckPointXtProc(Widget w, XtPointer client_data, 
			     XtPointer callData);
extern void ShutdownSaveXtProc(Widget w, XtPointer client_data, 
			       XtPointer callData);
extern void PopupBadSave(void);
extern void ShutdownDontSaveXtProc(Widget w, XtPointer client_data, 
				   XtPointer callData);
