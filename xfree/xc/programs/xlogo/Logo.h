/* $TOG: Logo.h /main/14 1998/02/09 14:09:28 kaleb $ */
/*

Copyright 1988, 1990, 1998  The Open Group

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

#ifndef _XawLogo_h
#define _XawLogo_h

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 foreground	     Foreground		Pixel		XtDefaultForeground
 height		     Height		Dimension	100
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 shapeWindow	     ShapeWindow	Boolean		False
 width		     Width		Dimension	100
 x		     Position		Position	0
 y		     Position		Position	0

*/

#define XtNshapeWindow "shapeWindow"
#define XtCShapeWindow "ShapeWindow"

typedef struct _LogoRec *LogoWidget;
typedef struct _LogoClassRec *LogoWidgetClass;

extern WidgetClass logoWidgetClass;

#endif /* _XawLogo_h */
