/*
 * .twmrc parsing externs
 *
 *
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * $XConsortium: parse.h,v 1.14 89/12/14 14:51:25 jim Exp $
 *
 *  8-Apr-88 Tom LaStrange        Initial Version.
 *
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_PARSE_H
#define _CTWM_PARSE_H

extern unsigned int mods_used;
extern int ConstrainedMoveTime;
extern int RaiseDelay;
extern bool ParseError;    /* error parsing the .twmrc file */

/* Needed in the lexer */
extern int (*twmInputFunc)(void);

bool LoadTwmrc(const char *filename);
void twmrc_error_prefix(void);

/*
 * Funcs in parse_be.c but used elsewhere in the codebase; these should
 * be looked at, because I think it probably means either they're
 * misused, misnamed, or the code calling them should be in parse_be
 * itself anyway.
 */
int ParseIRJustification(const char *s);
int ParseTitleJustification(const char *s);
int ParseAlignement(const char *s);
void assign_var_savecolor(void);

/*
 * This is in parse_be.c because it needs to look at keytable, but put
 * here because it's only built to be used early in main() as a sanity
 * test, to hopefully be seen by devs as soon as they mess up.
 */
void chk_keytable_order(void);

/*
 * Historical support for non-flex lex's is presumed no longer necessary.
 * Remnants kept for the moment just in case.
 */
#undef NON_FLEX_LEX
#ifdef NON_FLEX_LEX
void twmUnput(int c);
void TwmOutput(int c);
#endif /* NON_FLEX_LEX */


#endif /* _CTWM_PARSE_H */
