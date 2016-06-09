/* $NetBSD: x68kConfig.c,v 1.1.1.1 2016/06/09 09:07:59 mrg Exp $ */
/*-------------------------------------------------------------------------
 * Copyright (c) 1996 Yasushi Yamasaki
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Yasushi Yamasaki
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------*/

#include <stdarg.h>
#include "x68k.h"
#include "opaque.h"

static PixmapFormatRec *x68kFormat = NULL;
static PixmapFormatRec defaultFormat = {
	1,
	1,
	BITMAP_SCANLINE_PAD
};
static X68kScreenRec x68kScreen[X68K_FB_TYPES];
static X68kFbProcRec x68kFbProc[X68K_FB_TYPES];

/*-------------------------------------------------------------------------
 * function "x68kGetScreenRec"
 *
 *  purpose:  get corresponding screen record
 *  argument: (int)sindex       : screen index
 *  returns:  (X68kScreenRec *) : X68k dependent screen record
 *-----------------------------------------------------------------------*/
X68kScreenRec *
x68kGetScreenRec(int sindex)
{
    return &x68kScreen[sindex];
}

/*-------------------------------------------------------------------------
 * function "x68kGetScreenRecByType"
 *
 *  purpose:  search screen record by type
 *  argument: (int)type         : screen type
 *  returns:  (X68kScreenRec *) : X68k dependent screen record
 *-----------------------------------------------------------------------*/
X68kScreenRec *
x68kGetScreenRecByType(int type)
{
    int i;
    
    for (i = 0; i < X68K_FB_TYPES; i++) {
        if (x68kScreen[i].type == type)
            return &x68kScreen[i];
    }
    return NULL;
}

/*-------------------------------------------------------------------------
 * function "x68kGetFbProcRec"
 *
 *  purpose:  get corresponding frame buffer procedure record
 *  argument: (int)sindex       : screen index
 *  returns:  (X68kFbProcRec *) : frame buffer procedure record
 *-----------------------------------------------------------------------*/
X68kFbProcRec *
x68kGetFbProcRec(int sindex)
{
    return &x68kFbProc[sindex];
}

/*-------------------------------------------------------------------------
 * function "x68kRegisterPixmapFormats"
 *
 *  purpose:  register pixmap formats into ScreenInfo struct
 *  argument: (ScreenInfo *)pScreenInfo
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
x68kRegisterPixmapFormats(ScreenInfo *pScreenInfo)
{
    /* supports only one pixmap format for each screen type */
    pScreenInfo->numPixmapFormats = 1;
    pScreenInfo->formats[0] = defaultFormat;

    if (x68kFormat)
	pScreenInfo->formats[pScreenInfo->numPixmapFormats++] = *x68kFormat;
}

/*-------------------------------------------------------------------------
 * function "x68kConfig"
 *
 *  purpose:  process general configuration by reading "X68kConfig" file
 *            <X11_LIBDIR> is the default location of this file
 *  argument: nothing
 *  returns:  the number of screens
 *-----------------------------------------------------------------------*/
const char *hostConfigFilename = "/etc/X68kConfig";
const char *siteConfigFilename = X11_LIBDIR "/X68kConfig";
const char *configFilename = NULL;
static FILE *config;
char modeSet = FALSE;

static int parseCommand(void);

int
x68kConfig(void)
{
    if (configFilename)
	config = fopen(configFilename, "r");
    else {
	configFilename = hostConfigFilename;
	config = fopen(configFilename, "r");
	if (config == NULL) {
	    configFilename = siteConfigFilename;
	    config = fopen(configFilename, "r");
	}
    }
    if (config == NULL)
	FatalError("Can't open X68kConfig file");
    while (parseCommand())
        ;
    fclose(config);
    if (!modeSet)
        FatalError("No mode set.");
    return 1;
}


/*-------------------------------------------------------------------------
 *                       X68KConfig parsing part
 *-----------------------------------------------------------------------*/
void parseError(int line, const char *str, ...);

