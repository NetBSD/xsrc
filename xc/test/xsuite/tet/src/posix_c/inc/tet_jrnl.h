/*
 * Copyright 1990 Open Software Foundation (OSF)
 * Copyright 1990 Unix International (UI)
 * Copyright 1990 X/Open Company Limited (X/Open)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OSF, UI or X/Open not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  OSF, UI and X/Open make 
 * no representations about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * OSF, UI and X/Open DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO 
 * EVENT SHALL OSF, UI or X/Open BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/************************************************************************

SCCS:          @(#)tet_jrnl.h    1.9 03/09/92
NAME:          tet_jrnl.h
PRODUCT:       TET (Test Environment Toolkit)
AUTHOR:        OSF Validation & SQA
DATE CREATED:  14 May 1991
CONTENTS:

MODIFICATIONS:

               "TET Rework"
               David G. Sawyer, UniSoft Ltd,  July 1991.

************************************************************************/


/* TCC startup */
#define TET_JNL_TCC_START  0

/* invocation of test case */
#define TET_JNL_INVOKE_TC  10

/* start-up from TCM */
#define TET_JNL_TCM_START  15

/* new entry in configuration database */
#define TET_JNL_CFG_START    20

#define TET_JNL_CFG_VALUE    30

#define TET_JNL_CFG_END      40

#define TET_JNL_TC_MESSAGE   50

/* output from Scenario output command */
#define TET_JNL_SCEN_OUT  70

/* Test Case (exit) Status */
#define TET_JNL_TC_END  80

/* TCC aborted by user interrupt */
#define TET_USER_ABORT  90

/* Captured output from a test case */
#define TET_JNL_CAPTURED_OUTPUT 100

#define TET_JNL_BUILD_START  110
#define TET_JNL_BUILD_END  130

#define TET_JNL_TP_START   200
#define TET_JNL_TP_RESULT  220

#define TET_JNL_CLEAN_START  300
#define TET_JNL_CLEAN_OUTPUT 310
#define TET_JNL_CLEAN_END 320

#define TET_JNL_IC_START 400
#define TET_JNL_IC_END   410

#define TET_JNL_TCM_INFO 510
#define TET_JNL_TC_INFO  520

#define TET_JNL_TCC_END   900

/* END statuses */
#define TET_ESTAT_EXEC_FAILED  -1
#define TET_ESTAT_TIMEOUT      -2
#define TET_ESTAT_LOCK         -3
