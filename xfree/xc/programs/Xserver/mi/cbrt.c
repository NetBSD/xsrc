/* $TOG: cbrt.c /main/4 1998/02/09 14:45:52 kaleb $ */
/*

Copyright 1990, 1998  The Open Group

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
/* $XFree86: xc/programs/Xserver/mi/cbrt.c,v 3.1 1998/10/04 09:39:22 dawes Exp $ */

/* simple cbrt, in case your math library doesn't have a good one */

double pow();

double
cbrt(x)
    double x;
{
    if (x > 0.0)
	return pow(x, 1.0/3.0);
    else
	return -pow(-x, 1.0/3.0);
}