enum TokenType {
    TOKEN_EOF,
    TOKEN_SYMBOL,
    TOKEN_LITERAL,
    TOKEN_OPEN_PARENTHESIS,
    TOKEN_CLOSE_PARENTHESIS
};

typedef struct {
    enum TokenType type;
    int line;
    union {
        char *symbol;
        int literal;
    } content;
} Token;

/*-------------------------------------------------------------------------
 * function "getToken"
 *
 *  purpose:  cut out a token from configration file stream
 *  argument: nothing
 *  returns:  (Token *)  : cut token
 *-----------------------------------------------------------------------*/
static Token *
getToken(void)
{
    int c;
    static int line = 1;
    Token *ret;
    
    ret = (Token *)malloc(sizeof(Token));
    if (ret == NULL)
        FatalError("Out of memory");
    while (TRUE) {
        /* slip white spaces */
        do {
            c = fgetc(config);
            if (c == '\n')
                line++;
        } while (isspace(c));
        if (c != ';')
            break;
        /* skip a comment */
        do {
            c = fgetc(config);
        } while (c != '\n');
        line++;
    }
    ret->line = line;
    if (c == EOF) {
        ret->type = TOKEN_EOF;
        return ret;
    }
    /* is a symbol? */
    if (isalpha(c)) {
        int i = 0;
        ret->content.symbol = (char *)malloc(32 * sizeof(char));
        if (ret->content.symbol == NULL)
            FatalError("Out of memory");
        do {
            if (i < 31)
                ret->content.symbol[i++] = c;
            c = fgetc(config);
        } while (isalnum(c) || c == '_');
        ungetc(c, config);
        ret->content.symbol[i] = '\0';
        ret->type = TOKEN_SYMBOL;
        return ret;
    }
    /* is a literal number? */
    if (isdigit(c)) {
        char tmp[32];
        int i = 0;
        do {
            if (i < 31)
                tmp[i++] = c;
            c = fgetc(config);
        } while (isdigit(c));
        ungetc(c, config);
        tmp[i] = '\0';
        if (sscanf(tmp, "%d", &ret->content.literal) != 1)
            parseError(line, "illegal literal value");
        ret->type = TOKEN_LITERAL;
        return ret;
    }
    /* others */
    switch(c) {
        case '(':
            ret->type = TOKEN_OPEN_PARENTHESIS;
            break;
        case ')':
            ret->type = TOKEN_CLOSE_PARENTHESIS;
            break;
        default:
            parseError(line, NULL);
    }
    return ret;
}

typedef struct {
    const char *symbol;
    void (*proc)(int argc, Token **argv);
} Command;

static void parseModeDef(int argc, Token **argv);
static void parseMouse(int argc, Token **argv);
static void parseKeyboard(int argc, Token **argv);
static void parseMode(int argc, Token **argv);

Command command[] = {
    { "ModeDef", parseModeDef },
    { "Mouse", parseMouse },
    { "Keyboard", parseKeyboard },
    { "Mode", parseMode },
};
#define NCOMMANDS (sizeof(command)/sizeof(command[0])) 

/*-------------------------------------------------------------------------
 * function "parseCommand"
 *
 *  purpose:  parse generic command. every command parsing departs here.
 *  argument: nothing
 *  returns:  (int) : FALSE if there's no rest
 *                    TRUE  otherwise
 *-----------------------------------------------------------------------*/
