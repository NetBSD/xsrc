/* $TOG: lcUTF.c /main/25 1998/05/20 14:47:50 kaleb $ */
/******************************************************************

              Copyright 1993 by SunSoft, Inc.

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of SunSoft, Inc.
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.
SunSoft, Inc. makes no representations about the suitability of
this software for any purpose.  It is provided "as is" without
express or implied warranty.

SunSoft Inc. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL SunSoft, Inc. BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hiromu Inukai (inukai@Japan.Sun.COM) SunSoft, inc.

******************************************************************/
/* $XFree86: xc/lib/X11/lcUTF.c,v 3.3.2.3 1998/10/24 09:11:43 dawes Exp $ */


#ifdef X_LOCALE

#include "XlcUTF.h"

static int getutfrune(
#if NeedFunctionPrototypes
    char**,
    int*
#endif
);
static void our_wctomb(
#if NeedFunctionPrototypes
    wchar_t, 
    char **, 
    int *
#endif
);
static int our_mbtowc(
#if NeedFunctionPrototypes   
    wchar_t*, 
    char*, 
    size_t
#endif
);
static void	latin2rune(
#if NeedFunctionPrototypes
    unsigned char, 
    Rune*
#endif
);
static void	jis02012rune(
#if NeedFunctionPrototypes
    unsigned char, 
    Rune*
#endif
);
static void	jis02082rune(
#if NeedFunctionPrototypes
    unsigned char, 
    Rune*
#endif
);
static void	ksc2rune(
#if NeedFunctionPrototypes
    unsigned char, 
    Rune*
#endif
);
static void	gb2rune(
#if NeedFunctionPrototypes
    unsigned char, 
    Rune*
#endif
);
static void	init_latin1tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_latin2tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_latin3tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_latin4tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_cyrillictab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_koi8rtab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_arabictab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_greektab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_hebrewtab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_latin5tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_latin6tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_latin9tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_jis0201tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_jis0208tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_ksc5601tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);
static void	init_gb2312tab(
#if NeedFunctionPrototypes
    int*,
    wchar_t
#endif
);

static int*	tabkuten = NULL;
static int*	tabksc = NULL;
static int*	tabgb = NULL;

static UtfData utfdata_list = (UtfData)NULL;

static XlcUTFDataRec default_utf_data[] = 
{
    {"ISO8859-1", XlcGL, init_latin1tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-1", XlcGR, init_latin1tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-2", XlcGL, init_latin2tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-2", XlcGR, init_latin2tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-3", XlcGL, init_latin3tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-3", XlcGR, init_latin3tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-4", XlcGL, init_latin4tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-4", XlcGR, init_latin4tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-5", XlcGL, init_cyrillictab, latin2rune, N11n_none, 0x20},
    {"ISO8859-5", XlcGR, init_cyrillictab, latin2rune, N11n_none, 0x20},
    {"ISO8859-6", XlcGL, init_arabictab, latin2rune, N11n_none, 0x20},
    {"ISO8859-6", XlcGR, init_arabictab, latin2rune, N11n_none, 0x20},
    {"ISO8859-7", XlcGL, init_greektab, latin2rune, N11n_none, 0x20},
    {"ISO8859-7", XlcGR, init_greektab, latin2rune, N11n_none, 0x20},
    {"ISO8859-8", XlcGL, init_hebrewtab, latin2rune, N11n_none, 0x20},
    {"ISO8859-8", XlcGR, init_hebrewtab, latin2rune, N11n_none, 0x20},
    {"ISO8859-9", XlcGL, init_latin5tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-9", XlcGR, init_latin5tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-10", XlcGL, init_latin6tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-10", XlcGR, init_latin6tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-15", XlcGL, init_latin9tab, latin2rune, N11n_none, 0x20},
    {"ISO8859-15", XlcGR, init_latin9tab, latin2rune, N11n_none, 0x20},
    {"JISX0201.1976-0", XlcGL, init_jis0201tab, jis02012rune, N11n_none, 0x20},
    {"JISX0201.1976-0", XlcGR, init_jis0201tab, jis02012rune, N11n_none, 0x20},
    {"JISX0208.1983-0", XlcGL, init_jis0208tab, jis02082rune, N11n_ja, 0x2222},
    {"JISX0208.1983-0", XlcGR, init_jis0208tab, jis02082rune, N11n_ja, 0x2222},
    {"KSC5601.1987-0", XlcGL, init_ksc5601tab, ksc2rune, N11n_ko, 0x2160},
    {"KSC5601.1987-0", XlcGR, init_ksc5601tab, ksc2rune, N11n_ko, 0x2160},
    {"GB2312.1980-0", XlcGL, init_gb2312tab, gb2rune, N11n_zh, 0x2175},
    {"GB2312.1980-0", XlcGR, init_gb2312tab, gb2rune, N11n_zh, 0x2175},
    {"KOI8-R", XlcGL, init_koi8rtab, latin2rune, N11n_none, 0x20},
    {"KOI8-R", XlcGR, init_koi8rtab, latin2rune, N11n_none, 0x20},
};


