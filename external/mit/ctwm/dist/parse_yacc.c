/*
 * Parser routines called from yacc code (gram.y)
 *
 * This is very similar to the meaning of parse_be.c; the two may be
 * merged at some point.
 */

#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "functions_defs.h"
#include "util.h"
#include "screen.h"
#include "parse.h"
#include "parse_be.h"
#include "parse_yacc.h"
#include "win_decorations_init.h"

#include "gram.tab.h"

char *Action = "";
char *Name = "";
MenuRoot *root, *pull = NULL;

int cont = 0;
int mods = 0;
unsigned int mods_used = (ShiftMask | ControlMask | Mod1Mask);


void yyerror(char *s)
{
	twmrc_error_prefix();
	fprintf(stderr, "error in input file:  %s\n", s ? s : "");
	ParseError = true;
}

void InitGramVariables(void)
{
	mods = 0;
}

void RemoveDQuote(char *str)
{
	char *i, *o;
	int n;
	int count;

	for(i = str + 1, o = str; *i && *i != '\"'; o++) {
		if(*i == '\\') {
			switch(*++i) {
				case 'n':
					*o = '\n';
					i++;
					break;
				case 'b':
					*o = '\b';
					i++;
					break;
				case 'r':
					*o = '\r';
					i++;
					break;
				case 't':
					*o = '\t';
					i++;
					break;
				case 'f':
					*o = '\f';
					i++;
					break;
				case '0':
					if(*++i == 'x') {
						goto hex;
					}
					else {
						--i;
					}
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					n = 0;
					count = 0;
					while(*i >= '0' && *i <= '7' && count < 3) {
						n = (n << 3) + (*i++ - '0');
						count++;
					}
					*o = n;
					break;
hex:
				case 'x':
					n = 0;
					count = 0;
					while(i++, count++ < 2) {
						if(*i >= '0' && *i <= '9') {
							n = (n << 4) + (*i - '0');
						}
						else if(*i >= 'a' && *i <= 'f') {
							n = (n << 4) + (*i - 'a') + 10;
						}
						else if(*i >= 'A' && *i <= 'F') {
							n = (n << 4) + (*i - 'A') + 10;
						}
						else {
							break;
						}
					}
					*o = n;
					break;
				case '\n':
					i++;    /* punt */
					o--;    /* to account for o++ at end of loop */
					break;
				case '\"':
				case '\'':
				case '\\':
				default:
					*o = *i++;
					break;
			}
		}
		else {
			*o = *i++;
		}
	}
	*o = '\0';
}

MenuRoot *GetRoot(char *name, char *fore, char *back)
{
	MenuRoot *tmp;

	tmp = FindMenuRoot(name);
	if(tmp == NULL) {
		tmp = NewMenuRoot(name);
	}

	if(fore) {
		bool save;

		save = Scr->FirstTime;
		Scr->FirstTime = true;
		GetColor(COLOR, &tmp->highlight.fore, fore);
		GetColor(COLOR, &tmp->highlight.back, back);
		Scr->FirstTime = save;
	}

	return tmp;
}

void GotButton(int butt, int func)
{
	int i;
	MenuItem *item;

	for(i = 0; i < NUM_CONTEXTS; i++) {
		if((cont & (1 << i)) == 0) {
			continue;
		}

		if(func == F_MENU) {
			pull->prev = NULL;
			AddFuncButton(butt, i, mods, func, pull, NULL);
		}
		else {
			root = GetRoot(TWM_ROOT, NULL, NULL);
			item = AddToMenu(root, "x", Action, NULL, func, NULL, NULL);
			AddFuncButton(butt, i, mods, func, NULL, item);
		}
	}

	Action = "";
	pull = NULL;
	cont = 0;
	mods_used |= mods;
	mods = 0;
}

void GotKey(char *key, int func)
{
	int i;

	for(i = 0; i < NUM_CONTEXTS; i++) {
		if((cont & (1 << i)) == 0) {
			continue;
		}

		if(func == F_MENU) {
			pull->prev = NULL;
			if(!AddFuncKey(key, i, mods, func, pull, Name, Action)) {
				break;
			}
		}
		else if(!AddFuncKey(key, i, mods, func, NULL, Name, Action)) {
			break;
		}
	}

	Action = "";
	pull = NULL;
	cont = 0;
	mods_used |= mods;
	mods = 0;
}


void GotTitleButton(char *bitmapname, int func, bool rightside)
{
	if(!CreateTitleButton(bitmapname, func, Action, pull, rightside, true)) {
		twmrc_error_prefix();
		fprintf(stderr,
		        "unable to create %s titlebutton \"%s\"\n",
		        rightside ? "right" : "left", bitmapname);
	}
	Action = "";
	pull = NULL;
}


/* Check f.warptoscreen arg */
bool
CheckWarpScreenArg(const char *s)
{
	/* next/prev/back are valid */
	if(strcasecmp(s, WARPSCREEN_NEXT) == 0 ||
	                strcasecmp(s, WARPSCREEN_PREV) == 0 ||
	                strcasecmp(s, WARPSCREEN_BACK) == 0) {
		return true;
	}

	/* Or if the whole thing is numeric, it's valid too */
	for(; *s && Isascii(*s) && Isdigit(*s); s++) {
		/* nada */;
	}
	return (*s ? false : true);
}


/* f.warptoring arg */
bool
CheckWarpRingArg(const char *s)
{
	if(strcasecmp(s, WARPSCREEN_NEXT) == 0 ||
	                strcasecmp(s, WARPSCREEN_PREV) == 0) {
		return true;
	}

	return false;
}


/* f.colormap arg */
bool
CheckColormapArg(const char *s)
{
	if(strcasecmp(s, COLORMAP_NEXT) == 0 ||
	                strcasecmp(s, COLORMAP_PREV) == 0 ||
	                strcasecmp(s, COLORMAP_DEFAULT) == 0) {
		return true;
	}

	return false;
}