static int
parseCommand(void)
{
    Token **argv = 0, *token;
    int argc = 0;
    int i;
    
    token = getToken();
    if (token->type == TOKEN_EOF)
        return FALSE;
    if (token->type != TOKEN_OPEN_PARENTHESIS)
        parseError(token->line, "missing parenthesis");
    free(token);

    /* get command name and arguments */
    while (TRUE) {
        token = getToken();
        if (token->type == TOKEN_EOF)
            parseError(token->line, "reached EOF");
        if (token->type == TOKEN_CLOSE_PARENTHESIS) {
            free(token);
            break;
        }
        argc++;
        argv = (Token **)realloc(argv, sizeof(Token *) * argc);
        if (argv == NULL)
            FatalError("Out of memory");
        argv[argc-1] = token;
    }
    if (argc == 0)
        return TRUE;

    /* call corresponding command procedure */
    if (argv[0]->type != TOKEN_SYMBOL)
        parseError(argv[0]->line, "command name required");
    for (i = 0; i < NCOMMANDS; i++) {
        if (strcasecmp(command[i].symbol, argv[0]->content.symbol) == 0) {
            /* parse command */
            command[i].proc(argc, argv);        
            break;
        }
    }
    if (i == NCOMMANDS)
        parseError(argv[0]->line, "unknown command `%s'",
                   argv[0]->content.symbol);

    /* free arguments */
    for (i = 0; i < argc; i++) {
        if (argv[i]->type == TOKEN_SYMBOL)
            free(argv[i]->content.symbol);
        free(argv[i]);
    }
    free(argv);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * function "checkArguments"
 *
 *  purpose:  examine the number of arguments and the type of each
 *            argument.
 *  argument: (int)n                 : correct number of arguments
 *            (enum TokenType *)type : table of types
 *            (int)argc_m1           : actual number of arguments
 *            (Token **)argv         : command and arguments
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
checkArguments(int n, enum TokenType *type, int argc_m1, Token **argv)
{
    int i;
    
    if (argc_m1 < n)
        parseError(argv[0]->line, "too few arguments to command `%s'",
                   argv[0]->content.symbol);
    if (argc_m1 > n)
        parseError(argv[0]->line, "too many arguments to command `%s'",
                   argv[0]->content.symbol);
    for (i = 0; i < n; i++) {
        if (argv[i+1]->type != type[i])
            parseError(argv[i+1]->line,
                       "type mismatch. argument %d to command `%s'",
                       i+1, argv[0]->content.symbol);
    }
}

typedef struct _Mode {
    struct _Mode *next;
    char *name;
    int type;
    int depth;
    int class;
    int width, height;
    X68kFbReg reg;
} Mode;

Mode *modeList = NULL;

/*-------------------------------------------------------------------------
 * function "parseModeDef"
 *
 *  purpose:  define a mode
 *  argument: (int)argc, (Token **)argv : command and arguments
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
parseModeDef(int argc, Token **argv)
{
    enum TokenType argtype[] = {
        /* name       type          depth          class      */
        TOKEN_SYMBOL, TOKEN_SYMBOL, TOKEN_LITERAL, TOKEN_SYMBOL,
        /* width       height       */
        TOKEN_LITERAL, TOKEN_LITERAL,
        /* register values */
        TOKEN_LITERAL, TOKEN_LITERAL, TOKEN_LITERAL, TOKEN_LITERAL,
        TOKEN_LITERAL, TOKEN_LITERAL, TOKEN_LITERAL, TOKEN_LITERAL,
        TOKEN_LITERAL, TOKEN_LITERAL, TOKEN_LITERAL, TOKEN_LITERAL
    };
    Mode *mode;
    char *symbol;
    int class, width, height;
    
    checkArguments(18, argtype, argc-1, argv);

    mode = (Mode *)malloc(sizeof(Mode));
    if (mode == NULL)
        FatalError("Out of memory");
    mode->name = strdup(argv[1]->content.symbol);
    
    /* parse frame buffer type */
    symbol = argv[2]->content.symbol;
    if (strcasecmp("Text", symbol) == 0)
        mode->type = X68K_FB_TEXT;
    else if (strcasecmp("Graphic", symbol) == 0)
        mode->type = X68K_FB_GRAPHIC;
    else
        parseError(argv[2]->line, "unknown frame buffer type");
    mode->depth = argv[3]->content.literal;
    
    /* parse frame buffer class */
    symbol = argv[4]->content.symbol;
    if (strcasecmp("StaticGray", symbol) == 0)
        mode->class = StaticGray;
    else if (strcasecmp("GrayScale", symbol) == 0)
        mode->class = GrayScale;
    else if (strcasecmp("StaticColor", symbol) == 0)
        mode->class = StaticColor;
    else if (strcasecmp("PseudoColor", symbol) == 0)
        mode->class = PseudoColor;
    else if (strcasecmp("TrueColor", symbol) == 0)
        mode->class = TrueColor;
    else if (strcasecmp("DirectColor", symbol) == 0)
        mode->class = DirectColor;
    else
        parseError(argv[4]->line, "unknown frame buffer class");

    class = mode->class;
    width = mode->width = argv[5]->content.literal;
    height = mode->height = argv[6]->content.literal;
    
    /* examine whether type, depth, class, width, and height are
       a legal combination or not, and then set mode registers */
    switch (mode->type) {
        case X68K_FB_TEXT:
            if (mode->depth == 1 && class == StaticGray &&
                width <= 1024 && height <= 1024) {
                mode->reg.videoc.r1 = 0x21e4;
                mode->reg.videoc.r2 = 0x0020;
                goto legal;
            }
            break;
        case X68K_FB_GRAPHIC:
            switch (mode->depth) {
                case 4:
                    if ( (class == StaticGray || class == PseudoColor) &&
                         width <= 1024 && height <= 1024 ) {
                        mode->reg.videoc.r1 = 0x21e4;
                        mode->reg.videoc.r2 = 0x0010;
                        goto legal;
                    }
                    break;
                case 8:
                    if (class == PseudoColor &&
                        width <= 512 && height <= 512) {
                        mode->reg.videoc.r1 = 0x21e4;
                        mode->reg.videoc.r2 = 0x0003;
                        goto legal;
                    }
                    break;
                case 15:
                    if (class == TrueColor &&
                        width <= 512 && height <= 512) {
                        mode->reg.videoc.r1 = 0x21e4;
                        mode->reg.videoc.r2 = 0x000f;
                        goto legal;
                    }
                    break;
            }
            break;
    }
    parseError(argv[0]->line, "illegal combination of mode parameters");
  legal:

    /* store register values */
    mode->reg.crtc.r00 = argv[7]->content.literal;
    mode->reg.crtc.r01 = argv[8]->content.literal;
    mode->reg.crtc.r02 = argv[9]->content.literal;
    mode->reg.crtc.r03 = argv[10]->content.literal;
    mode->reg.crtc.r04 = argv[11]->content.literal;
    mode->reg.crtc.r05 = argv[12]->content.literal;
    mode->reg.crtc.r06 = argv[13]->content.literal;
    mode->reg.crtc.r07 = argv[14]->content.literal;
    mode->reg.crtc.r08 = argv[15]->content.literal;
    mode->reg.crtc.r20 = argv[16]->content.literal;
    mode->reg.videoc.r0 = argv[17]->content.literal;
    mode->reg.dotClock = argv[18]->content.literal;

    /* set scroll registers to zero */
    mode->reg.crtc.r12 = 0;    mode->reg.crtc.r13 = 0;
    mode->reg.crtc.r14 = 0;    mode->reg.crtc.r15 = 0;
    mode->reg.crtc.r16 = 0;    mode->reg.crtc.r17 = 0;
    mode->reg.crtc.r18 = 0;    mode->reg.crtc.r19 = 0;
    
    /* add new mode to linked mode list */
    mode->next = modeList;
    modeList = mode;
}