static void
set_latin_nop(table, default_val)
    int*	table;
    wchar_t	default_val;
{
    register int i;
    for(i = 0; i < LATINMAX; i++)
	table[i] = (int) default_val;
    return;
}

static void
set_cjk_nop(to_tbl, to_max, default_val)
    int*	to_tbl;
    wchar_t	default_val;
    int	to_max;
{
    register int i;
    for(i = 0; i < to_max; i++)
	to_tbl[i] = default_val;
    return;
}

static void
set_latin_tab(fptr, table, fb_default)
    FILE*	fptr;
    int*	table;
    wchar_t	fb_default;
{
    int		j = 0;
    int		rv;
    long	value;

    while((rv = fscanf(fptr, "%lx", &value)) != EOF) {
	if(rv != 0)
	    table[j++] = (wchar_t) value;
    }
}

static void
set_cjk_tab(fptr, to_tbl, from_tbl, to_max, fb_default)
    FILE*	fptr;
    int*	to_tbl;
    int*	from_tbl;
    int		to_max;
    wchar_t	fb_default;
{
    int		j = 0;
    int		rv;
    long	value;

    while((rv = fscanf(fptr, "%lx", &value)) != EOF) {
	if(rv != 0)
	    to_tbl[j++] = value;
    }
    for(j = 0; j < to_max; j++) {
	if((value = to_tbl[j]) != -1)
	    from_tbl[abs(value)] = j;
    }
}

extern int _XlcResolveI18NPath();
static char TBL_DATA_DIR[] = "tbl_data";

static void
#if NeedFunctionPrototypes
init_8859_tab(
    int*	tbl,
    wchar_t	fb_default,
    char*	which)
#else
init_8859_tab(tbl, fb_default, which)
    int*	tbl;
    wchar_t	fb_default;
    char*	which;
#endif
{
    FILE*	fp = NULL;
    char	dirname[BUFSIZE];
    char	filename[BUFSIZE];
    char*	p;
    char*	q;

    _XlcResolveI18NPath(dirname, BUFSIZE);
    p = dirname;
    while(p) {
	q = strchr(p, ':');
	if(q) {
	    *q = '\0';
	}

	if ((3 + (p ? strlen(p) : 0) + 
	    strlen (TBL_DATA_DIR) + strlen (which)) < BUFSIZE) {
	    sprintf(filename, "%s/%s/%s", p, TBL_DATA_DIR, which);
	    fp = fopen (filename, "r");
	}
	if(fp) {
	    set_latin_tab(fp, tbl, fb_default);
	    fclose(fp);
	    return;
	}
	if(q) {
	    p = q + 1;
	} else {
	    p = q;
	}
    }
    if(!fp) {
	set_latin_nop(tbl, fb_default);
    }
}

static void
#if NeedFunctionPrototypes
init_cjk_tab(
    int*	tbl,
    wchar_t	fb_default,
    char*	which,
    int**	tab,
    long	max)
#else
init_cjk_tab(tbl, fb_default, which, tab, max)
    int*	tbl;
    wchar_t	fb_default;
    char*	which;
    int**	tab;
    long	max;
