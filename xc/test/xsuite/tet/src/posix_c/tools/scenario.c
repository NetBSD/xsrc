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

SCCS:           @(#)scenario.c    1.9 03/09/92
NAME:           scenario.c
PRODUCT:        TET (Test Environment Toolkit)
AUTHOR:         OSF Validation & SQA
DATE CREATED:   14 May 1991
CONTENTS:

MODIFICATIONS:

                "TET Rework"
                David G. Sawyer, UniSoft Ltd,  July 1991.

                Fix syntax error on debug line
                Geoff Clare, UniSoft Ltd, 7 Oct 1991.

                process_line(): Pointer was not being dereferenced in if 
                                statement of switch case '\'.
                David G. Sawyer, UniSoft Ltd,  14 Nov 1991.

		Initialise variable "rc" in process_line().
                Geoff Clare, UniSoft Ltd, 26 Nov 1991.

		Check for a line thats too long in perform_scen()
		David G. Sawyer, UniSoft Ltd, 28 Jan 1991.

		Replaced all occurences of fnbc_p with linein in process_line.
		Took out resume option code from exec_tc().
		David G. Sawyer, UniSoft Ltd, 5 Feb 1991.

		process_line() and perform_scen() have been modified to
		support modifications and to resume and rerun.
		number_suffix() has been added.
		David G. Sawyer, UniSoft Ltd, 13 Feb 1992.

************************************************************************/

#include <tcc_env.h>
#include <tcc_mac.h>
#include <tet_jrnl.h>
#include <tcc_prot.h>

/* Initialise the results code table */
RESCODE res_tbl[MAX_RESCODES] = { 
    { 999,   "ENDENDEND",       "Abort"       } 
};

/* Default result codes and their corresponding actions */
RESCODE DEF_RES_CODE[] = {
    { 0,     "\"PASS\"",        "Continue"    },
    { 1,     "\"FAIL\"",        "Continue"    },
    { 2,     "\"UNRESOLVED\"",  "Continue"    },
    { 3,     "\"NOTINUSE\"",    "Continue"    },
    { 4,     "\"UNSUPPORTED\"", "Continue"    },
    { 5,     "\"UNTESTED\"",    "Continue"    },
    { 6,     "\"UNINITIATED\"", "Continue"    },
    { 7,     "\"NORESULT\"",    "Continue"    },
    { 999,   "ENDENDEND",       "Abort"       }
};

int g_max_rescode = 0;  /* The current number of defined result codes */
int exec_all_flag = 0;  /* Flag used to tell exec_tc() to add par. to exec() */
char zeetime[18];       /* Big enough to contain "HH:MM:YY YYYYMMDD" */


/* 
 *  process an scen from the scenario  file.
 *  the parameter 'scenario_name' is the value given
 *  on the tcc command line.
 */
int perform_scen()
{
    FILE *scen_fp;
    char *str1, *str2, *proc_ptr;
    bool found_it =  FALSE;
    char linein[INPUT_LINE_MAX];

#ifdef DBG
    (void) fprintf(stderr,"perform_scen()\n");
#endif
#ifdef DBG2
    (void) fprintf(stderr,"Scenario name is %s\n", scenario_name);
#endif

    /* Open the scenario file */
    if ((scen_fp = fopen(scen_file_name,"r")) == NULL)
    {
        (void) sprintf(error_mesg,"can't open scenario file : %s\n", scen_file_name);
        perror("fopen");
        BAIL_OUT2(error_mesg);
    }

    /* make sure the file descriptor is closed on an exec call  */
    (void) fcntl(fileno(scen_fp), F_SETFD, FD_CLOEXEC);

    /* search for the scen name */
    while ( ((fgets(linein,sizeof(linein),scen_fp)) != NULL)  && (abort_requested != TRUE))
    {
	/* Check for a line that is too big */
	if(strlen(linein) == (INPUT_LINE_MAX - 1))
	{
            (void) sprintf(error_mesg,"File %s. Line too long ! line begins \"%s\"\n", scen_file_name,linein);
            BAIL_OUT(error_mesg);
	}

        /* change newline to '\0' for string operations */
        if (linein[strlen(linein)-1] == '\n')
            linein[strlen(linein)-1] ='\0';

	++scenario_line;
	scenario_element = 0;

	/* Resume mode check */
	if (resume_status == 1)
	{
	    if (scenario_line == old_line)
	    {
#ifdef DBG2
                if(scenario_line != 1)
		    (void) fprintf(stderr,"\n");
#endif
		/* special case if this a scenario name line */
                if( (! isspace(linein[0])) && (linein[0] != ALI_CMNT_CHAR))
		{
            	    str1 = (char *) TET_MALLOC((size_t)( strlen(linein) + 1));
            	    (void) strcpy(str1, linein);

            	    /* get the first token */
            	    str2 = strtok(str1," \t");
            	    if( str2 == (char *) NULL)
	    	    {
		        TET_FREE((void *) str1);
                        continue;
	    	    }
                    proc_ptr = &(linein[strlen(str2) + 1]);
		    TET_FREE((void *) str1);
		}
		else
		    proc_ptr = linein;

	        found_it = TRUE;
	    }
	    else
	    {
#ifdef DBG2
                if(scenario_line == 1)
		    (void) fprintf(stderr,"Resume mode, looking for line %d; skipping scenario line: %d", old_line, scenario_line);
		else
		    (void) fprintf(stderr,",%d", scenario_line);
#endif
	        continue;
	    }
	}
	else
	    proc_ptr = linein;

        if( found_it == TRUE)
        {
            /* Check for end of scenario */
            if( (! isspace(*proc_ptr)) && (*proc_ptr != ALI_CMNT_CHAR))
                break;
        }

        if(found_it != TRUE)
        {
            str1 = (char *) TET_MALLOC((size_t)( strlen(linein) + 1));
            (void) strcpy(str1, linein);

            /* get the first token */
            str2 = strtok(str1," \t");
            if( str2 == (char *) NULL)
	    {
		TET_FREE((void *) str1);
                continue;
	    }

            /* is it scenario of interest */
            if (strcmp(scenario_name,str2) == 0)  
            {
                found_it = TRUE;    /* set the flag */
                process_line(&(linein[strlen(str2) + 1]));
		TET_FREE((void *) str1);
            }
            else
	    {
		TET_FREE((void *) str1);
                continue;
	    }
        }
        else
            process_line(proc_ptr);

	if (resume_status == -1) /* Error has occurred */
	    break; /* break out of file scanning loop */
    }

    (void) fclose(scen_fp);

    if (resume_status == 1) /* Still looking for the TC line to resume */
    {
        (void) sprintf(error_mesg,"Expected \"%s\" to be on the %d%s line of the scenario file.\n", g_tc_line, old_line, number_suffix(old_line));
        ERROR(error_mesg);
    }
    else if (found_it == FALSE)
    {
        (void) sprintf(error_mesg,"requested scenario \"%s\" not found in file %s\n", scenario_name, scen_file_name);
        BAIL_OUT2(error_mesg);
    }


    if (resume_status != 0)
	return(FAILURE);

    return(SUCCESS);
}


/*
 * Process a line from the 'scenario' file 
 */
void process_line(linein)
char *linein;
{
    char line2[_POSIX_PATH_MAX];
    char *file_name, *tmp_ptr, *cp;
    FILE *inc_fp;
    int rc, *ics, ctr;
    char runline[INPUT_LINE_MAX];
  
    if (linein[strlen(linein)-1] == '\n')
        linein[strlen(linein)-1]='\0';

#ifdef DBG
    (void) fprintf(stderr,"process_line(%s)\n", linein);
#endif

    if (strlen(linein) == 0) /* A blank line - simply return */
        return;

    /* Comments must start with the first non-space */
    if (*linein == ALI_CMNT_CHAR) /* A 'comment' line - simply return */
    {
	++scenario_element;
        return;
    }

    /* find First Non Blank Char (linein) */
    for (;((isspace(*linein)) && (*linein != '\0') ); linein++);

    while( *linein != '\0')
    {
        switch((int) *linein)
        {
        case ALI_MSG_CHAR: /* A message to be included in the journal */
	    ++scenario_element;
            for( tmp_ptr = (linein+1); *tmp_ptr != ALI_MSG_CHAR; tmp_ptr++)
            {
                if( *tmp_ptr == '\0')
                {
                    (void) sprintf(error_mesg,"Scenario file : unmatched \" in : %s\n", linein);
                    BAIL_OUT2(error_mesg);
                }
            }
            ++tmp_ptr;
            if(tmp_ptr == '\0')
            {
                (void) jnl_entry_scen(linein);
                return;
            }
            else
            {
                *tmp_ptr = '\0';
                (void) jnl_entry_scen(linein);
                linein = ++tmp_ptr;
            }
            break;

        case '/': /* An invocable component */
	    ++scenario_element;
            for( tmp_ptr = linein;(*tmp_ptr != '\0')&&(!isspace(*tmp_ptr));
                                   tmp_ptr++);
            if(*tmp_ptr != '\0')
            {
                *tmp_ptr = '\0';
                ++tmp_ptr;
            }

	    (void) strcpy(runline, linein);

	    if (resume_status == 1) /* resume mode active */
	    {

	    if(scenario_element == old_element)
	    {

		cp = strchr(runline,IC_START_MARK);
		if (cp != NULL)
		    *cp = '\0';

		/* Check TC names match */
		if (strcmp(runline, g_tc_line))
		{
		    (void) sprintf(error_mesg, "Expected \"%s\" to be the %d%s line of the scenario file, found \"%s\" instead.\n", g_tc_line, old_line, number_suffix(old_line), runline);
		    ERROR(error_mesg);
		    resume_status = -1;
		    linein = tmp_ptr;
		    TET_FREE((void *) g_tc_line);
		    break; 			/* break out of switch */
		}
		else
		    TET_FREE((void *) g_tc_line);

	   	if (cp != NULL)
		{
    		    ics = (int *) TET_MALLOC((size_t) (sizeof(int) * 50));
    		    ics[0] = -2;
		    make_ic_list(cp, &ics);
		    for(ctr = 0; ics[ctr] != -2; ctr++)
		    {
		        if (ics[ctr] == g_ic_num)
			    break;
		    }
		    if (ics[ctr] == -2) /* didn't find IC */
		    {
		        (void) sprintf(error_mesg,"Expected to find IC %d on line %d of the scenario file, line begins \"%s\".\n", g_ic_num, old_line, runline);
		        ERROR(error_mesg);
		        linein = tmp_ptr;
		        break; /* break out of switch */
		    }
		    else
		    {
		        /* build up list of remaining ICs in list */

		        /* should be careful about line length, but */
		        (void) sprintf(runline,"%s{%d",runline,ics[ctr]);
		        for (++ctr; ics[ctr] != -2; ctr++)
		            (void) sprintf(runline,"%s,%d", runline, ics[ctr]);
		        (void) sprintf(runline,"%s}",runline);
		    }
		    TET_FREE((void *) ics);
		}
		else
		{
		    if (g_ic_num != -1)
		    {
		        (void) sprintf(runline, "%s{%d}", runline, g_ic_num);
		        exec_all_flag = TRUE;
		    }

		    /* TC line with no ICs specified */

		}

	    }
	    else
	    {
		linein = tmp_ptr;
		break;             /* break out of switch */
	    }

		resume_status = 0; /* Resume option no longer active */
	    }

            /*
             * We have found a test case that is to be built and/or
             * cleaned and/or executed; ensure the right environment is being
             * used for each mode.
             */

	    rc = SUCCESS;

            if (build_mode == TRUE)
            {
                tet_env = bld_env;

                /* do the 'build' */
                rc = tool_tc(runline,BUILD);
            }      

            if ((exec_mode == TRUE) && (rc != FAILURE))
            {
                tet_env = exe_env;

                /* do the 'execute' */
                rc = exec_tc(runline);
            }

            if ((clean_mode == TRUE) && (rc != FAILURE))
            {
                tet_env = cln_env;

                /* do the 'clean' */
                rc = tool_tc(runline,CLEAN);
            }

            linein = tmp_ptr;

            break;

        case ':': /* An include */
	    ++scenario_element;
            if (strncmp(ALI_INC_STRNG,linein,strlen(ALI_INC_STRNG)) == 0)
            {
                /*
                 * An 'include' line - process the "included" scenario file
                 */
                linein += strlen(ALI_INC_STRNG);
                for( tmp_ptr = linein;(*tmp_ptr != '\0')&&(!isspace(*tmp_ptr));
                                       tmp_ptr++);
                if(tmp_ptr != '\0')
                {
                    *tmp_ptr = '\0';
                    ++tmp_ptr;
                }

                /* If alt_exec_dir is defined it should be relative to that */
                if( alt_exec_set)
                {
                    file_name = (char *) TET_MALLOC(strlen(alt_exec_dir) + strlen(linein) + 2);
                    (void) sprintf(file_name,"%s/%s", alt_exec_dir, linein);
                }
                else
                {
                    /* otherwise it should be relative to the test suite root */
                    file_name = (char *) TET_MALLOC(strlen(test_suite_root) + strlen(linein) + 2);
                    (void) sprintf(file_name,"%s/%s", test_suite_root, linein);
                }

                inc_fp = fopen(file_name,"r");
                if (inc_fp == NULL)
                {
                    (void) sprintf(error_mesg,"error opening include file %s\n",file_name);
                    perror("fopen");
                    BAIL_OUT2(error_mesg);
                }

                if( alt_exec_set)
                    TET_FREE((void *) file_name);

                /* make sure the file descriptor is closed on an exec call */
                (void) fcntl(fileno(inc_fp), F_SETFD, FD_CLOEXEC);

                /*
                 * Note the recursive call to process the "included" file
                 */
                while (fgets(line2,sizeof(line2),inc_fp) != NULL)
                    process_line(line2);

                (void) fclose(inc_fp);

                linein = tmp_ptr;
            }
            else
            {
                (void) sprintf(error_mesg,"Scenario file : unrecognised element : %s\n", linein);
                BAIL_OUT2(error_mesg);
            }
            break;

        case ALI_CMNT_CHAR:
	    ++scenario_element;
            (void) sprintf(error_mesg,"Scenario file : comment does not begin in column one ? : %s", linein);
            BAIL_OUT2(error_mesg);
            break;

        default:
            (void) sprintf(error_mesg,"Scenario file : unrecognised element : %s", linein);
            BAIL_OUT2(error_mesg);
            break;
        }
    }

    return;
}


/*
 * Execute a test case; arguments are:
 * pointer to test case name proper (i.e. first non-blank char)
 * pointer to context message (debugging information)
 * pointer to scenario name.
 */
int exec_tc(fnbc_p)
char *fnbc_p;
{
    char *command_name_p, *arg_p, *ic_end_mark_p;
    char *ic_start_mark_p, *ic_string, *output_capture_p;
    char *exec_in_place, tc_dir[_POSIX_PATH_MAX];
    char object_path[_POSIX_PATH_MAX];
    char curr_dir[_POSIX_PATH_MAX], rm_dir[_POSIX_PATH_MAX];
    int exec_rc;
    struct sigaction on_alarm;

#ifdef DBG
    (void) fprintf(stderr,"exec_tc(%s)\n",fnbc_p);
#endif

    /* check to see whether this line should be processed */
    if(check_line(fnbc_p) == FAILURE)
    {
#ifdef DBG2
        (void) fprintf(stderr,"line not to be processed.\n");
#endif
        /* Not a failure - just don't want it that's all */
        return(SUCCESS);
    }

    /* Make sure we are back in this directory at the end of the TC */
    if (getcwd(curr_dir,sizeof(curr_dir)) == NULL)
    {
        perror("get current dir");
        BAIL_OUT("getcwd");
    }

    /* Is exec in place set for execution */
    exec_in_place = get_tetenv("TET_EXEC_IN_PLACE");
    if (exec_in_place == NULL || ((*exec_in_place == 'F') || 
                         (*exec_in_place == 'f')))
        eip_set = FALSE;
    else
        eip_set = TRUE;

    /* Is output capture set for execution */
    output_capture_p = get_tetenv("TET_OUTPUT_CAPTURE");
    if ( (output_capture_p != NULL) && ((*output_capture_p == 'T') || 
                         (*output_capture_p == 't')) )
        oc_set = TRUE;
    else
        oc_set = FALSE;

    (void) strcpy(tc_dir, fnbc_p);
    /* make tc_dir the right length */
    *(strrchr(tc_dir, DIR_SEP_CHAR)) = '\0';

    /* set  the "object" directory (where the T.C.'s come from) */
    if(alt_exec_set)
        (void) sprintf(object_path, "%s/%s", alt_exec_dir, tc_dir);
    else
        (void) sprintf(object_path, "%s/%s", test_suite_root, tc_dir);

#ifdef DBG2
    (void) fprintf(stderr,"changing dir to %s\n", object_path);
#endif
    if(chdir(object_path) == FAILURE)
    {
        (void) sprintf(error_mesg,"Can't change to test case dir: %s\n", object_path);
        ERROR(error_mesg);
        return(FAILURE);
    }

    command_name_p = (char *)TET_MALLOC(strlen(fnbc_p)+1);
    (void) strcpy(command_name_p, fnbc_p);

    /* search for beginning "{" */
    ic_start_mark_p = strchr(command_name_p,IC_START_MARK);
    if( ic_start_mark_p != NULL)
    {
        ic_string = (char *) TET_MALLOC(strlen(ic_start_mark_p) + 2);
        (void) strcpy(ic_string, ic_start_mark_p);

        /* Make command_name_p the right length */
        *ic_start_mark_p = '\0';
        
        ic_start_mark_p = ic_string;

        /* Get a nice list of ICs with no surrounding braces */
        ic_start_mark_p++;
        ic_end_mark_p = strchr(ic_string,IC_END_MARK);
        if(ic_end_mark_p == NULL)
            BAIL_OUT2("No closing brace for IC list\n");
        *ic_end_mark_p = '\0';
    }

    if(produce_output)
    {
        (void) get_time();
        (void) fprintf(stdout,"%s  Execute  %s\n", zeetime, fnbc_p);
    }

    /* extract Invocable Component name(s) */

    if (ic_start_mark_p == NULL)
        arg_p = NULL;
    else
    {
        arg_p = (char *)TET_MALLOC(strlen(ic_start_mark_p) +1);
        (void) strcpy(arg_p,ic_start_mark_p);
	TET_FREE((void *) ic_string);
    }

    /* If there is an IC list and it wasn't made for resume mode */
    if(arg_p != NULL && exec_all_flag == 0)
        (void) sprintf(line,", ICs {%s}", arg_p);
    else
        line[0] = '\0';

    /* Make an entry in the journal to say what we're about to exec */
    (void) jnl_entry_invoke_tc(exec_ctr,command_name_p,line);


    if ( eip_set == FALSE)
    {
        /*
         *  Because we don't want to execute the TCs inplace we do a recursive
         *  copy of the execution directory to the temp directory 
         *  Locking is taken care of in this function aswell.
         */
        if(do_copy(object_path) == FAILURE)
            return(FAILURE);
        /*
         * Make a note of the directory were in so that we can remove it
         * after we have run the TC. This means we won't start eating up
         * loads of disc space.
         */
        if (getcwd(rm_dir,sizeof(rm_dir)) == NULL)
        {
            perror("get current dir");
            BAIL_OUT("getcwd");
        }
    }
    else
    {
        /* We want to exec in place so we need to get an exclusive lock */
        if (obtain_exe_excl_lock() == FAILURE)
        {
            jnl_tc_end(exec_ctr++,TET_ESTAT_LOCK,"TC End");
            return(FAILURE);
        }
    }

    /* get rid of the old tet_xres file */
    (void) unlink(TMP_RES_FILE); 

    /* Set the timeout alarm call */
    on_alarm.sa_handler = exec_sig_hdlr;
    on_alarm.sa_flags = 0;
    (void) sigaction(SIGALRM, &on_alarm, (struct sigaction *)NULL);
    (void) alarm((unsigned int)g_timeout);

    /* This is the function that actually execs the TC */
    exec_rc = start_tc(command_name_p,arg_p);

    /* if things haven't been reset already */
    if(cpid != 0)
    {
        /* No more child */
        cpid = 0;

        /* Reset the timeout alarm call */
        (void) alarm(0);
    }
    on_alarm.sa_handler = SIG_DFL;
    on_alarm.sa_flags = 0; 
    (void) sigaction(SIGALRM, &on_alarm, (struct sigaction *)NULL);

    /* Save any files that we listed as wanting to be saved */
    do_save(command_name_p);

    if (eip_set == FALSE)
    {
        /* Need to get rid of the directory we copied across */
        do_rm(rm_dir);
    }    
    else
        release_exe_excl_lock();

    /* Append the Test Case finish line to the journal */
    jnl_tc_end(exec_ctr++,context_status,line);

    /* Get back to where we were */
    if(chdir(curr_dir) == FAILURE)
        BAIL_OUT("chdir to curr. dir");

    /* Completed another action so make a note of the change in context */
    context++;

    if (arg_p != NULL)
        TET_FREE((void *)arg_p);

    TET_FREE((void *) command_name_p);

    return(exec_rc);
}

/*
 * Set the default result codes & actions, then override/supplement
 * them by any user-supplied result codes files that exist - firstly
 * $TET_ROOT/$TET_RESCODE_FILE, then $TET_ROOT/suite-name/$TET_RESCODE_FILE.
 * No arguments; returns SUCCESS
 */
int get_rescode_file()
{
    char *rcf_name, *rcfn_p;
    FILE *rc_fp;
    int ctr;

#ifdef DBG
    (void) fprintf(stderr,"get_rescode_file()\n");
#endif

    /* initialize with default values */
    for (ctr = 0 ; (strcmp(DEF_RES_CODE[ctr].name,"ENDENDEND") != 0); ctr++)
    {
        res_tbl[ctr].num = DEF_RES_CODE[ctr].num;
        res_tbl[ctr].name = (char *) TET_MALLOC2((size_t) (strlen(DEF_RES_CODE[ctr].name) + 1));
        (void) strcpy(res_tbl[ctr].name,DEF_RES_CODE[ctr].name);
        (void) strcpy(res_tbl[ctr].action,DEF_RES_CODE[ctr].action);
    }

    g_max_rescode = ctr;
    rcfn_p = get_tetenv("TET_RESCODES_FILE");

    /* pick up values from TET_ROOT file */
    rcf_name = (char *)TET_MALLOC(strlen(tet_root)+strlen(rcfn_p)+3);

    (void) strcpy(rcf_name,tet_root);
    if ((rcf_name)[strlen(rcf_name)-1] != DIR_SEP_CHAR)
        (void) strcat(rcf_name,DIR_SEP_STR);
    (void) strcat(rcf_name,rcfn_p);

    rc_fp = fopen(rcf_name,"r");

    if (rc_fp != NULL)
    {
        if(do_rescode_file(rc_fp) == FAILURE)
        {
            (void) fclose(rc_fp);
            BAIL_OUT2("Error in results code file.");
        }
        (void) fclose(rc_fp);
    }

    TET_FREE((void *)rcf_name);

    /* pick up values from suite specific file */
    rcf_name = (char *)TET_MALLOC(strlen(tet_root) + strlen(suite_name) + strlen(rcfn_p) + 4);

    (void) strcpy(rcf_name,tet_root);
    if ((rcf_name)[strlen(rcf_name)-1] != DIR_SEP_CHAR)
        (void) strcat(rcf_name,DIR_SEP_STR);
    (void) strcat(rcf_name,suite_name);
    if ((rcf_name)[strlen(rcf_name)-1] != DIR_SEP_CHAR)
        (void) strcat(rcf_name,DIR_SEP_STR);
    (void) strcat(rcf_name,rcfn_p);

    rc_fp = fopen(rcf_name,"r");

    if (rc_fp != NULL)
    {
        if(do_rescode_file(rc_fp) == FAILURE)
        {
            (void) fclose(rc_fp);
            BAIL_OUT2("Error in results code file.");
        }
        (void) fclose(rc_fp);
    }

    TET_FREE((void *)rcf_name);

    return(SUCCESS);
}


/*
 * Read a result codes file, and store its contents in the table.
 * Arguments:
 * file pointer to opened result codes file.
 * Returns SUCCESS (fatal error -> exit() call being made!)
 * The format of this file is checked quite carefully ! - I hope...
 * The journal is open at this point so all the errors just go to stderr
 */
int do_rescode_file(rc_fp)
FILE *rc_fp;
{
    int idx, num, i, rc = SUCCESS;
    char linein[INPUT_LINE_MAX];
    char *name = NULL, action[RESCODE_AC_LEN];
    char *myline = NULL;
    char *num_str = NULL, *name_str = NULL, *action_str = NULL;

#ifdef DBG
    (void) fprintf(stderr,"do_rescode_file(<FILE *>)\n");
#endif
        
    /*
     * Keep getting lines from the results file until you can't get any more
     * or until the maximum number of results codes is reached
     */
    while ( (fgets(linein,sizeof(linein),rc_fp) != NULL) &&
        (!feof(rc_fp)) && (g_max_rescode < MAX_RESCODES) )
    {
        if (linein[0] == CFG_CMNT_CHAR)
            continue;
        linein[strlen(linein) - 1] = '\0';

        /* Ignore blank lines */
        for(i = 0; linein[i] != '\0'; i++)
        {
            if(linein[i] != '\t' && linein[i] != ' ' && linein[i] != '\n')
                break;
        }
        if(linein[i] == '\0')
            continue;

        if(name_str != NULL)
            TET_FREE((void *) name_str);
        name_str = (char *) TET_MALLOC(strlen(linein) + 5);

        if(num_str != NULL)
            TET_FREE((void *) num_str);
        num_str = (char *) TET_MALLOC(strlen(linein) + 5);

        if(action_str != NULL)
            TET_FREE((void *) action_str);
        action_str = (char *) TET_MALLOC(strlen(linein) + 5);

        if(myline != NULL)
            TET_FREE((void *) myline);
        myline = (char *) TET_MALLOC(strlen(linein) + 5);

        action[0] = '\0';

        /* put a copy of linein into myline so that we can play with it */
        if (linein != (char *) NULL)
            (void) strcpy(myline, linein);

        /* Look for the first field, which should be a number */
        /* This should cope with any leading white spaces in the line */
        if(strpbrk(myline,"0123456789") != (char *) NULL)
            (void) strcpy(myline,strpbrk(myline,"0123456789"));
        else
        {
            ERROR("Bad format line in tet_code file (no first field)\n");
            rc = FAILURE;
            break;
        }

        (void) strcpy(num_str, myline);

        /* Look for the beginning of the second field - shown by a quote */
        if( strchr(myline,'"') != (char *) NULL)
            (void) strcpy(name_str, strchr(myline,'"'));
        else
        {
            ERROR("Bad format line in tet_code file (no first quote)\n");
            rc = FAILURE;
            break;
        }
        num_str[strlen(num_str) - strlen(name_str) +1] = '\0';
        /* num_str should now contain a number followed by some white space */

        /* Get the number - and make sure that we got one */
        num = atoi(num_str);
        if(num == 0 && num_str[0] != '0')
        {
            ERROR("Bad format line in tet_code file (can't get first field num)\n");
            rc = FAILURE;
            break;
        }

        /* Get the end of the second field */
        if(strrchr(name_str+1,'"') != (char *) NULL)
            (void) strcpy(action_str, strrchr(name_str+1,'"'));
        else
        {
            ERROR("Bad format line in tet_code file (no second quote)\n");
            rc = FAILURE;
            break;
        }
        name_str[strlen(name_str)-strlen(action_str) +1] = '\0';

        /*
         *  Look for the third field; the action to take. If it is set
         *  to Abort or Continue then use that value otherwise use the
         *  default action - Continue I think
         */
        if(strpbrk(action_str,"CA") != (char *) NULL)
            (void) strcpy(action_str,strpbrk(action_str,"CA"));
        if( name != NULL)
            TET_FREE((void *) name);
        name = (char *) TET_MALLOC((size_t)(strlen(name_str)+1));
        (void) strcpy(name, name_str);
        if(action_str == (char *) NULL)
        {
            action[0] = '\0'; /* default action will be assumed */
        }
        else
        {
            (void) sscanf(action_str,"%s", action);
            if(action == (char *) NULL)
            {
                action[0] = '\0'; /* default action will be assumed */
            }
            else if(strcmp(action,"Abort") && strcmp(action,"Continue"))
            {
                action[0] = '\0'; /* default action will be assumed */
            }
        }

        /* Check to see if the number is already in the table */
        if ((idx = rescode_num_to_index(num)) != FAILURE)
        {
            /* if not reserved,  the name can be changed */
            if (num > MAX_RSVD_RES_CODE)
            {
                res_tbl[idx].name = (char *) TET_REALLOC((void *)res_tbl[idx].name, (size_t) (strlen(name)+1));
                (void) strcpy(res_tbl[idx].name,name);
            }
            else if (strcmp(res_tbl[idx].name,name) != 0)
            {
                (void) fprintf(stderr,"Ignored name change for reserved result code %s\n", res_tbl[idx].name);
            }

            /* in any event the action can be changed */
            if (action[0] == '\0')
                (void) strcpy(res_tbl[g_max_rescode].action, DEFAULT_ACTION_NAME);
            else
                (void) strcpy(res_tbl[g_max_rescode].action, action);
        }
        else
        {
            /* Add an entry to the results codes table */
            if (g_max_rescode >= MAX_RESCODES)
            {
                BAIL_OUT2("rescodes table full\n");
            }
            res_tbl[g_max_rescode].name = (char *) TET_MALLOC2((size_t) (strlen(name)+1));
            (void) strcpy(res_tbl[g_max_rescode].name,name);
            if (action[0] == '\0')
                (void) strcpy(res_tbl[g_max_rescode].action, DEFAULT_ACTION_NAME);
            else
                (void) strcpy(res_tbl[g_max_rescode].action, action);
            res_tbl[g_max_rescode].num = num;
            g_max_rescode++;
        }
    }

    TET_FREE((void *)name_str);
    TET_FREE((void *)num_str);
    TET_FREE((void *)action_str);
    TET_FREE((void *)myline);
    TET_FREE((void *)name);

#ifdef DBG2
    for(i = 0; res_tbl[i].name != NULL; i++)
        (void) fprintf(stderr, "\t%15s : %s\n", res_tbl[i].name, res_tbl[i].action);
#endif

    return(rc);
}


/*
 *  Pass it a name and it will return the corresponding number for that name
 *  in the results code table if one exists otherwise return FAILURE.
 */
int rescode_name_to_num(name)
char *name;
{
    int ctr;

    for (ctr = 0 ; (ctr < MAX_RESCODES) ; ctr++)
    {
        if (! strcmp(res_tbl[ctr].name, name))
            return(res_tbl[ctr].num);
    }
    return(FAILURE);
}


/*
 *  Pass it a number and it will return the corresponding action for that number
 *  in the results code table if one exists otherwise return an empty string
 */
char *rescode_num_to_action(num)
int num;
{
    int ctr;

    for (ctr = 0 ; (ctr < MAX_RESCODES) ; ctr++)
        if (res_tbl[ctr].num == num)
            return(res_tbl[ctr].action);
    return("");
}


/* 
 *  Pass it a number and it will look for it in the results code table and
 *  and return its index number if it exists or else FAILURE.
 */
int rescode_num_to_index(num)
int num;
{
    int ctr;

    for (ctr = 0 ; (ctr < MAX_RESCODES) ; ctr++)
        if (num == res_tbl[ctr].num)
            return(ctr);
    return(FAILURE);
}


/*
 * Take a scenario line that is due for processing and checking to see
 * whether it matches any of the yes-exec and/or no-exec texts.
 * If there are yes texts then a piece of their text must appear in the
 * line otherwise FAILURE is returned. Any line passing this point is
 * checked for a match against the no texts. A match gives a FAILURE
 * return. Otherwise SUCCESS is returned.
 */
int check_line(query_str)
char *query_str;
{
    int num;
    char *ok_str = NULL;

#ifdef DBG
    (void) fprintf(stderr,"check_line(%s)\n", query_str);
#endif

    /* If there is any yes-exec text */
    if(yes_list != NULL)
    {
        for(num = 0; yes_list[num] != NULL; num++)
        {
            if((ok_str = findstr(query_str, yes_list[num])) != NULL)
                break;
        }
        if(ok_str == NULL)
            return(FAILURE);
    }
    ok_str = NULL;

    /* If there is any no-exec text */
    if(no_list != NULL)
    {
        for(num = 0; no_list[num] != NULL; num++)
        {
            if((ok_str = findstr(query_str, no_list[num])) != NULL)
                break;
        }
        if(ok_str != NULL)
            return(FAILURE);
    }
    return(SUCCESS);
}


char *number_suffix( num)
int num;
{
    static char suf[3];

#ifdef DBG
    (void) fprintf(stderr,"number_suffix(%d)\n", num);
#endif

    switch (num%10)
    {
	case 1:
	    (void) strcpy(suf, "st");
	    break;
	case 2:
	    (void) strcpy(suf, "nd");
	    break;
	case 3:
	    (void) strcpy(suf, "rd");
	    break;
	default:
	    (void) strcpy(suf, "th");
	    break;
    }

    return(suf);
}
