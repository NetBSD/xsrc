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

SCCS:          @(#)journal.c    1.9 03/09/92
NAME:          journal.c
PRODUCT:       TET (Test Environment Toolkit)
AUTHOR:        OSF Validation & SQA
DATE CREATED:  14 May 1991
CONTENTS:

MODIFICATIONS: Changed Phoenix References to TET / 14May91 / jbg

               "TET Rework"
               David G. Sawyer, UniSoft Ltd,  July 1991.

               tet_error(): replaced malloc with use of fixed array
               David G. Sawyer, UniSoft Ltd,  31 Oct 1991.

               jnl_entry(): replaced BAIL_OUT with fprintf(stderr," ...")
               David G. Sawyer, UniSoft Ltd,  26 Nov 1991.

	       Added line length check to jnl_entry() and tet_error()
	       David G. Sawyer, Unisoft Ltd, 31 Jan 1992.

	       Changed tet_error() to give better formatted messages.
	       David G. Sawyer, Unisoft Ltd, 12 Feb 1992.

************************************************************************/

#include <tcc_env.h>
#include <tcc_mac.h>
#include <tet_jrnl.h>
#include <tcc_prot.h>
#include <pwd.h>
#if __STDC__
#include <stdarg.h>
#endif
#include <dirent.h>


#define  MAXRETRY  10    	/* Number of retries in race for filename */

char lineout[BIG_BUFFER];       /* The line to be output to the journal */
char zeetime[18];         	/* Big enough to contain "HH:MM:YY YYYYMMDD" */
time_t time_val;          	/* Variable to hold the time stucture */


/*
 *  Write a line of text to the journal.
 *  NOTE: No BAIL_OUTS are permitted in this function !
 */
void jnl_entry(linep)
char *linep;
{
	int	too_long = 0;

#ifndef DBG  
    /*
     *  if jnl_fd gets initialized to 0,  and the jnl file isn't
     *  open, the write occurs to File #0 (== STDOUT).
     */
    if (jnl_open_flag == TRUE)
#endif
    {
	if((int) strlen(linep) >= JNL_LINE_MAX)
	{
	    linep[JNL_LINE_MAX-1] = '\0';
	    linep[JNL_LINE_MAX-2] = '\n';
	    too_long = 1;
	}

        if( write(jnl_fd,(void *)linep,strlen(linep)) == -1)
            (void) fprintf(stderr,"write() to journal failed: %s", linep);

	if(too_long)
	{
	    (void) sprintf(error_mesg, "Warning journal line truncated because it exceeded %d bytes\n", JNL_LINE_MAX);

	    /* This ends up being a recursive call to this function ! */
	    ERROR(error_mesg);
	}
    }
}


/*
 *  Write the tool start entry to the journal file.
 */
void jnl_tool_start(ctr,tcname,text, what_mode)
int ctr;
char *tcname;
char *text;
int what_mode;
{
    (void) get_time();
    (void) sprintf(lineout, "%d|%d %s %s|%s, %s %d-%d\n", 
	((what_mode == BUILD) ? TET_JNL_BUILD_START : TET_JNL_CLEAN_START),
	ctr, tcname,zeetime,text,"scenario ref",(scenario_line == 0 ? old_line : scenario_line),(scenario_element == 0 ? old_element : scenario_element));
    (void) jnl_entry(lineout);
}


/*
 *  Write the tool finish entry to the journal file.
 */
void jnl_tool_end(ctr,status,text,what_mode)
int ctr;
int status;
char *text;
int what_mode;
{
    (void) get_time();
    (void) sprintf(lineout,((text == NULL) ? "%d|%d %d %s|\n" :
            "%d|%d %d %s|%s\n"),((what_mode == BUILD) ? TET_JNL_BUILD_END :
            TET_JNL_CLEAN_END),ctr, status,zeetime,text);
    
    (void) jnl_entry(lineout);
}


/*
 *  Write a configuration variable and value to the journal file.
 */
void jnl_config_value(text)
char *text;
{
    (void) sprintf(lineout,((text == NULL) ? "%d||\n" : "%d||%s\n"),
        TET_JNL_CFG_VALUE,text);

    (void) jnl_entry(lineout);
}


/*
 *  Write a message contained in the scenario file to the journal
 */
void jnl_entry_scen(text)
char *text;
{
    (void) sprintf(lineout,"%d||%s\n",TET_JNL_SCEN_OUT,text);
    (void) jnl_entry(lineout);
}


/*
 *  Write the wonderous TCC startup message to the journal, text
 *  represents the tcc command line.
 */
void jnl_entry_tcc_start(text)
char *text;
{
    int userid;
    char *username;
    struct tm *tp;

    (void) time(&time_val);
    /*
     * For START message, include date in YYYYMMDD format
     */
    tp = localtime(&time_val);
    (void)sprintf(zeetime, "%02d:%02d:%02d %4d%02d%02d",
	tp->tm_hour, tp->tm_min, tp->tm_sec, tp->tm_year+1900,
	tp->tm_mon+1, tp->tm_mday);
    userid = (int) getuid();
    username = getlogin();
    if (username == (char *) NULL)
    {
        struct passwd *pw;

        pw = getpwuid((uid_t) userid);
        if (pw == (struct passwd *) NULL || pw->pw_name == (char *) NULL)
            username = "unknown";
        else
            username = pw->pw_name;
    }

    (void) sprintf(lineout,((text == NULL) ?
            "%d|%s %s|User: %s (%d)\n" :
            "%d|%s %s|User: %s (%d) %s\n"), TET_JNL_TCC_START, TET_VERSION,
            zeetime,username,userid,text);
    (void) jnl_entry(lineout);
}


/*
 *  Write the TCC end startup message to the journal
 */
void jnl_entry_tcc_end(text)
char *text;
{
    (void) get_time();
    (void) sprintf(lineout,((text == NULL) ? "%d|%s|\n":"%d|%s|%s\n"),
        TET_JNL_TCC_END,zeetime,text);
    (void) jnl_entry(lineout);
}


/*
 *  Write the Invocable Test Case line to the journal
 */
void jnl_entry_invoke_tc(ctr,name,ictext)
int ctr;
char *name;
char *ictext;
{
    (void) get_time();
    (void) sprintf(lineout,((*ictext == '\0') ? "%d|%d %s %s|%s %d-%d\n" :
            "%d|%d %s %s|%s %d-%d%s\n"), TET_JNL_INVOKE_TC,ctr,
            name,zeetime,"TC Start, scenario ref", (scenario_line == 0 ? old_line : scenario_line),(scenario_element == 0 ? old_element : scenario_element), ictext);
    (void) jnl_entry(lineout);
}


/*
 *  Output from a tool or TC during output_capture is written to the journal
 */
void jnl_entry_captured(ctr, text)
int ctr;
char *text;
{
    /* this "special case" is due to the use of newline as a record 
       separator in the journal file (and the aesthetic problem people 
       have with seeing two newlines in the journal after captured output). 
       The use of a byte count, would permit transparent data in the 
       journal. */

    if ((text != NULL) && (text[strlen(text)-1] == '\n'))
        (void) sprintf(lineout,"%d|%d|%s",TET_JNL_CAPTURED_OUTPUT,ctr,text);
    else
        (void) sprintf(lineout,((text == NULL) ? "%d|%d|\n" : "%d|%d|%s\n"),
            TET_JNL_CAPTURED_OUTPUT,ctr,text);
    (void) jnl_entry(lineout);
}


/*
 *  Write the Test Case finish line to the journal
 */
void jnl_tc_end(ctr,status,text)
int ctr;
int status;
char *text;
{
    (void) get_time();
    (void) sprintf(lineout,((text == NULL) ? "%d|%d %d %s|\n" :
            "%d|%d %d %s|%s\n"), TET_JNL_TC_END,ctr,status,
            zeetime,text);
    (void) jnl_entry(lineout);
}


/*
 *  Write the Test Purpose result line to the journal
 */
void jnl_tp_result(ctr,tpnum,status,text)
int ctr;
int tpnum;
int status;
char *text;
{
    (void) get_time();
    (void) sprintf(lineout,((text == NULL) ? "%d|%d %d %d %s|\n" :
            "%d|%d %d %d %s|%s\n"), TET_JNL_TP_RESULT,ctr,
            tpnum,status,zeetime,text);

    (void) jnl_entry(lineout);
}


/*
 *  Write the configuration start message, including the config being used,
 *  to the journal.
 */
void jnl_config_start(path,mode)
char *path;
int mode;
{
    (void) sprintf(lineout,"%d|%s %d|Config Start\n",TET_JNL_CFG_START, path,mode);
    (void) jnl_entry(lineout);
}


/*
 *  Write the configuration end message to the journal
 */
void jnl_config_end()
{
    (void) sprintf(lineout,"%d||Config End\n",TET_JNL_CFG_END);
    (void) jnl_entry(lineout);
}


/*
 *  If the TCC encounters an error it writes a message to the journal.
 *  NOTE: No BAIL_OUTS are permitted in this function !
 */
void jnl_tc_message(text)
char *text;
{
    (void) sprintf(lineout,((text == NULL) ? "%d||" : "%d||%s"),
        TET_JNL_TC_MESSAGE,text);
    (void) jnl_entry(lineout);
}


/*
 *  The user has generated a SIGINT and aborted the TC - write a message
 *  to the journal file.
 */
void jnl_user_abort(text)
char *text;
{
    (void) get_time();
    (void) sprintf(lineout,((text == NULL) ? "%d|%s|\n" : "%d|%s|%s\n"),
        TET_USER_ABORT, zeetime, text);
    (void) jnl_entry(lineout);
}


/*
 * Get the time & format it into zeetime for inclusion in a journal entry.
 */
void get_time()
{
    struct tm *tp;

    (void) time(&time_val);
    tp = localtime(&time_val);
    (void)sprintf(zeetime, "%02d:%02d:%02d",
        tp->tm_hour, tp->tm_min, tp->tm_sec);
}


/*
 *  Display an error message both on stderr and in the journal file.
 *  NOTE: No BAIL_OUTS are permitted in this function !
 */
void tet_error(fmt, err_file, line_no)
char *fmt, *err_file;
int line_no;
{
    char jnl_fmt[BIG_BUFFER];

    (void) fprintf(stderr,"\nTcc error, message from source file %s line %d :\n%s",err_file, line_no, fmt);

    (void) sprintf(jnl_fmt,"Message from source file %s line %d: %s", err_file, line_no, fmt);

    jnl_fmt[JNL_LINE_MAX-1] = '\0';

    (void) jnl_tc_message(jnl_fmt);
}


/*
 * Determines & creates the directory for the journal file name
 * (TET_ROOT/results/n, where 'n' is one greater than the highest
 * existing numbered directory).  To cope with more than one tcc competing
 * with each other, we try again up to MAXRETRY times if the directory we
 * try to create already exists). 
 * The full dir name includes the modes of invocation, this dir is created
 * after we have our unique sequential number, the number only dir is then
 * removed.
 * Returns a pointer to the determined directory, NULL in case of failure.
 */
char *get_jnl_dir()
{
    DIR *dir_pointer;
    struct dirent *dp;
    char tmp_name[_POSIX_PATH_MAX], full_name[_POSIX_PATH_MAX];
    int num, max, rc;

#ifdef DBG
    (void) fprintf(stderr,"get_jnl_dir()\n");
#endif

    if ((dir_pointer = opendir(results_dir)) == NULL)
    {
        BAIL_OUT("cannot open results directory\n");
        /* NOTREACHED */
    }

    /*
     * Find highest-numbered directory in existence
     * (atoi() will return 0 for dirs like "." & "..")
     */
    max = 0;
    while ((dp = readdir(dir_pointer)) != (struct dirent *) NULL)
    {
        /* atoi() ignores the letters after the number portion of the name */
        num = atoi(dp->d_name);
        if (num > max)
            max = num;
    }
    (void) closedir(dir_pointer);
    /*
     * Create new directory numbered one greater than current max;
     * if beaten to it, retry MAXRETRY times
     */
    for (num = 0; num < MAXRETRY; num++)
    {
        (void) sprintf(tmp_name, "%s/%4.4d", results_dir, ++max);
        errno = 0;
        rc = mkdir(tmp_name, (S_IRWXU|S_IRWXG|S_IRWXO));
        if (rc == FAILURE && errno == EEXIST)
            continue;    /* Try again if file already exists */
        else
            break;    /* Succeeded, or failed for another reason */
    }

    if (rc == FAILURE)
    {
        return((char *) NULL);
    }
    else
    {
        /* 
         * We now have our exclusively numbered dir, however we want to
         * include the modes of invocation of the program in the dir
         * name so we create the dir we intend to use, ie 0001bec and then
         * remove the uniquely numbered dir, ie 0001
         */
         
        (void) strcpy(full_name, tmp_name);
        if(build_mode == TRUE)
            (void) strcat(full_name,"b");
        if(exec_mode == TRUE)
            (void) strcat(full_name,"e");
        if(clean_mode == TRUE)
            (void) strcat(full_name,"c");
        rc = mkdir(full_name, (S_IRWXU|S_IRWXG|S_IRWXO));
        (void) rmdir(tmp_name);
        if (rc == FAILURE)
            return((char *) NULL);
    }
    return(full_name);
}
