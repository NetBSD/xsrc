/* $XConsortium: mitauth.c,v 1.8 94/04/17 20:27:03 gildea Exp $ */
/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * MIT-MAGIC-COOKIE-1 authorization scheme
 * Author:  Keith Packard, MIT X Consortium
 */

#include "X.h"
#include "os.h"
#include "dixstruct.h"

static struct auth {
    struct auth	*next;
    unsigned short	len;
    char	*data;
    XID		id;
} *mit_auth;

int
MitAddCookie (data_length, data, id)
unsigned short	data_length;
char	*data;
XID	id;
{
    struct auth	*new;

    new = (struct auth *) xalloc (sizeof (struct auth));
    if (!new)
	return 0;
    new->data = (char *) xalloc ((unsigned) data_length);
    if (!new->data) {
	xfree(new);
	return 0;
    }
    new->next = mit_auth;
    mit_auth = new;
    memmove(new->data, data, (int) data_length);
    new->len = data_length;
    new->id = id;
    return 1;
}

XID
MitCheckCookie (data_length, data, client, reason)
    unsigned short	data_length;
    char	*data;
    ClientPtr client;
    char	**reason;
{
    struct auth	*auth;

    for (auth = mit_auth; auth; auth=auth->next) {
        if (data_length == auth->len &&
	   memcmp (data, auth->data, (int) data_length) == 0)
	    return auth->id;
    }
    *reason = "Invalid MIT-MAGIC-COOKIE-1 key";
    return (XID) -1;
}

int
MitResetCookie ()
{
    struct auth	*auth, *next;

    for (auth = mit_auth; auth; auth=next) {
	next = auth->next;
	xfree (auth->data);
	xfree (auth);
    }
    mit_auth = 0;
    return 0;
}

XID
MitToID (data_length, data)
unsigned short	data_length;
char	*data;
{
    struct auth	*auth;

    for (auth = mit_auth; auth; auth=auth->next) {
	if (data_length == auth->len &&
	    memcmp (data, auth->data, data_length) == 0)
	    return auth->id;
    }
    return (XID) -1;
}

MitFromID (id, data_lenp, datap)
XID id;
unsigned short	*data_lenp;
char	**datap;
{
    struct auth	*auth;

    for (auth = mit_auth; auth; auth=auth->next) {
	if (id == auth->id) {
	    *data_lenp = auth->len;
	    *datap = auth->data;
	    return 1;
	}
    }
    return 0;
}

MitRemoveCookie (data_length, data)
unsigned short	data_length;
char	*data;
{
    struct auth	*auth, *prev;

    prev = 0;
    for (auth = mit_auth; auth; auth=auth->next) {
	if (data_length == auth->len &&
	    memcmp (data, auth->data, data_length) == 0)
 	{
	    if (prev)
		prev->next = auth->next;
	    else
		mit_auth = auth->next;
	    xfree (auth->data);
	    xfree (auth);
	    return 1;
	}
    }
    return 0;
}
