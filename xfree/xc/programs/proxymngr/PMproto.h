/* $TOG: PMproto.h /main/2 1998/02/09 13:45:54 kaleb $ */

/*
Copyright 1996, 1998  The Open Group

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

/*		Proxy Management Protocol		*/

#ifndef _PMPROTO_H_
#define _PMPROTO_H_

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;	/* == 1 */
    CARD16	authLen B16;
    CARD32	length B32;
    /* STRING	   proxy-service */
    /* STRING	   server-address */
    /* STRING	   host-address */
    /* STRING	   start-options */
    /* STRING	   auth-name (if authLen > 0) */
    /* LISTofCARD8 auth-data (if authLen > 0) */
} pmGetProxyAddrMsg;

#define sz_pmGetProxyAddrMsg 8


typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;	/* == 2 */
    CARD8	status;
    CARD8	unused;
    CARD32	length B32;
    /* STRING	proxy-address */
    /* STRING	failure-reason */
} pmGetProxyAddrReplyMsg;

#define sz_pmGetProxyAddrReplyMsg 8


typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;	/* == 3 */
    CARD16	unused B16;
    CARD32	length B32;
    /* STRING	  proxy-service */
} pmStartProxyMsg;

#define sz_pmStartProxyMsg 8


#endif /* _PMPROTO_H_ */
