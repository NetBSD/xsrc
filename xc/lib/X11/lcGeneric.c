/* $XConsortium: lcGeneric.c /main/7 1995/11/18 16:08:54 kaleb $ */
/*
 * Copyright 1992, 1993 by TOSHIBA Corp.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of TOSHIBA not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. TOSHIBA make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * TOSHIBA DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * TOSHIBA BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Katsuhisa Yano	TOSHIBA Corp.
 *			   	mopi@osa.ilab.toshiba.co.jp
 */

#include <stdio.h>
#include "Xlibint.h"
#include "XlcGeneric.h"

static XLCd create();
static Bool initialize();
static void destroy();

static XLCdPublicMethodsRec genericMethods = {
    { NULL },                   /* use default methods */
    {
	NULL,
	create,
	initialize,
	destroy,
	NULL
    }
};

XLCdMethods _XlcGenericMethods = (XLCdMethods) &genericMethods;

static XLCd
create(name, methods)
    char *name;
    XLCdMethods methods;
{
    XLCd lcd;
    XLCdPublicMethods new;

    lcd = (XLCd) Xmalloc(sizeof(XLCdRec));
    if (lcd == NULL)
        return (XLCd) NULL;
    bzero((char *) lcd, sizeof(XLCdRec));

    lcd->core = (XLCdCore) Xmalloc(sizeof(XLCdGenericRec));
    if (lcd->core == NULL)
	goto err;
    bzero((char *) lcd->core, sizeof(XLCdGenericRec));

    new = (XLCdPublicMethods) Xmalloc(sizeof(XLCdPublicMethodsRec));
    if (new == NULL)
	goto err;
    *new = *((XLCdPublicMethods) methods);
    lcd->methods = (XLCdMethods) new;

    return lcd;

err:
    Xfree(lcd);
    return (XLCd) NULL;
}

static Bool
string_to_encoding(str, encoding)
    char *str;
    char *encoding;
{
    char *next;
    long value;
    int base;

    while (*str) {
	if (*str == '\\') {
	    switch (*(str + 1)) {
		case 'x':
		case 'X':
		    base = 16;
		    break;
		default:
		    base = 8;
		    break;
	    }
	    value = strtol(str + 2, &next, base);
	    if (str + 2 != next) {
		*((unsigned char *) encoding++) = (unsigned char) value;
		str = next;
		continue;
	    }
	}
	*encoding++ = *str++;
    }

    *encoding = '\0';

    return True;
}

static Bool
string_to_ulong(str, value)
char	*str;
unsigned long	*value;
{
     char	*tmp1 = str;
     int	 base;

     if(*tmp1++ != '\\'){
	  tmp1--;
	  base = 10;
     }else{
	  switch(*tmp1++){
	  case 'x':
	       base = 16;
	       break;
	  case 'o':
	       base = 8;
	       break;
	  case 'd':
	       base = 10;
	       break;
	  default:
	       return(False);
	  }
     }
    *value = (unsigned) strtol(tmp1, NULL, base);
     return(True);
}


static Bool
add_charset(codeset, charset)
    CodeSet codeset;
    XlcCharSet charset;
{
    XlcCharSet *new_list;
    int num;

    if (num = codeset->num_charsets)
        new_list = (XlcCharSet *) Xrealloc(codeset->charset_list,
                                        (num + 1) * sizeof(XlcCharSet));
    else
        new_list = (XlcCharSet *) Xmalloc(sizeof(XlcCharSet));

    if (new_list == NULL)
	return False;

    new_list[num] = charset;
    codeset->charset_list = new_list;
    codeset->num_charsets = num + 1;

    return True;
}

static CodeSet
add_codeset(gen)
    XLCdGenericPart *gen;
{
    CodeSet new, *new_list;
    int num;

    new = (CodeSet) Xmalloc(sizeof(CodeSetRec));
    if (new == NULL)
        return NULL;
    bzero((char *) new, sizeof(CodeSetRec));

    if (num = gen->codeset_num)
        new_list = (CodeSet *) Xrealloc(gen->codeset_list,
                                        (num + 1) * sizeof(CodeSet));
    else
        new_list = (CodeSet *) Xmalloc(sizeof(CodeSet));

    if (new_list == NULL)
        goto err;

    new_list[num] = new;
    gen->codeset_list = new_list;
    gen->codeset_num = num + 1;

    return new;

err:
    Xfree(new);

    return NULL;
}

