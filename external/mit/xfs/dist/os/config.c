/*
Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

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
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation 
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices,
 * or Digital not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Network Computing Devices, or Digital
 * make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES, AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, OR DIGITAL BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include	"config.h"

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<X11/Xtrans/Xtrans.h>
#include	<X11/Xos.h>
#include	"misc.h"
#include	"configstr.h"
#include	"osdep.h"
#include	"globals.h"
#include	"access.h"
#include	"difsutils.h"
#include	"difs.h"
#include        <X11/fonts/libxfont2.h>

/* libXfont/src/bitmap/snfstr.h */
extern void SnfSetFormat(int bit, int byte, int glyph, int scan);

static const char * const default_config_files[] = {
#ifdef DEFAULT_CONFIG_FILE
    DEFAULT_CONFIG_FILE,
#else
    "/usr/lib/X11/fs/config",
#endif
    NULL
};

static char *font_catalogue = NULL;

static char *config_set_int(ConfigOptionPtr parm, char *val);
static char *config_set_bool(ConfigOptionPtr parm, char *val);
static char *config_set_catalogue(ConfigOptionPtr parm, char *val);
static char *config_set_glyph_caching_mode(ConfigOptionPtr parm, char *val);
static char *config_set_list(ConfigOptionPtr parm, char *val);
static char *config_set_file(ConfigOptionPtr parm, char *val);
static char *config_set_resolutions(ConfigOptionPtr parm, char *val);
static char *config_set_ignored_transports(ConfigOptionPtr parm, char *val);
static char *config_set_snf_format(ConfigOptionPtr parm, char *val);

/* these need to be in lower case and alphabetical order so a
 * binary search lookup can be used
 */
static ConfigOptionRec config_options[] = {
    {"alternate-servers", config_set_list},
    {"catalogue", config_set_catalogue},
    {"client-limit", config_set_int},
    {"clone-self", config_set_bool},
    {"default-point-size", config_set_int},
    {"default-resolutions", config_set_resolutions},
    {"deferglyphs", config_set_glyph_caching_mode},
    {"error-file", config_set_file},
    {"no-listen", config_set_ignored_transports},
    {"port", config_set_int},
    {"server-number", config_set_int},
    {"snf-format", config_set_snf_format},
    {"trusted-clients", config_set_list},
    {"use-syslog", config_set_bool},
    {NULL, NULL},
};

/* max size in bytes of config file */
#define	CONFIG_MAX_FILESIZE	32767

#define	CONFIG_ERR_MEMORY \
    "CONFIG: insufficient memory to load configuration file \"%s\"\n"
#define	CONFIG_ERR_OPEN "CONFIG: can't open configuration file \"%s\"\n"
#define	CONFIG_ERR_READ "CONFIG: error reading configuration file \"%s\"\n"
#define	CONFIG_ERR_VALUE "CONFIG: bad value \"%s\" for parameter \"%s\"\n"
#define	CONFIG_ERR_UNKNOWN "CONFIG: unknown parameter \"%s\"\n"
#define	CONFIG_ERR_NOEQUALS "CONFIG: missing '=' after parameter \"%s\"\n"
#define	CONFIG_ERR_RANGE "CONFIG: value out of range for parameter \"%s\"\n"
#define	CONFIG_ERR_SYNTAX "CONFIG: syntax error near parameter \"%s\"\n"
#define	CONFIG_ERR_NOVALUE "CONFIG: missing value for parameter \"%s\"\n"
#define	CONFIG_ERR_EXTRAVALUE "CONFIG: extra value for parameter \"%s\"\n"

#define	iseol(c)	((c) == '\n' || (c) == '\r' || (c) == '\f')
#define	skip_whitespace(c)	while(isspace(*(c)) || *(c) == ',') (c)++;
#define	skip_val(c)	while(!isspace(*(c)) && *(c) != ',' && *(c) != '\0')\
						(c) ++;
#define	skip_list_val(c)	while(!isspace(*(c)) && *(c) != '\0')\
						(c) ++;
#define	blank_comment(c)	while (!iseol(*(c)) && *(c) != '\0')	\
						*(c)++= ' ';

