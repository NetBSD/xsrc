/*
 * Copyright (c) 2001 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the
 * XFree86 Project.
 *
 * Author: Paulo César Pereira de Andrade
 */

/* $XFree86: xc/programs/xedit/lisp/debugger.c,v 1.11 2001/10/28 03:34:29 tsi Exp $ */

#include <ctype.h>
#include "debugger.h"

#define DebuggerHelp		0
#define DebuggerAbort		1
#define DebuggerBacktrace	2
#define DebuggerContinue	3
#define DebuggerFinish		4
#define DebuggerFrame		5
#define DebuggerNext		6
#define DebuggerNexti		7
#define DebuggerPrint		8
#define DebuggerStep		9
#define DebuggerStepi		10
#define DebuggerBreak		11
#define DebuggerDelete		12
#define DebuggerDown		13
#define DebuggerUp		14
#define DebuggerInfo		15
#define DebuggerWatch		16

#define DebuggerInfoBreakpoints	0
#define DebuggerInfoBacktrace	1

/*
 * Prototypes
 */
static void LispDebuggerCommand(LispMac*, LispObj *obj);

/*
 * Initialization
 */
static struct {
    char *name;
    int action;
} commands[] = {
    {"help",		DebuggerHelp},
    {"abort",		DebuggerAbort},
    {"backtrace",	DebuggerBacktrace},
    {"b",		DebuggerBreak},
    {"break",		DebuggerBreak},
    {"bt",		DebuggerBacktrace},
    {"continue",	DebuggerContinue},
    {"d",		DebuggerDelete},
    {"delete",		DebuggerDelete},
    {"down",		DebuggerDown},
    {"finish",		DebuggerFinish},
    {"frame",		DebuggerFrame},
    {"info",		DebuggerInfo},
    {"n",		DebuggerNext},
    {"next",		DebuggerNext},
    {"ni",		DebuggerNexti},
    {"nexti",		DebuggerNexti},
    {"print",		DebuggerPrint},
    {"run",		DebuggerContinue},
    {"s",		DebuggerStep},
    {"step",		DebuggerStep},
    {"si",		DebuggerStepi},
    {"stepi",		DebuggerStepi},
    {"up",		DebuggerUp},
    {"watch",		DebuggerWatch},
};

static struct {
    char *name;
    int subaction;
} info_commands[] = {
    {"breakpoints",	DebuggerInfoBreakpoints},
    {"stack",		DebuggerInfoBacktrace},
    {"watchpoints",	DebuggerInfoBreakpoints},
};

static char debugger_help[] =
"Available commands are:\n\
\n\
help		- This message.\n\
abort		- Abort the current execution, and return to toplevel.\n\
backtrace, bt	- Print backtrace.\n\
b, break	- Set breakpoint at function name argument.\n\
continue	- Continue execution.\n\
d, delete	- Delete breakpoint(s), all breakpoint if no arguments given.\n\
down		- Set environment to frame called by the current one.\n\
finish		- Executes until current form is finished.\n\
frame		- Set environment to selected frame.\n\
info		- Prints information about the debugger state.\n\
n, next		- Evaluate next form.\n\
nexti, ni	- Evaluate next form, including variables.\n\
print		- Print value of variable name argument.\n\
run		- Continue execution.\n\
s, step		- Evaluate next form, stopping on any subforms.\n\
si, stepi	- Evaluate next form including variables, stopping on subforms.\n\
up		- Set environment to frame that called the current one.\n\
\n\
Commands may be abbreviated.\n";

static char debugger_info_help[] =
"Available subcommands are:\n\
\n\
breakpoints	- List and prints status of breakpoints, and watchpoints.\n\
stack		- Backtrace of stack.\n\
watchpoints	- List and prints status of watchpoints, and breakpoints.\n\
\n\
Subcommands may be abbreviated.\n";