static Bool
add_parse_list(gen, type, encoding, codeset)
    XLCdGenericPart *gen;
    EncodingType type;
    char *encoding;
    CodeSet codeset;
{
    ParseInfo new, *new_list;
    char *str;
    unsigned char ch;
    int num;

    str = (char *) Xmalloc(strlen(encoding) + 1);
    if (str == NULL)
        return False;
    strcpy(str, encoding);

    new = (ParseInfo) Xmalloc(sizeof(ParseInfoRec));
    if (new == NULL)
        goto err;
    bzero((char *) new, sizeof(ParseInfoRec));

    if (gen->mb_parse_table == NULL) {
        gen->mb_parse_table = (unsigned char *) Xmalloc(256); /* 2^8 */
        if (gen->mb_parse_table == NULL)
            goto err;
        bzero((char *) gen->mb_parse_table, 256);
    }

    if (num = gen->mb_parse_list_num)
        new_list = (ParseInfo *) Xrealloc(gen->mb_parse_list,
                                          (num + 2) * sizeof(ParseInfo));
    else {
        new_list = (ParseInfo *) Xmalloc(2 * sizeof(ParseInfo));
    }

    if (new_list == NULL)
        goto err;

    new_list[num] = new;
    new_list[num + 1] = NULL;
    gen->mb_parse_list = new_list;
    gen->mb_parse_list_num = num + 1;

    ch = (unsigned char) *str;
    if (gen->mb_parse_table[ch] == 0)
        gen->mb_parse_table[ch] = num + 1;

    new->type = type;
    new->encoding = str;
    new->codeset = codeset;

    if (codeset->parse_info == NULL)
        codeset->parse_info = new;

    return True;

err:
    Xfree(str);
    if (new)
        Xfree(new);

    return False;
}

static void
free_charset(lcd)
    XLCd lcd;
{
    XLCdGenericPart *gen = XLC_GENERIC_PART(lcd);
    ParseInfo *parse_info;
    int num;

    if (gen->mb_parse_table)
        Xfree(gen->mb_parse_table);
    if (num = gen->mb_parse_list_num) {
        for (parse_info = gen->mb_parse_list; num-- > 0; parse_info++) {
            if ((*parse_info)->encoding)
                Xfree((*parse_info)->encoding);
            Xfree(*parse_info);
        }
        Xfree(gen->mb_parse_list);
    }

    if (num = gen->codeset_num)
        Xfree(gen->codeset_list);
}

