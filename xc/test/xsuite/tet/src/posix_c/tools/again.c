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

SCCS:           @(#)again.c    1.9 03/09/92
NAME:           again.c
PRODUCT:        TET (Test Environment Toolkit)
AUTHOR:         OSF Validation & SQA
DATE CREATED:    
CONTENTS:

MODIFICATIONS:

                "TET Rework"
                David G. Sawyer, UniSoft Ltd, July 1991.

		Rewrote do_rerun(), added make_ic_list().
		David G. Sawyer, UniSoft Ltd, 31 Jan 1992.

		do_rerun() and do_resume() combined into do_again().
		David G. Sawyer, UniSoft Ltd, 12 Feb 1992.

		 *** TET 1.9 for MIT X Test suite ***
			Patched cast problem in compare_ics.
			Stuart Boutell, 12 March 1992

************************************************************************/
#include <tcc_env.h>
#include <tcc_mac.h>
#include <tet_jrnl.h>
#include <tcc_prot.h>


char  linein[JNL_LINE_MAX];   /* A line read in from an old journal file */
char  *g_tc_line;             /* Global tc line - used in resume mode */
int   g_ic_num = -1;          /* Global ic number - used in resume mode */


/*
 *  This routine combines the functionality described for both the rerun and
 *  resume options.
 */
int do_again(rerun)
int rerun;
{
    int var, ctr, ctr2, saved_ic_num, status, curr_mode = -1;
    int ic_max = 0, *ic_buf, perform_rerun = FALSE, match;
    int *ics, has_ic_list, prev_line, prev_element, new_line, new_element;
    char *cp, *saved_tc_name = NULL, *tmp_ptr, copyline[JNL_LINE_MAX];

#ifdef DBG
    (void) fprintf(stderr,"do_again(%s)\n", (rerun == RERUN ? "rerun" : "resume"));
#endif

    /* Invocable component list buffer */
    ic_buf = (int *) TET_MALLOC((size_t) (sizeof(int) * 50));
    ics = (int *) TET_MALLOC((size_t) (sizeof(int) * 50));
    ics[0] = -2;
    ic_buf[0] = -2;

    /* Look through the old journal file */
    while (fgets(linein,sizeof(linein),old_jnl_fp) != NULL)
    {
        if (linein[strlen(linein)-1] == '\n')
            linein[strlen(linein)-1] = '\0';
	
	prev_line = old_line;
	prev_element = old_element;
        if ((var = modestart(linein)) > 0) /* A mode start line ? */
        {
	    /* The call to rerun_flush corrupts linein so .. */
	    (void) strcpy(copyline, linein); 

            if (var <= curr_mode && perform_rerun == TRUE && rerun)
            {
		new_line = old_line;
		new_element = old_element;
		old_line = prev_line;
		old_element = prev_element;
                rerun_flush(build_mode, exec_mode, clean_mode, ic_max, saved_tc_name, ic_buf);
		old_line = new_line;
		old_element = new_element;

                if (abort_requested == TRUE)
		    return(SUCCESS);

		perform_rerun = FALSE;
            }

	    if (var <= curr_mode || curr_mode == -1) /* New test case */
	    {
		TET_FREE((void *) ic_buf);
                ic_buf = (int *) TET_MALLOC((size_t) (sizeof(int) * 50));
                ic_buf[0] = -2;

		TET_FREE((void *) ics);
                ics = (int *) TET_MALLOC((size_t) (sizeof(int) * 50));
                ic_max = 0;
		ics[0] = -2;
	    }

            curr_mode = var;

            /* Save the TC name */
            cp = strchr(copyline,TC_START_MARK);
            if (cp == NULL)
            {
                (void) sprintf(error_mesg, "Bad TC start line format, old journal:%s\n",copyline);
                ERROR(error_mesg);
                continue;
            }
            tmp_ptr = strtok(cp," \t");

            if (saved_tc_name != NULL)
                TET_FREE((void *)saved_tc_name);

            saved_tc_name = (char *)TET_MALLOC(strlen(tmp_ptr) + 1);
            (void) strcpy(saved_tc_name,tmp_ptr);

	    has_ic_list = 0;
	    /* strip off any specified ICs */
	    cp = strchr(saved_tc_name,IC_START_MARK);
	    if (cp != NULL)
	    {
	        has_ic_list = 1;
		make_ic_list(cp, &ics);
		*cp = '\0';
	    }
        }
        else if (icstart(linein)) /* Is this line an IC start line ? */
        {
            if (sscanf(linein,"%*d|%*d %d",&saved_ic_num) != 1)
            {
                ERROR("Bad IC start line,  format in old journal file.\n");
            }

	    /* if the TC didn't have an IC list make one as we go along */
	    if(! has_ic_list)
	    {
	        for(ctr = 0; ics[ctr] != -2; ctr++)
	        {
	            if(ctr % 50 == 48)
	            {
		        ics = (int *) TET_REALLOC((void *) ics, (size_t) (sizeof(int) * (ctr + 51)));
		        for(ctr2 = ctr; ctr2 < (ctr + 50); ics[ctr2++] = -1);
		        ics[ctr2] = -2;
	            }
	        }
	        ics[ctr] = saved_ic_num;
		ics[++ctr] = -2;
	    }
        }
        else if (icend(linein)) /* Is this line an IC start line ? */
        {
            if (sscanf(linein,"%*d|%*d %d",&var) != 1)
            {
                ERROR("Bad IC start line,  format in old journal file.\n");
            }
	    for(ctr = 0; ics[ctr] != -2; ctr++)
	    {
		if (ics[ctr] == var)
		    ics[ctr] = -1;
	    }

        }
	else if ((var = modeend(linein)) != 0) /* The end of a mode */
	{
            if (sscanf(linein,"%*d|%*d %d", &status) != 1)
            {
                ERROR("Bad TP status line format in old journal file.\n");
            }
	    if (status != 0) /* Didn't run to completion */
	    {
		if ((result_mode[0] == 1 && var == 1) || (result_mode[1] == 1 && var == 2) || (result_mode[2] == 1 && var == 3))
		{
		    if (! rerun)
		    {
		        g_tc_line = (char *) TET_MALLOC(strlen(saved_tc_name)+2);
		        (void) strcpy(g_tc_line, saved_tc_name);
		        TET_FREE((void *) saved_tc_name);

			/* look for unended ICs */
			for(ctr = 0; ics[ctr] != -2; ctr++)
			{
			    if(ics[ctr] != -1)
			    {
		                g_ic_num = ics[ctr];
				break;
			    }
			}

			if (ics != NULL)
			     TET_FREE((void *) ics);
			if (ic_buf != NULL)
			     TET_FREE((void *) ic_buf);
#ifdef DBG2
		        (void) fprintf(stderr,"Incomplete mode (%d), looking for : g_tc_line: %s  g_ic_num: %d  old_line: %d\n", var, g_tc_line, g_ic_num, old_line);
#endif
		        return(SUCCESS);
		    }

		    perform_rerun = TRUE;
		    if(var == 2) /* Exec */
		    {
			for(var = 0; ics[var] != -2; var++)
			{
			    /* Check that IC number isn't already in the list */
			    for(ctr = 0; ctr < ic_max; ctr++)
			    {
			        if (ic_buf[ctr] == ics[var])
				    break;
			    }

			    /* Not already in list */
			    if ((ctr == ic_max) && (ics[var] != -1))
			    {
			        /* Run out of space - make some more */
			        if(ic_max % 50 == 48)
				    ic_buf = (int *) TET_REALLOC((void *) ic_buf, (size_t) (sizeof(int) * (ic_max + 51)));

			        /* Add the IC to the list for this test case */
			        ic_buf[ic_max++] = ics[var];
			    }
			}
		    }
		}
	    }
	}
        else if (tpend(linein,0)) /* Is this line a test purpose end line */
        {
            if (sscanf(linein,"%*d|%*d %*d %d", &status) != 1)
            {
                ERROR("Bad TP status line format in old journal file.\n");
            }
            /* 
             *  Look for a matching result code, make sure we 
             *  aren't running the same invocation case, but
             *  for a different test purpose as this causes repitition. 
             */
	    match = 0;
            for (var = 0 ; result_list[var] != -1; var++)
	    {
                if (status == result_list[var])
			match = 1;
	    }
	    if (status != 0)
	    {
		if ((result_mode[0] == 1 && curr_mode == 1) || (result_mode[1] == 1 && curr_mode == 2) || (result_mode[2] == 1 && curr_mode == 3))
		    match = 1;
	    }
            if (match == 1)
            {
		if (! rerun)
		{
	            g_tc_line = (char *) TET_MALLOC(strlen(saved_tc_name)+2);
	            (void) strcpy(g_tc_line, saved_tc_name);
	            g_ic_num = saved_ic_num;
	            TET_FREE((void *) saved_tc_name);

      		    if (ics != NULL)
			 TET_FREE((void *) ics);
		    if (ic_buf != NULL)
			 TET_FREE((void *) ic_buf);
#ifdef DBG2
		    (void) fprintf(stderr,"Matched TP result, looking for : g_tc_line: %s  g_ic_num: %d  old_line: %d\n", g_tc_line, g_ic_num, old_line);
#endif
	            return(SUCCESS);
		}

		/* Check to see IC number isn't already in the list */
	 	for(ctr = 0; ctr < ic_max; ctr++)
	 	{
		    if (ic_buf[ctr] == saved_ic_num)
		        break;
		}

		if (ctr == ic_max) /* Not already in list */
		{
		    /* Run out of space - make some more */
                    if(ic_max % 50 == 48)
                        ic_buf = (int *) TET_REALLOC((void *) ic_buf, (size_t) (sizeof(int) * (ic_max + 51)));

		    /* Add the IC to the list for this test case */
                    ic_buf[ic_max++] = saved_ic_num;

		    perform_rerun = TRUE;
		}
            }
        }
    }

    if( perform_rerun == TRUE && rerun)
        rerun_flush(build_mode, exec_mode, clean_mode, ic_max, saved_tc_name, ic_buf);

    if (saved_tc_name != NULL)
        TET_FREE((void *)saved_tc_name);

    if (ic_buf != NULL)
        TET_FREE((void *)ic_buf);

    return(SUCCESS);
}


/*
 *  Flush the modes gathered in the do_again(RERUN) routine
 */
void rerun_flush(yes_build, yes_exec, yes_clean, ic_max, saved_tc_name, ic_buf)
int yes_build, yes_exec, yes_clean, ic_max;
char *saved_tc_name;
int *ic_buf;
{
    int ctr, rc;
    char scen_line[INPUT_LINE_MAX];

#ifdef DBG
    (void) fprintf(stderr,"rerun_flush(b:%d,e:%d,c:%d,ic_max:%d,%s,ic_buf)\n",yes_build, yes_exec, yes_clean, ic_max, saved_tc_name);
#endif

    rc = SUCCESS;

    /* flush mode buffers */
    if(yes_build == TRUE)
    {
        /* Get specific configuration settings */
        tet_env = bld_env;
        rc = tool_tc(saved_tc_name,BUILD);
    }
    if(yes_exec == TRUE && rc != FAILURE)
    {
	(void) strcpy(scen_line, saved_tc_name);

	/* sort list of ICS */
	if (ic_max > 1)
	{
	    qsort((void *) ic_buf, (size_t) ic_max, sizeof(ic_buf[0]), compare_ics);
	}
        for (ctr = 0; ctr < ic_max; ctr++)
        {
	    if (ctr == 0)
		(void) strcat(scen_line,"{");

            (void) sprintf(scen_line,"%s%d",scen_line,ic_buf[ctr]);

	    if ((ctr+1) == ic_max)
		(void) strcat(scen_line,"}");
	    else
		(void) strcat(scen_line,",");
        }

        /* Get specific configuration settings */
        tet_env = exe_env;
        rc = exec_tc(scen_line);
    }
    if(yes_clean == TRUE && rc != FAILURE && abort_requested != TRUE)
    {
        /* Get specific configuration settings */
        tet_env = cln_env;
        rc = tool_tc(saved_tc_name,CLEAN);
    }
}


/*
 * Looks for a string inside another one and returns a pointer to its
 * occurance or else a NULL.
 */
char *findstr(line_str, string)
char *line_str, *string;
{
    char *cp;

    for (cp = line_str ; (*cp != '\0') ; cp++)
    {
        cp = strchr(cp,*string); /* find the first char. */
        if (cp == NULL)
            return(NULL);
        else
            if (strncmp(cp,string,strlen(string)) == 0)
                return(cp);
    }

    return(NULL);
}

/* 
 * Expand an invocable component specification into a list of numbers for use
 * in the rerun routine.
 */
void make_ic_list(str, icsp)
char *str;
int **icsp;
{
    char *ptr, *cp;
    int ctr = 0, last = 0, num1, num2;

#ifdef DBG
    (void) fprintf(stderr,"make_ic_list(%s,icsp)\n", str);
#endif

    ptr = str;
    ++ptr; /* advance past opening bracket */

    cp = strchr(ptr,IC_END_MARK);
    *cp = '\0';

    while ( ! last)
    {
        cp = strchr(ptr,IC_SEP_MARK);
        if (cp == NULL)
        {
	    if( ! strcmp(ptr,"all"))
	        break;
	    else
	        last = 1;
        }
        else
	    *cp = '\0';

        if (strchr(ptr,'-') == NULL)
        {
	    (void) sscanf(ptr,"%d", &num1);
	    num2 = num1;
        }
        else
	    (void) sscanf(ptr,"%d-%d", &num1, &num2);

        for ( ; num1 <= num2; num1++)
	{
	    /* Run out of space - make some more */
	    if(ctr % 50 == 48)
		*icsp = (int *) TET_REALLOC((void *) *icsp, (size_t) (sizeof(int) * (ctr + 51)));

	    (*icsp)[ctr++] = num1;
	}

	if ( !last)
		ptr = (cp+1);
    }

    (*icsp)[ctr] = -2;

#ifdef DBG2
    (void) fprintf(stderr,"*icsp:");

    if((*icsp)[0] == -2)
        (void) fprintf(stderr,"all");
    else
        (void) fprintf(stderr,"%d", *icsp[0]);

    for(ctr = 1; (*icsp)[ctr] != -2; ctr++)
        (void) fprintf(stderr,",%d", (*icsp)[ctr]);

    (void) fprintf(stderr,"\n");
#endif
}


/*
 *  Comparison function used for qsort() in rerun_flush()
 */
int compare_ics(ic1, ic2)
#if __STDC__
const void  *ic1, *ic2;
#else
int *ic1, *ic2;
#endif
{
    return((*(int *)ic1) - (*(int *)ic2)); /* Patched XXX */
}
