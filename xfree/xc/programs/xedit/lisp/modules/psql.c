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

/* $XFree86: xc/programs/xedit/lisp/modules/psql.c,v 1.3 2001/10/15 15:36:51 paulo Exp $ */

#include <stdlib.h>
#include <libpq-fe.h>
#undef USE_SSL		/* cannot get it to compile... */
#include <postgres.h>
#include <utils/geo_decls.h>
#include "internal.h"

/*
 * Prototypes
 */
int psqlLoadModule(LispMac *mac);

LispObj *Lisp_PQbackendPID(LispMac*, LispObj*, char*);
LispObj *Lisp_PQclear(LispMac*, LispObj*, char*);
LispObj *Lisp_PQconsumeInput(LispMac*, LispObj*, char*);
LispObj *Lisp_PQdb(LispMac*, LispObj*, char*);
LispObj *Lisp_PQerrorMessage(LispMac*, LispObj*, char*);
LispObj *Lisp_PQexec(LispMac*, LispObj*, char*);
LispObj *Lisp_PQfinish(LispMac*, LispObj*, char*);
LispObj *Lisp_PQfname(LispMac*, LispObj*, char*);
LispObj *Lisp_PQfnumber(LispMac*, LispObj*, char*);
LispObj *Lisp_PQfsize(LispMac*, LispObj*, char*);
LispObj *Lisp_PQftype(LispMac*, LispObj*, char*);
LispObj *Lisp_PQgetlength(LispMac*, LispObj*, char*);
LispObj *Lisp_PQgetvalue(LispMac*, LispObj*, char*);
LispObj *Lisp_PQhost(LispMac*, LispObj*, char*);
LispObj *Lisp_PQnfields(LispMac*, LispObj*, char*);
LispObj *Lisp_PQnotifies(LispMac*, LispObj*, char*);
LispObj *Lisp_PQntuples(LispMac*, LispObj*, char*);
LispObj *Lisp_PQoptions(LispMac*, LispObj*, char*);
LispObj *Lisp_PQpass(LispMac*, LispObj*, char*);
LispObj *Lisp_PQport(LispMac*, LispObj*, char*);
LispObj *Lisp_PQresultStatus(LispMac*, LispObj*, char*);
LispObj *Lisp_PQsetdb(LispMac*, LispObj*, char*);
LispObj *Lisp_PQsocket(LispMac*, LispObj*, char*);
LispObj *Lisp_PQstatus(LispMac*, LispObj*, char*);
LispObj *Lisp_PQtty(LispMac*, LispObj*, char*);
LispObj *Lisp_PQuser(LispMac*, LispObj*, char*);

/*
 * Initialization
 */
static LispBuiltin lispbuiltins[] = {
    {"PQ-BACKEND-PID",		Lisp_PQbackendPID,	1, 1, 1,},
    {"PQ-CLEAR",		Lisp_PQclear,		1, 1, 1,},
    {"PQ-CONSUME-INPUT",	Lisp_PQconsumeInput,	1, 1, 1,},
    {"PQ-DB",			Lisp_PQdb,		1, 1, 1,},
    {"PQ-ERROR-MESSAGE",	Lisp_PQerrorMessage,	1, 1, 1,},
    {"PQ-EXEC",			Lisp_PQexec,		1, 2, 2,},
    {"PQ-FINISH",		Lisp_PQfinish,		1, 1, 1,},
    {"PQ-FNAME",		Lisp_PQfname,		1, 2, 2,},
    {"PQ-FNUMBER",		Lisp_PQfnumber,		1, 2, 2,},
    {"PQ-FSIZE",		Lisp_PQfsize,		1, 2, 2,},
    {"PQ-FTYPE",		Lisp_PQftype,		1, 2, 2,},
    {"PQ-GETLENGTH",		Lisp_PQgetlength,	1, 3, 3,},
    {"PQ-GETVALUE",		Lisp_PQgetvalue,	1, 3, 4,},
    {"PQ-HOST",			Lisp_PQhost,		1, 1, 1,},
    {"PQ-NFIELDS",		Lisp_PQnfields,		1, 1, 1,},
    {"PQ-NOTIFIES",		Lisp_PQnotifies,	1, 1, 1,},
    {"PQ-NTUPLES",		Lisp_PQntuples,		1, 1, 1,},
    {"PQ-OPTIONS",		Lisp_PQoptions,		1, 1, 1,},
    {"PQ-PASS",			Lisp_PQpass,		1, 1, 1,},
    {"PQ-PORT",			Lisp_PQport,		1, 1, 1,},
    {"PQ-RESULT-STATUS",	Lisp_PQresultStatus,	1, 1, 1,},
    {"PQ-SETDB",		Lisp_PQsetdb,		1, 5, 5,},
    {"PQ-SETDB-LOGIN",		Lisp_PQsetdb,		1, 7, 7,},
    {"PQ-SOCKET",		Lisp_PQsocket,		1, 1, 1,},
    {"PQ-STATUS",		Lisp_PQstatus,		1, 1, 1,},
    {"PQ-TTY",			Lisp_PQtty,		1, 1, 1,},
    {"PQ-USER",			Lisp_PQuser,		1, 1, 1,},
};

