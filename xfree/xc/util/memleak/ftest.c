/*
 * $TOG: ftest.c /main/5 1998/02/09 11:39:56 kaleb $
 *
Copyright 1992, 1998  The Open Group

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

static char    *foo, *bar, *bletch;
static char    *glorf[100];

extern char *malloc ();

main ()
{
    int	    i;

    foo = malloc (1000);
    bar = malloc (2000);
    bletch = malloc (3000);
    for (i = 0; i < 100; i++)
	glorf[i] = malloc (i * 200);
    for (i = 0; i < 100; i++) {
	free (glorf[i]);
	glorf[i] = 0;
    }
    free (foo);
    free (bletch);
    bletch = 0;
    *foo = 'a';
    bar = 0;
    CheckMemory ();
}