#endif
{
    FILE*	fp = NULL;
    char	dirname[BUFSIZE];
    char	filename[BUFSIZE];
    char*	p;
    char*	q;

    if((*tab = (int*)Xmalloc(max * sizeof(int))) == NULL) {
	return;
    }
    _XlcResolveI18NPath(dirname, BUFSIZE);
    p = dirname;
    while(p) {
	q = strchr(p, ':');
	if(q) {
	    *q = '\0';
	}
	if ((3 + (p ? strlen(p) : 0) + 
	    strlen (TBL_DATA_DIR) + strlen (which)) < BUFSIZE) {
	    sprintf(filename, "%s/%s/%s", p, TBL_DATA_DIR, which);
	    fp = fopen (filename, "r");
	}
	if(fp) {
	    set_cjk_tab(fp, *tab, tbl, max, fb_default);
	    fclose(fp);
	    return;
	}
	if(q) {
	    p = q + 1;
	} else {
	    p = q;
	}
    }
    if(!fp) {
	set_cjk_nop(*tab, max, fb_default);
    }
}

static void
#if NeedFunctionPrototypes
init_latin1tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_latin1tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_1);
}

static void
#if NeedFunctionPrototypes
init_latin2tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_latin2tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_2);
}

static void
#if NeedFunctionPrototypes
init_latin3tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_latin3tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_3);
}

static void
#if NeedFunctionPrototypes
init_latin4tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_latin4tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_4);
}

static void
#if NeedFunctionPrototypes
init_cyrillictab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_cyrillictab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_5);
}

static void
#if NeedFunctionPrototypes
init_koi8rtab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_koi8rtab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tabkoi8_r);
}

static void
#if NeedFunctionPrototypes
init_arabictab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_arabictab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_6);
}

static void
#if NeedFunctionPrototypes
init_greektab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_greektab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_7);
}

static void
#if NeedFunctionPrototypes
init_hebrewtab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_hebrewtab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_8);
}

static void
#if NeedFunctionPrototypes
init_latin5tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_latin5tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_9);
}

static void
#if NeedFunctionPrototypes
init_latin6tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_latin6tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_10);
}

static void
#if NeedFunctionPrototypes
init_latin9tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_latin9tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_8859_tab (tbl, fb_default, tab8859_15);
}

static void
#if NeedFunctionPrototypes
init_jis0201tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_jis0201tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
	int i;
	for(i = 0; i < NRUNE; i++)
	    tbl[i] = -1;
}

static void
#if NeedFunctionPrototypes
init_jis0208tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_jis0208tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_cjk_tab (tbl, fb_default, jis0208, &tabkuten, KUTENMAX);
}

static void
#if NeedFunctionPrototypes
init_ksc5601tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_ksc5601tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_cjk_tab (tbl, fb_default, ksc5601, &tabksc, KSCMAX);
}

static void
#if NeedFunctionPrototypes
init_gb2312tab(
    int*	tbl,
    wchar_t	fb_default)
#else
init_gb2312tab(tbl, fb_default)
    int*	tbl;
    wchar_t	fb_default;
#endif
{
    init_cjk_tab (tbl, fb_default, gb2312, &tabgb, GBMAX);
}

static UtfData
make_entry()
{
    UtfData tmp = (UtfData)Xmalloc(sizeof(UtfDataRec));
    bzero(tmp, sizeof(UtfDataRec));
    return tmp;
}

static int	once = 0;

static int
InitUTFInfo(lcd)
XLCd	lcd;
{
    if(!once) {
	int	i;
	CodeSet	*codeset_list = XLC_GENERIC(lcd, codeset_list);
	int	codeset_num = XLC_GENERIC(lcd, codeset_num);
	UtfData	pdata;

	if(!utfdata_list) {
	    utfdata_list = make_entry();
	}
	pdata = utfdata_list;
	for(i=0; i < codeset_num; i++) {
	    XlcCharSet	charset = *codeset_list[i]->charset_list;
	    while(pdata->next) {
		if(charset == pdata->charset) {
		    break;
		}
		pdata = pdata->next;
	    }
	    if(pdata->next) {
		continue;
	    } else {
		int j;
		for(j = 0; j < MAX_UTF_CHARSET; j++) {
		    if(_XlcCompareISOLatin1(charset->encoding_name, default_utf_data[j].name) ||
		       charset->side != default_utf_data[j].side) {
			continue;
		    } else {
 			pdata->initialize = default_utf_data[j].initialize;
			pdata->fromtbl = (int *)Xmalloc(LATINMAX * sizeof(int));
			(*pdata->initialize)(pdata->fromtbl, default_utf_data[j].fallback_value);
			pdata->already_init = True;
			pdata->charset = charset;
			pdata->cstorune = default_utf_data[j].cstorune;
			pdata->type = default_utf_data[j].type;
			pdata->next = make_entry();
			break;
		    }
		}
	    }
	}
	once = 1;
    }
    return 1;
}

