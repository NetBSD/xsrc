/* $TOG: saveutil.h /main/4 1998/02/09 14:15:19 kaleb $ */
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
/* $XFree86: xc/programs/xsm/saveutil.h,v 1.4 1999/03/28 15:33:12 dawes Exp $ */

extern void set_session_save_file_name(char *session_name);
extern int ReadSave(char *session_name, char **sm_id);
extern void WriteSave(char *sm_id);
extern Status DeleteSession(char *session_name);
extern Bool getnextline(char **pbuf, int *plen, FILE *f);