/* Debugger variables layout (if you change it, update description):
 *
 * DBG
 *	is a macro for mac->dbglist
 *	is a NIL terminated list
 *	every element is a list in the format (NOT NIL terminated):
 *	(list* NAM ARG ENV LEX)
 *	where
 *		NAM is an ATOM for the function/macro name
 *		    or NIL for lambda expressions
 *		ARG is NAM arguments (a LIST)
 *		ENV is the contents of the global ENV (a LIST)
 *		LEX is the contents of the global LEX (a LIST)
 *	new elements are added to the beggining of the list
 *
 * BRK
 *	is macro for mac->brklist
 *	is a NIL terminated list
 *	every element is a list in the format (NIL terminated):
 *	(list NAM IDX TYP HIT VAR VAL FRM)
 *	where
 *		NAM is an ATOM for the name of the object at
 *		    wich the breakpoint was added
 *		IDX is a REAL, the breakpoint number
 *		    must be stored, as breakpoints may be deleted
 *		TYP is a REAL that must be an integer of enum LispBreakType
 *		HIT is a REAL, with the number of times this breakpoint was
 *		    hitted.
 *		VAR variable to watch a SYMBOL	(not needed for breakpoints)
 *		VAL value of watched variable	(not needed for breakpoints)
 *		FRM frame where variable started being watched
 *						(not needed for breakpoints)
 *	new elements are added to the end of the list
 */

/*
 * Implementation
 */