static int
utftocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv	conv;
    char	**from;
    int		*from_left;
    char	**to;
    int		*to_left;
    XPointer	*args;
    int		num_args;
{
    char	*utfptr;
    char 	*bufptr;
    int		utf_len, buf_len;
    wchar_t	wc;
    XlcCharSet	tmpcharset = (XlcCharSet)NULL;
    UtfData	pdata = utfdata_list;

    if (from == NULL || *from == NULL)
	return 0;

    utfptr = *from;
    bufptr = *to;
    utf_len = *from_left;
    buf_len = *to_left;

    while(utf_len > 0 && buf_len > 0) {
	char *p = utfptr;
        int rune = getutfrune(&p, &utf_len);
	if(rune == -1) {
	    return -1;
	} else {
	    wc = (wchar_t) rune;
	    while(pdata->next) {
		wchar_t		r;
		int*		tbl;

		tbl = pdata->fromtbl;
		tbl += wc;
		if (*tbl == -1) {
		    if(tmpcharset) {
			    goto end;
		    } else {
			pdata = pdata->next;
			continue;
		    }
		} else {
		    r = *tbl;
		    utfptr = p;
		    if(!tmpcharset) tmpcharset = pdata->charset;
		}
		if(r < 128) {
		    *bufptr++ = r;
		    buf_len--;
		} else {
		    switch(pdata->type) {
			case N11n_ja:
			    *bufptr++ = (r/100 + ' ');
			    *bufptr++ = (r%100 + ' ');
			    break;
			case N11n_ko:
			    *bufptr++ = (r/94 + 0x21);
			    *bufptr++ = (r%94 + 0x21);
			    break;
			case N11n_zh:
			    *bufptr++ = 0x20 + (r/100);
			    *bufptr++ = 0x20 + (r%100);
			    break;
			default:
			    break;
		    }
		    buf_len -= 2;
		}
		break;
	    }
	    if(!tmpcharset) return -1; /* Unknown Codepoint */
	}
    }
end:
    if((num_args > 0) && tmpcharset)
	*((XlcCharSet *) args[0]) = tmpcharset;

    *from_left -= utfptr - *from;
    *from = utfptr;

    *to_left -= bufptr - *to;
    *to = bufptr;

    return 0;
}

static int
utf1tocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv	conv;
    char	**from;
    int		*from_left;
    char	**to;
    int		*to_left;
    XPointer	*args;
    int		num_args;
{
    char	**ptr = NULL;
    char	char_ptr[UTFmax];
    int		i = 0;
    wchar_t	dummy = (wchar_t)0;

    if (from == NULL || *from == NULL)
	return utftocs(conv, from, from_left, to, to_left, args, num_args);

    ptr = from;
    for(i = 0; i < UTFmax; char_ptr[i++] = *(*ptr)++);
    i=0;
    while(our_mbtowc(&dummy, (char*)&char_ptr[0], i) <= 0)
	i++;
    return utftocs(conv, from, &i, to, to_left, args, num_args);
}

