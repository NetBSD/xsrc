/*
Copyright (c) 2001 by Juliusz Chroboczek

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
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <X11/fonts/fontenc.h>
#include "charset.h"
#include "parser.h"

#ifndef NULL
#define NULL 0
#endif

static unsigned int
IdentityRecode(unsigned int n, CharsetPtr self)
{
    return n;
}

static int
IdentityReverse(unsigned int n, CharsetPtr self)
{
#define IS_GL(n) ((n) >= 0x20 && (n) < 0x80)
    switch(self->type) {
    case T_94:
    case T_96:
        if (IS_GL(n)) return n; else return -1;
    case T_128:
        if (n < 0x80) return n; else return -1;
    case T_9494:
    case T_9696:
        if(IS_GL(n>>8) && IS_GL(n&0xFF))
            return n;
        else
            return -1;
    case T_94192:
        if(IS_GL(n>>8) && IS_GL(n&0x7F))
            return n;
        else
            return -1;
    default:
        abort();
    }
#undef IS_GL
}

static int
NullReverse(unsigned int n, CharsetPtr self)
{
    return -1;
}

CharsetRec Unknown94Charset = 
{ "Unknown (94)", T_94, 0, IdentityRecode, NullReverse, 0, 0};
CharsetRec Unknown96Charset = 
{ "Unknown (96)", T_96, 0, IdentityRecode, NullReverse, 0, 0};
CharsetRec Unknown9494Charset = 
{ "Unknown (94x94)", T_9494, 0, IdentityRecode, NullReverse, 0, 0};
CharsetRec Unknown9696Charset = 
{ "Unknown (96x96)", T_9696, 0, IdentityRecode, NullReverse, 0, 0};

typedef struct _FontencCharset {
    char *name;
    int type;
    unsigned char final;
    char *xlfd;
    int shift;
    FontMapPtr mapping;
    FontMapReversePtr reverse;
} FontencCharsetRec, *FontencCharsetPtr;

FontencCharsetRec fontencCharsets[] = {
    {"ISO 646 (1973)", T_94, '@', "iso646.1973-0", 0x00, 0, 0},
    {"ASCII", T_94, 'B', "iso8859-1", 0x00, 0, 0},
    {"JIS X 0201:GL", T_94, 'J', "jisx0201.1976-0", 0x00, 0, 0},
    {"JIS X 0201:GR", T_94, 'I', "jisx0201.1976-0", 0x80, 0, 0},
    {"DEC Special", T_94, '0', "dec-special", 0x00, 0, 0},
    {"DEC Technical", T_94, '>', "dec-dectech", 0x00, 0, 0},

    {"ISO 8859-1", T_96, 'A', "iso8859-1", 0x80, 0, 0},
    {"ISO 8859-2", T_96, 'B', "iso8859-2", 0x80, 0, 0},
    {"ISO 8859-3", T_96, 'C', "iso8859-3", 0x80, 0, 0},
    {"ISO 8859-4", T_96, 'D', "iso8859-4", 0x80, 0, 0},
    {"ISO 8859-5", T_96, 'L', "iso8859-5", 0x80, 0, 0},
    {"ISO 8859-6", T_96, 'G', "iso8859-6", 0x80, 0, 0},
    {"ISO 8859-7", T_96, 'F', "iso8859-7", 0x80, 0, 0},
    {"ISO 8859-8", T_96, 'H', "iso8859-8", 0x80, 0, 0},
    {"ISO 8859-9", T_96, 'M', "iso8859-9", 0x80, 0, 0},
    {"ISO 8859-15", T_96, 'b', "iso8859-15", 0x80, 0, 0},
    {"KOI8-E", T_96, '@', "koi8-e", 0x80, 0, 0},

    {"GB 2312", T_9494, 'A', "gb2312.1980-0", 0x0000, 0, 0},
    {"JIS X 0208", T_9494, 'B', "jisx0208.1990-0", 0x0000, 0, 0},
    {"KSC 5601", T_9494, 'C', "ksc5601.1987-0", 0x0000, 0, 0},
    {"JIS X 0212", T_9494, 'D', "jisx0212.1990-0", 0x0000, 0, 0},

    {"GB 2312", T_9696, 'A', "gb2312.1980-0", 0x0000, 0, 0},
    {"JIS X 0208", T_9696, 'B', "jisx0208.1990-0", 0x0000, 0, 0},
    {"KSC 5601", T_9696, 'C', "ksc5601.1987-0", 0x0000, 0, 0},
    {"JIS X 0212", T_9696, 'D', "jisx0212.1990-0", 0x0000, 0, 0},

    {"KOI8-R", T_128, 0, "koi8-r", 0x80, 0, 0},
    {"KOI8-U", T_128, 0, "koi8-u", 0x80, 0, 0},
    {"KOI8-RU", T_128, 0, "koi8-ru", 0x80, 0, 0},
    {"CP 1252", T_128, 0, "microsoft-cp1252", 0x80, 0, 0},
    {"CP 1251", T_128, 0, "microsoft-cp1251", 0x80, 0, 0},
    {"CP 1250", T_128, 0, "microsoft-cp1250", 0x80, 0, 0},

    {"Big 5", T_94192, 0, "big5.eten-0", 0x8000, 0, 0},
    {0, 0, 0, 0, 0, 0, 0}
};



static int
compare(char *s, char *t)
{
    while(*s || *t) {
        if(*s && (isspace(*s) || *s == '-' || *s == '_'))
            s++;
        else if(*t && (isspace(*t) || *t == '-' || *t == '_'))
            t++;
        else if(*s && *t && tolower(*s) == tolower(*t)) {
            s++; 
            t++;
        } else
            return 1;
    }
    return 0;
}

static unsigned int
FontencCharsetRecode(unsigned int n, CharsetPtr self)
{
    FontencCharsetPtr fc = (FontencCharsetPtr)(self->data);

    return FontEncRecode(n + fc->shift, fc->mapping);
}

static int
FontencCharsetReverse(unsigned int i, CharsetPtr self)
{
    FontencCharsetPtr fc = (FontencCharsetPtr)(self->data);
    int n;

    n = fc->reverse->reverse(i, fc->reverse->data);
    if(n == 0 || n < fc->shift)
        return -1;
    else
        n -= fc->shift;

#define IS_GL(n) ((n) >= 0x20 && (n) < 0x80)
    switch(self->type) {
    case T_94: case T_96:
        if (IS_GL(n)) return n; else return -1;
        break;
    case T_128:
        if (n < 0x80) return n; else return -1;
    case T_9494: case T_9696:
        if(IS_GL(n>>8) && IS_GL(n&0xFF))
            return n;
        else
            return -1;
        break;
    case T_94192:
        if(IS_GL(n>>8) && IS_GL(n&0x7F))
            return n;
        else
            return -1;
        break;
    default:
        abort();
    }
#undef IS_GL
}


static CharsetPtr cachedCharsets = NULL;

static CharsetPtr 
getCachedCharset(unsigned char final, int type, char *name)
{
    CharsetPtr c;
    for(c = cachedCharsets; c; c = c->next) {
        if(((c->type == type && c->final == final) ||
            (name && !compare(c->name, name))) &&
           (c->type != T_FAILED))
            return c;
    }
    return NULL;
}

static void
cacheCharset(CharsetPtr c) {
    c->next = cachedCharsets;
    cachedCharsets = c;
}

static CharsetPtr
getFontencCharset(unsigned char final, int type, char *name)
{
    FontencCharsetPtr fc;
    CharsetPtr c;
    FontMapPtr mapping;
    FontMapReversePtr reverse;

    fc = fontencCharsets;
    while(fc->name) {
        if(((fc->type == type && fc->final == final) ||
            (name && !compare(fc->name, name))) &&
           (fc->type != T_FAILED))
            break;
        fc++;
    }

    if(!fc->name)
        return NULL;
    
    c = malloc(sizeof(CharsetRec));
    if(c == NULL)
        return NULL;

    mapping = FontEncMapFind(fc->xlfd, FONT_ENCODING_UNICODE, -1, -1, NULL);
    if(!mapping) {
        fc->type = T_FAILED;
        return NULL;
    }

    reverse = FontMapReverse(mapping);
    if(!reverse) {
        fc->type = T_FAILED;
        return NULL;
    }

    fc->mapping = mapping;
    fc->reverse = reverse;

    c->name = fc->name;
    c->type = fc->type;
    c->final = fc->final;
    c->recode = FontencCharsetRecode;
    c->reverse = FontencCharsetReverse;
    c->data = fc;

    cacheCharset(c);
    return c;
}

CharsetPtr 
getUnknownCharset(type)
{
    switch(type) {
    case T_94: return &Unknown94Charset;
    case T_96: return &Unknown96Charset;
    case T_9494: return &Unknown9494Charset;
    case T_9696: return &Unknown9696Charset;
    default: return &Unknown94Charset;
    }
}

CharsetPtr 
getCharset(unsigned char final, int type)
{
    CharsetPtr c;

    c = getCachedCharset(final, type, NULL);
    if(c)
        return c;

    c = getFontencCharset(final, type, NULL);
    if(c)        
        return c;

    return getUnknownCharset(type);
}

CharsetPtr 
getCharsetByName(char *name)
{
    CharsetPtr c;

    if(name == NULL)
        return getUnknownCharset(T_94);

    c = getCachedCharset(0, 0, name);
    if(c)
        return c;

    c = getFontencCharset(0, 0, name);
    if(c)        
        return c;

    return getUnknownCharset(T_94);
}

LocaleCharsetRec localeCharsets[] = {
    { "C", 0, 2, "ASCII", NULL, "ISO 8859-1", NULL},
    { "ISO8859-1", 0, 2, "ASCII", NULL, "ISO 8859-1", NULL},
    { "ISO8859-2", 0, 2, "ASCII", NULL, "ISO 8859-2", NULL},
    { "ISO8859-3", 0, 2, "ASCII", NULL, "ISO 8859-3", NULL},
    { "ISO8859-4", 0, 2, "ASCII", NULL, "ISO 8859-4", NULL},
    { "ISO8859-5", 0, 2, "ASCII", NULL, "ISO 8859-5", NULL},
    { "ISO8859-6", 0, 2, "ASCII", NULL, "ISO 8859-6", NULL},
    { "ISO8859-7", 0, 2, "ASCII", NULL, "ISO 8859-7", NULL},
    { "ISO8859-8", 0, 2, "ASCII", NULL, "ISO 8859-8", NULL},
    { "ISO8859-9", 0, 2, "ASCII", NULL, "ISO 8859-9", NULL},
    { "ISO8859-15", 0, 2, "ASCII", NULL, "ISO 8859-15", NULL},
    { "KOI8-R", 0, 2, "ASCII", NULL, "KOI8-R", NULL},
    { "CP1251", 0, 2, "ASCII", NULL, "CP 1251", NULL},
    { "eucCN", 0, 1, "ASCII", "GB 2312", NULL, NULL},
    { "GB2312", 0, 1, "ASCII", "GB 2312", NULL, NULL},
    { "eucJP", 0, 1, "ASCII", "JIS X 0208", "JIS X 0201:GR", "JIS X 0212"},
    { "eucKR", 0, 1, "ASCII", "KSC 5601", NULL, NULL},
    { "eucCN", 0, 1, "ASCII", "GB 2312", NULL, NULL},
    { "Big5", 0, 1, "ASCII", "Big 5", NULL, NULL},
    { 0, 0, 0, 0, 0, 0, 0}
};

void
reportCharsets()
{
    LocaleCharsetPtr p;
    FontencCharsetPtr q;
    printf("Known locale encodings:\n\n");
    for(p = localeCharsets; p->name; p++) {
        printf("  %s: GL -> G%d, GR -> G%d", p->name, p->gl, p->gr);
        if(p->g0) printf(", G0: %s", p->g0);
        if(p->g1) printf(", G1: %s", p->g1);
        if(p->g2) printf(", G2: %s", p->g2);
        if(p->g3) printf(", G3: %s", p->g3);
        printf("\n");
    }

    printf("\n\nKnown charsets (not all may be available):\n\n");
    for(q = fontencCharsets; q->name; q++)
        printf("  %s%s\n", 
               q->name, q->final?" (ISO 2022)":"");
}

int
getLocaleState(char *locale, 
               int *gl_return, int *gr_return,
               CharsetPtr *g0_return, CharsetPtr *g1_return,
               CharsetPtr *g2_return, CharsetPtr *g3_return)
{
    char *resolved, *charset;
    LocaleCharsetPtr p;
    resolved = resolveLocale(locale);
    if(!resolved)
        return -1;

    charset = strrchr(resolved, '.');
    if(charset)
        charset++;
    else
        charset = resolved;

    for(p = localeCharsets; p->name; p++) {
        if(!strcmp(p->name, charset))
            break;
    }

    if(p->name == NULL) {
        free(resolved);
        return -1;
    }

    *gl_return = p->gl;
    *gr_return = p->gr;
    *g0_return = getCharsetByName(p->g0);
    *g1_return = getCharsetByName(p->g1);
    *g2_return = getCharsetByName(p->g2);
    *g3_return = getCharsetByName(p->g3);

    return 0;
}

