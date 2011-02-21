#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20100216

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)

#define YYPREFIX "yy"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
#ifdef YYPARSE_PARAM_TYPE
#define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
#else
#define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
#endif
#else
#define YYPARSE_DECL() yyparse(void)
#endif /* YYPARSE_PARAM */

extern int YYPARSE_DECL();

#line 92 "xkbparse.y"
#ifdef DEBUG
#define	YYDEBUG 1
#endif
#define	DEBUG_VAR parseDebug
#include "parseutils.h"
#include <X11/keysym.h>
#include <X11/extensions/XKBgeom.h>
#include <stdlib.h>

unsigned int parseDebug;

#line 110 "xkbparse.y"
typedef union	{
	int		 ival;
	unsigned	 uval;
	char		*str;
	Atom	 	sval;
	ParseCommon	*any;
	ExprDef		*expr;
	VarDef		*var;
	VModDef		*vmod;
	InterpDef	*interp;
	KeyTypeDef	*keyType;
	SymbolsDef	*syms;
	ModMapDef	*modMask;
	GroupCompatDef	*groupCompat;
	IndicatorMapDef	*ledMap;
	IndicatorNameDef *ledName;
	KeycodeDef	*keyName;
	KeyAliasDef	*keyAlias;
	ShapeDef	*shape;
	SectionDef	*section;
	RowDef		*row;
	KeyDef		*key;
	OverlayDef	*overlay;
	OverlayKeyDef	*olKey;
	OutlineDef	*outline;
	DoodadDef	*doodad;
	XkbFile		*file;
} YYSTYPE;
#line 75 "xkbparse.c"
#define END_OF_FILE 0
#define ERROR_TOK 255
#define XKB_KEYMAP 1
#define XKB_KEYCODES 2
#define XKB_TYPES 3
#define XKB_SYMBOLS 4
#define XKB_COMPATMAP 5
#define XKB_GEOMETRY 6
#define XKB_SEMANTICS 7
#define XKB_LAYOUT 8
#define INCLUDE 10
#define OVERRIDE 11
#define AUGMENT 12
#define REPLACE 13
#define ALTERNATE 14
#define VIRTUAL_MODS 20
#define TYPE 21
#define INTERPRET 22
#define ACTION_TOK 23
#define KEY 24
#define ALIAS 25
#define GROUP 26
#define MODIFIER_MAP 27
#define INDICATOR 28
#define SHAPE 29
#define KEYS 30
#define ROW 31
#define SECTION 32
#define OVERLAY 33
#define TEXT 34
#define OUTLINE 35
#define SOLID 36
#define LOGO 37
#define VIRTUAL 38
#define EQUALS 40
#define PLUS 41
#define MINUS 42
#define DIVIDE 43
#define TIMES 44
#define OBRACE 45
#define CBRACE 46
#define OPAREN 47
#define CPAREN 48
#define OBRACKET 49
#define CBRACKET 50
#define DOT 51
#define COMMA 52
#define SEMI 53
#define EXCLAM 54
#define INVERT 55
#define STRING 60
#define INTEGER 61
#define FLOAT 62
#define IDENT 63
#define KEYNAME 64
#define PARTIAL 70
#define DEFAULT 71
#define HIDDEN 72
#define ALPHANUMERIC_KEYS 73
#define MODIFIER_KEYS 74
#define KEYPAD_KEYS 75
#define FUNCTION_KEYS 76
#define ALTERNATE_GROUP 77
#define YYERRCODE 256
static const short yylhs[] = {                           -1,
    0,    0,    0,   71,   71,   70,    5,    5,    5,   67,
   67,   68,   69,    6,    6,    6,    6,    6,   12,   12,
   11,   11,   10,   10,   10,   10,   10,   10,   10,   10,
   21,   21,   22,   22,   22,   22,   22,   22,   22,   22,
   22,   22,   22,   22,   22,   22,   22,   36,   36,   36,
   51,   52,   40,   41,   41,   42,   42,   43,   44,   44,
   37,   37,   45,   46,   38,   38,   38,   39,   39,   39,
   39,   39,   29,   29,   48,   47,   49,   50,   50,   53,
   53,   54,   55,   55,   56,   56,   56,   56,   56,   57,
   57,   58,   58,   59,   59,   60,   60,   61,   62,   62,
   63,   64,   64,   65,   65,   65,   35,   35,   34,   66,
    9,    9,    9,    9,   17,   17,   19,   19,   19,   19,
   19,   19,   19,   19,   19,   19,   19,    8,    8,    7,
    7,    7,    7,    7,   23,   23,   24,   24,   25,   25,
   25,   25,   25,   25,   26,   26,   26,   26,   26,   26,
   26,   26,   33,   33,   32,   27,   27,   27,   27,   28,
   28,   28,   28,   30,   30,   31,   31,   16,   16,   16,
    4,    4,    1,    1,    3,    2,   13,   18,   18,   20,
   15,   15,   14,
};
static const short yylen[] = {                            2,
    1,    1,    1,    2,    1,    7,    1,    1,    1,    2,
    1,    7,    4,    1,    1,    1,    1,    1,    1,    0,
    2,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    2,    0,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    4,    2,    3,
    4,    5,    3,    3,    1,    1,    3,    6,    3,    1,
    2,    1,    6,    6,    3,    1,    0,    3,    3,    1,
    2,    1,    3,    3,    5,    6,    6,    5,    6,    6,
    6,    6,    2,    1,    5,    1,    1,    1,    1,    2,
    1,    5,    1,    3,    1,    1,    3,    6,    3,    1,
    3,    3,    1,    3,    5,    3,    3,    1,    5,    6,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    0,    1,
    1,    1,    1,    1,    1,    0,    3,    1,    3,    3,
    3,    3,    3,    1,    2,    2,    2,    2,    1,    4,
    1,    3,    3,    1,    4,    1,    3,    4,    6,    1,
    1,    1,    1,    1,    0,    3,    1,    1,    1,    1,
    2,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    0,    1,
};
static const short yydefred[] = {                         0,
   23,   24,   25,   26,   27,   28,   29,   30,    0,   22,
    0,    0,    0,   11,    3,    5,    0,   21,    7,   14,
   15,   17,   16,   18,    8,    9,    0,    0,    0,   10,
    0,    4,  183,  181,    0,    0,    0,    0,   32,    0,
    0,    0,    0,  130,  132,  131,  133,  134,    0,    0,
   31,    0,    0,   47,    0,    0,    0,  117,    0,    0,
    0,    0,    0,    0,  125,    0,    0,  112,  113,  114,
    0,    0,  178,  177,  179,    0,    0,    0,    0,  116,
    0,   33,   34,   35,   38,   39,   40,   41,   42,   43,
   36,   37,   44,   45,   46,    6,   12,    0,    0,   55,
  180,    0,  169,  176,  168,  170,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   49,    0,    0,    0,   53,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   50,
    0,  119,  118,  120,  121,  122,  123,  124,  126,  127,
    0,    0,    0,    0,    0,  175,  161,  162,  163,    0,
  115,  160,    0,  144,    0,  151,    0,    0,    0,    0,
   54,   62,    0,    0,    0,    0,    0,    0,    0,   72,
    0,   66,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  108,    0,    0,  103,    0,    0,    0,   86,   88,
    0,   84,   89,   87,    0,    0,  146,  149,  145,    0,
  147,  148,    0,    0,    0,    0,    0,   51,    0,  158,
    0,   48,    0,   61,    0,    0,    0,  167,    0,    0,
    0,  154,    0,   71,    0,    0,    0,   52,   75,    0,
    0,   78,    0,    0,    0,  174,  173,  172,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   83,    0,    0,
  152,    0,    0,    0,    0,  139,  142,    0,    0,   63,
   58,    0,   73,    0,   74,    0,    0,   69,   64,   65,
   76,    0,   77,  104,  171,    0,    0,    0,   81,  107,
   80,  102,    0,   93,    0,   91,    0,   82,   79,  110,
  150,  159,    0,  166,  153,    0,    0,    0,    0,   90,
    0,    0,  100,  155,  109,  105,    0,   96,    0,   95,
   85,    0,    0,    0,    0,    0,    0,  101,   98,   99,
   97,   92,   94,
};
static const short yydgoto[] = {                          9,
  248,  157,  158,  249,   27,   28,   49,   50,   76,   10,
   11,   29,  159,   34,   35,  107,  160,  161,   80,  162,
   40,   51,  262,  263,  186,  164,  165,  166,  180,  230,
  231,  232,  233,  192,  193,  172,  173,  181,  182,   83,
   99,  100,   84,  108,   85,   86,   87,   88,  200,   90,
   91,   92,   93,   94,  201,  202,  295,  296,  319,  320,
  203,  312,  313,  194,  195,  204,   13,   14,   15,   16,
   17,
};
static const short yysindex[] = {                       456,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  456,  975,  456,    0,    0,    0,  456,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  -40,  -40,  534,    0,
   15,    0,    0,    0,   -1,   11,  -40,  456,    0,  658,
   11,  123,   16,    0,    0,    0,    0,    0,    3,   14,
    0,    6,   13,    0,   25,   32,   -8,    0,   37,   37,
   52,   25,  145,   32,    0,   32,    0,    0,    0,    0,
   88,   25,    0,    0,    0,   32,   92,   80,  108,    0,
  131,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  138,  161,    0,
    0,  118,    0,    0,    0,    0,  140,  143,  146,  162,
  170,  181,  189,  188,  193,  215,   52,  218,  238,  633,
  633,  941,    0,  633,  633,   25,    0,  903,  633,  903,
  738,   37,  633,  633,  633,  903,    9,  266,  240,    0,
  903,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  633,  633,  633,  633,  633,    0,    0,    0,    0,  311,
    0,    0,  103,    0,  245,    0,  414,  166,  208,  583,
    0,    0,  752,  583,  790,  882,   25,    0,  256,    0,
   47,    0,  251,  212,   97,  583,  225,  806,  270,   45,
  274,    0,  137,  211,    0,   32,  304,   32,    0,    0,
  679,    0,    0,    0,  633,  844,    0,    0,    0,  537,
    0,    0,  633,  633,  633,  633,  633,    0,  633,    0,
  633,    0,  298,    0,  306,    0,    0,    0,  326,  345,
  324,    0,   98,    0,  541,  347,  738,    0,    0,  360,
  633,    0,  364,  385,  354,    0,    0,    0,  376,  587,
  387,  270,  407,    2,  865,  403,  410,    0,  280,  427,
    0,  418,  438,  395,  395,    0,    0,  583,  474,    0,
    0,  633,    0,   -8,    0,  941,  583,    0,    0,    0,
    0,  583,    0,    0,    0,   45,  270,  583,    0,    0,
    0,    0,  467,    0,  700,    0,   37,    0,    0,    0,
    0,    0,  453,    0,    0,  491,  413,  -32,  500,    0,
  516,  437,    0,    0,    0,    0,  633,    0,  545,    0,
    0,   37,  513,   37,  546,  518,  -32,    0,    0,    0,
    0,    0,    0,
};
static const short yyrindex[] = {                       983,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  991,    0,  399,    0,    0,    0,  334,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  529,  343,    0,    0,
    0,    0,    0,    0,    0,  398,  529,  680,    0,  450,
    0,  680,  471,    0,    0,    0,    0,    0,  523,    0,
    0,    0,    0,    0,    0,   20,   60,    0,   84,    0,
   87,  152,  190,  194,    0,  235,   74,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  536,  296,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  415,    0,    0,
    0,    0,    0,    0,    0,    0,  539,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  547,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  265,
    0,    0,    0,    0,  341,    0,    0,  401,    0,  458,
    0,    0,    0,  544,    0,  557,    0,  230,    0,    0,
    0,    0,    0,    0,    0,  292,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  190,  299,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  572,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   65,  110,    0,    0,    0,
  586,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  574,  134,  175,    0,    0,  585,    0,    0,
    0,  572,    0,    0,    0,    0,  554,    0,    0,    0,
    0,  344,    0,    0,    0,    0,    0,  571,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,
};
static const short yygindex[] = {                         0,
  394,  -56,    0,  357,    0,  611,    0,    0,    0,  634,
    0,   57,  -50,    0,   39, -170,   36,  -47,    0,  -45,
  605,    0,  374, -132,  111,  368,  -33,    0,  417,    0,
    0,  377,    0,  411, -177,  -36,  -66,    0,  429,    0,
    0,  550,    0,    0,    0,    0,    0,    0,  623,    0,
    0,    0,    0,    0,    0,  476,    0,  383,    0,  352,
    0,    0,  365,    0,  436,  631,  653,   -9,    0,  675,
    0,
};
#define YYTABLESIZE 1012
static const short yytable[] = {                         77,
  106,  185,   79,   30,  111,  228,  113,   98,  109,  110,
  102,  244,  317,   82,  112,   19,   81,  114,  115,   33,
  116,   25,   26,  103,  118,   44,   45,   46,   47,   48,
  119,   74,   30,   55,   56,   57,   58,   59,   60,   61,
   62,   63,   64,   38,   65,   66,  189,   67,   68,   69,
   70,   71,  104,  189,  105,   39,   12,  190,   96,  119,
  139,   53,   54,  175,   73,   97,   36,   72,  119,  188,
  119,   73,   75,   31,  206,   41,   73,   74,   98,   75,
   79,  183,   79,  178,   75,   78,  245,   73,   79,  191,
   79,  101,  236,   79,   81,   75,   81,  179,  237,  118,
   74,  199,   81,  304,   81,  246,  247,   81,  118,  307,
  118,  126,  104,  127,  169,  117,  169,  208,  208,  106,
  208,  208,  127,  120,  127,   79,  121,   79,  121,  234,
  122,  120,  120,  111,  120,  121,  224,  121,  224,   81,
   79,   81,  240,  214,  215,  216,  217,  275,  241,  276,
  114,  224,  256,   79,   81,  218,  178,  168,   79,  168,
  123,  168,  128,   78,  199,   78,   78,   81,   52,  224,
  124,   78,   81,   78,  140,  140,   78,  125,  140,  140,
  129,  140,  251,  140,  325,  140,  140,  130,  252,  178,
  131,  122,    1,    2,    3,    4,    5,    6,    7,    8,
  122,  132,  122,  179,  101,  104,  191,   79,   78,  133,
   78,  229,  126,  127,  221,  141,  141,  106,  294,  141,
  141,   81,  141,   78,  141,  134,  141,  141,  135,  123,
  163,  167,  136,  124,  169,  170,   78,  137,  123,  174,
  123,   78,  124,  184,  124,  187,  311,   79,  214,  215,
  216,  217,  214,  215,  216,  217,  253,  318,  294,  138,
  222,   81,  254,  210,  239,  214,  215,  216,  217,  115,
  140,  328,   78,  311,  126,   70,  318,  242,  115,  205,
  115,   70,  141,  126,  219,  126,  142,  143,   58,  144,
   78,  145,  146,  196,  148,  235,  197,  149,  198,   67,
   68,   69,   70,  238,  156,  156,  156,  156,  156,  156,
  156,  229,  156,  250,  156,  259,  156,  156,  190,   72,
  214,  215,  216,  217,  264,  265,  266,  267,   73,  268,
   78,  269,  299,    1,   20,  115,   75,  138,  125,  138,
   20,   20,  182,  138,  115,  277,  115,  125,  255,  125,
  270,  282,  182,  182,  182,  182,  182,  213,  271,  121,
  288,  122,  182,  182,  182,  182,  182,  182,  182,  182,
  182,  182,  272,  182,  182,  274,  182,  182,  182,  182,
  182,  149,  149,  149,  149,  149,  149,  182,  149,  137,
  149,  137,  149,  149,  273,  137,  182,   32,    2,  279,
   20,   20,   20,   20,   20,  182,  182,   32,   32,   32,
   32,   32,  281,  182,  246,  247,  283,   32,   32,   32,
   32,   32,   32,   32,   32,   32,   32,  286,   32,   32,
  284,   32,   32,   32,   32,   32,  252,  216,  217,  289,
  157,  157,  157,  157,  157,  157,  157,  297,  157,   13,
  157,   32,  157,  157,  214,  215,  216,  217,  316,  291,
   32,   32,  298,  220,  252,  301,   56,   56,   32,  129,
  129,  129,  129,  129,  129,  129,  129,  129,  129,  300,
  129,  129,  323,  129,  129,  129,  129,  129,  324,  241,
  129,  129,  129,  129,  129,  129,  129,  129,  129,  129,
  314,  129,  129,  129,  129,  129,  129,  129,  129,   57,
   57,  308,  129,  129,  214,  215,  216,  217,  207,  209,
  129,  211,  212,  302,  129,    1,    2,    3,    4,    5,
    6,    7,    8,  129,  129,   20,   21,   22,   23,   24,
  315,  129,  128,  128,  128,  128,  128,  128,  128,  128,
  128,  128,  321,  128,  128,  322,  128,  128,  128,  128,
  128,  142,  143,   58,  144,  329,  145,  146,  147,  148,
  332,   65,  149,  182,  150,  156,  128,  214,  215,  216,
  217,  151,  152,   60,  261,  128,  128,  153,   59,  176,
  326,  331,   67,  128,  154,  155,  327,  241,   67,   68,
  101,  104,  156,   73,   74,   68,  165,  142,  143,   58,
  144,   75,  145,  146,  147,  148,  106,   65,  149,  136,
  150,  135,  106,  214,  215,  216,  217,  151,  152,  143,
  143,  287,  143,  153,  143,  164,  143,  143,  285,   37,
  154,  155,  306,   43,   18,  303,  101,  104,  156,   73,
   74,  278,  305,  142,  143,   58,  144,   75,  145,  146,
  147,  148,  290,   65,  149,  280,  150,   44,   45,   46,
   47,   48,   89,  151,  152,  171,  258,  310,  333,  153,
   95,   20,   20,   20,   20,   20,  154,  155,  330,  292,
   42,   32,  101,  104,  156,   73,   74,    0,    0,  142,
  143,   58,  144,   75,  145,  146,  196,  148,    0,  197,
  149,  198,   67,   68,   69,   70,    0,    0,    0,    0,
  142,  143,   58,  144,  257,  145,  146,  147,  148,  293,
   65,  149,   72,  150,    0,    0,    0,    0,    0,    0,
    0,   73,    0,    0,    0,  309,    0,    0,    0,   75,
    0,    0,    0,   72,    0,    0,    0,    0,  142,  143,
   58,  144,   73,  145,  146,  147,  148,    0,   65,  149,
   75,  150,  142,  143,   58,  144,    0,  145,  146,  147,
  148,    0,   65,  149,    0,  150,  176,    0,    0,    0,
    0,  177,    0,    0,    0,    0,    0,  223,    0,    0,
   73,    0,    0,    0,    0,   72,    0,    0,   75,    0,
  142,  143,   58,  144,   73,  145,  146,  147,  148,    0,
   65,  149,   75,  150,    0,    0,  142,  143,   58,  144,
    0,  145,  146,  147,  148,  225,   65,  149,    0,  150,
    0,    0,    0,   72,    0,    0,    0,    0,    0,    0,
    0,  243,   73,    0,    0,    0,    0,    0,    0,   72,
   75,    0,    0,    0,  142,  143,   58,  144,   73,  145,
  146,  147,  148,    0,   65,  149,   75,  150,    0,    0,
    0,    0,    0,    0,    0,  142,  143,   58,  144,  260,
  145,  146,  147,  148,  293,   65,  149,   72,  150,    0,
    0,    0,  142,  143,   58,  144,   73,  145,  146,  147,
  148,    0,   65,  226,   75,  150,    0,    0,   72,    0,
    0,    0,    0,  142,  143,   58,  144,   73,  145,  146,
  147,  148,    0,   65,  149,   75,  150,    0,    0,    0,
    0,    0,  104,    0,  227,    0,    0,    0,    0,    0,
    0,    0,   75,    0,    0,    0,   72,    0,    0,    0,
    0,  142,  143,   58,  144,   73,  145,  146,  147,  148,
    0,   65,  149,   75,  150,   19,   20,   21,   22,   23,
   24,   25,   26,   20,   20,   20,   20,   20,   20,   20,
   20,   19,   19,   19,   19,   19,   19,   19,   19,    0,
    0,    0,    0,   73,    0,    0,    0,    0,    0,    0,
    0,   75,
};
static const short yycheck[] = {                         50,
   57,  134,   50,   13,   61,  176,   63,   55,   59,   60,
   56,  189,   45,   50,   62,    1,   50,   63,   64,   60,
   66,    7,    8,   32,   72,   10,   11,   12,   13,   14,
   76,   64,   42,   20,   21,   22,   23,   24,   25,   26,
   27,   28,   29,   45,   31,   32,   45,   34,   35,   36,
   37,   38,   61,   45,   63,   45,    0,   49,   53,   40,
  117,   46,   60,  130,   63,   53,   28,   54,   49,  136,
   51,   63,   71,   17,  141,   37,   63,   64,  126,   71,
  128,  132,  130,  131,   71,   50,   42,   63,  136,  137,
  138,   60,   46,  141,  128,   71,  130,  131,   52,   40,
   64,  138,  136,  274,  138,   61,   62,  141,   49,  287,
   51,   47,   61,   40,   50,   28,   52,  151,  152,  176,
  154,  155,   49,   40,   51,  173,   40,  175,   49,  177,
   51,   40,   49,   60,   51,   49,  173,   51,  175,  173,
  188,  175,   46,   41,   42,   43,   44,   50,   52,   52,
  196,  188,  198,  201,  188,   53,   47,  122,  206,   50,
   53,   52,   45,  128,  201,  130,  131,  201,   46,  206,
   40,  136,  206,  138,   41,   42,  141,   40,   45,   46,
   41,   48,   46,   50,  317,   52,   53,   45,   52,  237,
   45,   40,   70,   71,   72,   73,   74,   75,   76,   77,
   49,   40,   51,  237,   60,   61,  254,  255,  173,   40,
  175,  176,   52,   53,   49,   41,   42,  274,  255,   45,
   46,  255,   48,  188,   50,   45,   52,   53,   40,   40,
  120,  121,   45,   40,  124,  125,  201,   45,   49,  129,
   51,  206,   49,  133,   51,  135,  297,  295,   41,   42,
   43,   44,   41,   42,   43,   44,   46,  308,  295,   45,
   53,  295,   52,  153,   53,   41,   42,   43,   44,   40,
   53,  322,  237,  324,   40,   46,  327,   53,   49,   40,
   51,   52,   45,   49,   40,   51,   21,   22,   23,   24,
  255,   26,   27,   28,   29,   40,   31,   32,   33,   34,
   35,   36,   37,   53,   40,   41,   42,   43,   44,   45,
   46,  276,   48,   40,   50,  205,   52,   53,   49,   54,
   41,   42,   43,   44,  214,  215,  216,  217,   63,  219,
  295,  221,   53,    0,    1,   40,   71,   46,   40,   48,
    7,    8,    0,   52,   49,  235,   51,   49,   45,   51,
   53,  241,   10,   11,   12,   13,   14,   47,   53,   49,
  250,   51,   20,   21,   22,   23,   24,   25,   26,   27,
   28,   29,   47,   31,   32,   52,   34,   35,   36,   37,
   38,   41,   42,   43,   44,   45,   46,   45,   48,   46,
   50,   48,   52,   53,   50,   52,   54,    0,    0,   53,
    2,    3,    4,    5,    6,   63,   64,   10,   11,   12,
   13,   14,   53,   71,   61,   62,   53,   20,   21,   22,
   23,   24,   25,   26,   27,   28,   29,   52,   31,   32,
   46,   34,   35,   36,   37,   38,   52,   43,   44,   53,
   40,   41,   42,   43,   44,   45,   46,   45,   48,    0,
   50,   54,   52,   53,   41,   42,   43,   44,   46,   53,
   63,   64,   53,   50,   52,   48,   52,   53,   71,   20,
   21,   22,   23,   24,   25,   26,   27,   28,   29,   53,
   31,   32,   46,   34,   35,   36,   37,   38,   52,   52,
   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
   48,   31,   32,   54,   34,   35,   36,   37,   38,   52,
   53,   45,   63,   64,   41,   42,   43,   44,  151,  152,
   71,  154,  155,   50,   54,   70,   71,   72,   73,   74,
   75,   76,   77,   63,   64,    2,    3,    4,    5,    6,
   50,   71,   20,   21,   22,   23,   24,   25,   26,   27,
   28,   29,   53,   31,   32,   40,   34,   35,   36,   37,
   38,   21,   22,   23,   24,   53,   26,   27,   28,   29,
   53,   31,   32,   45,   34,   40,   54,   41,   42,   43,
   44,   41,   42,   45,   48,   63,   64,   47,   45,   49,
   46,   46,   46,   71,   54,   55,   52,   52,   52,   46,
   60,   61,   62,   63,   64,   52,   50,   21,   22,   23,
   24,   71,   26,   27,   28,   29,   46,   31,   32,   48,
   34,   48,   52,   41,   42,   43,   44,   41,   42,   45,
   46,   45,   48,   47,   50,   50,   52,   53,  245,   29,
   54,   55,  286,   39,   11,  272,   60,   61,   62,   63,
   64,  235,  276,   21,   22,   23,   24,   71,   26,   27,
   28,   29,  252,   31,   32,  237,   34,   10,   11,   12,
   13,   14,   50,   41,   42,  126,  201,  295,  327,   47,
   50,    2,    3,    4,    5,    6,   54,   55,  324,  254,
   38,   17,   60,   61,   62,   63,   64,   -1,   -1,   21,
   22,   23,   24,   71,   26,   27,   28,   29,   -1,   31,
   32,   33,   34,   35,   36,   37,   -1,   -1,   -1,   -1,
   21,   22,   23,   24,   46,   26,   27,   28,   29,   30,
   31,   32,   54,   34,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   63,   -1,   -1,   -1,   46,   -1,   -1,   -1,   71,
   -1,   -1,   -1,   54,   -1,   -1,   -1,   -1,   21,   22,
   23,   24,   63,   26,   27,   28,   29,   -1,   31,   32,
   71,   34,   21,   22,   23,   24,   -1,   26,   27,   28,
   29,   -1,   31,   32,   -1,   34,   49,   -1,   -1,   -1,
   -1,   54,   -1,   -1,   -1,   -1,   -1,   46,   -1,   -1,
   63,   -1,   -1,   -1,   -1,   54,   -1,   -1,   71,   -1,
   21,   22,   23,   24,   63,   26,   27,   28,   29,   -1,
   31,   32,   71,   34,   -1,   -1,   21,   22,   23,   24,
   -1,   26,   27,   28,   29,   46,   31,   32,   -1,   34,
   -1,   -1,   -1,   54,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   46,   63,   -1,   -1,   -1,   -1,   -1,   -1,   54,
   71,   -1,   -1,   -1,   21,   22,   23,   24,   63,   26,
   27,   28,   29,   -1,   31,   32,   71,   34,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   21,   22,   23,   24,   46,
   26,   27,   28,   29,   30,   31,   32,   54,   34,   -1,
   -1,   -1,   21,   22,   23,   24,   63,   26,   27,   28,
   29,   -1,   31,   32,   71,   34,   -1,   -1,   54,   -1,
   -1,   -1,   -1,   21,   22,   23,   24,   63,   26,   27,
   28,   29,   -1,   31,   32,   71,   34,   -1,   -1,   -1,
   -1,   -1,   61,   -1,   63,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   71,   -1,   -1,   -1,   54,   -1,   -1,   -1,
   -1,   21,   22,   23,   24,   63,   26,   27,   28,   29,
   -1,   31,   32,   71,   34,    1,    2,    3,    4,    5,
    6,    7,    8,    1,    2,    3,    4,    5,    6,    7,
    8,    1,    2,    3,    4,    5,    6,    7,    8,   -1,
   -1,   -1,   -1,   63,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   71,
};
#define YYFINAL 9
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 255
#if YYDEBUG
static const char *yyname[] = {

"end-of-file","XKB_KEYMAP","XKB_KEYCODES","XKB_TYPES","XKB_SYMBOLS",
"XKB_COMPATMAP","XKB_GEOMETRY","XKB_SEMANTICS","XKB_LAYOUT",0,"INCLUDE",
"OVERRIDE","AUGMENT","REPLACE","ALTERNATE",0,0,0,0,0,"VIRTUAL_MODS","TYPE",
"INTERPRET","ACTION_TOK","KEY","ALIAS","GROUP","MODIFIER_MAP","INDICATOR",
"SHAPE","KEYS","ROW","SECTION","OVERLAY","TEXT","OUTLINE","SOLID","LOGO",
"VIRTUAL",0,"EQUALS","PLUS","MINUS","DIVIDE","TIMES","OBRACE","CBRACE","OPAREN",
"CPAREN","OBRACKET","CBRACKET","DOT","COMMA","SEMI","EXCLAM","INVERT",0,0,0,0,
"STRING","INTEGER","FLOAT","IDENT","KEYNAME",0,0,0,0,0,"PARTIAL","DEFAULT",
"HIDDEN","ALPHANUMERIC_KEYS","MODIFIER_KEYS","KEYPAD_KEYS","FUNCTION_KEYS",
"ALTERNATE_GROUP",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"ERROR_TOK",
};
static const char *yyrule[] = {
"$accept : XkbFile",
"XkbFile : XkbCompMapList",
"XkbFile : XkbMapConfigList",
"XkbFile : XkbConfig",
"XkbCompMapList : XkbCompMapList XkbCompositeMap",
"XkbCompMapList : XkbCompositeMap",
"XkbCompositeMap : OptFlags XkbCompositeType OptMapName OBRACE XkbMapConfigList CBRACE SEMI",
"XkbCompositeType : XKB_KEYMAP",
"XkbCompositeType : XKB_SEMANTICS",
"XkbCompositeType : XKB_LAYOUT",
"XkbMapConfigList : XkbMapConfigList XkbMapConfig",
"XkbMapConfigList : XkbMapConfig",
"XkbMapConfig : OptFlags FileType OptMapName OBRACE DeclList CBRACE SEMI",
"XkbConfig : OptFlags FileType OptMapName DeclList",
"FileType : XKB_KEYCODES",
"FileType : XKB_TYPES",
"FileType : XKB_COMPATMAP",
"FileType : XKB_SYMBOLS",
"FileType : XKB_GEOMETRY",
"OptFlags : Flags",
"OptFlags :",
"Flags : Flags Flag",
"Flags : Flag",
"Flag : PARTIAL",
"Flag : DEFAULT",
"Flag : HIDDEN",
"Flag : ALPHANUMERIC_KEYS",
"Flag : MODIFIER_KEYS",
"Flag : KEYPAD_KEYS",
"Flag : FUNCTION_KEYS",
"Flag : ALTERNATE_GROUP",
"DeclList : DeclList Decl",
"DeclList :",
"Decl : OptMergeMode VarDecl",
"Decl : OptMergeMode VModDecl",
"Decl : OptMergeMode InterpretDecl",
"Decl : OptMergeMode KeyNameDecl",
"Decl : OptMergeMode KeyAliasDecl",
"Decl : OptMergeMode KeyTypeDecl",
"Decl : OptMergeMode SymbolsDecl",
"Decl : OptMergeMode ModMapDecl",
"Decl : OptMergeMode GroupCompatDecl",
"Decl : OptMergeMode IndicatorMapDecl",
"Decl : OptMergeMode IndicatorNameDecl",
"Decl : OptMergeMode ShapeDecl",
"Decl : OptMergeMode SectionDecl",
"Decl : OptMergeMode DoodadDecl",
"Decl : MergeMode STRING",
"VarDecl : Lhs EQUALS Expr SEMI",
"VarDecl : Ident SEMI",
"VarDecl : EXCLAM Ident SEMI",
"KeyNameDecl : KeyName EQUALS Expr SEMI",
"KeyAliasDecl : ALIAS KeyName EQUALS KeyName SEMI",
"VModDecl : VIRTUAL_MODS VModDefList SEMI",
"VModDefList : VModDefList COMMA VModDef",
"VModDefList : VModDef",
"VModDef : Ident",
"VModDef : Ident EQUALS Expr",
"InterpretDecl : INTERPRET InterpretMatch OBRACE VarDeclList CBRACE SEMI",
"InterpretMatch : KeySym PLUS Expr",
"InterpretMatch : KeySym",
"VarDeclList : VarDeclList VarDecl",
"VarDeclList : VarDecl",
"KeyTypeDecl : TYPE String OBRACE VarDeclList CBRACE SEMI",
"SymbolsDecl : KEY KeyName OBRACE SymbolsBody CBRACE SEMI",
"SymbolsBody : SymbolsBody COMMA SymbolsVarDecl",
"SymbolsBody : SymbolsVarDecl",
"SymbolsBody :",
"SymbolsVarDecl : Lhs EQUALS Expr",
"SymbolsVarDecl : Lhs EQUALS ArrayInit",
"SymbolsVarDecl : Ident",
"SymbolsVarDecl : EXCLAM Ident",
"SymbolsVarDecl : ArrayInit",
"ArrayInit : OBRACKET OptKeySymList CBRACKET",
"ArrayInit : OBRACKET ActionList CBRACKET",
"GroupCompatDecl : GROUP Integer EQUALS Expr SEMI",
"ModMapDecl : MODIFIER_MAP Ident OBRACE ExprList CBRACE SEMI",
"IndicatorMapDecl : INDICATOR String OBRACE VarDeclList CBRACE SEMI",
"IndicatorNameDecl : INDICATOR Integer EQUALS Expr SEMI",
"IndicatorNameDecl : VIRTUAL INDICATOR Integer EQUALS Expr SEMI",
"ShapeDecl : SHAPE String OBRACE OutlineList CBRACE SEMI",
"ShapeDecl : SHAPE String OBRACE CoordList CBRACE SEMI",
"SectionDecl : SECTION String OBRACE SectionBody CBRACE SEMI",
"SectionBody : SectionBody SectionBodyItem",
"SectionBody : SectionBodyItem",
"SectionBodyItem : ROW OBRACE RowBody CBRACE SEMI",
"SectionBodyItem : VarDecl",
"SectionBodyItem : DoodadDecl",
"SectionBodyItem : IndicatorMapDecl",
"SectionBodyItem : OverlayDecl",
"RowBody : RowBody RowBodyItem",
"RowBody : RowBodyItem",
"RowBodyItem : KEYS OBRACE Keys CBRACE SEMI",
"RowBodyItem : VarDecl",
"Keys : Keys COMMA Key",
"Keys : Key",
"Key : KeyName",
"Key : OBRACE ExprList CBRACE",
"OverlayDecl : OVERLAY String OBRACE OverlayKeyList CBRACE SEMI",
"OverlayKeyList : OverlayKeyList COMMA OverlayKey",
"OverlayKeyList : OverlayKey",
"OverlayKey : KeyName EQUALS KeyName",
"OutlineList : OutlineList COMMA OutlineInList",
"OutlineList : OutlineInList",
"OutlineInList : OBRACE CoordList CBRACE",
"OutlineInList : Ident EQUALS OBRACE CoordList CBRACE",
"OutlineInList : Ident EQUALS Expr",
"CoordList : CoordList COMMA Coord",
"CoordList : Coord",
"Coord : OBRACKET SignedNumber COMMA SignedNumber CBRACKET",
"DoodadDecl : DoodadType String OBRACE VarDeclList CBRACE SEMI",
"DoodadType : TEXT",
"DoodadType : OUTLINE",
"DoodadType : SOLID",
"DoodadType : LOGO",
"FieldSpec : Ident",
"FieldSpec : Element",
"Element : ACTION_TOK",
"Element : INTERPRET",
"Element : TYPE",
"Element : KEY",
"Element : GROUP",
"Element : MODIFIER_MAP",
"Element : INDICATOR",
"Element : SHAPE",
"Element : ROW",
"Element : SECTION",
"Element : TEXT",
"OptMergeMode : MergeMode",
"OptMergeMode :",
"MergeMode : INCLUDE",
"MergeMode : AUGMENT",
"MergeMode : OVERRIDE",
"MergeMode : REPLACE",
"MergeMode : ALTERNATE",
"OptExprList : ExprList",
"OptExprList :",
"ExprList : ExprList COMMA Expr",
"ExprList : Expr",
"Expr : Expr DIVIDE Expr",
"Expr : Expr PLUS Expr",
"Expr : Expr MINUS Expr",
"Expr : Expr TIMES Expr",
"Expr : Lhs EQUALS Expr",
"Expr : Term",
"Term : MINUS Term",
"Term : PLUS Term",
"Term : EXCLAM Term",
"Term : INVERT Term",
"Term : Lhs",
"Term : FieldSpec OPAREN OptExprList CPAREN",
"Term : Terminal",
"Term : OPAREN Expr CPAREN",
"ActionList : ActionList COMMA Action",
"ActionList : Action",
"Action : FieldSpec OPAREN OptExprList CPAREN",
"Lhs : FieldSpec",
"Lhs : FieldSpec DOT FieldSpec",
"Lhs : FieldSpec OBRACKET Expr CBRACKET",
"Lhs : FieldSpec DOT FieldSpec OBRACKET Expr CBRACKET",
"Terminal : String",
"Terminal : Integer",
"Terminal : Float",
"Terminal : KeyName",
"OptKeySymList : KeySymList",
"OptKeySymList :",
"KeySymList : KeySymList COMMA KeySym",
"KeySymList : KeySym",
"KeySym : IDENT",
"KeySym : SECTION",
"KeySym : Integer",
"SignedNumber : MINUS Number",
"SignedNumber : Number",
"Number : FLOAT",
"Number : INTEGER",
"Float : FLOAT",
"Integer : INTEGER",
"KeyName : KEYNAME",
"Ident : IDENT",
"Ident : DEFAULT",
"String : STRING",
"OptMapName : MapName",
"OptMapName :",
"MapName : STRING",

};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;

