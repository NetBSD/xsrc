/*
 * Header for the various parse_yacc/gram.y backend funcs
 */
#ifndef _CTWM_PARSE_YACC_H
#define _CTWM_PARSE_YACC_H

void yyerror(char *s);

void InitGramVariables(void);
void RemoveDQuote(char *str);

MenuRoot *GetRoot(char *name, char *fore, char *back);

bool CheckWarpScreenArg(const char *s);
bool CheckWarpRingArg(const char *s);
bool CheckColormapArg(const char *s);
void GotButton(int butt, int func);
void GotKey(char *key, int func);
void GotTitleButton(char *bitmapname, int func, bool rightside);


extern char *Action;
extern char *Name;
extern MenuRoot *root, *pull;

extern int cont;
extern int mods;


#define DEFSTRING "default"


#endif /* _CTWM_PARSE_YACC_H */
