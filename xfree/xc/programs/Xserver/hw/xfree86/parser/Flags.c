/* $XFree86: xc/programs/Xserver/hw/xfree86/parser/Flags.c,v 1.14 2000/10/20 14:59:02 alanh Exp $ */
/* 
 * 
 * Copyright (c) 1997  Metro Link Incorporated
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Metro Link shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Metro Link.
 * 
 */

/* View/edit this file with tab stops set to 4 */

#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"
#include <math.h>

extern LexRec val;

static xf86ConfigSymTabRec ServerFlagsTab[] =
{
	{COMMENT, "###"},
	{ENDSECTION, "endsection"},
	{NOTRAPSIGNALS, "notrapsignals"},
	{DONTZAP, "dontzap"},
	{DONTZOOM, "dontzoom"},
	{DISABLEVIDMODE, "disablevidmodeextension"},
	{ALLOWNONLOCAL, "allownonlocalxvidtune"},
	{DISABLEMODINDEV, "disablemodindev"},
	{MODINDEVALLOWNONLOCAL, "allownonlocalmodindev"},
	{ALLOWMOUSEOPENFAIL, "allowmouseopenfail"},
	{OPTION, "option"},
	{BLANKTIME, "blanktime"},
	{STANDBYTIME, "standbytime"},
	{SUSPENDTIME, "suspendtime"},
	{OFFTIME, "offtime"},
	{DEFAULTLAYOUT, "defaultserverlayout"},
	{-1, ""},
};

#define CLEANUP xf86freeFlags

XF86ConfFlagsPtr
xf86parseFlagsSection (void)
{
	parsePrologue (XF86ConfFlagsPtr, XF86ConfFlagsRec)

	while ((token = xf86getToken (ServerFlagsTab)) != ENDSECTION)
	{
		int hasvalue = FALSE;
		int strvalue = FALSE;
		int tokentype;
		switch (token)
		{
		case COMMENT:
		    if ((token = xf86getToken (NULL)) != STRING)
				Error (QUOTE_MSG, "###");
		    break;
			/* 
			 * these old keywords are turned into standard generic options.
			 * we fall through here on purpose
			 */
		case DEFAULTLAYOUT:
			strvalue = TRUE;
		case BLANKTIME:
		case STANDBYTIME:
		case SUSPENDTIME:
		case OFFTIME:
			hasvalue = TRUE;
		case NOTRAPSIGNALS:
		case DONTZAP:
		case DONTZOOM:
		case DISABLEVIDMODE:
		case ALLOWNONLOCAL:
		case DISABLEMODINDEV:
		case MODINDEVALLOWNONLOCAL:
		case ALLOWMOUSEOPENFAIL:
			{
				int i = 0;
				while (ServerFlagsTab[i].token != -1)
				{
					char *tmp;

					if (ServerFlagsTab[i].token == token)
					{
						char *valstr = NULL;
						/* can't use strdup because it calls malloc */
						tmp = xf86configStrdup (ServerFlagsTab[i].name);
						if (hasvalue)
						{
							tokentype = xf86getToken(NULL);
							if (strvalue) {
							    if (tokentype != STRING)
								Error (QUOTE_MSG, tmp);
							    valstr = val.str;
							} else {
							    if (tokentype != NUMBER)
								Error (NUMBER_MSG, tmp);
							    valstr = xf86confmalloc(16);
							    if (valstr)
								sprintf(valstr, "%d", val.num);
							}
						}
						ptr->flg_option_lst = xf86addNewOption
							(ptr->flg_option_lst, tmp, valstr);
					}
					i++;
				}
			}
			break;
		case OPTION:
			{
				char *name;
				if ((token = xf86getToken (NULL)) != STRING)
				{
					Error (BAD_OPTION_MSG, NULL);
					break;
				}

				name = val.str;
				if ((token = xf86getToken (NULL)) == STRING)
				{
					ptr->flg_option_lst = xf86addNewOption (ptr->flg_option_lst,
														name, val.str);
				}
				else
				{
					ptr->flg_option_lst = xf86addNewOption (ptr->flg_option_lst,
														name, NULL);
					xf86unGetToken (token);
				}
			}
			break;

		case EOF_TOKEN:
			Error (UNEXPECTED_EOF_MSG, NULL);
			break;
		default:
			Error (INVALID_KEYWORD_MSG, xf86tokenString ());
			break;
		}
	}

#ifdef DEBUG
	printf ("Flags section parsed\n");
#endif

	return ptr;
}

#undef CLEANUP

void
xf86printServerFlagsSection (FILE * f, XF86ConfFlagsPtr flags)
{
	XF86OptionPtr p;

	if ((!flags) || (!flags->flg_option_lst))
		return;
	p = flags->flg_option_lst;
	fprintf (f, "Section \"ServerFlags\"\n");
	if (p->opt_comment)
	    fprintf (f, "\t###    \"%s\" \n", p->opt_comment);
	while (p)
	{
		if (p->opt_val)
			fprintf (f, "\tOption \"%s\" \"%s\"\n", p->opt_name, p->opt_val);
		else
			fprintf (f, "\tOption \"%s\"\n", p->opt_name);
		p = p->list.next;
	}
	fprintf (f, "EndSection\n\n");
}

static XF86OptionPtr
addNewOption2 (XF86OptionPtr head, char *name, char *val, int used)
{
        XF86OptionPtr new, old = NULL;

	/* Don't allow duplicates */
 	if (head != NULL && (old = xf86findOption(head, name)) != NULL)
 	    new = old;
 	else {
	    new = xf86confcalloc (1, sizeof (XF86OptionRec));
 	    new->list.next = NULL;
 	}
 	
 	new->opt_name = name;
 	new->opt_val = val;
 	new->opt_used = used;
	
  	if (old == NULL)
	    return ((XF86OptionPtr) xf86addListItem ((glp) head, (glp) new));
 	else 
 	    return head;
}

