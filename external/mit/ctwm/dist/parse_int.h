/*
 * Parser internals that need to be shared across broken-up files
 */
#ifndef _CTWM_PARSE_INT_H
#define _CTWM_PARSE_INT_H

/* Stuff in parse_m4.c, if enabled */
#ifdef USEM4
FILE *start_m4(FILE *fraw);
#endif

#endif /* _CTWM_PARSE_INT_H */