LispModuleData psqlLispModuleData = {
    LISP_MODULE_VERSION,
    psqlLoadModule
};

static int PGconn_t, PGresult_t;

/*
 * Implementation
 */
int
psqlLoadModule(LispMac *mac)
{
    int i;
    char *fname = "INTERNAL:PSQL-LOAD-MODULE";

    PGconn_t = LispRegisterOpaqueType(mac, "PGconn*");
    PGresult_t = LispRegisterOpaqueType(mac, "PGresult*");
    GCProtect();

    /* NOTE: Implemented just enough to make programming examples
     * (and my needs) work.
     * Completing this is an exercise to the reader, or may be implemented
     * when/if required.
     */
    LispExecute(mac,
		"(DEFSTRUCT PG-NOTIFY RELNAME BE-PID)\n"
		"(DEFSTRUCT PG-POINT X Y)\n"
		"(DEFSTRUCT PG-BOX HIGH LOW)\n"
		"(DEFSTRUCT PG-POLYGON SIZE NUM-POINTS BOUNDBOX POINTS)\n");

    /* enum ConnStatusType */
    (void)LispSetVariable(mac, ATOM2("PG-CONNECTION-OK"),
			  REAL(CONNECTION_OK), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PG-CONNECTION-BAD"),
			  REAL(CONNECTION_BAD), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PG-CONNECTION-STARTED"),
			  REAL(CONNECTION_STARTED), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PG-CONNECTION-MADE"),
			  REAL(CONNECTION_MADE), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PG-CONNECTION-AWAITING-RESPONSE"),
			  REAL(CONNECTION_AWAITING_RESPONSE), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PG-CONNECTION-AUTH-OK"),
			  REAL(CONNECTION_AUTH_OK), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PG-CONNECTION-SETENV"),
			  REAL(CONNECTION_SETENV), fname, 0);


    /* enum ExecStatusType */
    (void)LispSetVariable(mac, ATOM2("PGRES-EMPTY-QUERY"),
			  REAL(PGRES_EMPTY_QUERY), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PGRES-COMMAND-OK"),
			  REAL(PGRES_COMMAND_OK), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PGRES-TUPLES-OK"),
			  REAL(PGRES_TUPLES_OK), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PGRES-COPY-OUT"),
			  REAL(PGRES_COPY_OUT), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PGRES-COPY-IN"),
			  REAL(PGRES_COPY_IN), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PGRES-BAD-RESPONSE"),
			  REAL(PGRES_BAD_RESPONSE), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PGRES-NONFATAL-ERROR"),
			  REAL(PGRES_NONFATAL_ERROR), fname, 0);
    (void)LispSetVariable(mac, ATOM2("PGRES-FATAL-ERROR"),
			  REAL(PGRES_FATAL_ERROR), fname, 0);
    GCUProtect();

    for (i = 0; i < sizeof(lispbuiltins) / sizeof(lispbuiltins[0]); i++)
	LispAddBuiltinFunction(mac, &lispbuiltins[i]);

    return (1);
}

LispObj *
Lisp_PQbackendPID(LispMac *mac, LispObj *list, char *fname)
{
    int pid;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    pid = PQbackendPID(conn);

    return (REAL(pid));
}