static char *
next_assign(char *c)
{
    int         nesting = 0;

    while (*c != '\0') {
	if (*c == '(')
	    nesting++;
	else if (*c == ')')
	    nesting--;
	else if (*c == '=' && nesting == 0)
	    return c;
	c++;
    }
    return (char *) 0;
}

static void
strip_comments(char *data)
{
    char       *c;

    c = data;
    while ((c = strchr(c, '#')) != NULL) {
	if (c == data || *(c - 1) != '\\') {
	    blank_comment(c);
	} else {
	    c++;
	}
    }
}

static ConfigOptionPtr
match_param_name(char *name)
{
    int         pos,
                rc,
                low,
                high;

    low = 0;
    high = sizeof(config_options) / sizeof(ConfigOptionRec) - 2;
    pos = high >> 1;

    while (low <= high) {
	rc = strcmp(name, config_options[pos].parm_name);
	if (rc == 0) {
	    return &config_options[pos];
	} else if (rc < 0) {
	    high = pos - 1;
	} else {
	    low = pos + 1;
	}
	pos = ((high + low) >> 1);
    }
    return NULL;
}

static int
parse_config(char *data)
{
    char       *c,
               *val = NULL,
               *next_eq,
               *consumed,
               *p;
    char        param_name[64];
    Bool        equals_missing;
    ConfigOptionPtr param;

    c = data;
    skip_whitespace(c);

    while (*c != '\0') {
	equals_missing = FALSE;

	/* get parm name in lower case */
	p = c;
	while (isalnum(*c) || *c == '-') {
	    if (isupper(*c))
		*c = tolower(*c);
	    c++;
	}
	memcpy(param_name, p, min(sizeof(param_name), (int) (c - p)));
	param_name[(int) (c - p)] = '\0';

	/* check for junk */
	if (!isspace(*c) && *c != '=') {
	    ErrorF(CONFIG_ERR_SYNTAX, param_name);
	    /* eat garbage */
	    while (!isspace(*c) && *c != '=' && *c != '\0')
		c++;
	}
	skip_whitespace(c);
	if (*c != '=') {
	    ErrorF(CONFIG_ERR_NOEQUALS, param_name);
	    equals_missing = TRUE;
	} else {
	    c++;
	}

	skip_whitespace(c);

	/* find next assignment to guess where the value ends */
	if ((next_eq = next_assign(c)) != NULL) {
	    /* back up over whitespace */
	    for (val = next_eq - 1; val >= c &&
		    (isspace(*val) || *val == ',');
		    val--);

	    /* back over parm name */
	    for (; val >= c && (isalnum(*val) || *val == '-'); val--);

	    if (val <= c) {
		/* no value, ignore */
		ErrorF(CONFIG_ERR_NOVALUE, param_name);
		continue;
	    }
	    *val = '\0';
	} else if (*c == '\0') {
	    /* no value, ignore */
	    ErrorF(CONFIG_ERR_NOVALUE, param_name);
	    continue;
	}
	/* match parm name */
	if (equals_missing) {
	    equals_missing = FALSE;
	} else if ((param = match_param_name(param_name)) == NULL) {
	    ErrorF(CONFIG_ERR_UNKNOWN, param_name);
	} else {
	    consumed = (param->set_func) (param, c);

	    skip_whitespace(consumed);
	    if (*consumed != '\0') {
		ErrorF(CONFIG_ERR_EXTRAVALUE,
		       param_name);
	    }
	}

	if (next_eq != NULL)
	    c = val + 1;
	else			/* last setting */
	    break;
    }
    return FSSuccess;
}

/*
 * handles anything that should be set once the file is parsed
 */
void
SetConfigValues(void)
{
    int         err,
                num;

    if (font_catalogue == NULL) {
	FatalError("font catalogue is missing/empty\n");
    }

    err = SetFontCatalogue(font_catalogue, &num);
    if (err != FSSuccess) {
	FatalError("element #%d (starting at 0) of font path is bad or has a bad font:\n\"%s\"\n",
		   num, font_catalogue);
    }
    InitErrors();
    fsfree((char *) font_catalogue);
    font_catalogue = NULL;
}