static int
ucstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv	conv;
    XPointer	*from;
    int		*from_left;
    char	**to;
    int		*to_left;
    XPointer	*args;
    int		num_args;
{
    wchar_t	*ucsptr;
    char 	*bufptr;
    int		ucs_len, buf_len;
    XlcCharSet	tmpcharset = (XlcCharSet)NULL;
    UtfData	pdata = utfdata_list;

    if (from == NULL || *from == NULL)
	return 0;

    ucsptr = (wchar_t *)*from;
    bufptr = *to;
    ucs_len = *from_left;
    buf_len = *to_left;

    while(ucs_len > 0 && buf_len > 0) {
	while(pdata->next) {
	    wchar_t	r;
	    int*	tbl;

	    tbl = pdata->fromtbl;
	    tbl += *ucsptr;
	    if(*tbl == -1) {
		if(tmpcharset) {
		    goto end;
		} else {
		    pdata = pdata->next;
		    continue;
		}
	    } else {
		r = *tbl;
		if(!tmpcharset) tmpcharset = pdata->charset;
	    }
	    ucsptr++;
	    if(r < 128) {
		*bufptr++ = r;
		ucs_len--;
		buf_len--;
	    } else {
		switch(pdata->type) {
		case N11n_ja:
		    *bufptr++ = (r/100 + ' ');
		    *bufptr++ = (r%100 + ' ');
		    break;
		case N11n_ko:
		    *bufptr++ = (r/94 + 0x21);
		    *bufptr++ = (r%94 + 0x21);
		    break;
		case N11n_zh:
		    *bufptr++ = 0x20 + (r/100);
		    *bufptr++ = 0x20 + (r%100);
		    break;
		default:
		    break;
		}
		ucs_len--;
		buf_len -= 2;
	    }
	    break;
	}
	if(!tmpcharset) return  -1; /* Unknown Codepoint */
    }
end:
    if((num_args > 0) && tmpcharset)
	*((XlcCharSet *) args[0]) = tmpcharset;

    *from_left -= ucsptr - (wchar_t *)*from;
    *from = (XPointer)ucsptr;

    *to_left -= bufptr - *to;
    *to = bufptr;

    return 0;
}

static int
#if NeedFunctionPrototypes
getutfrune(char **read_from, int *from_len)
#else
getutfrune(read_from, from_len)
char **read_from;
int *from_len;
#endif
{
    int c, i;
    char str[UTFmax]; /* MB_LEN_MAX really */
    wchar_t wc;
    int n;

    str[0] = '\0';
    for(i = 0; i <= UTFmax;) {
	c = **read_from;
	(*read_from)++;
	str[i++] = c;
	n = our_mbtowc(&wc, str, i);
	if(n == -1)
	    return -1;
	if(n > 0) {
	    *from_len -= n;
	    return wc;
	}
    }
    return -1;
}

static int
cstoutf(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    char **to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XlcCharSet charset;
    char *csptr, *utfptr;
    int csstr_len, utf_len;
    int               cmp_len = 0;
    void	(*putrune)(
#if NeedFunctionPrototypes
                            unsigned char c,
                            Rune *r
#endif
			   ) = NULL;
    Rune	r = (Rune)0;
    UtfData	pdata = utfdata_list;

    if (from == NULL || *from == NULL)
	return 0;
    
    if (num_args < 1)
        return -1;

    csptr = *from;
    utfptr = *to;
    csstr_len = *from_left;
    utf_len = *to_left;

    charset = (XlcCharSet)args[0];
    cmp_len = strchr(charset->name, ':') - charset->name;
    while(pdata->next) {
        if(!_XlcNCompareISOLatin1(charset->name, pdata->charset->name, cmp_len)) {
	    putrune = pdata->cstorune;
	    break;
	} else {
	    pdata = pdata->next;
	}
    }
    if(!putrune)
	return -1;

    while(csstr_len-- > 0 && utf_len > 0) {
	(*putrune)(*csptr++, &r);
	if(!r) {
	    continue;
	}
	our_wctomb(r, &utfptr, &utf_len);
	r = 0;
    }
 
    *from_left -= csptr - *from;
    *from = csptr;
 
    *to_left -= utfptr - *to;
    *to = utfptr;
 
    return 0;
}

static int
cstoucs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XlcCharSet charset;
    char *csptr;
    wchar_t *ucsptr;
    int csstr_len, ucs_len;
    int               cmp_len = 0;
    void	(*putrune)(
#if NeedFunctionPrototypes
                            unsigned char c,
                            Rune *r
#endif
			   ) = NULL;
    Rune	r = (Rune)0;
    UtfData	pdata = utfdata_list;

    if (from == NULL || *from == NULL)
	return 0;
    
    if (num_args < 1)
        return -1;

    csptr = *from;
    ucsptr = (wchar_t *)*to;
    csstr_len = *from_left;
    ucs_len = *to_left;
    charset = (XlcCharSet)args[0];
    cmp_len = strchr(charset->name, ':') - charset->name;

    while(pdata->next) {
        if(!_XlcNCompareISOLatin1(charset->name, pdata->charset->name, cmp_len)) {
	    putrune = pdata->cstorune;
	    break;
	} else {
	    pdata = pdata->next;
	}
    }
    if(!putrune)
	return -1;

    while(csstr_len-- > 0 && ucs_len > 0) {
	(*putrune)(*csptr++, &r);
	if(!r) {
	    continue;
	}
	*ucsptr = (wchar_t)r;
	ucsptr++;
	ucs_len--;
	r = 0;
    }
 
    *from_left -= csptr - *from;
    *from = csptr;
 
    *to_left -= ucsptr - (wchar_t *)*to;
    *to = (XPointer)ucsptr;
 
    return 0;
}