LispObj *
Lisp_PQclear(LispMac *mac, LispObj *list, char *fname)
{
    PGresult *res;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    res = (PGresult*)(CAR(list)->data.opaque.data);
    PQclear(res);

    return (NIL);
}

LispObj *
Lisp_PQconsumeInput(LispMac *mac, LispObj *list, char *fname)
{
    int result;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    result = PQconsumeInput(conn);

    return (REAL(result));
}

LispObj *
Lisp_PQdb(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    str = PQdb(conn);

    return (str ? STRING(str) : NIL);
}

LispObj *
Lisp_PQerrorMessage(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    str = PQerrorMessage(conn);

    return (str ? STRING(str) : NIL);
}

LispObj *
Lisp_PQexec(LispMac *mac, LispObj *list, char *fname)
{
    PGconn *conn;
    PGresult *res;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (CAR(list)->type != LispString_t)
	LispDestroy(mac, "expecting string, at %s", fname);

    res = PQexec(conn, STRPTR(CAR(list)));

    return (res ? OPAQUE(res, PGresult_t) : NIL);
}

LispObj *
Lisp_PQfinish(LispMac *mac, LispObj *list, char *fname)
{
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    PQfinish(conn);

    return (NIL);
}

LispObj *
Lisp_PQfname(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    int field;
    PGresult *res;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    res = (PGresult*)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (CAR(list)->type != LispReal_t || CAR(list)->data.real < 0 ||
	(int)CAR(list)->data.real != CAR(list)->data.real)
	LispDestroy(mac, "expecting positive integer, at %s", fname);
    field = (int)CAR(list)->data.real;

    str = PQfname(res, field);

    return (str ? STRING(str) : NIL);
}