/* If argument is NULL, uses first file found from default_config_files */
int
ReadConfigFile(const char *filename)
{
    FILE       *fp = NULL;
    int         ret;
    int         len;
    int         i;
    char       *data;

    data = (char *) fsalloc(CONFIG_MAX_FILESIZE);
    if (!data) {
	ErrorF(CONFIG_ERR_MEMORY, filename);
	return FSBadAlloc;
    }
    if (filename != NULL) {
	fp = fopen(filename, "r");
	if (fp == NULL) {
	    ErrorF(CONFIG_ERR_OPEN, filename);
	}
    } else {
	for (i = 0; default_config_files[i] != NULL; i++) {
	    filename = default_config_files[i];
	    if ((fp = fopen(filename, "r")) != NULL) {
		if (configfilename == NULL) {
		    configfilename = strdup(filename); /* save for clones */
		}
		break;
	    }
	}
	if (fp == NULL) {
	    for (i = 0; default_config_files[i] != NULL; i++) {
		ErrorF(CONFIG_ERR_OPEN, default_config_files[i]);
	    }
	}
    }
    if (fp == NULL) {
	fsfree(data);
	return FSBadName;
    }
    ret = fread(data, sizeof(char), CONFIG_MAX_FILESIZE, fp);
    if (ret <= 0) {
	fsfree(data);
	(void) fclose(fp);
	ErrorF(CONFIG_ERR_READ, filename);
	return FSBadName;
    }
    len = ftell(fp);
    len = min(len, CONFIG_MAX_FILESIZE);
    data[len] = '\0';		/* NULL terminate the data */

    (void) fclose(fp);

    strip_comments(data);
    ret = parse_config(data);

    fsfree(data);

    return ret;
}

struct nameVal {
    const char *name;
    int         val;
};

static char *
config_parse_nameVal (
    ConfigOptionPtr parm,
    char       *c,
    int        *ret,
    int		*pval,
    struct nameVal   *name_val)
{
    char       *start,
                t;
    int         i,
                len;

    start = c;
    skip_val(c);
    t = *c;
    *c = '\0';
    len = c - start;

    for (i = 0; name_val[i].name; i++) {
	if (!strncmpnocase(start, name_val[i].name, len)) {
	    *pval = name_val[i].val;
	    *ret = 0;
	    *c = t;
	    return c;
	}
    }
    ErrorF(CONFIG_ERR_VALUE, start, parm->parm_name);
    *c = t;
    *ret = -1;
    return c;
}

static char *
config_parse_bool (
    ConfigOptionPtr parm,
    char	*c,
    int		*ret,
    Bool	*pval)
{
    static struct nameVal bool_val[] = {
    	    { "yes",   TRUE },
    	    { "on",    TRUE },
    	    { "1",     TRUE },
    	    { "true",  TRUE },
    	    { "no",    FALSE },
    	    { "off",   FALSE },
    	    { "0",     FALSE },
    	    { "false", FALSE },
    	    { (char *) 0, 0 },
    };
    return config_parse_nameVal (parm, c, ret, pval, bool_val);
}

static char *
config_parse_int(
    ConfigOptionPtr parm,
    char       *c,
    int        *ret,
    int        *pval)
{
    char       *start,
                t;

    start = c;
    while (*c != '\0' && !isspace(*c) && *c != ',') {
	if (!isdigit(*c)) {	/* error */
	    skip_val(c);
	    t = *c;
	    *c = '\0';
	    ErrorF(CONFIG_ERR_VALUE, start, parm->parm_name);
	    *ret = -1;
	    *c = t;
	    return c;
	}
	c++;
    }
    t = *c;
    *c = '\0';
    *ret = 0;
    *pval = atoi(start);
    *c = t;
    return c;
}


/* config option sets */
/* these have to know how to do the real work and tweak the proper things */
static char *
config_set_int(
    ConfigOptionPtr parm,
    char       *val)
{
    int         ival,
                ret;

    val = config_parse_int(parm, val, &ret, &ival);
    if (ret == -1)
	return val;

    /* now do individual attribute checks */
    if (!strcmp(parm->parm_name, "port") && !portFromCmdline) {
	ListenPort = ival;
    } else if (!strcmp(parm->parm_name, "client-limit")) {
	AccessSetConnectionLimit(ival);
    } else if (!strcmp(parm->parm_name, "default-point-size")) {
	SetDefaultPointSize(ival);
    }
    return val;
}