/*-------------------------------------------------------------------------
 * function "parseMode"
 *
 *  purpose:  choose a mode from predefined modes
 *  argument: (int)argc, (Token **)argv : command and arguments
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
parseMode(int argc, Token **argv)
{
    enum TokenType argtype[]= { TOKEN_SYMBOL };
    Mode *mode;

    checkArguments(1, argtype, argc-1, argv);

    /* search mode to set from mode list */
    for (mode = modeList; mode != NULL; mode = mode->next) {
        if (strcmp(mode->name, argv[1]->content.symbol) == 0)
            break;
    }
    if (mode == NULL)
        parseError(argv[1]->line, "undefined mode `%s'",
                   argv[1]->content.symbol);

    x68kScreen[0].type = mode->type;
    x68kScreen[0].depth = mode->depth;
    x68kScreen[0].class = mode->class;
    x68kScreen[0].dpi = 75;
    x68kScreen[0].x68kreg = mode->reg;
    x68kScreen[0].scr_width = mode->width;
    x68kScreen[0].scr_height = mode->height;
    
    switch (mode->type) {
        /* for TVRAM frame buffer */
        case X68K_FB_TEXT:
            x68kFbProc[0].open = x68kTextOpen;
            x68kFbProc[0].init = x68kTextInit;
            x68kFbProc[0].close = x68kTextClose;
            x68kScreen[0].fb_width = 1024;
            x68kScreen[0].fb_height = 1024;
            break;
        /* for GVRAM frame buffer */
        case X68K_FB_GRAPHIC:
            x68kFbProc[0].open = x68kGraphOpen;
            x68kFbProc[0].init = x68kGraphInit;
            x68kFbProc[0].close = x68kGraphClose;
	    x68kFormat = (PixmapFormatRec*) malloc (sizeof(PixmapFormatRec));
	    x68kFormat->scanlinePad = BITMAP_SCANLINE_PAD;
            x68kFormat->bitsPerPixel = 16;
            switch (mode->depth) {
                case 4:
                    x68kFormat->depth = 4;
                    x68kScreen[0].fb_width = 1024;
                    x68kScreen[0].fb_height = 1024;
                    break;
                case 8:
                    x68kFormat->depth = 8;
                    x68kScreen[0].fb_width = 512;
                    x68kScreen[0].fb_height = 512;
                    break;
                case 15:
                    x68kFormat->depth = 15;
                    x68kScreen[0].fb_width = 512;
                    x68kScreen[0].fb_height = 512;
            }
    }
    modeSet = TRUE;
}