void
LispDebugger(LispMac *mac, LispDebugCall call, LispObj *name, LispObj *arg)
{
    int force = 0;
    LispObj *obj, *prev;

    switch (call) {
	case LispDebugCallBegin:
	    ++mac->debug_level;
	    GCProtect();
	    DBG = CONS(CONS(name, CONS(arg, CONS(ENV, LEX))), DBG);
	    GCUProtect();
	    for (obj = BRK; obj != NIL; obj = CDR(obj))
		if (STRPTR(CAR(CAR(obj))) == STRPTR(name) &&
		    CAR(CDR(CDR(CAR(obj))))->data.real == LispDebugBreakFunction)
		    break;
	    if (obj != NIL) {
		if (!mac->newline)
		    fputc('\n', lisp_stdout);
		fprintf(lisp_stdout, "BREAK #");
		LispPrintObj(mac, NIL, CAR(CDR(CAR(obj))), 1);
		fprintf(lisp_stdout, "> (");
		LispPrintObj(mac, NIL, CAR(CAR(DBG)), 1);
		fputc(' ', lisp_stdout);
		LispPrintObj(mac, NIL, CAR(CDR(CAR(DBG))), 0);
		fprintf(lisp_stdout, ")\n");
		force = 1;
		/* update hits counter */
		CAR(CDR(CDR(CDR(CAR(obj)))))->data.real += 1.0;
	    }
	    break;
	case LispDebugCallEnd:
	    DBG = CDR(DBG);
	    if (mac->debug_level < mac->debug_step)
		mac->debug_step = mac->debug_level;
	    --mac->debug_level;
	    break;
	case LispDebugCallFatal:
	    LispDebuggerCommand(mac, NIL);
	    return;
	case LispDebugCallBegini:
	case LispDebugCallEndi:
	    if (mac->debug != LispDebugNexti && mac->debug != LispDebugStepi)
		return;
	    break;
	case LispDebugCallWatch:
	    break;
    }

    /* didn't return, check watchpoints */
    if (call == LispDebugCallEnd || call == LispDebugCallWatch) {
watch_again:
	for (prev = obj = BRK; obj != NIL; prev = obj, obj = CDR(obj)) {
	    if (CAR(CDR(CDR(CAR(obj))))->data.real == LispDebugBreakVariable) {
		/* the variable */
		LispObj *wat = CAR(CDR(CDR(CDR(CDR(CAR(obj))))));
		LispObj *sym = LispGetVarCons(mac, CAAR(obj));
		LispObj *frm = CAR(CDR(CDR(CDR(CDR(CDR(CDR(CAR(obj))))))));

		if ((sym == NULL && mac->debug_level <= 0) ||
		    (sym != wat && frm->data.real > mac->debug_level)) {
		    fprintf(lisp_stdout, "WATCH #%g> %s deleted. Variable does "
			    "not exist anymore.\n",
			    CAR(CDR(CAR(obj)))->data.real,
			    STRPTR(CAR(CAR(obj))));
		    /* force debugger to stop */
		    force = 1;
		    if (obj == prev) {
			BRK = CDR(BRK);
			goto watch_again;
		    }
		    else
			CDR(prev) = CDR(obj);
		    obj = prev;
		}
		else {
		    /* current value */
		    LispObj *cur = CDR(wat);
		    /* last value */
		    LispObj *val = CAR(CDR(CDR(CDR(CDR(CDR(CAR(obj)))))));
		    if (_LispEqual(mac, val, cur) == NIL) {
			fprintf(lisp_stdout, "WATCH #%g> %s\n",
				CAR(CDR(CAR(obj)))->data.real,
				STRPTR(CAR(CAR(obj))));
			fprintf(lisp_stdout, "OLD: ");
			LispPrintObj(mac, NIL, val, 1);
			fprintf(lisp_stdout, "\nNEW: ");
			LispPrintObj(mac, NIL, cur, 1);
			fputc('\n', lisp_stdout);

			/* update current value */
			CAR(CDR(CDR(CDR(CDR(CDR(CAR(obj))))))) = cur;
			/* update hits counter */
			CAR(CDR(CDR(CDR(CAR(obj)))))->data.real += 1.0;
			/* force debugger to stop */
			force = 1;
		    }
		}
	    }
	}

	if (call == LispDebugCallWatch)
	    /* special call, just don't keep gc protected variables that may be
	     * using a lot of memory... */
	    return;
    }

    switch (mac->debug) {
	case LispDebugUnspec:
	    LispDebuggerCommand(mac, NIL);
	    goto debugger_done;
	case LispDebugRun:
	    if (force)
		LispDebuggerCommand(mac, NIL);
	    goto debugger_done;
	case LispDebugFinish:
	    if (!force &&
		(call != LispDebugCallEnd || mac->debug_level != mac->debug_step))
		goto debugger_done;
	    break;
	case LispDebugNext:
	    if (call == LispDebugCallBegin) {
		if (!force && mac->debug_level != mac->debug_step)
		    goto debugger_done;
	    }
	    else if (call == LispDebugCallEnd) {
		if (!force && mac->debug_level >= mac->debug_step)
		    goto debugger_done;
	    }
	    break;
	case LispDebugStep:
	    break;
	case LispDebugNexti:
	    if (call == LispDebugCallBegini) {
		if (!force && mac->debug_level != mac->debug_step)
		    goto debugger_done;
	    }
	    else if (call == LispDebugCallEndi) {
		if (!force && mac->debug_level >= mac->debug_step)
		    goto debugger_done;
	    }
	    break;
	case LispDebugStepi:
	    break;
    }

    if (call == LispDebugCallBegin) {
	if (!mac->newline)
	    fputc('\n', lisp_stdout);
	fprintf(lisp_stdout, "#%d> ", mac->debug_level);

	fputc('(', lisp_stdout);
	LispPrintObj(mac, NIL, CAR(CAR(DBG)), 1);
	fputc(' ', lisp_stdout);
	LispPrintObj(mac, NIL, CAR(CDR(CAR(DBG))), 0);
	fprintf(lisp_stdout, ")\n");
	LispDebuggerCommand(mac, NIL);
    }
    else if (call == LispDebugCallEnd) {
	if (!mac->newline)
	    fputc('\n', lisp_stdout);
	fprintf(lisp_stdout, "#%d= ", mac->debug_level + 1);

	LispPrintObj(mac, NIL, arg, 1);
	fputc('\n', lisp_stdout);
	LispDebuggerCommand(mac, NIL);
    }
    else if (call == LispDebugCallBegini) {
	if (!mac->newline)
	    fputc('\n', lisp_stdout);
	fprintf(lisp_stdout, "#%d+> ",	mac->debug_level + 1);

	LispPrintObj(mac, NIL, arg, 1);
	fputc('\n', lisp_stdout);
	LispDebuggerCommand(mac, arg);
    }
    else if (call == LispDebugCallEndi) {
	if (!mac->newline)
	    fputc('\n', lisp_stdout);
	fprintf(lisp_stdout, "#%d+= ", mac->debug_level + 1);

	LispPrintObj(mac, NIL, arg, 1);
	fputc('\n', lisp_stdout);
	mac->newline = 1;
	LispDebuggerCommand(mac, arg);
    }
    else if (force)
	LispDebuggerCommand(mac, arg);

debugger_done:
    return;
}

