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

SCCS:          @(#)tcc_env.h    1.9 03/09/92
NAME:          tcc_env.h
PRODUCT:       TET (Test Environment Toolkit)
AUTHOR:        OSF Validation & SQA
DATE CREATED:  14 May 1991
CONTENTS:

MODIFICATIONS:

               "TET Rework"
               David G. Sawyer, UniSoft Ltd,  July 1991.

	       Introduced separate buffers for file input/output.
	       David G. Sawyer, UniSoft Ltd, 28 Jan 1992.

	       Added RERUN and RESUME macro definitions.
	       David G. Sawyer, UniSoft Ltd, 4 Feb 1992

************************************************************************/


/* common include files */

#include <sys/types.h>
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h> 
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <signal.h>
#include <limits.h>


typedef short int bool;


#define TET_VERSION        "1.9" /* The version number of the TET */

#define ENV_BIGGER         10    /* How much the env grows by when needed */

#define BUILD_FAIL_STATUS  1     /* The exit status if the build tool failed */

#define INPUT_LINE_MAX     2048  /* Used for various line buffers */
#define JNL_LINE_MAX       512   /* Max length of journal line */

#define BIG_BUFFER 	(INPUT_LINE_MAX+1024) /* for error buffers etc */


#define MAX_RESCODES       128   /* Maximum number of allowed result codes */
#define RESCODE_AC_LEN     20    /* Max length of result code action string */
#define MAX_RSVD_RES_CODE  31    /* Max number of TET reserved result codes */

#define DEFAULT_ACTION_NAME  "Continue"    /* for the result codes */


#define TMP_RES_FILE   "tet_xres"   /* Name of the temporary results file */
#define LOCK_FILE      "tet_lock"   /* Name of the lock file, or directory */


#define DIR_SEP_CHAR   '/'      /* Directory separator character */
#define DIR_SEP_STR    "/"      /* Directory separator string */
#define CUR_DIR_SPEC   "./"     /* CURrent DIR SPECifier */


#define NAME_SEP_CHAR  ','      /* Separation char for names in SAVE_FILES */


#define WAIT_INTERVAL  10       /* Time to wait before retrying for a lock */


/* For which mode are we looking for a configuration file */

#define MODE_BLD  0
#define MODE_EXEC 1
#define MODE_CLN  2


/* What type of tool is this */

#define BUILD     1
#define CLEAN     2


#define RERUN     1
#define RESUME    0

#define FALSE            0
#define TRUE             1

#define PIPE_RD_SIDE     0
#define PIPE_WRT_SIDE    1

#define FILENO_STDIN     0
#define FILENO_STDOUT    1
#define FILENO_STDERR    2


/* Configuration file syntax elements */

#define CFG_SEP_CHAR   '='      /* var._name SEParator_CHAR value */
#define CFG_CMNT_CHAR  '#'      /* CoMmeNT CHAR */
#define CFG_TEMP_VAL   FALSE    /* The configuration value is temporary */
#define CFG_PERM_VAL   TRUE     /* The configuration value is permanent */



/* Scenario file syntax elements */

#define ALI_CMNT_CHAR  '#'          /* CoMmeNT CHAR */
#define ALI_MSG_CHAR   '"'          /* MeSsaGe CHAR */
#define IC_START_MARK  '{'          /* I.C. START MARKer */
#define IC_END_MARK    '}'          /* I.C. END MARKer */
#define IC_SEP_MARK    ','          /* I.C. SEParation MARKer */
#define TC_START_MARK  '/'          /* Test Case Start Marker */
#define ALI_INC_STRNG  ":include:/"  /* Include specifier */


/* exit codes for the TCC */

#define EXIT_OK         0
#define EXIT_BAD_MISC   1  
#define EXIT_BAD_INIT   2  
#define EXIT_BAD_CHILD  255  
