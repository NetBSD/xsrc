/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: TestMng.c,v 1.11 94/04/17 21:01:32 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

#include "XstlibInt.h"
#include "setjmp.h"
#include "stdlib.h"
#include "tet_api.h"
#include "xtest.h"

extern	char *TestName;

/*
 * Variables set from TET execution configuration parameters.
 */
char   *Xst_server_node = SERVER_DEF;/* the X server */
int     Xst_required_byte_sex = SEX_BOTH;	/* byte sex wanted */
int  Xst_timeout_value = 10;	/* seconds that Expect will wait */
int  Xst_visual_check = 0;	/* seconds to delay at Visual_Check calls */
int  Xst_protocol_version = X_PROTOCOL;
int  Xst_protocol_revision = X_PROTOCOL_REVISION;
int  Xst_override = False;

/*
 * Other test-wide globals variables.
 */
int     Xst_byte_sex = SEX_NATIVE;   /* client byte sex for this connection */
int  Xst_error_count = 0;	/* number of calls to Log_Error */
int  Xst_delete_count = 0;	/* number of calls to Log_Del */
int  Xst_untested_count = 0;	/* indicates that Untested has been called */
char *Xst_def_font8 = "xtfont0";/* default 8-bit font to use */
char *Xst_def_font16 = "xtfont2";/* default 16-bit font to use */

static struct {
    char   *name;
    int     code;
}               Sexes[] = {
    { "NATIVE", SEX_NATIVE }              ,
    { "native", SEX_NATIVE }              ,
    { "REVERSE", SEX_REVERSE }              ,
    { "reverse", SEX_REVERSE }              ,
    { "MSB", SEX_MSB }              ,
    { "msb", SEX_MSB }              ,
    { "LSB", SEX_LSB }              ,
    { "lsb", SEX_LSB }              ,
    { NULL, 0 }
};

int
Required_Byte_Sex()
{
	return(Xst_required_byte_sex);
}

void
Set_Required_Byte_Sex(set_to)
int	set_to;
{
	Xst_required_byte_sex = set_to;
}

void
Set_Byte_Sex(set_to)
int	set_to;
{
	Xst_byte_sex = set_to;
}

void
Exit_OK ()
{
    /*
     * This routine is called if the test reaches a conclusion and passes 
     * all path check points.
     * Log_Close() is called to ensure there have been no other errors.
     */
    exit(Log_Close());
}

void
Exit () {
    /*
     * This routine is called when the test wishes to exit 
     * on encountering an error.
     * Log_Close() is called to report the number of errors.
     */
    if(Log_Close() != EXIT_FAILURE) {
	report("Exit() was called when the error count was zero.");
        tet_result(TET_UNRESOLVED);
    }
    exit(EXIT_FAILURE);
}

void
Abort () {
    if (Xst_error_count <= 0)
	Xst_error_count++;
    Exit();
}

void
Delete () {
    if (Xst_delete_count <= 0)
	Xst_delete_count++;
    Exit();
}

void
Untested () {
    if (Xst_untested_count <= 0)
	Xst_untested_count++;
    Exit();
}

void
Finish(client)
int client;
{
	if (Get_Test_Type(client) == SETUP)
		Delete();
	else
		Abort();
}

void
checkconfig ()
{
    int	j;

    /*
     * Initialise X Protocol test suite variables to values obtained
     * from TET execution configuration parameters.
     */
    Xst_server_node = config.display;
    Xst_timeout_value = 10 * ((config.speedfactor<=0)?1:config.speedfactor);
    Xst_visual_check = config.debug_visual_check;
    Xst_protocol_version = config.protocol_version;
    Xst_protocol_revision = config.protocol_revision;
    Xst_override = config.debug_override_redirect;

    /* 
     * The byte sex to be tested includes, by default, both cases 
     * SEX_NATIVE and SEX_REVERSE. So each test purpose makes a separate 
     * connection in each mode.
     */ 
    Xst_required_byte_sex = SEX_BOTH;

    /*
     * For debugging purposes, just one mode can be requested by setting
     * XT_DEBUG_BYTE_SEX to NATIVE, REVERSE, MSB or LSB.
     */
    if (config.debug_byte_sex != NULL)
	for (j = 0; Sexes[j].name != NULL; j++) {
	    if (strcmp (Sexes[j].name, config.debug_byte_sex) != 0)
		continue;
	    Xst_required_byte_sex = Sexes[j].code;
	}

    /*
     * To keep the test purposes simple, the SEX_MSB and SEX_LSB cases
     * are here mapped onto either SEX_NATIVE or SEX_REVERSE.
     */
    if(Xst_required_byte_sex == SEX_MSB)
	Xst_required_byte_sex = native_byte_sex() ? SEX_NATIVE : SEX_REVERSE;
    if(Xst_required_byte_sex == SEX_LSB)
	Xst_required_byte_sex = native_byte_sex() ? SEX_REVERSE : SEX_NATIVE;
}

void
testfunc(func)
void	(*func)();
{
int	pass = 0;
int	fail = 0;

	if(Required_Byte_Sex() != SEX_REVERSE) {
		Set_Byte_Sex(SEX_NATIVE);
		Log_Trace("About to test with Native byte-sex (%s)\n",
			native_byte_sex() ? "MSB" : "LSB");
		if(tet_fork(func, TET_NULLFP, 0, 0xFF) == EXIT_SUCCESS)
			CHECK;
		else
			fail++;
	} else
		CHECK;

	if(Required_Byte_Sex() != SEX_NATIVE) {
		Set_Byte_Sex(SEX_REVERSE);
		Log_Trace("About to test with Reverse byte-sex (%s)\n",
			native_byte_sex() ? "LSB" : "MSB");
		if(tet_fork(func, TET_NULLFP, 0, 0xFF) == EXIT_SUCCESS)
			CHECK;
		else
			fail++;
	} else
		CHECK;

	if (fail == 0)
		CHECKPASS(2);
	/* only expect path-check to succeed if no failures occurred */
	/* if func() didn't register the failure then the api will complain. */
}