typedef struct {
    unsigned stacksize;
    short    *s_base;
    short    *s_mark;
    short    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;

#define YYPURE 0

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 764 "xkbparse.y"
void
yyerror(const char *s)
{
    if (warningLevel>0) {
	(void)fprintf(stderr,"%s: line %d of %s\n",s,lineNum,
					(scanFile?scanFile:"(unknown)"));
	if ((warningLevel>3))
	    (void)fprintf(stderr,"last scanned symbol is: %s\n",scanBuf);
    }
    return;
}


int
yywrap(void)
{
   return 1;
}

#line 790 "xkbparse.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = data->s_mark - data->s_base;
    newss = (data->s_base != 0)
          ? (short *)realloc(data->s_base, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    data->s_base  = newss;
    data->s_mark = newss + i;

    newvs = (data->l_base != 0)
          ? (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack)) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 1:
#line 169 "xkbparse.y"
	{ yyval.file= rtrnValue= yystack.l_mark[0].file; }
break;
case 2:
#line 171 "xkbparse.y"
	{ yyval.file= rtrnValue= yystack.l_mark[0].file;  }
break;
case 3:
#line 173 "xkbparse.y"
	{ yyval.file= rtrnValue= yystack.l_mark[0].file; }
break;
case 4:
#line 177 "xkbparse.y"
	{ yyval.file= (XkbFile *)AppendStmt(&yystack.l_mark[-1].file->common,&yystack.l_mark[0].file->common); }
break;
case 5:
#line 179 "xkbparse.y"
	{ yyval.file= yystack.l_mark[0].file; }
break;
case 6:
#line 185 "xkbparse.y"
	{ yyval.file= CreateXKBFile(yystack.l_mark[-5].uval,yystack.l_mark[-4].str,&yystack.l_mark[-2].file->common,yystack.l_mark[-6].uval); }
break;
case 7:
#line 188 "xkbparse.y"
	{ yyval.uval= XkmKeymapFile; }
break;
case 8:
#line 189 "xkbparse.y"
	{ yyval.uval= XkmSemanticsFile; }
break;
case 9:
#line 190 "xkbparse.y"
	{ yyval.uval= XkmLayoutFile; }
break;
case 10:
#line 194 "xkbparse.y"
	{ yyval.file= (XkbFile *)AppendStmt(&yystack.l_mark[-1].file->common,&yystack.l_mark[0].file->common); }
break;
case 11:
#line 196 "xkbparse.y"
	{ yyval.file= yystack.l_mark[0].file; }
break;
case 12:
#line 202 "xkbparse.y"
	{ yyval.file= CreateXKBFile(yystack.l_mark[-5].uval,yystack.l_mark[-4].str,yystack.l_mark[-2].any,yystack.l_mark[-6].uval); }
break;
case 13:
#line 206 "xkbparse.y"
	{ yyval.file= CreateXKBFile(yystack.l_mark[-2].uval,yystack.l_mark[-1].str,yystack.l_mark[0].any,yystack.l_mark[-3].uval); }
break;
case 14:
#line 210 "xkbparse.y"
	{ yyval.uval= XkmKeyNamesIndex; }
break;
case 15:
#line 211 "xkbparse.y"
	{ yyval.uval= XkmTypesIndex; }
break;
case 16:
#line 212 "xkbparse.y"
	{ yyval.uval= XkmCompatMapIndex; }
break;
case 17:
#line 213 "xkbparse.y"
	{ yyval.uval= XkmSymbolsIndex; }
break;
case 18:
#line 214 "xkbparse.y"
	{ yyval.uval= XkmGeometryIndex; }
break;
case 19:
#line 217 "xkbparse.y"
	{ yyval.uval= yystack.l_mark[0].uval; }
break;
case 20:
#line 218 "xkbparse.y"
	{ yyval.uval= 0; }
break;
case 21:
#line 221 "xkbparse.y"
	{ yyval.uval= ((yystack.l_mark[-1].uval)|(yystack.l_mark[0].uval)); }
break;
case 22:
#line 222 "xkbparse.y"
	{ yyval.uval= yystack.l_mark[0].uval; }
break;
case 23:
#line 225 "xkbparse.y"
	{ yyval.uval= XkbLC_Partial; }
break;
case 24:
#line 226 "xkbparse.y"
	{ yyval.uval= XkbLC_Default; }
break;
case 25:
#line 227 "xkbparse.y"
	{ yyval.uval= XkbLC_Hidden; }
break;
case 26:
#line 228 "xkbparse.y"
	{ yyval.uval= XkbLC_AlphanumericKeys; }
break;
case 27:
#line 229 "xkbparse.y"
	{ yyval.uval= XkbLC_ModifierKeys; }
break;
case 28:
#line 230 "xkbparse.y"
	{ yyval.uval= XkbLC_KeypadKeys; }
break;
case 29:
#line 231 "xkbparse.y"
	{ yyval.uval= XkbLC_FunctionKeys; }
break;
case 30:
#line 232 "xkbparse.y"
	{ yyval.uval= XkbLC_AlternateGroup; }
break;
case 31:
#line 236 "xkbparse.y"
	{ yyval.any= AppendStmt(yystack.l_mark[-1].any,yystack.l_mark[0].any); }
break;
case 32:
#line 237 "xkbparse.y"
	{ yyval.any= NULL; }
break;
case 33:
#line 241 "xkbparse.y"
	{
			    yystack.l_mark[0].var->merge= StmtSetMerge(&yystack.l_mark[0].var->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].var->common;
			}
break;
case 34:
#line 246 "xkbparse.y"
	{
			    yystack.l_mark[0].vmod->merge= StmtSetMerge(&yystack.l_mark[0].vmod->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].vmod->common;
			}
break;
case 35:
#line 251 "xkbparse.y"
	{
			    yystack.l_mark[0].interp->merge= StmtSetMerge(&yystack.l_mark[0].interp->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].interp->common;
			}
break;
case 36:
#line 256 "xkbparse.y"
	{
			    yystack.l_mark[0].keyName->merge= StmtSetMerge(&yystack.l_mark[0].keyName->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].keyName->common;
			}
break;
case 37:
#line 261 "xkbparse.y"
	{
			    yystack.l_mark[0].keyAlias->merge= StmtSetMerge(&yystack.l_mark[0].keyAlias->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].keyAlias->common;
			}
break;
case 38:
#line 266 "xkbparse.y"
	{
			    yystack.l_mark[0].keyType->merge= StmtSetMerge(&yystack.l_mark[0].keyType->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].keyType->common;
			}
break;
case 39:
#line 271 "xkbparse.y"
	{
			    yystack.l_mark[0].syms->merge= StmtSetMerge(&yystack.l_mark[0].syms->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].syms->common;
			}
break;
case 40:
#line 276 "xkbparse.y"
	{
			    yystack.l_mark[0].modMask->merge= StmtSetMerge(&yystack.l_mark[0].modMask->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].modMask->common;
			}
break;
case 41:
#line 281 "xkbparse.y"
	{
			    yystack.l_mark[0].groupCompat->merge= StmtSetMerge(&yystack.l_mark[0].groupCompat->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].groupCompat->common;
			}
break;
case 42:
#line 286 "xkbparse.y"
	{
			    yystack.l_mark[0].ledMap->merge= StmtSetMerge(&yystack.l_mark[0].ledMap->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].ledMap->common;
			}
break;
case 43:
#line 291 "xkbparse.y"
	{
			    yystack.l_mark[0].ledName->merge= StmtSetMerge(&yystack.l_mark[0].ledName->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].ledName->common;
			}
break;
case 44:
#line 296 "xkbparse.y"
	{
			    yystack.l_mark[0].shape->merge= StmtSetMerge(&yystack.l_mark[0].shape->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].shape->common;
			}
break;
case 45:
#line 301 "xkbparse.y"
	{
			    yystack.l_mark[0].section->merge= StmtSetMerge(&yystack.l_mark[0].section->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].section->common;
			}
break;
case 46:
#line 306 "xkbparse.y"
	{
			    yystack.l_mark[0].doodad->merge= StmtSetMerge(&yystack.l_mark[0].doodad->common,yystack.l_mark[-1].uval);
			    yyval.any= &yystack.l_mark[0].doodad->common;
			}
break;
case 47:
#line 311 "xkbparse.y"
	{
			    if (yystack.l_mark[-1].uval==MergeAltForm) {
				yyerror("cannot use 'alternate' to include other maps");
				yyval.any= &IncludeCreate(scanBuf,MergeDefault)->common;
			    }
			    else {
				yyval.any= &IncludeCreate(scanBuf,yystack.l_mark[-1].uval)->common;
			    }
                        }
break;
case 48:
#line 323 "xkbparse.y"
	{ yyval.var= VarCreate(yystack.l_mark[-3].expr,yystack.l_mark[-1].expr); }
break;
case 49:
#line 325 "xkbparse.y"
	{ yyval.var= BoolVarCreate(yystack.l_mark[-1].sval,1); }
break;
case 50:
#line 327 "xkbparse.y"
	{ yyval.var= BoolVarCreate(yystack.l_mark[-1].sval,0); }
break;
case 51:
#line 331 "xkbparse.y"
	{
			    KeycodeDef *def;

			    def= KeycodeCreate(yystack.l_mark[-3].str,yystack.l_mark[-1].expr);
			    if (yystack.l_mark[-3].str)
				free(yystack.l_mark[-3].str);
			    yyval.keyName= def;
			}
break;
case 52:
#line 342 "xkbparse.y"
	{ 
			    KeyAliasDef	*def;
			    def= KeyAliasCreate(yystack.l_mark[-3].str,yystack.l_mark[-1].str); 
			    if (yystack.l_mark[-3].str)	free(yystack.l_mark[-3].str);	
			    if (yystack.l_mark[-1].str)	free(yystack.l_mark[-1].str);	
			    yyval.keyAlias= def;
			}
break;
case 53:
#line 352 "xkbparse.y"
	{ yyval.vmod= yystack.l_mark[-1].vmod; }
break;
case 54:
#line 356 "xkbparse.y"
	{ yyval.vmod= (VModDef *)AppendStmt(&yystack.l_mark[-2].vmod->common,&yystack.l_mark[0].vmod->common); }
break;
case 55:
#line 358 "xkbparse.y"
	{ yyval.vmod= yystack.l_mark[0].vmod; }
break;
case 56:
#line 362 "xkbparse.y"
	{ yyval.vmod= VModCreate(yystack.l_mark[0].sval,NULL); }
break;
case 57:
#line 364 "xkbparse.y"
	{ yyval.vmod= VModCreate(yystack.l_mark[-2].sval,yystack.l_mark[0].expr); }
break;
case 58:
#line 370 "xkbparse.y"
	{
			    yystack.l_mark[-4].interp->def= yystack.l_mark[-2].var;
			    yyval.interp= yystack.l_mark[-4].interp;
			}
break;
case 59:
#line 377 "xkbparse.y"
	{ yyval.interp= InterpCreate(XStringToKeysym(yystack.l_mark[-2].str), yystack.l_mark[0].expr); }
break;
case 60:
#line 379 "xkbparse.y"
	{ yyval.interp= InterpCreate(XStringToKeysym(yystack.l_mark[0].str), NULL); }
break;
case 61:
#line 383 "xkbparse.y"
	{ yyval.var= (VarDef *)AppendStmt(&yystack.l_mark[-1].var->common,&yystack.l_mark[0].var->common); }
break;
case 62:
#line 385 "xkbparse.y"
	{ yyval.var= yystack.l_mark[0].var; }
break;
case 63:
#line 391 "xkbparse.y"
	{ yyval.keyType= KeyTypeCreate(yystack.l_mark[-4].sval,yystack.l_mark[-2].var); }
break;
case 64:
#line 397 "xkbparse.y"
	{ yyval.syms= SymbolsCreate(yystack.l_mark[-4].str,(ExprDef *)yystack.l_mark[-2].var); }
break;
case 65:
#line 401 "xkbparse.y"
	{ yyval.var= (VarDef *)AppendStmt(&yystack.l_mark[-2].var->common,&yystack.l_mark[0].var->common); }
break;
case 66:
#line 403 "xkbparse.y"
	{ yyval.var= yystack.l_mark[0].var; }
break;
case 67:
#line 404 "xkbparse.y"
	{ yyval.var= NULL; }
break;
case 68:
#line 408 "xkbparse.y"
	{ yyval.var= VarCreate(yystack.l_mark[-2].expr,yystack.l_mark[0].expr); }
break;
case 69:
#line 410 "xkbparse.y"
	{ yyval.var= VarCreate(yystack.l_mark[-2].expr,yystack.l_mark[0].expr); }
break;
case 70:
#line 412 "xkbparse.y"
	{ yyval.var= BoolVarCreate(yystack.l_mark[0].sval,1); }
break;
case 71:
#line 414 "xkbparse.y"
	{ yyval.var= BoolVarCreate(yystack.l_mark[0].sval,0); }
break;
case 72:
#line 416 "xkbparse.y"
	{ yyval.var= VarCreate(NULL,yystack.l_mark[0].expr); }
break;
case 73:
#line 420 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[-1].expr; }
break;
case 74:
#line 422 "xkbparse.y"
	{ yyval.expr= ExprCreateUnary(ExprActionList,TypeAction,yystack.l_mark[-1].expr); }
break;
case 75:
#line 426 "xkbparse.y"
	{ yyval.groupCompat= GroupCompatCreate(yystack.l_mark[-3].ival,yystack.l_mark[-1].expr); }
break;
case 76:
#line 430 "xkbparse.y"
	{ yyval.modMask= ModMapCreate(yystack.l_mark[-4].sval,yystack.l_mark[-2].expr); }
break;
case 77:
#line 434 "xkbparse.y"
	{ yyval.ledMap= IndicatorMapCreate(yystack.l_mark[-4].sval,yystack.l_mark[-2].var); }
break;
case 78:
#line 438 "xkbparse.y"
	{ yyval.ledName= IndicatorNameCreate(yystack.l_mark[-3].ival,yystack.l_mark[-1].expr,False); }
break;
case 79:
#line 440 "xkbparse.y"
	{ yyval.ledName= IndicatorNameCreate(yystack.l_mark[-3].ival,yystack.l_mark[-1].expr,True); }
break;
case 80:
#line 444 "xkbparse.y"
	{ yyval.shape= ShapeDeclCreate(yystack.l_mark[-4].sval,(OutlineDef *)&yystack.l_mark[-2].outline->common); }
break;
case 81:
#line 446 "xkbparse.y"
	{ 
			    OutlineDef *outlines;
			    outlines= OutlineCreate(None,yystack.l_mark[-2].expr);
			    yyval.shape= ShapeDeclCreate(yystack.l_mark[-4].sval,outlines);
			}
break;
case 82:
#line 454 "xkbparse.y"
	{ yyval.section= SectionDeclCreate(yystack.l_mark[-4].sval,yystack.l_mark[-2].row); }
break;
case 83:
#line 458 "xkbparse.y"
	{ yyval.row=(RowDef *)AppendStmt(&yystack.l_mark[-1].row->common,&yystack.l_mark[0].row->common);}
break;
case 84:
#line 460 "xkbparse.y"
	{ yyval.row= yystack.l_mark[0].row; }
break;
case 85:
#line 464 "xkbparse.y"
	{ yyval.row= RowDeclCreate(yystack.l_mark[-2].key); }
break;
case 86:
#line 466 "xkbparse.y"
	{ yyval.row= (RowDef *)yystack.l_mark[0].var; }
break;
case 87:
#line 468 "xkbparse.y"
	{ yyval.row= (RowDef *)yystack.l_mark[0].doodad; }
break;
case 88:
#line 470 "xkbparse.y"
	{ yyval.row= (RowDef *)yystack.l_mark[0].ledMap; }
break;
case 89:
#line 472 "xkbparse.y"
	{ yyval.row= (RowDef *)yystack.l_mark[0].overlay; }
break;
case 90:
#line 476 "xkbparse.y"
	{ yyval.key=(KeyDef *)AppendStmt(&yystack.l_mark[-1].key->common,&yystack.l_mark[0].key->common);}
break;
case 91:
#line 478 "xkbparse.y"
	{ yyval.key= yystack.l_mark[0].key; }
break;
case 92:
#line 482 "xkbparse.y"
	{ yyval.key= yystack.l_mark[-2].key; }
break;
case 93:
#line 484 "xkbparse.y"
	{ yyval.key= (KeyDef *)yystack.l_mark[0].var; }
break;
case 94:
#line 488 "xkbparse.y"
	{ yyval.key=(KeyDef *)AppendStmt(&yystack.l_mark[-2].key->common,&yystack.l_mark[0].key->common);}
break;
case 95:
#line 490 "xkbparse.y"
	{ yyval.key= yystack.l_mark[0].key; }
break;
case 96:
#line 494 "xkbparse.y"
	{ yyval.key= KeyDeclCreate(yystack.l_mark[0].str,NULL); }
break;
case 97:
#line 496 "xkbparse.y"
	{ yyval.key= KeyDeclCreate(NULL,yystack.l_mark[-1].expr); }
break;
case 98:
#line 500 "xkbparse.y"
	{ yyval.overlay= OverlayDeclCreate(yystack.l_mark[-4].sval,yystack.l_mark[-2].olKey); }
break;
case 99:
#line 504 "xkbparse.y"
	{ 
			    yyval.olKey= (OverlayKeyDef *)
				AppendStmt(&yystack.l_mark[-2].olKey->common,&yystack.l_mark[0].olKey->common);
			}
break;
case 100:
#line 509 "xkbparse.y"
	{ yyval.olKey= yystack.l_mark[0].olKey; }
break;
case 101:
#line 513 "xkbparse.y"
	{ yyval.olKey= OverlayKeyCreate(yystack.l_mark[-2].str,yystack.l_mark[0].str); }
break;
case 102:
#line 517 "xkbparse.y"
	{ yyval.outline=(OutlineDef *)AppendStmt(&yystack.l_mark[-2].outline->common,&yystack.l_mark[0].outline->common);}
break;
case 103:
#line 519 "xkbparse.y"
	{ yyval.outline= yystack.l_mark[0].outline; }
break;
case 104:
#line 523 "xkbparse.y"
	{ yyval.outline= OutlineCreate(None,yystack.l_mark[-1].expr); }
break;
case 105:
#line 525 "xkbparse.y"
	{ yyval.outline= OutlineCreate(yystack.l_mark[-4].sval,yystack.l_mark[-1].expr); }
break;
case 106:
#line 527 "xkbparse.y"
	{ yyval.outline= OutlineCreate(yystack.l_mark[-2].sval,yystack.l_mark[0].expr); }
break;
case 107:
#line 531 "xkbparse.y"
	{ yyval.expr= (ExprDef *)AppendStmt(&yystack.l_mark[-2].expr->common,&yystack.l_mark[0].expr->common); }
break;
case 108:
#line 533 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[0].expr; }
break;
case 109:
#line 537 "xkbparse.y"
	{
			    ExprDef *expr;
			    expr= ExprCreate(ExprCoord,TypeUnknown);
			    expr->value.coord.x= yystack.l_mark[-3].ival;
			    expr->value.coord.y= yystack.l_mark[-1].ival;
			    yyval.expr= expr;
			}
break;
case 110:
#line 547 "xkbparse.y"
	{ yyval.doodad= DoodadCreate(yystack.l_mark[-5].uval,yystack.l_mark[-4].sval,yystack.l_mark[-2].var); }
break;
case 111:
#line 550 "xkbparse.y"
	{ yyval.uval= XkbTextDoodad; }
break;
case 112:
#line 551 "xkbparse.y"
	{ yyval.uval= XkbOutlineDoodad; }
break;
case 113:
#line 552 "xkbparse.y"
	{ yyval.uval= XkbSolidDoodad; }
break;
case 114:
#line 553 "xkbparse.y"
	{ yyval.uval= XkbLogoDoodad; }
break;
case 115:
#line 556 "xkbparse.y"
	{ yyval.sval= yystack.l_mark[0].sval; }
break;
case 116:
#line 557 "xkbparse.y"
	{ yyval.sval= yystack.l_mark[0].sval; }
break;
case 117:
#line 561 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"action",False); }
break;
case 118:
#line 563 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"interpret",False); }
break;
case 119:
#line 565 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"type",False); }
break;
case 120:
#line 567 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"key",False); }
break;
case 121:
#line 569 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"group",False); }
break;
case 122:
#line 571 "xkbparse.y"
	{yyval.sval=XkbInternAtom(NULL,"modifier_map",False);}
break;
case 123:
#line 573 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"indicator",False); }
break;
case 124:
#line 575 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"shape",False); }
break;
case 125:
#line 577 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"row",False); }
break;
case 126:
#line 579 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"section",False); }
break;
case 127:
#line 581 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"text",False); }
break;
case 128:
#line 584 "xkbparse.y"
	{ yyval.uval= yystack.l_mark[0].uval; }
break;
case 129:
#line 585 "xkbparse.y"
	{ yyval.uval= MergeDefault; }
break;
case 130:
#line 588 "xkbparse.y"
	{ yyval.uval= MergeDefault; }
break;
case 131:
#line 589 "xkbparse.y"
	{ yyval.uval= MergeAugment; }
break;
case 132:
#line 590 "xkbparse.y"
	{ yyval.uval= MergeOverride; }
break;
case 133:
#line 591 "xkbparse.y"
	{ yyval.uval= MergeReplace; }
break;
case 134:
#line 592 "xkbparse.y"
	{ yyval.uval= MergeAltForm; }
break;
case 135:
#line 595 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[0].expr; }
break;
case 136:
#line 596 "xkbparse.y"
	{ yyval.expr= NULL; }
break;
case 137:
#line 600 "xkbparse.y"
	{ yyval.expr= (ExprDef *)AppendStmt(&yystack.l_mark[-2].expr->common,&yystack.l_mark[0].expr->common); }
break;
case 138:
#line 602 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[0].expr; }
break;
case 139:
#line 606 "xkbparse.y"
	{ yyval.expr= ExprCreateBinary(OpDivide,yystack.l_mark[-2].expr,yystack.l_mark[0].expr); }
break;
case 140:
#line 608 "xkbparse.y"
	{ yyval.expr= ExprCreateBinary(OpAdd,yystack.l_mark[-2].expr,yystack.l_mark[0].expr); }
break;
case 141:
#line 610 "xkbparse.y"
	{ yyval.expr= ExprCreateBinary(OpSubtract,yystack.l_mark[-2].expr,yystack.l_mark[0].expr); }
break;
case 142:
#line 612 "xkbparse.y"
	{ yyval.expr= ExprCreateBinary(OpMultiply,yystack.l_mark[-2].expr,yystack.l_mark[0].expr); }
break;
case 143:
#line 614 "xkbparse.y"
	{ yyval.expr= ExprCreateBinary(OpAssign,yystack.l_mark[-2].expr,yystack.l_mark[0].expr); }
break;
case 144:
#line 616 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[0].expr; }
break;
case 145:
#line 620 "xkbparse.y"
	{ yyval.expr= ExprCreateUnary(OpNegate,yystack.l_mark[0].expr->type,yystack.l_mark[0].expr); }
break;
case 146:
#line 622 "xkbparse.y"
	{ yyval.expr= ExprCreateUnary(OpUnaryPlus,yystack.l_mark[0].expr->type,yystack.l_mark[0].expr); }
break;
case 147:
#line 624 "xkbparse.y"
	{ yyval.expr= ExprCreateUnary(OpNot,TypeBoolean,yystack.l_mark[0].expr); }
break;
case 148:
#line 626 "xkbparse.y"
	{ yyval.expr= ExprCreateUnary(OpInvert,yystack.l_mark[0].expr->type,yystack.l_mark[0].expr); }
break;
case 149:
#line 628 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[0].expr;  }
break;
case 150:
#line 630 "xkbparse.y"
	{ yyval.expr= ActionCreate(yystack.l_mark[-3].sval,yystack.l_mark[-1].expr); }
break;
case 151:
#line 632 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[0].expr;  }
break;
case 152:
#line 634 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[-1].expr;  }
break;
case 153:
#line 638 "xkbparse.y"
	{ yyval.expr= (ExprDef *)AppendStmt(&yystack.l_mark[-2].expr->common,&yystack.l_mark[0].expr->common); }
break;
case 154:
#line 640 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[0].expr; }
break;
case 155:
#line 644 "xkbparse.y"
	{ yyval.expr= ActionCreate(yystack.l_mark[-3].sval,yystack.l_mark[-1].expr); }
break;
case 156:
#line 648 "xkbparse.y"
	{
			    ExprDef *expr;
                            expr= ExprCreate(ExprIdent,TypeUnknown);
                            expr->value.str= yystack.l_mark[0].sval;
                            yyval.expr= expr;
			}
break;
case 157:
#line 655 "xkbparse.y"
	{
                            ExprDef *expr;
                            expr= ExprCreate(ExprFieldRef,TypeUnknown);
                            expr->value.field.element= yystack.l_mark[-2].sval;
                            expr->value.field.field= yystack.l_mark[0].sval;
                            yyval.expr= expr;
			}
break;
case 158:
#line 663 "xkbparse.y"
	{
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= None;
			    expr->value.array.field= yystack.l_mark[-3].sval;
			    expr->value.array.entry= yystack.l_mark[-1].expr;
			    yyval.expr= expr;
			}
break;
case 159:
#line 672 "xkbparse.y"
	{
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= yystack.l_mark[-5].sval;
			    expr->value.array.field= yystack.l_mark[-3].sval;
			    expr->value.array.entry= yystack.l_mark[-1].expr;
			    yyval.expr= expr;
			}
break;
case 160:
#line 683 "xkbparse.y"
	{
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeString);
                            expr->value.str= yystack.l_mark[0].sval;
                            yyval.expr= expr;
			}