static Bool
load_generic(lcd)
    XLCd lcd;
{
    XLCdGenericPart *gen = XLC_GENERIC_PART(lcd);
    char **value;
    int num;
    unsigned long l;
    int i;

    gen->codeset_num = 0;

    /***** wc_encoding_mask *****/
    _XlcGetResource(lcd, "XLC_XLOCALE", "wc_encoding_mask", &value, &num);
    if (num > 0) {
	if (string_to_ulong(value[0], &l) == False) 
	    goto err;
	gen->wc_encode_mask = l;
    }
    /***** wc_shift_bits *****/
    _XlcGetResource(lcd, "XLC_XLOCALE", "wc_shift_bits", &value, &num);
    if (num > 0)
	gen->wc_shift_bits = atoi(value[0]);
    if (gen->wc_shift_bits < 1)
	gen->wc_shift_bits = 8;
#ifndef X_NOT_STDC_ENV
    /***** use_stdc_env *****/
    _XlcGetResource(lcd, "XLC_XLOCALE", "use_stdc_env", &value, &num);
    if (num > 0 && !_XlcCompareISOLatin1(value[0], "True"))
	gen->use_stdc_env = True;
    else
	gen->use_stdc_env = False;
    /***** force_convert_to_mb *****/
    _XlcGetResource(lcd, "XLC_XLOCALE", "force_convert_to_mb", &value, &num);
    if (num > 0 && !_XlcCompareISOLatin1(value[0], "True"))
	gen->force_convert_to_mb = True;
    else
	gen->force_convert_to_mb = False;
#endif
    
    for (i = 0; ; i++) {
	CodeSetRec *codeset = NULL;
	char cs[16];
	char name[BUFSIZ];

	sprintf(cs, "cs%d", i);

	/***** codeset.side *****/
	sprintf(name, "%s.%s", cs , "side");
	_XlcGetResource(lcd, "XLC_XLOCALE", name, &value, &num);
	if (num > 0) {
	    char *tmp;

	    if (codeset == NULL && (codeset = add_codeset(gen)) == NULL)
		goto err;
	    codeset->side = _XlcNCompareISOLatin1(value[0], "GL", 2) ?
			    XlcGR : XlcGL;
	    tmp = strrchr(value[0], ':');
	    if (tmp != NULL && !_XlcCompareISOLatin1(tmp + 1, "Default")) {
		if (codeset->side == XlcGR)
		    gen->initial_state_GR = codeset;
		else
		    gen->initial_state_GL = codeset;
	    }
	}

	/***** codeset.length *****/
	sprintf(name, "%s.%s", cs , "length");
	_XlcGetResource(lcd, "XLC_XLOCALE", name, &value, &num);
	if (num > 0) {
	    if (codeset == NULL && (codeset = add_codeset(gen)) == NULL)
		goto err;
	    codeset->length = atoi(value[0]);
	    if (codeset->length < 1)
		codeset->length = 1;
	}

	/***** codeset.mb_encoding *****/
	sprintf(name, "%s.%s", cs, "mb_encoding");
	_XlcGetResource(lcd, "XLC_XLOCALE", name, &value, &num);
	if (num > 0) {
	    static struct { 
		char *str;
		int type;
	    } shifts[] = {
		{"<SS>", E_SS},
		{"<LSL>", E_LSL},
		{"<LSR>", E_LSR},
		0
	    };
	    int j;

	    if (codeset == NULL && (codeset = add_codeset(gen)) == NULL)
		goto err;
	    for ( ; num-- > 0; value++) {
		char encoding[256];
		char *tmp = *value;
		int type = E_SS;    /* for BC */
		for (j = 0; shifts[j].str; j++) {
		    if (!_XlcNCompareISOLatin1(tmp, shifts[j].str,
					       strlen(shifts[j].str))) {
			type = shifts[j].type;
			tmp += strlen(shifts[j].str);
			break;
		    }
		}
		if (string_to_encoding(tmp, encoding) == False)
			goto err;
		add_parse_list(gen, type, encoding, codeset);
	    }
	}

	/***** codeset.wc_encoding *****/
	sprintf(name, "%s.%s", cs, "wc_encoding");
	_XlcGetResource(lcd, "XLC_XLOCALE", name, &value, &num);
	if (num > 0) {
	    if (codeset == NULL && (codeset = add_codeset(gen)) == NULL)
		goto err;
	    if (string_to_ulong(value[0], &l) == False) 
		goto err;
	    codeset->wc_encoding = l;
	}
  
	/***** codeset.ct_encoding *****/
	sprintf(name, "%s.%s", cs, "ct_encoding");
	_XlcGetResource(lcd, "XLC_XLOCALE", name, &value, &num);
	if (num > 0) {
	    XlcCharSet charset;
	    char *encoding;

	    if (codeset == NULL && (codeset = add_codeset(gen)) == NULL)
		goto err;
	    for ( ; num-- > 0; value++) {
		string_to_encoding(*value, name);
		charset = NULL;
		if ((encoding = strchr(name, ':')) &&
		    (encoding = strchr(encoding + 1, ':'))) {
		    *encoding++ = '\0';
		    charset = _XlcAddCT(name, encoding);
		}
		if (charset == NULL) {
		    charset = _XlcGetCharSet(name);
		    if (charset == NULL &&
			(charset = _XlcCreateDefaultCharSet(name, ""))) {
			charset->side = codeset->side;
			charset->char_size = codeset->length;
			_XlcAddCharSet(charset);
		    }
		}
		if (charset) {
		    if (add_charset(codeset, charset) == False)
			goto err;
		}
	    }
	}

	if (codeset == NULL)
	    break;
	codeset->cs_num = i;
    }
	 
    return True;

err:
    free_charset(lcd);

    return False;
}

static Bool
initialize(lcd)
    XLCd lcd;
{
    XLCdPublicMethods superclass = (XLCdPublicMethods) _XlcPublicMethods;

    XLC_PUBLIC_METHODS(lcd)->superclass = superclass;

    if (superclass->pub.initialize) {
	if ((*superclass->pub.initialize)(lcd) == False)
	    return False;
    }

    if(load_generic(lcd) == False)
	return False;

    return True;
}

static void
destroy(lcd)
    XLCd lcd;
{
    XLCdPublicMethods superclass = XLC_PUBLIC_METHODS(lcd)->superclass;

    if (superclass && superclass->pub.destroy)
	(*superclass->pub.destroy)(lcd);
}
