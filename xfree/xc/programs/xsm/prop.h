/* $TOG: prop.h /main/2 1998/02/09 14:14:46 kaleb $ */
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
/* $XFree86: xc/programs/xsm/prop.h,v 1.3 1999/03/07 14:23:42 dawes Exp $ */

extern void FreePropValues(List *propValues);
extern void FreeProp(Prop *prop);
extern void SetInitialProperties(ClientRec *client, List *props);
extern void SetProperty(ClientRec *client, SmProp *theProp, Bool freeIt);
extern void DeleteProperty(ClientRec *client, char *propname);
extern void SetPropertiesProc(SmsConn smsConn, SmPointer managerData, 
			      int numProps, SmProp **props);
extern void DeletePropertiesProc(SmsConn smsConn, SmPointer managerData, 
				 int numProps, char **propNames);
extern void GetPropertiesProc(SmsConn smsConn, SmPointer managerData);