XF86OptionPtr
xf86addNewOption (XF86OptionPtr head, char *name, char *val)
{
	return addNewOption2(head, name, val, 0);
}

void
xf86freeFlags (XF86ConfFlagsPtr flags)
{
	if (flags == NULL)
		return;
	xf86optionListFree (flags->flg_option_lst);
	xf86conffree (flags);
}

XF86OptionPtr
xf86optionListDup (XF86OptionPtr opt)
{
	XF86OptionPtr newopt = NULL;

	while (opt)
	{
		newopt = xf86addNewOption(newopt, opt->opt_name, opt->opt_val);
		newopt->opt_used = opt->opt_used;
		opt = opt->list.next;
	}
	return newopt;
}

void
xf86optionListFree (XF86OptionPtr opt)
{
	XF86OptionPtr prev;

	while (opt)
	{
		TestFree (opt->opt_name);
		TestFree (opt->opt_val);
		prev = opt;
		opt = opt->list.next;
		xf86conffree (prev);
	}
}

char *
xf86optionName(XF86OptionPtr opt)
{
    if (opt)
	return opt->opt_name;
    return 0;
}

char *
xf86optionValue(XF86OptionPtr opt)
{
    if (opt)
	return opt->opt_val;
    return 0;
}

XF86OptionPtr
xf86newOption(char *name, char *value)
{
    XF86OptionPtr opt;

    opt = xf86confcalloc(1, sizeof (XF86OptionRec));
    if (!opt)
	return NULL;

    opt->opt_used = 0;
    opt->list.next = 0;
    opt->opt_name = name;
    opt->opt_val = value;

    return opt;
}

XF86OptionPtr
xf86nextOption(XF86OptionPtr list)
{
    if (!list)
	return NULL;
    return list->list.next;
}

/*
 * this function searches the given option list for the named option and
 * returns a pointer to the option rec if found. If not found, it returns
 * NULL
 */

XF86OptionPtr
xf86findOption (XF86OptionPtr list, const char *name)
{
	while (list)
	{
		if (xf86nameCompare (list->opt_name, name) == 0)
			return (list);
		list = list->list.next;
	}
	return (NULL);
}

/*
 * this function searches the given option list for the named option. If
 * found and the option has a parameter, a pointer to the parameter is
 * returned.  If the option does not have a parameter an empty string is
 * returned.  If the option is not found, a NULL is returned.
 */

char *
xf86findOptionValue (XF86OptionPtr list, const char *name)
{
	XF86OptionPtr p = xf86findOption (list, name);

	if (p)
	{
		if (p->opt_val)
			return (p->opt_val);
		else
			return "";
	}
	return (NULL);
}

XF86OptionPtr
xf86optionListCreate( const char **options, int count, int used )
{
	XF86OptionPtr p = NULL;
	char *t1, *t2;
	int i;

	if (count == -1)
	{
		for (count = 0; options[count]; count++)
			;
	}
	if( (count % 2) != 0 )
	{
		fprintf( stderr, "xf86optionListCreate: count must be an even number.\n" );
		return (NULL);
	}
	for (i = 0; i < count; i += 2)
	{
		/* can't use strdup because it calls malloc */
		t1 = xf86confmalloc (sizeof (char) *
				(strlen (options[i]) + 1));
		strcpy (t1, options[i]);
		t2 = xf86confmalloc (sizeof (char) *
				(strlen (options[i + 1]) + 1));
		strcpy (t2, options[i + 1]);
		p = addNewOption2 (p, t1, t2, used);
	}

	return (p);
}

/* the 2 given lists are merged. If an option with the same name is present in
 * both, the option from the user list is used. The end result is a single
 * valid list of options. Duplicates are freed, and the original lists are no
 * longer guaranteed to be complete.
 */
XF86OptionPtr
xf86optionListMerge (XF86OptionPtr head, XF86OptionPtr tail)
{
	XF86OptionPtr a, b, ap = NULL, bp = NULL, f = NULL;

	a = head;
	while (a)
	{
		bp = NULL;
		b = tail;
		while (b)
		{
			if (xf86nameCompare (a->opt_name, b->opt_name) == 0)
			{
				if ((a == head) && (b == tail))
				{
					head = b;
					tail = b->list.next;
					b->list.next = a->list.next;
					bp = tail;
				}
				else if (a == head)
				{
					head = b;
					bp->list.next = b->list.next;
					b->list.next = a->list.next;
				}
				else if (b == tail)
				{
					tail = b->list.next;
					ap->list.next = b;
					b->list.next = a->list.next;
					bp = tail;
				}
				else
				{
					ap->list.next = b;
					bp->list.next = b->list.next;
					b->list.next = a->list.next;
				}
				a->list.next = f;
				f = a;
				a = b;
				b = bp;
				continue;
			}
			bp = b;
			b = b->list.next;
		}
		ap = a;
		a = a->list.next;
	}

	ap->list.next = tail;

	xf86optionListFree (f);
	return (head);
}

char *
xf86uLongToString(unsigned long i)
{
	char *s;
    int l;

	l = (int)(ceil(log10((double)i) + 2.5));
	s = xf86confmalloc(l);
	if (!s)
		return NULL;
	sprintf(s, "%lu", i);
	return s;
}

void
xf86debugListOptions(XF86OptionPtr Options)
{
    while (Options) {
	ErrorF("Option: %s Value: %s\n",Options->opt_name,Options->opt_val);
	Options = Options->list.next;
    }
}