static void
#if NeedFunctionPrototypes
our_wctomb(wchar_t r, char **utfptr, int *utf_len)
#else
our_wctomb(r, utfptr, utf_len)
wchar_t r;
char **utfptr;
int *utf_len;
#endif
{
    if(!utfptr || !*utfptr)
	return;               /* no shift states */
    if(r & ~Wchar2) {
	if(r & ~Wchar4) {
	    if(r & ~Wchar5) {
		/* 6 bytes */
		*(*utfptr)++ = T6 | ((r >> 5*Bitx) & Mask6);
		*(*utfptr)++ = Tx | ((r >> 4*Bitx) & Maskx);
		*(*utfptr)++  = Tx | ((r >> 3*Bitx) & Maskx);
		*(*utfptr)++  = Tx | ((r >> 2*Bitx) & Maskx);
		*(*utfptr)++  = Tx | ((r >> 1*Bitx) & Maskx);
		*(*utfptr)++  = Tx |  (r & Maskx);
		*utf_len -= 6;
		return;
	    }
	    /* 5 bytes */
	    *(*utfptr)++ = T5 |  (r >> 4*Bitx);
	    *(*utfptr)++ = Tx | ((r >> 3*Bitx) & Maskx);
	    *(*utfptr)++ = Tx | ((r >> 2*Bitx) & Maskx);
	    *(*utfptr)++ = Tx | ((r >> 1*Bitx) & Maskx);
	    *(*utfptr)++ = Tx |  (r & Maskx);
	    *utf_len -= 5;
	    return;
	}
	if(r & ~Wchar3) {
	    /* 4 bytes */
	    *(*utfptr)++ = T4 |  (r >> 3*Bitx);
	    *(*utfptr)++ = Tx | ((r >> 2*Bitx) & Maskx);
	    *(*utfptr)++ = Tx | ((r >> 1*Bitx) & Maskx);
	    *(*utfptr)++ = Tx |  (r & Maskx);
	    *utf_len -= 4;
	    return;
	}
	/* 3 bytes */
	*(*utfptr)++ = T3 |  (r >> 2*Bitx);
	*(*utfptr)++ = Tx | ((r >> 1*Bitx) & Maskx);
	*(*utfptr)++ = Tx |  (r & Maskx);
	*utf_len -= 3;
	return;
    }
    if(r & ~Wchar1) {
	/* 2 bytes */
	*(*utfptr)++ = T2 | (r >> 1*Bitx);
	*(*utfptr)++ = Tx | (r & Maskx);
	*utf_len -= 2;
	return;
    }
    /* 1 byte */
    *(*utfptr)++ = T1 | r;
    *utf_len -= 1;
    return;
}

static void
#if NeedFunctionPrototypes
latin2rune(unsigned char c, Rune *r)
#else
latin2rune(c, r)
unsigned char c;
Rune *r;
#endif
{
    *r = (Rune)c;
    return;
}

static void
#if NeedFunctionPrototypes
ksc2rune(unsigned char c, Rune *r)
#else
ksc2rune(c, r)
unsigned char c;
Rune *r;
#endif
{
    static enum { init, cs1last} state = init;
    static int korean646 = 1; /* fixed to 1 for now. */
    static int lastc;
    unsigned char	ch = (c|0x80); /* XXX */
    int n;
    wchar_t l;

    switch(state) {
    case init:
	if (ch < 128){
	    if(korean646 && (ch=='\\')){
		emit(0x20A9);
	    } else {
		emit(ch);
	    }
	}else{
	    lastc = ch;
	    state = cs1last;
	}
	return;
        
    case cs1last: /* 2nd byte of codeset 1 (KSC 5601) */
	n = ((lastc&0x7f)-33)*94 + (ch&0x7f)-33;
	if((l = tabksc[n]) == 0){
	    emit(BADMAP);
	} else {
	    emit(l);
	}
	state = init;
	return;
    }
}

