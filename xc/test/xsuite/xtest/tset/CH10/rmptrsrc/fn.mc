*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: fn.mc,v 1.9 94/04/17 21:10:12 rws Exp $
 */

>># resource manager test common functions
>># Used by:
>>#	XrmPutLineResource
>>#	XrmPutResource
>>#	XrmQPutResource
>>#	XrmPutStringResource
>>#	XrmQPutStringResource
>>#	XrmGetResource
>>#	XrmQGetResource
>>#	XrmMergeDatabases
>>#	XrmDestroyDatabase
>>#	XrmGetFileDatabase
>>#	XrmGetStringDatabase
>>#	XrmPutFileDatabase

>>EXTERN

static XrmDatabase
xrm_create_database(data)
char *data;
{
	/* Create a new database for testing with */
	return(XrmGetStringDatabase(data));
}

static void
xrm_fill_value(value, data)
XrmValue *value;
char *data;
{
	value->addr = (caddr_t)data;
	value->size = (unsigned int)(strlen(data)+1);
}

static int
xrm_check_entry(dbase, fullspec, fullclass, type, val)
XrmDatabase dbase;
char *fullspec;
char *fullclass;
char *type;
char *val;
{
	int ret_val;
	char	*type_ret;
	XrmValue	value_ret;

	ret_val=0;
	type_ret=(char *)NULL;
	value_ret.size=0;
	value_ret.addr=(caddr_t)NULL;

	if(XrmGetResource(dbase, fullspec, fullclass, &type_ret, &value_ret)
		==False) {
		report("XrmGetResource failed to find database entry");
		report("Specifier was: %s", fullspec);
		ret_val++;
	} else {
		if (type_ret==NULL || strcmp(type_ret, type)) {
			report("XrmGetResource returned unexpected type information.");
			report("Specifier was: %s", fullspec);
			report("Expected type: '%s'", type);
			report("Returned type: '%s'",
				(type_ret==NULL)?"<NULL POINTER>":type_ret);
			ret_val++;
		}

		if (( value_ret.addr==(caddr_t)NULL)
			|| (strncmp((char *)value_ret.addr, val, strlen(val)))) {
			report("XrmGetResource returned unexpected value information.");
			report("Specifier was: %s", fullspec);
			report("Expected value: '%s'", val);
			if(value_ret.addr == (caddr_t)NULL) {
				report("Returned value: <NULL POINTER>");
			} else {
				report("Returned value: '%.*s' (%u bytes)",
					value_ret.size, (char *)value_ret.addr, value_ret.size);
			}
			ret_val++;
		}
	}

	return(ret_val);
}

static int
xrm_tabulate(from, into)
char *from, *into;
{
	int i,j;

	j = strlen(from);
	for(i=0; i<j; i++) {
		if(from[i]=='T') {
			into[i]='\t';
		} else {
			into[i]=from[i];
		}
	}
	return(j);
}