/*-------------------------------------------------------------------------
 * function "parseMouse"
 *
 *  purpose:  set mouse attribute.
 *  argument: (int)argc, (Token **)argv : command and arguments
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
parseMouse(int argc, Token **argv)
{
    enum TokenType argtype[] = { TOKEN_SYMBOL };
    
    checkArguments(1, argtype, argc-1, argv);
    /* only `standard' mouse allowed */
    if (strcasecmp("standard", argv[1]->content.symbol) != 0)
        parseError(argv[1]->line, "unknown mouse type `%s'",
                   argv[1]->content.symbol);
}

/*-------------------------------------------------------------------------
 * function "parseKeyboard"
 *
 *  purpose:  select keyboard map
 *  argument: (int)argc, (Token **)argv : command and arguments
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
parseKeyboard(int argc, Token **argv)
{
    enum TokenType argtype[] = { TOKEN_SYMBOL };
    
    checkArguments(1, argtype, argc-1, argv);
    if (strcasecmp("standard", argv[1]->content.symbol) == 0) {
        x68kKeySyms = &jisKeySyms;
        x68kKbdPriv.type = X68K_KB_STANDARD;
    } else if (strcasecmp("ascii", argv[1]->content.symbol) == 0) {
        x68kKeySyms = &asciiKeySyms;
        x68kKbdPriv.type = X68K_KB_ASCII;
    } else        
        parseError(argv[1]->line, "unknown keyboard type `%s'",
                   argv[1]->content.symbol);
}

/*-------------------------------------------------------------------------
 * function "parseError"
 *
 *  purpose:  print error message to stderr and abort Xserver.
 *            this uses the same procedure of the function "FatalError"
 *  argument: (int)line   : the line in which some error was detected
 *            (char *)str : error message
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
parseError(int line, const char *str, ...)
{
    va_list arglist;
    
    fprintf(stderr, "%s:%d: ", configFilename, line);
    if (str != NULL) {
	va_start(arglist, str);
	vfprintf(stderr, str, arglist);
	va_end(arglist);
	fputc('\n', stderr);
    } else
        fprintf(stderr, "parse error\n");
    fflush(stderr);
    if (CoreDump)
        abort();
    exit(1);
}

/* EOF x68kConfig.c */
