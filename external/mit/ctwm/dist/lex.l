%{
/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * Copyright 1992 Claude Lecommandeur.
 */

/***********************************************************************
 *
 * $XConsortium: lex.l,v 1.62 89/12/10 17:46:33 jim Exp $
 *
 * .twmrc lex file
 *
 * 12-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#include "ctwm.h"
#include "functions_defs.h"
#include "parse.h"
#include "parse_be.h"

#include "gram.tab.h"

/*
 * flex uses a YY_INPUT macro internally rather than input.  It doesn't
 * need unput since it maintains state internally.
 */
#ifdef FLEX_SCANNER
#  undef YY_INPUT
#  define YY_INPUT(buf,result,max_size) {buf[0]=twmInputFunc();result=(buf[0] != 0);}
#endif

/*
 * YY_FLEX_{MAJOR,MINOR}_VERSION was added in 2.5.1 (28Mar95); just in
 * case we need to do version checks of stuff.
 */

/*
 * flex listens to these to avoid declaring these functions, which we
 * don't use, so suppress the warning.
 */
#define YY_NO_UNPUT
#define YY_NO_INPUT

%}

string				\"([^"]|\\.)*\"
number				[0-9]+

 /* Requires flex 2.5.1 (28Mar95) */
%option noyywrap
 /* Requires flex 2.5.2 (25Apr95) */
%option nounput

%%
"{"				{ return (LB); }
"}"				{ return (RB); }
"("				{ return (LP); }
")"				{ return (RP); }
"="				{ return (EQUALS); }
":"				{ return (COLON); }
"+"				{ return PLUS; }
"-"				{ return MINUS; }
"|"				{ return OR; }

[a-zA-Z\.]+			{ int token = parse_keyword (yytext, 
							     &yylval.num);
				  if (token == ERRORTOKEN) {
				      twmrc_error_prefix();
				      fprintf (stderr,
				       "ignoring unknown keyword:  %s\n", 
					       yytext);
				      ParseError = true;
				  } else 
				    return token;
				}

"!"				{ yylval.num = F_EXEC; return FSKEYWORD; }

{string}			{ yylval.ptr = yytext; return STRING; }
{number}			{ sscanf(yytext, "%d", &yylval.num);
				  return NUMBER; }
\#[^\n]*\n			{;}
[\n\t ]				{;}
.				{
				  twmrc_error_prefix();
				  fprintf (stderr, 
					   "ignoring character \"%s\"\n",
					   yytext);
				  ParseError = true;
				}
%%

/*
 * In flex versions before 2.4.1 (30Nov93), yywrap was a macro, not a
 * function.  There's no way we really support versions that old, but
 * what the heck...
 *
 * This function should actually be unused due to the noyywrap %option
 * specified above, but is left around in case of weird edge cases.
 */
#ifndef yywrap
int yywrap(void) { return(1);}
#endif

/* AT&T lex uses the input/unput macros */
#ifndef FLEX_SCANNER
/*
 * I believe Solaris at least recently recently (and maybe currently)
 * ships with an AT&T lex, but also with flex beside it.  Plan
 * 9 might ship only A&T lex?
 *
 * However, our current build toolchain doesn't look for any lex other
 * than flex.  So #error out if we get here; adventurous users might take
 * this out, and it might work; let us know!
 */
#error Not supported on non-flex; remove this line at your own risk
#ifdef NON_FLEX_LEX
#undef unput
#undef input
#undef output
#undef feof
#define unput(c)	twmUnput(c)
#define input()		(*twmInputFunc)()
#define output(c)	TwmOutput(c)
#define feof()		(1)
#endif /* NON_FLEX_LEX */
#endif /* !FLEX_SCANNER */