static void
#if NeedFunctionPrototypes
jis02012rune(unsigned char c, Rune *r)
#else
jis02012rune(c, r)
    unsigned char c;
    Rune *r;
#endif
{
/* To Be Implemented */
}

static void
#if NeedFunctionPrototypes
gb2rune(unsigned char c, Rune *r)
#else
gb2rune(c, r)
    unsigned char c;
    Rune *r;
#endif
{
    static enum { state0, state1 } state = state0;
    static int lastc;
    long n;
    unsigned char	ch1 = (c|0x80); /* XXX */

    switch(state) {
    case state0:    /* idle state */
	if(ch1 >= 0xA1){
	    lastc = ch1;
	    state = state1;
	    return;
	}
        emit(ch1);
        return;

    case state1:    /* seen a font spec */
	if(ch1 >= 0xA1)
	    n = (lastc-0xA0)*100 + (ch1-0xA0);
        else {
	    emit(BADMAP);
	    state = state0;
	    return;
        }
        if(tabgb[n] < 0){
	    emit(BADMAP);
        } else
	    emit(tabgb[n]);
        state = state0;
    }
}

static void
#if NeedFunctionPrototypes
jis02082rune(unsigned char c, Rune *r)
#else
jis02082rune(c, r)
    unsigned char c;
    Rune *r;
#endif
{
    static enum { state0, state1} state = state0;
    static int lastc;
    unsigned char	ch = (c|0x80); /* XXX */
    int n;
    wchar_t l;

again:
    switch(state) {
    case state0:    /* idle state */
	lastc = ch;
	state = state1;
	return;

    case state1:    /* two part char */
	if((lastc&0x80) != (ch&0x80)){
	    emit(lastc);
	    state = state0;
	    goto again;
	}
	if(CANS2J(lastc, ch)){
	    int h = lastc, l = ch;
	    S2J(h, l);
	    n = h*100 + l - 3232;
	} else
	    n = (lastc&0x7F)*100 + (ch&0x7f) - 3232; /* kuten */
	if(tabkuten[n] == -1){
	    emit(BADMAP);
	} else {          
	    emit(tabkuten[n]);
	}        
	state = state0;
    }
}

static int
#if NeedFunctionPrototypes
our_mbtowc(wchar_t *p, char *s, size_t n)
#else
our_mbtowc(p, s, n)
    wchar_t *p;
    char *s;
    size_t n;
#endif
{
    unsigned char *us;
    int c0, c1, c2, c3, c4, c5;
    wchar_t wc;

    if(s == 0)
	return 0;		/* no shift states */

    if(n < 1)
	goto badlen;
    us = (unsigned char*)s;
    c0 = us[0];
    if(c0 >= T3) {
	if(n < 3)
	    goto badlen;
	c1 = us[1] ^ Tx;
	c2 = us[2] ^ Tx;
	if((c1|c2) & T2) {
	    goto bad;
	}
	if(c0 >= T5) {
	    if(n < 5)
		goto badlen;
	    c3 = us[3] ^ Tx;
	    c4 = us[4] ^ Tx;
	    if((c3|c4) & T2) {
		goto bad;
	    }
	    if(c0 >= T6) {
		/* 6 bytes */
		if(n < 6)
		    goto badlen;
		c5 = us[5] ^ Tx;
		if(c5 & T2) {
		    goto bad;
		}
		wc = ((((((((((c0 & Mask6) << Bitx) | c1) 
					   << Bitx) | c2) 
					   << Bitx) | c3) 
					   << Bitx) | c4) 
					   << Bitx) | c5;
		if(wc <= Wchar5) {
		    goto bad;
		}
		*p = wc;
		return 6;
	    }
	    /* 5 bytes */
	    wc = ((((((((c0 & Mask5) << Bitx) | c1) 
				     << Bitx) | c2) 
				     << Bitx) | c3) 
				     << Bitx) | c4;
	    if(wc <= Wchar4) {
				goto bad;
	    }
	    *p = wc;
	    return 5;
	}
	if(c0 >= T4) {
	    /* 4 bytes */
	    if(n < 4)
		goto badlen;
	    c3 = us[3] ^ Tx;
	    if(c3 & T2) {
		goto bad;
	    }
	    wc = ((((((c0 & Mask4) << Bitx) | c1) 
				   << Bitx) | c2) 
				   << Bitx) | c3;
	    if(wc <= Wchar3) {
		goto bad;
	    }
	    *p = wc;
	    return 4;
	}
	/* 3 bytes */
	wc = ((((c0 & Mask3) << Bitx) | c1) 
			     << Bitx) | c2;
	if(wc <= Wchar2) {
	    goto bad;
	}
	*p = wc;
	return 3;
    }
    if(c0 >= T2) {
	/* 2 bytes */
	if(n < 2)
	    goto badlen;
	c1 = us[1] ^ Tx;
	if(c1 & T2) {
	    goto bad;
	}
	wc = ((c0 & Mask2) << Bitx) | c1;
	if(wc <= Wchar1) {
	    goto bad;
	}
	*p = wc;
	return 2;
    }
    /* 1 byte */
    if(c0 >= Tx) {
	goto bad;
    }
    *p = c0;
    return 1;

bad:
    errno = EILSEQ;
    return -1;
badlen:
    return -2;
}