LispObj *
Lisp_PQfnumber(LispMac *mac, LispObj *list, char *fname)
{
    int num;
    PGresult *res;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    res = (PGresult*)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (CAR(list)->type != LispString_t)
	LispDestroy(mac, "expecting string, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    num = PQfnumber(res, STRPTR(CAR(list)));

    return (REAL(num));
}

LispObj *
Lisp_PQfsize(LispMac *mac, LispObj *list, char *fname)
{
    int size, field;
    PGresult *res;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    res = (PGresult*)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (CAR(list)->type != LispReal_t || CAR(list)->data.real < 0 ||
	(int)CAR(list)->data.real != CAR(list)->data.real)
	LispDestroy(mac, "expecting positive integer, at %s", fname);
    field = (int)CAR(list)->data.real;

    size = PQfsize(res, field);

    return (REAL(size));
}

LispObj *
Lisp_PQftype(LispMac *mac, LispObj *list, char *fname)
{
    Oid oid;
    int field;
    PGresult *res;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    res = (PGresult*)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (CAR(list)->type != LispReal_t || CAR(list)->data.real < 0 ||
	(int)CAR(list)->data.real != CAR(list)->data.real)
	LispDestroy(mac, "expecting positive integer, at %s", fname);
    field = (int)CAR(list)->data.real;

    oid = PQftype(res, field);

    return (REAL(oid));
}

LispObj *
Lisp_PQgetlength(LispMac *mac, LispObj *list, char *fname)
{
    PGresult *res;
    int tuple, field, len;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    res = (PGresult*)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (CAR(list)->type != LispReal_t || CAR(list)->data.real < 0 ||
	(int)CAR(list)->data.real != CAR(list)->data.real)
	LispDestroy(mac, "expecting positive integer, at %s", fname);
    tuple = (int)CAR(list)->data.real;
    list = CDR(list);
    if (CAR(list)->type != LispReal_t || CAR(list)->data.real < 0 ||
	(int)CAR(list)->data.real != CAR(list)->data.real)
	LispDestroy(mac, "expecting positive integer, at %s", fname);
    field = (int)CAR(list)->data.real;

    len = PQgetlength(res, tuple, field);

    return (REAL(len));
}

/* (pq-getvalue connection tuple field &optional type-specifier) */
LispObj *
Lisp_PQgetvalue(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    double real = 0.0;
    PGresult *res;
    int tuple, field, isreal = 0;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    res = (PGresult*)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (CAR(list)->type != LispReal_t || CAR(list)->data.real < 0 ||
	(int)CAR(list)->data.real != CAR(list)->data.real)
	LispDestroy(mac, "expecting positive integer, at %s", fname);
    tuple = (int)CAR(list)->data.real;
    list = CDR(list);
    if (CAR(list)->type != LispReal_t || CAR(list)->data.real < 0 ||
	(int)CAR(list)->data.real != CAR(list)->data.real)
	LispDestroy(mac, "expecting positive integer, at %s", fname);
    field = (int)CAR(list)->data.real;

    str = PQgetvalue(res, tuple, field);

    list = CDR(list);
    if (list != NIL) {
	if (CAR(list)->type != LispAtom_t)
	    LispDestroy(mac, "expecting atom, at %s");
	if (strcmp(STRPTR(CAR(list)), "INT16") == 0) {
	    real = *(short*)str;
	    isreal = 1;
	    goto simple_type;
	}
	else if (strcmp(STRPTR(CAR(list)), "INT32") == 0) {
	    real = *(int*)str;
	    isreal = 1;
	    goto simple_type;
	}
	else if (strcmp(STRPTR(CAR(list)), "FLOAT") == 0) {
	    real = *(float*)str;
	    isreal = 1;
	    goto simple_type;
	}
	else if (strcmp(STRPTR(CAR(list)), "REAL") == 0) {
	    real = *(double*)str;
	    isreal = 1;
	    goto simple_type;
	}
	else if (strcmp(STRPTR(CAR(list)), "PG-POLYGON") == 0)
	    goto polygon_type;
	else if (strcmp(STRPTR(CAR(list)), "STRING") != 0)
	    LispDestroy(mac, "unknown type specifier %s, at %s",
			STRPTR(CAR(list)), fname);
    }

simple_type:
    return (isreal ? REAL(real) : (str ? STRING(str) : NIL));

polygon_type:
  {
    LispObj *poly, *box, *p = NIL, *cdr, *obj;
    POLYGON *polygon;
    int i, size;

    size = PQgetlength(res, tuple, field);
    polygon = (POLYGON*)(str - sizeof(int));

    GCProtect();
    /* get polygon->boundbox */
    cdr = EVAL(CONS(ATOM("MAKE-PG-POINT"),
		    CONS(ATOM(":X"),
			 CONS(REAL(polygon->boundbox.high.x),
			      CONS(ATOM(":Y"),
				   CONS(REAL(polygon->boundbox.high.y), NIL))))));
    obj = EVAL(CONS(ATOM("MAKE-PG-POINT"),
		    CONS(ATOM(":X"),
			 CONS(REAL(polygon->boundbox.low.x),
			      CONS(ATOM(":Y"),
				   CONS(REAL(polygon->boundbox.low.y), NIL))))));
    box = EVAL(CONS(ATOM("MAKE-PG-BOX"),
		    CONS(ATOM(":HIGH"),
			 CONS(cdr,
			      CONS(ATOM(":LOW"),
				   CONS(obj, NIL))))));
    /* get polygon->p values */
    for (i = 0; i < polygon->npts; i++) {
	obj = EVAL(CONS(ATOM("MAKE-PG-POINT"),
			CONS(ATOM(":X"),
			     CONS(REAL(polygon->p[i].x),
			      CONS(ATOM(":Y"),
				   CONS(REAL(polygon->p[i].y), NIL))))));
	if (i == 0)
	    p = cdr = CONS(obj, NIL);
	else {
	    CDR(cdr) = CONS(obj, NIL);
	    cdr = CDR(cdr);
	}
    }

    /* make result */
    poly = EVAL(CONS(ATOM("MAKE-PG-POLYGON"),
		     CONS(ATOM(":SIZE"),
			  CONS(REAL(size),
			       CONS(ATOM(":NUM-POINTS"),
				    CONS(REAL(polygon->npts),
					 CONS(ATOM(":BOUNDBOX"),
					      CONS(box,
						   CONS(ATOM(":POINTS"),
							CONS(QUOTE(p), NIL))))))))));
    GCUProtect();

    return (poly);
  }
}

LispObj *
Lisp_PQhost(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    str = PQhost(conn);

    return (str ? STRING(str) : NIL);
}

LispObj *
Lisp_PQnfields(LispMac *mac, LispObj *list, char *fname)
{
    int nfields;
    PGresult *res;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    res = (PGresult*)(CAR(list)->data.opaque.data);
    nfields = PQnfields(res);

    return (REAL(nfields));
}

LispObj *
Lisp_PQnotifies(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res, *code, *frm = FRM;
    PGconn *conn;
    PGnotify *notifies;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    if ((notifies = PQnotifies(conn)) == NULL)
	return (NIL);

    GCProtect();
    code = CONS(ATOM("MAKE-PG-NOTIFY"),
		  CONS(ATOM(":RELNAME"),
		       CONS(STRING(notifies->relname),
			    CONS(ATOM(":BE-PID"),
				 CONS(REAL(notifies->be_pid), NIL)))));
    FRM = CONS(code, FRM);
    GCUProtect();
    res = EVAL(code);
    FRM = frm;

    free(notifies);

    return (res);
}

LispObj *
Lisp_PQntuples(LispMac *mac, LispObj *list, char *fname)
{
    int ntuples;
    PGresult *res;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    res = (PGresult*)(CAR(list)->data.opaque.data);
    ntuples = PQntuples(res);

    return (REAL(ntuples));
}

LispObj *
Lisp_PQoptions(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    str = PQoptions(conn);

    return (str ? STRING(str) : NIL);
}

LispObj *
Lisp_PQpass(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    str = PQpass(conn);

    return (str ? STRING(str) : NIL);
}

LispObj *
Lisp_PQport(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    str = PQport(conn);

    return (str ? STRING(str) : NIL);
}

LispObj *
Lisp_PQsetdb(LispMac *mac, LispObj *list, char *fname)
{
    PGconn *conn;
    LispObj *obj;
    char *host, *port, *options, *tty, *dbname, *login, *pass;

    for (obj = list; obj != NIL; obj = CDR(obj))
	if (CAR(obj) != NIL && CAR(obj)->type != LispString_t)
	    LispDestroy(mac, "expecting string, at %s", fname);

    host = CAR(list) == NIL ? NULL : STRPTR(CAR(list));
    list = CDR(list);
    port = CAR(list) == NIL ? NULL : STRPTR(CAR(list));
    list = CDR(list);
    options = CAR(list) == NIL ? NULL : STRPTR(CAR(list));
    list = CDR(list);
    tty = CAR(list) == NIL ? NULL : STRPTR(CAR(list));
    list = CDR(list);
    dbname = CAR(list) == NIL ? NULL : STRPTR(CAR(list));
    list = CDR(list);
    if (list != NIL) {
	login = CAR(list) == NIL ? NULL : STRPTR(CAR(list));
	list = CDR(list);
	pass = CAR(list) == NIL ? NULL : STRPTR(CAR(list));
    }
    else
	login = pass = NULL;

    conn = PQsetdbLogin(host, port, options, tty, dbname, login, pass);

    return (conn ? OPAQUE(conn, PGconn_t) : NIL);
}

LispObj *
Lisp_PQresultStatus(LispMac *mac, LispObj *list, char *fname)
{
    int status;
    PGresult *res;

    if (!CHECKO(CAR(list), PGresult_t))
	LispDestroy(mac, "cannot convert %s to PGresult*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    res = (PGresult*)(CAR(list)->data.opaque.data);
    status = PQresultStatus(res);

    return (REAL(status));
}

LispObj *
Lisp_PQsocket(LispMac *mac, LispObj *list, char *fname)
{
    int sock;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    sock = PQsocket(conn);

    return (REAL(sock));
}

LispObj *
Lisp_PQstatus(LispMac *mac, LispObj *list, char *fname)
{
    int status;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    status = PQstatus(conn);

    return (REAL(status));
}

LispObj *
Lisp_PQtty(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    str = PQtty(conn);

    return (str ? STRING(str) : NIL);
}

LispObj *
Lisp_PQuser(LispMac *mac, LispObj *list, char *fname)
{
    char *str;
    PGconn *conn;

    if (!CHECKO(CAR(list), PGconn_t))
	LispDestroy(mac, "cannot convert %s to PGconn*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    conn = (PGconn*)(CAR(list)->data.opaque.data);
    str = PQuser(conn);

    return (str ? STRING(str) : NIL);
}