break;
case 161:
#line 690 "xkbparse.y"
	{
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeInt);
                            expr->value.ival= yystack.l_mark[0].ival;
                            yyval.expr= expr;
			}
break;
case 162:
#line 697 "xkbparse.y"
	{
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeFloat);
			    expr->value.ival= yystack.l_mark[0].ival;
			    yyval.expr= expr;
			}
break;
case 163:
#line 704 "xkbparse.y"
	{
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeKeyName);
			    memset(expr->value.keyName,0,5);
			    strncpy(expr->value.keyName,yystack.l_mark[0].str,4);
			    free(yystack.l_mark[0].str);
			    yyval.expr= expr;
			}
break;
case 164:
#line 714 "xkbparse.y"
	{ yyval.expr= yystack.l_mark[0].expr; }
break;
case 165:
#line 715 "xkbparse.y"
	{ yyval.expr= NULL; }
break;
case 166:
#line 719 "xkbparse.y"
	{ yyval.expr= AppendKeysymList(yystack.l_mark[-2].expr,yystack.l_mark[0].str); }
break;
case 167:
#line 721 "xkbparse.y"
	{ yyval.expr= CreateKeysymList(yystack.l_mark[0].str); }
break;
case 168:
#line 724 "xkbparse.y"
	{ yyval.str= strdup(scanBuf); }