static void
close_converter(conv)
    XlcConv conv;
{
    Xfree((char *) conv);
}

static XlcConv
create_conv(lcd, methods)
    XLCd	lcd;
    XlcConvMethods	methods;
{
    XlcConv conv;

    conv = (XlcConv) Xmalloc(sizeof(XlcConvRec));
    if (conv == (XlcConv) NULL)
	return (XlcConv) NULL;

    conv->methods = methods;

    conv->state = NULL;
    InitUTFInfo(lcd);

    return conv;
}

static XlcConvMethodsRec mbtocs_methods = {
    close_converter,
    utf1tocs,
    NULL
};

static XlcConv
open_mbtocs(from_lcd, from, to_lcd, to)
    XLCd from_lcd;
    char *from;
    XLCd to_lcd;
    char *to;
{
    return create_conv(from_lcd, &mbtocs_methods);
}

static XlcConvMethodsRec mbstocs_methods = {
    close_converter,
    utftocs,
    NULL
};

static XlcConv
open_mbstocs(from_lcd, from, to_lcd, to)
    XLCd from_lcd;
    char *from;
    XLCd to_lcd;
    char *to;
{
    return create_conv(from_lcd, &mbstocs_methods);
}

static XlcConvMethodsRec wcstocs_methods = {
    close_converter,
    ucstocs,
    NULL
};

static XlcConv
open_wcstocs(from_lcd, from, to_lcd, to)
    XLCd from_lcd;
    char *from;
    XLCd to_lcd;
    char *to;
{
    return create_conv(from_lcd, &wcstocs_methods);
}

static XlcConvMethodsRec cstombs_methods = {
    close_converter,
    cstoutf,
    NULL
};

static XlcConv
open_cstombs(from_lcd, from, to_lcd, to)
    XLCd from_lcd;
    char *from;
    XLCd to_lcd;
    char *to;
{
    return create_conv(from_lcd, &cstombs_methods);
}

static XlcConvMethodsRec cstowcs_methods = {
    close_converter,
    cstoucs,
    NULL
};

static XlcConv
open_cstowcs(from_lcd, from, to_lcd, to)
    XLCd from_lcd;
    char *from;
    XLCd to_lcd;
    char *to;
{
    return create_conv(from_lcd, &cstowcs_methods);
}


XLCd
_XlcUtfLoader(name)
    _Xconst char *name;
{
    XLCd lcd;

    lcd = _XlcCreateLC(name, _XlcGenericMethods);
    if (lcd == (XLCd) NULL)
	return lcd;
    
    if (!XLC_PUBLIC_PART(lcd)->codeset ||
	(_XlcCompareISOLatin1(XLC_PUBLIC_PART(lcd)->codeset, "utf"))) {
	_XlcDestroyLC(lcd);
	return (XLCd) NULL;
    }

    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCharSet, open_mbstocs);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCharSet, open_wcstocs);

    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNChar, open_mbtocs);

    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNMultiByte, open_cstombs);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNWideChar, open_cstowcs);

    return lcd;
}

#else
typedef int dummy;
#endif /* X_LOCALE */