static char *
config_set_bool(
    ConfigOptionPtr parm,
    char       *val)
{
    int
                ret;
    Bool        bval;

    val = config_parse_bool(parm, val, &ret, &bval);
    if (ret == -1)
	return val;

    /* now do individual attribute checks */
    if (!strcmp(parm->parm_name, "use-syslog")) {
	UseSyslog = bval;
    } else if (!strcmp(parm->parm_name, "clone-self")) {
	CloneSelf = bval;
    }
    return val;
}

static char *
config_set_file(
    ConfigOptionPtr parm,
    char       *val)
{
    char       *start = val,
                t;

    skip_val(val);
    t = *val;
    *val = '\0';
    if (!strcmp(parm->parm_name, "error-file")) {
	memmove( ErrorFile, start, val - start + 1);
    }
    *val = t;
    return val;
}

static char *
config_set_catalogue(
    ConfigOptionPtr parm,
    char       *val)
{
    char       *b;

    if (!strcmp(parm->parm_name, "catalogue")) {
	/* stash it for later */
	fsfree((char *) font_catalogue);	/* dump any previous one */
	b = font_catalogue = (char *) fsalloc(strlen(val) + 1);
	if (!font_catalogue)
	    FatalError("insufficient memory for font catalogue\n");
	while (*val) {		/* remove all the gunk */
	    if (!isspace(*val)) {
		*b++ = *val;
	    }
	    val++;
	}
	*b = '\0';
    }
    return val;
}

static char *
config_set_list(
    ConfigOptionPtr parm,
    char       *val)
{
    char       *start = val,
                t;

    skip_list_val(val);
    t = *val;
    *val = '\0';
    if (!strcmp(parm->parm_name, "alternate-servers")) {
	SetAlternateServers(start);
    }
    *val = t;
    return val;
}

static char *
config_set_ignored_transports(
    ConfigOptionPtr parm,
    char       *val)
{
    char       *start = val,
                t;

    skip_list_val(val);
    t = *val;
    *val = '\0';
    _FontTransNoListen(start);
    *val = t;
    return val;
}

static char *
config_set_glyph_caching_mode(
    ConfigOptionPtr parm,
    char       *val)
{
    char       *start = val,
                t;

    skip_list_val(val);
    t = *val;
    *val = '\0';
    if (!strcmp(parm->parm_name, "deferglyphs")) {
	xfont2_parse_glyph_caching_mode(start);
    }
    *val = t;
    return val;
}

static char *
config_set_resolutions(
    ConfigOptionPtr parm,
    char       *val)
{
    char       *start = val,
                t;
    int         err;

    skip_list_val(val);
    t = *val;
    *val = '\0';
    if (!strcmp(parm->parm_name, "default-resolutions")) {
	err = SetDefaultResolutions(start);
	if (err != FSSuccess) {
	    FatalError("bogus resolution list \"%s\"\n", start);
	}
    }
    *val = t;
    return val;
}


static char *
config_parse_endian(
    ConfigOptionPtr parm,
    char       *c,
    int        *ret,
    int		*pval)
{
    static struct nameVal endian_val[] = {
	{ "lsb",      LSBFirst },
	{ "little",   LSBFirst },
	{ "lsbfirst", LSBFirst },
	{ "msb",      MSBFirst },
	{ "big",      MSBFirst },
	{ "msbfirst", MSBFirst },
	{ (char *) 0, 0 },
    };
    return config_parse_nameVal (parm, c, ret, pval, endian_val);
}

/* ARGSUSED */
static char *
config_set_snf_format (
    ConfigOptionPtr parm,
    char	    *val)
{
    int	    bit, byte, glyph, scan;
    int	    ret;
    
    val = config_parse_endian (parm, val, &ret, &bit);
    if (ret == -1)
	return val;
    skip_whitespace (val);
    val = config_parse_endian (parm, val, &ret, &byte);
    if (ret == -1)
	return val;
    skip_whitespace (val);
    val = config_parse_int (parm, val, &ret, &glyph);
    if (ret == -1)
	return val;
    skip_whitespace (val);
    val = config_parse_int (parm, val, &ret, &scan);
    if (ret == -1)
	return val;
#ifdef XFONT_SNFFORMAT
    SnfSetFormat (bit, byte, glyph, scan);
#endif
    return val;
}