break;
case 169:
#line 725 "xkbparse.y"
	{ yyval.str= strdup("section"); }
break;
case 170:
#line 727 "xkbparse.y"
	{
			    if (yystack.l_mark[0].ival<10)	{ yyval.str= malloc(2); yyval.str[0]= '0' + yystack.l_mark[0].ival; yyval.str[1]= '\0'; }
			    else	{ yyval.str= malloc(19); snprintf(yyval.str, 19, "0x%x", yystack.l_mark[0].ival); }
			}
break;
case 171:
#line 733 "xkbparse.y"
	{ yyval.ival= -yystack.l_mark[0].ival; }
break;
case 172:
#line 734 "xkbparse.y"
	{ yyval.ival= yystack.l_mark[0].ival; }
break;
case 173:
#line 737 "xkbparse.y"
	{ yyval.ival= scanInt; }
break;
case 174:
#line 738 "xkbparse.y"
	{ yyval.ival= scanInt*XkbGeomPtsPerMM; }
break;
case 175:
#line 741 "xkbparse.y"
	{ yyval.ival= scanInt; }
break;
case 176:
#line 744 "xkbparse.y"
	{ yyval.ival= scanInt; }
break;
case 177:
#line 747 "xkbparse.y"
	{ yyval.str= strdup(scanBuf); }
break;
case 178:
#line 750 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,scanBuf,False); }
break;
case 179:
#line 751 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,"default",False); }
break;
case 180:
#line 754 "xkbparse.y"
	{ yyval.sval= XkbInternAtom(NULL,scanBuf,False); }
break;
case 181:
#line 757 "xkbparse.y"
	{ yyval.str= yystack.l_mark[0].str; }
break;
case 182:
#line 758 "xkbparse.y"
	{ yyval.str= NULL; }
break;
case 183:
#line 761 "xkbparse.y"
	{ yyval.str= strdup(scanBuf); }
break;
#line 1853 "xkbparse.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (short) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