static void
LispDebuggerCommand(LispMac *mac, LispObj *args)
{
    LispObj *obj, *frm, *curframe,
	    *old_frm = FRM, *old_env = ENV, *old_lex = LEX;
    int i = 0, frame, matches, action = -1, subaction = 0;
    char *cmd, *arg, *ptr, line[256];

    frame = mac->debug_level;
    curframe = CAR(DBG);

    line[0] = '\0';
    arg = line;
    for (;;) {
	fprintf(lisp_stdout, "%s", DBGPROMPT);
	fflush(lisp_stdout);
	if (fgets(line, sizeof(line), lisp_stdin) == NULL) {
	    fputc('\n', lisp_stdout);
	    return;
	}
	/* get command */
	ptr = line;
	while (*ptr && isspace(*ptr))
	    ++ptr;
	cmd = ptr;
	while (*ptr && !isspace(*ptr))
	    ++ptr;
	if (*ptr)
	    *ptr++ = '\0';

	if (*cmd) {	/* if *cmd is nul, then arg may be still set */
	    /* get argument(s) */
	    while (*ptr && isspace(*ptr))
		++ptr;
	    arg = ptr;
	    /* goto end of line */
	    if (*ptr) {
		while (*ptr)
		    ++ptr;
		--ptr;
		while (*ptr && isspace(*ptr))
		    --ptr;
		if (*ptr)
		    *++ptr = '\0';
	    }
	}

	if (*cmd == '\0') {
	    if (action < 0) {
		if (mac->debug == LispDebugFinish)
		    action = DebuggerFinish;
		else if (mac->debug == LispDebugNext)
		    action = DebuggerNext;
		else if (mac->debug == LispDebugStep)
		    action = DebuggerStep;
		else if (mac->debug == LispDebugNexti)
		    action = DebuggerNexti;
		else if (mac->debug == LispDebugStepi)
		    action = DebuggerStepi;
		else if (mac->debug == LispDebugRun)
		    action = DebuggerContinue;
		else
		    continue;
	    }
	}
	else {
	    for (i = matches = 0; i < sizeof(commands) / sizeof(commands[0]);
		 i++) {
		char *str = commands[i].name;

		ptr = cmd;
		while (*ptr && *ptr == *str) {
		    ++ptr;
		    ++str;
		}
		if (*ptr == '\0') {
		    action = commands[i].action;
		    if (*str == '\0') {
			matches = 1;
			break;
		    }
		    ++matches;
		}
	    }
	    if (matches == 0) {
		fprintf(lisp_stdout, "* Command unknown: %s. "
			"Type help for help.\n", cmd);
		continue;
	    }
	    else if (matches > 1) {
		fprintf(lisp_stdout, "* Command is ambiguous: %s. "
			"Type help for help.\n", cmd);
		continue;
	    }
	}

	switch (action) {
	    case DebuggerHelp:
		fprintf(lisp_stdout, debugger_help);
		break;
	    case DebuggerInfo:
		if (*arg == '\0') {
		    fprintf(lisp_stdout, debugger_info_help);
		    break;
		}

		for (i = matches = 0;
		     i < sizeof(info_commands) / sizeof(info_commands[0]);
		     i++) {
		    char *str = info_commands[i].name;

		    ptr = arg;
		    while (*ptr && *ptr == *str) {
			++ptr;
			++str;
		    }
		    if (*ptr == '\0') {
			subaction = info_commands[i].subaction;
			if (*str == '\0') {
			    matches = 1;
			    break;
			}
			++matches;
		    }
		}
		if (matches == 0) {
		    fprintf(lisp_stdout, "* Command unknown: %s. "
			    "Type info for help.\n", arg);
		    continue;
		}
		else if (matches > 1) {
		    fprintf(lisp_stdout, "* Command is ambiguous: %s. "
			    "Type info for help.\n", arg);
		    continue;
		}

		switch (subaction) {
		    case DebuggerInfoBreakpoints:
			fprintf(lisp_stdout, "Num\tHits\tType\t\tWhat\n");
			for (obj = BRK; obj != NIL; obj = CDR(obj)) {
			    /* breakpoint number */
			    fputc('#', lisp_stdout);
			    LispPrintObj(mac, NIL, CAR(CDR(CAR(obj))), 1);

			    /* number of hits */
			    fprintf(lisp_stdout, "\t");
			    LispPrintObj(mac, NIL,
					 CAR(CDR(CDR(CDR(CAR(obj))))), 1);

			    /* breakpoint type */
			    fprintf(lisp_stdout, "\t");
			    switch ((int)CAR(CDR(CDR(CAR(obj))))->data.real) {
				case LispDebugBreakFunction:
				    fprintf(lisp_stdout, "Function");
				    break;
				case LispDebugBreakVariable:
				    fprintf(lisp_stdout, "Variable");
				    break;
			    }

			    /* breakpoint object */
			    fprintf(lisp_stdout, "\t");
			    LispPrintObj(mac, NIL, CAR(CAR(obj)), 1);
			    fputc('\n', lisp_stdout);
			}
			break;
		    case DebuggerInfoBacktrace:
			goto debugger_print_backtrace;
		}
		break;
	    case DebuggerAbort:
		/* Almost the same code as LispDestroy */
		while (mac->mem.mem_level)
		    free(mac->mem.mem[--mac->mem.mem_level]);

		LispTopLevel(mac);
		if (mac->st) {
		    mac->cp = &(mac->st[strlen(mac->st)]);
		    mac->tok = 0;
		}
		mac->column = 0;
		mac->newline = 1;
		siglongjmp(mac->jmp, 1);/* don't need to restore environment */
		/*NOTREACHED*/
		break;
	    case DebuggerBreak:
		for (ptr = arg; *ptr; ptr++) {
		    if (isspace(*ptr))
			break;
		    else
			*ptr = toupper(*ptr);
		}

		if (!*arg || *ptr || strchr(arg, '(') || strchr(arg, '(') ||
		    strchr(arg, ';')) {
		    fprintf(lisp_stdout, "* Bad function name "
			    "'%s' specified.\n", arg);
		}
		else {
		    for (obj = frm = BRK; obj != NIL; frm = obj, obj = CDR(obj))
			;
		    i = mac->debug_break;
		    ++mac->debug_break;
		    GCProtect();
		    obj = CONS(ATOM2(arg),
			       CONS(REAL(i),
				    CONS(REAL(LispDebugBreakFunction),
					 CONS(REAL(0), NIL))));
		    if (BRK == NIL)
			BRK = CONS(obj, NIL);
		    else
			CDR(frm) = CONS(obj, NIL);
		    GCUProtect();
		}
		break;
	    case DebuggerWatch: {
		int vframe;
		LispObj *sym, *val, *atom;

		/* make variable name uppercase, an ATOM */
		ptr = arg;
		while (*ptr) {
		    *ptr = toupper(*ptr);
		    ++ptr;
		}
		atom = ATOM2(arg);
		val = LispGetVar(mac, atom);
		if (val == NULL) {
		    fprintf(lisp_stdout, "* No variable named '%s' "
			    "in the selected frame.\n", arg);
		    break;
		}

		/* variable is available at the current frame */
		sym = LispGetVarCons(mac, atom);

		/* find the lowest frame where the variable is visible */
		vframe = 0;
		if (frame) {
		    for (; vframe < frame; vframe++) {
			for (frm = DBG, i = mac->debug_level; i > vframe;
			     frm = CDR(frm), i--)
			    ;
			obj = CAR(frm);
			if (FRM == old_frm) {
			    /* if first time selecting a new frame */
			    GCProtect();
			    FRM = CONS(ENV, old_frm);
			    GCUProtect();
			}
			ENV = CAR(CDR(CDR(obj)));
			LEX = CDR(CDR(CDR(obj)));

			if (LispGetVarCons(mac, atom) == sym)
			    /* got variable initial frame */
			    break;
		    }
		    vframe = i;
		    if (vframe != frame) {
			/* restore environment */
			for (frm = DBG, i = mac->debug_level; i > frame;
			     frm = CDR(frm), i--)
			    ;
			obj = CAR(frm);
			ENV = CAR(CDR(CDR(obj)));
			LEX = CDR(CDR(CDR(obj)));
		    }
		}

		i = mac->debug_break;
		++mac->debug_break;
		for (obj = frm = BRK; obj != NIL; frm = obj, obj = CDR(obj))
		    ;

		GCProtect();
		obj = CONS(atom,				      /* NAM */
			   CONS(REAL(i),			      /* IDX */
				CONS(REAL(LispDebugBreakVariable),    /* TYP */
				     CONS(REAL(0),		      /* HIT */
					  CONS(sym,		      /* VAR */
					       CONS(val,	      /* VAL */
						    CONS(REAL(vframe),/* FRM */
							      NIL)))))));

		/* add watchpoint */
		if (BRK == NIL)
		    BRK = CONS(obj, NIL);
		else
		    CDR(frm) = CONS(obj, NIL);
		GCUProtect();
	    }	break;
	    case DebuggerDelete:
		if (*arg == 0) {
		    int confirm = 0;

		    for (;;) {
			int ch;

			fprintf(lisp_stdout, "* Delete all breakpoints? "
				"(y or n) ");
			fflush(lisp_stdout);
			if ((ch = fgetc(lisp_stdin)) == '\n')
			    continue;
			while ((i = fgetc(lisp_stdin)) != '\n' && i != EOF)
			    ;
			if (tolower(ch) == 'n')
			    break;
			else if (tolower(ch) == 'y') {
			    confirm = 1;
			    break;
			}
		    }
		    if (confirm)
			BRK = NIL;
		}
		else {
		    for (ptr = arg; *ptr;) {
			while (*ptr && isdigit(*ptr))
			    ++ptr;
			if (*ptr && !isspace(*ptr)) {
			    *ptr = '\0';
			    fprintf(lisp_stdout, "* Bad breakpoint number "
				    "'%s' specified.\n", arg);
			    break;
			}
			i = atoi(arg);
			for (obj = frm = BRK; frm != NIL;
			     obj = frm, frm = CDR(frm))
			    if (CAR(CDR(CAR(frm)))->data.real == i)
				break;
			if (frm == NIL) {
			    fprintf(lisp_stdout, "* No breakpoint number "
				    "%d available.\n", i);
			    break;
			}
			if (obj == frm)
			    BRK = CDR(BRK);
			else
			    CDR(obj) = CDR(frm);
			while (*ptr && isspace(*ptr))
			    ++ptr;
			arg = ptr;
		    }
		}
		break;
	    case DebuggerFrame:
		i = -1;
		ptr = arg;
		if (*ptr) {
		    i = 0;
		    while (*ptr && isdigit(*ptr)) {
			i *= 10;
			i += *ptr - '0';
			++ptr;
		    }
		    if (*ptr) {
			fprintf(lisp_stdout, "* Frame identifier must "
				"be a positive number.\n");
			break;
		    }
		}
		else
		    goto debugger_print_frame;
		if (i >= 0 && i <= mac->debug_level)
		    goto debugger_new_frame;
		fprintf(lisp_stdout, "* No such frame %d.\n", i);
		break;
	    case DebuggerDown:
		if (frame + 1 > mac->debug_level) {
		    fprintf(lisp_stdout, "* Cannot go down.\n");
		    break;
		}
		i = frame + 1;
		goto debugger_new_frame;
		break;
	    case DebuggerUp:
		if (frame == 0) {
		    fprintf(lisp_stdout, "* Cannot go up.\n");
		    break;
		}
		i = frame - 1;
		goto debugger_new_frame;
		break;
	    case DebuggerPrint:
		ptr = arg;
		while (*ptr) {
		    *ptr = toupper(*ptr);
		    ++ptr;
		}
		obj = LispGetVar(mac, ATOM2(arg));
		if (obj) {
		    LispPrintObj(mac, NIL, obj, 1);
		    fputc('\n', lisp_stdout);
		}
		else
		    fprintf(lisp_stdout, "* No variable named '%s' "
			    "in the selected frame.\n", arg);
		break;
	    case DebuggerBacktrace:
debugger_print_backtrace:
		if (DBG == NIL) {
		    fprintf(lisp_stdout, "* No stack.\n");
		    break;
		}
		DBG = LispReverse(DBG);
		for (obj = DBG, i = 0; obj != NIL; obj = CDR(obj), i++) {
		    frm = CAR(obj);
		    fprintf(lisp_stdout, "#%d> (", i);
		    LispPrintObj(mac, NIL, CAR(frm), 1);
		    fputc(' ', lisp_stdout);
		    LispPrintObj(mac, NIL, CAR(CDR(frm)), 0);
		    fprintf(lisp_stdout, ")\n");
		}
		DBG = LispReverse(DBG);
		if (mac->debug == LispDebugNexti ||
		    mac->debug == LispDebugStepi) {
		    fprintf(lisp_stdout, "#%d+> ", i);
		    LispPrintObj(mac, NIL, args, 1);
		    fputc('\n', lisp_stdout);
		}
		break;
	    case DebuggerContinue:
		mac->debug = LispDebugRun;
		goto debugger_command_done;
	    case DebuggerFinish:
		if (mac->debug != LispDebugFinish) {
		    mac->debug_step = mac->debug_level - 2;
		    mac->debug = LispDebugFinish;
		}
		else
		    mac->debug_step = mac->debug_level - 1;
		goto debugger_command_done;
	    case DebuggerNext:
		if (mac->debug != LispDebugNext &&
		    mac->debug != LispDebugNexti) {
		    mac->debug = LispDebugNext;
		    mac->debug_step = mac->debug_level + 1;
		}
		goto debugger_command_done;
	    case DebuggerNexti:
		if (mac->debug != LispDebugNext &&
		    mac->debug != LispDebugNexti) {
		    mac->debug = LispDebugNexti;
		    mac->debug_step = mac->debug_level + 1;
		}
		goto debugger_command_done;
	    case DebuggerStep:
		mac->debug = LispDebugStep;
		goto debugger_command_done;
	    case DebuggerStepi:
		mac->debug = LispDebugStepi;
		goto debugger_command_done;
	}
	continue;

debugger_new_frame:
	/* goto here with i as the new frame value, after error checking */
	if (i != frame) {
	    frame = i;
	    for (frm = DBG, i = mac->debug_level; i > frame; frm = CDR(frm), i--)
		;
	    curframe = CAR(frm);

	    if (FRM == old_frm) {
		/* if first time selecting a new frame */
		GCProtect();
		FRM = CONS(ENV, old_frm);
		GCUProtect();
	    }
	    ENV = CAR(CDR(CDR(curframe)));
	    LEX = CDR(CDR(CDR(curframe)));
	}
debugger_print_frame:
	fprintf(lisp_stdout, "#%d> (", frame);
	LispPrintObj(mac, NIL, CAR(curframe), 1);
	fputc(' ', lisp_stdout);
	LispPrintObj(mac, NIL, CAR(CDR(curframe)), 0);
	fprintf(lisp_stdout, ")\n");


    }

debugger_command_done:
    FRM = old_frm;
    ENV = old_env;
    LEX = old_lex;
}
