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

SCCS:          @(#)startit.c    1.9 03/09/92
NAME:          startit.c
PRODUCT:       TET (Test Environment Toolkit)
AUTHOR:        OSF Validation & SQA
DATE CREATED:  14 May 1991
CONTENTS:

MODIFICATIONS:

               "TET Rework"
               David G. Sawyer, UniSoft Ltd,  July 1991.

               start_tc(): Added FD_CLOEXEC for exec_pipe[WRT_SIDE] in child.
                           Read from OC pipe first rather than exec_pipe.
                           Corrected error handling when an exec() of a test
                             case fails.
                           Distinguish between fatality of exec() failures for
                             test cases and an exec_tool.
                           Changed debug_tool variable to exec_tool to improve
                             readability
                           Copied across and modified code from do_tool() to
                             deal with TET_EXEC_FILE.
                           Added dummy IC start, TP start and IC end journal
                             entries for when output capture set.
                           Changed if statement that excluded a non-zero exit 
                             status from the test case.
               David G. Sawyer, UniSoft Ltd,  30 Oct - 1 Nov 1991

	       Check for a line thats too long in copy_results_file()
	       David G. Sawyer, UniSoft Ltd, 28 Jan 1991.

	       Made last_tpnum variable in pr_list_to_jnl() static to allow
	       for TCM aborting test cases.
	       Added icend() and modeend().
	       David G. Sawyer, UniSoft Ltd, 31 Jan 1992.

               Split order_tp_results() into make_tprs_list() and
	       parse_tprs_list().
	       Made alterations to pr_list_to_jnl() and pr_array_to_jnl().
	       Restructured parse_tprs_list() and collate_and_sort().
	       David G. Sawyer, UniSoft Ltd, 17-19 Feb 1992.

	       Fixed Abort result code so that the tcc shuts down.
	       David G. Sawyer, UniSoft Ltd, 17-19 Feb 1992.

	       Alterted modestart() to support the resume option.
	       David G. Sawyer, UniSoft Ltd, 06 Mar 1992.

		 **** TET 1.9 for MIT X Test Suite ****
		Fixed comparison bug in compare_seq_num()
		Stuart Boutell,UniSoft Ltd, 18 Mar 1992

************************************************************************/

#include <tcc_env.h>
#include <tcc_mac.h>
#include <tet_jrnl.h>
#include <tcc_prot.h>


char *full_path = NULL;    /* The absolute pathname for an exec() call */
char linein[JNL_LINE_MAX]; /* Misc input line */
int  exec_ctr = 0;         /* The execution counter */
char lineout[BIG_BUFFER];  /* The line to be output to the journal */
char zeetime[18];          /* Big enough to contain "HH:MM:YY YYYYMMDD" */

sigjmp_buf jmpbuf;         /* this is the allocated jmpbuf */
TPRS *rootp;               /* The root pointer to the linked list of TPRS's */

/* The environment that is passed in export */
char *env_lines[5] = { NULL,NULL,NULL,NULL,NULL};


/*
 * Start a test case.
 * Arguments:
 * pointer to pathname of test case
 * pointer to list of Invocable Component(s)
 */
int start_tc(pathname,arg_p)
char *pathname, *arg_p;
{
    int down_pipe[2], up_pipe[2], exec_pipe[2], statloc, sequence;
    int rc, argc = 0, tries = 0, waitstatus, user_interrupt = FALSE;
    static int return_val, timed_out;
    char tmp_cwd[_POSIX_PATH_MAX], *exec_file_p;
    char *argv[16], *tmp_ptr, *exec_tool_p = NULL, *tc_name;
    char *str1, *str2, *str3;
    FILE *fp, *res_fp, *exec_fp;


#ifdef DBG
    (void) fprintf(stderr,"start_tc(%s, %s)\n", pathname, arg_p);
#endif

    /* These two are static because they would be used after a longjmp */
    timed_out = 0;
    return_val = SUCCESS;

    if ( oc_set == TRUE)
    {
        /* The Test Case reads from the down pipe and writes to the up pipe */

        /* downward: tcc to tc */
        rc = pipe(down_pipe);
        BAIL_OUT_ON(rc,"down_pipe");

        /* upward: tc to tcc */
        rc = pipe(up_pipe);
        BAIL_OUT_ON(rc,"up_pipe");
    }

    /* Set up the pipe that informs the parent if the exec call failed */
    rc = pipe(exec_pipe);
    BAIL_OUT_ON(rc,"exec pipe");

    /* Make sure all the necessary stuff is in the environment */
    rc = export(tet_env);
#ifdef DBG
    ERROR_ON(rc,"Failure in export().\n");
#endif

    /* Get value of TET_EXEC_TOOL from the tetenv */
    exec_tool_p = get_tetenv("TET_EXEC_TOOL");

    /* This #if is so that you debug the child before it execs, if set */
#ifdef ONE_PROC
    if ((cpid = 0) == 0)
#else

    /* Have a maximum of five goes at getting the fork() to work */
    while((cpid = fork()) == -1)
    {
        tries++;
        if(tries >= 5)
            break;
        (void) sleep(5);
    }
    if (cpid == 0)
#endif
    {
        /* in child */

        /* 
         * ensure the exec pipe is closed upon a sucessful execution so that
         * the parent is not left waiting to read the pipe if the IC should
         * hang for some reason
         */
        (void) fcntl(exec_pipe[PIPE_WRT_SIDE], F_SETFD, FD_CLOEXEC);

        /*
         *  Create absolute pathname for exec
         *  changed 'pathname' references to 'full_path'
         */

        if (getcwd(tmp_cwd,sizeof(tmp_cwd)) == NULL)
        {
            perror("get current dir");
            BAIL_OUT("getcwd");
        }
        (void) strcat(tmp_cwd, DIR_SEP_STR);

        /*
         * Append just the test-case name to the directory prefix
         */
        (void) strcat(tmp_cwd, basename(pathname));

        full_path = (char *) TET_MALLOC(strlen(tmp_cwd) + 1);
        (void) strcpy(full_path, tmp_cwd);
        

        /*
         *  If TET_EXEC_TOOL is set then this tool is execed with the
         *  TET_EXEC_FILE as a possible argument followed by TC name and its
         *  arguments.
         */

        if ((exec_tool_p != NULL) && (*exec_tool_p != '\0'))
        {
            /* set str2 to the same so that we can free the space afterwards */
            str1 = str2 = (char *)TET_MALLOC(strlen(exec_tool_p) + 1);
            (void) strcpy(str1,exec_tool_p);

            /*
             * This bit gets any parameters supplied with the exec tool
             * and makes sure they are used during the exec() call.
             */
        
            str3 = strtok(str1," \t");

            argv[argc] = (char *) TET_MALLOC (strlen(str3) + 1);
            (void) strcpy(argv[argc++], str3);

            /* Dont want to change the env variable so... */
            exec_tool_p = (char *)TET_MALLOC(strlen(str3) + 1);
            (void) strcpy(exec_tool_p, str3);

            str3 = strtok((char *) NULL," \t");
            while( str3 != NULL)
            {
                argv[argc] = (char *) TET_MALLOC (strlen(str3) + 1);
                (void) strcpy(argv[argc++], str3);
                str3 = strtok((char *) NULL," \t");
            }

            TET_FREE((void *) str2);

            exec_file_p = get_tetenv("TET_EXEC_FILE");

            if (exec_file_p != NULL)
            {
                /* set str2 to the same so we can free the space afterwards */
                str1 = str2 = (char *)TET_MALLOC(strlen(exec_file_p) + 1);
                (void) strcpy(str1,exec_file_p);

                str3 = strtok(str1," \t");

                argv[argc] = (char *) TET_MALLOC (strlen(str3) + 1);
                (void) strcpy(argv[argc++], str3);

                str3 = strtok((char *) NULL," \t");
                while( str3 != NULL)
                {
                    argv[argc] = (char *) TET_MALLOC (strlen(str3) + 1);
                    (void) strcpy(argv[argc++], str3);
                    str3 = strtok((char *) NULL," \t");
                }

                TET_FREE((void *) str2);
            }

            /* get the test case name leaf name from the full relative path */
            tc_name = basename(full_path);

            /* Add the leaf name of the TC to the list of argv */
            argv[argc++] = tc_name;
        }
        else
        {
            argv[argc++] = full_path;
        }

        argv[argc++] = arg_p;
        /* special case for resume mode without IC num list */
        if(exec_all_flag)
        {
            exec_all_flag = 0;
            argv[argc] = (char *) TET_MALLOC(strlen("all") +2);
            (void) strcpy(argv[argc], "all");
            argc++;
        }

        /* Make sure that argv is NULL terminated, and that argc is correct */
        argv[argc++] = NULL;

#ifdef DBG2
        if(exec_tool_p == NULL || *exec_tool_p == '\0')
            (void) fprintf(stderr,"About to execvp %s ", full_path);
        else
            (void) fprintf(stderr,"About to execvp %s ", exec_tool_p);
        for(tries = 0; argv[tries] != NULL; tries++)
            (void) fprintf(stderr,"%s ", argv[tries]);
        (void) fprintf(stderr, "\n");
#endif

        if ( oc_set == TRUE)
        {
            rc = close(down_pipe[PIPE_WRT_SIDE]);
            ERROR_ON(rc,"close down pipe / wrt side\n");
            rc = close(up_pipe[PIPE_RD_SIDE]);
            ERROR_ON(rc,"close up pipe / rd side\n");

            /* child's STDIN from downward pipe */
            rc = dup2(down_pipe[PIPE_RD_SIDE],FILENO_STDIN);
            ERROR_ON(rc,"dup2 down pipe -> stdin\n");

            /* child's STDOUT and STDERR to upward pipe */
            rc = dup2(up_pipe[PIPE_WRT_SIDE],FILENO_STDOUT);
            ERROR_ON(rc,"dup2 up pipe -> stdout\n");

            rc = dup2(up_pipe[PIPE_WRT_SIDE],FILENO_STDERR);
            ERROR_ON(rc,"dup2 up pipe -> stderr\n");

            rc = close(down_pipe[PIPE_RD_SIDE]);
            ERROR_ON(rc,"close down pipe rd side\n");
            rc = close(up_pipe[PIPE_WRT_SIDE]);
            ERROR_ON(rc,"close up pipe wrt side\n");
        }

        if(exec_tool_p != NULL && *exec_tool_p != '\0')
        {
            /* Exec() the debug tool with the test case as an argument */
            rc = execvp(exec_tool_p,argv);
        }
        else
        {
            /* Exec() the Test Case */
            rc = execvp(full_path,argv);
        }

        /* The exec failed */
        if((exec_fp = fdopen( exec_pipe[PIPE_WRT_SIDE], "w")) != NULL)
            (void) fprintf(exec_fp, "TET_EXEC_FAILED");
        exit(EXIT_BAD_CHILD);
    }
    else
    {
        /* In the parent of the fork() call */

        /* Bail out if the fork() failed */
        BAIL_OUT_ON(cpid,"fork");

        /* The default string for the TC End message in the journal */
        (void) sprintf(line,"TC End");

        /*
         *  If the test times out this is where control will return to,
         *  and in that event timed_out will be set to non-zero.
         */
        timed_out = sigsetjmp(jmpbuf, 1);

        if (timed_out == 0)
        {
            /* If output capture is set then read info from the test */
            if ( oc_set == TRUE)
            {
                /* make dummy IC and TP start entries in journal */
                (void) get_time();
                (void) sprintf(lineout,"%d|%d 0 1 %s|IC Start\n",TET_JNL_IC_START,
                                        exec_ctr, zeetime);
                (void) jnl_entry(lineout);
                (void) sprintf(lineout,"%d|%d 0 %s|TP Start\n",TET_JNL_TP_START,
                                        exec_ctr, zeetime);
                (void) jnl_entry(lineout);

                fp = fdopen(up_pipe[PIPE_RD_SIDE],"r");
                if ( fp == NULL)
                    BAIL_OUT("fdopen up_pipe[PIPE_RD_SIDE]");

                rc = close(up_pipe[PIPE_WRT_SIDE]);
                ERROR_ON(rc,"close up pipe / wrt. side\n");

		sequence = 0;
                while ( fgets(linein,sizeof(linein),fp) != NULL)
		{
		    int n = strlen(linein);
		    if (linein[n-1] == '\n')
		        linein[n-1] = '\0';
		    (void) sprintf(lineout,"%d|%d 0 0 0 %d|%s\n",TET_JNL_TC_INFO,
					    exec_ctr, ++sequence, linein);
		    (void) jnl_entry(lineout);
		}
                (void) fclose(fp);

                rc = close(down_pipe[PIPE_RD_SIDE]);
                ERROR_ON(rc,"close down pipe / rd side\n");
                rc = close(down_pipe[PIPE_WRT_SIDE]);
                ERROR_ON(rc,"close down pipe / wrt side\n");
            }

            /* Check to see if exec() call failed */
            exec_fp = fdopen(exec_pipe[PIPE_RD_SIDE], "r");
            if (exec_fp == NULL)
                BAIL_OUT("fdopen exec_pipe[PIPE_RD_SIDE]");

            (void) close (exec_pipe[PIPE_WRT_SIDE]);
            while(fgets(linein, sizeof(linein), exec_fp) != NULL)
            {
                if( !strcmp(linein, "TET_EXEC_FAILED"))
                {
                    if(exec_tool_p == NULL || *exec_tool_p == '\0')
                    {
                        /* exec() of TC failure = non fatal */
                        ERROR("Failed exec of Test Case\n");
                        (void) fclose(exec_fp);
                        context_status = TET_ESTAT_EXEC_FAILED;
                        return(FAILURE);
                    }
                    else
                    {
                        /* exec() of exec_tool failure = fatal */
                        /* Note this jnl_tc_end is here coz of the BAIL_OUT */
                        jnl_tc_end(exec_ctr++,TET_ESTAT_EXEC_FAILED,"TC End, Exec() of exec tool");
                        BAIL_OUT2("Exec tool failure.");
                    }
                }
            }
            (void) fclose(exec_fp);

            /* Wait for the child to complete */
            while(waitpid((pid_t)cpid,&statloc,0) == FAILURE)
            {
                /* If the child recieved a signal */
                if(errno == EINTR)
                    continue;
                ERROR("waitpid child\n");
                return(FAILURE);
            }

            if(WIFSIGNALED(statloc))
            {
                waitstatus = WTERMSIG(statloc);

                /* if user interrupt */
                if((waitstatus == SIGTERM && interrupted == 2) ||
                        (waitstatus == SIGINT && interrupted == 1))
                    user_interrupt = TRUE;

                context_status = waitstatus + 1000;
            }
            else
                context_status = WEXITSTATUS(statloc);

            /* make sure user interrupt flag is cleared */
            interrupted = 0;

            /* If they haven't already been reset... */
            if (cpid != 0)
            {
                /* No more child */
                cpid = 0;

                /* Reset the timeout alarm call */
                (void) alarm(0);
            }

            /* if exited */
            if ( WIFEXITED(statloc))
            {
                /* copy the results file into the journal file */
                if (debug_suite == TRUE)
                {
                    /* Gives the user a chance to play with the results file */
                    (void) fprintf(stderr, "EXEC,  about to process results file. Name is %s\n", results_file);
                    (void) fprintf(stderr,"newline/CR to continue:");
                    (void) gets(linein);
                }

                /*
                 *  If no ouput capture then get the results from the 
                 *  results file instead.
                 */
                if (oc_set == FALSE)
                {
                    res_fp = fopen(results_file,"r");
                    if (res_fp == NULL) 
                    {
                        ERROR("exec_mode: can't open results file\n");
                        return(FAILURE);
                    }
                    else
                    {
                        /* copy the results into the journal file */
                        (void) copy_results_file(res_fp);
                        (void) fclose(res_fp);
                    }
                }

                (void) sprintf(line,"TC End");

                /* if output capture, add the TP result line to the jnl */
                if (oc_set == TRUE)
                {
                        if((tmp_ptr = rescode_num_to_name(context_status)) == (char *) NULL)
                        {
                            if( context_status < 1000)
                                (void) sprintf(error_mesg,"Couldn't match status %d of Test Case to a result string.\n", context_status);
                            else
                                (void) sprintf(error_mesg,"Test Case terminated by signal %d.\n", context_status%1000);
                            ERROR(error_mesg);
                            (void) jnl_tp_result(exec_ctr,0,7, "NORESULT");
                        }
                        else
                            (void) jnl_tp_result(exec_ctr,0,context_status,tmp_ptr);
                        /* make dummy IC end entry in the journal */
                        (void) get_time();
                        (void) sprintf(lineout,"%d|%d 0 1 %s|IC End\n",TET_JNL_IC_END,
                                                exec_ctr, zeetime);
                        (void) jnl_entry(lineout);
                }
            }
            else  /* didn't exit */
            {
                 if(context_status >= 1000)
                 {
                     (void) sprintf(line, "TC End, Test Case terminated by signal %d", context_status%1000);
                     if(user_interrupt == TRUE)
                          (void) strcat(line, " (User Interrupt)");
                 }

                 return_val = FAILURE;
            }

            if (abort_requested == TRUE)
                 return_val = FAILURE;

        }
        else
        {
#ifdef DBG2 
            (void ) fprintf(stderr,"Back in start_tc after time out.\n");
#endif
            (void) alarm(0);
            context_status = TET_ESTAT_TIMEOUT;
            return_val = FAILURE;
        }
    }
    return(return_val);
}


/*
 *  Export the TET environment. There are several stages.
 *  Write all the configuration variables to a file called "CONFIG" and then
 *  set TET_CONFIG to this files full pathname and export it into the env.
 *  Then do the same with the results codes, using a file called tet_code and
 *  a variable called TET_CODE.
 *  Next set TET_ACTIVITY to the current context and export it. If an alternate
 *  execution directory is defined set TET_EXECUTE equal to it and then export
 *  that to the env aswell.
 *  Finally export the tet root to the environment via the variable TET_ROOT
 */
int export(env)
ENV_T env[];
{
    char *fn;
    int ctr, len;
    FILE *fp;

#ifdef DBG
    (void) fprintf(stderr,"export(<env[]>)  ");
#endif
#ifdef DBG
    if(tet_env == bld_env)
        (void) fprintf(stderr,"Build env\n");
    else
        (void) fprintf(stderr,"%s env\n",((tet_env == exe_env) ? "Execute" : "Clean"));
#endif

    /*
     * The file that the configuration variables are to be written to resides
     * under the the "temp_dir".
     */
    fn = (char *)TET_MALLOC(strlen(temp_dir)+strlen("CONFIG")+2);
    (void) strcpy(fn,temp_dir);

    if (fn[strlen(fn)-1] != DIR_SEP_CHAR)
        (void) strcat(fn,DIR_SEP_STR);
    (void) strcat(fn,"CONFIG");

    fp = fopen(fn,"w");
    if (fp == NULL)
    {
        (void) sprintf(error_mesg,"filename %s\n", fn);
        ERROR(error_mesg);
        BAIL_OUT("open config. file for export");
    }

    /* Write all the config variables to the file */
    for (ctr = 0 ; env[ctr].name != NULL; ctr++)
    {
        if (env[ctr].value == NULL)
            (void) fprintf(fp,"%s=\n",env[ctr].name);
        else
            (void) fprintf(fp,"%s=%s\n",env[ctr].name,env[ctr].value);
    }

    (void) fclose(fp);

    /* set the env var TET_CONFIG equal to the full config file name */
    env_lines[0] = (char *)TET_MALLOC(strlen("TET_CONFIG")+3+strlen(fn));
    (void) sprintf(env_lines[0],"%s=%s","TET_CONFIG",(fn == NULL) ? "" : fn);

    (void) tet_putenv(env_lines[0]);
#ifdef DBG2
    (void) fprintf(stderr,"%s\n", env_lines[0]);
#endif
    TET_FREE((void *)env_lines[0]);

    TET_FREE((void *)fn);


    /* The file containing the results codes with reside under "temp_dir" */
    fn = (char *)TET_MALLOC(strlen(temp_dir)+strlen("/tet_code")+2);
    (void) strcpy(fn,temp_dir);
    if (fn[strlen(fn)-1] != DIR_SEP_CHAR)
        (void) strcat(fn,DIR_SEP_STR);
    (void) strcat(fn,"tet_code");

    fp = fopen(fn,"w");
    if (fp == NULL)
    {
        (void) sprintf(error_mesg,"filename %s\n", fn);
        ERROR(error_mesg);
        BAIL_OUT("open rescodes file for export");
    }

    /* Write results codes and actions into the file */
    for (ctr = 0 ; (ctr < g_max_rescode) ; ctr++)
        (void) fprintf(fp,"%d\t%s\t%s\n",res_tbl[ctr].num,res_tbl[ctr].name, res_tbl[ctr].action);

    (void) fclose(fp);

    /* set the env var TET_CODE equal to the full pathname of the file */
    env_lines[1] = (char *)TET_MALLOC(strlen(temp_dir)+3+strlen("TET_CODE")+strlen("/tet_code"));
    (void) sprintf(env_lines[1],"%s=%s%s","TET_CODE",temp_dir,"/tet_code");
    (void) tet_putenv(env_lines[1]);
#ifdef DBG2
    (void) fprintf(stderr,"%s\n", env_lines[1]);
#endif
    TET_FREE((void *)env_lines[1]);

    TET_FREE((void *)fn);

    /* set the env var TET_ACTIVITY equal to the current context */
    env_lines[2] = (char *)TET_MALLOC(strlen("TET_ACTIVITY")+3+40);
    (void) sprintf(env_lines[2],"%s=%d","TET_ACTIVITY",context);
    (void) tet_putenv(env_lines[2]);
#ifdef DBG2
    (void) fprintf(stderr,"%s\n", env_lines[2]);
#endif
    TET_FREE((void *)env_lines[2]);

    /* If alt_exec_dir is defined set TET_EXECUTE equal to it otherwise NULL */
    len = ((alt_exec_set) ? strlen(alt_exec_dir) : 1);
    len += strlen("TET_EXECUTE")+3;
    env_lines[3] = (char *)TET_MALLOC((size_t)len);
    (void) sprintf(env_lines[3],"%s=%s","TET_EXECUTE",((alt_exec_set == TRUE) ? alt_exec_dir : ""));

    (void) tet_putenv(env_lines[3]);
#ifdef DBG2
    (void) fprintf(stderr,"%s\n", env_lines[3]);
#endif
    TET_FREE((void *)env_lines[3]);

    /* set the env var TET_ROOT equal to the tet root directory */
    env_lines[4] = (char *)TET_MALLOC(strlen("TET_ROOT")+3+strlen(tet_root));
    (void) sprintf(env_lines[4],"%s=%s","TET_ROOT",(tet_root == NULL) ? "" : tet_root);

    (void) tet_putenv(env_lines[4]);
#ifdef DBG2
    (void) fprintf(stderr,"%s\n", env_lines[4]);
#endif
    TET_FREE((void *)env_lines[4]);

    return(SUCCESS);
}


/*
 * This function performs the first stage in organising the results file
 * contents and copying them into the journal file. The order of the lines
 * in the results file can get jumbled by concurrently executing processes,
 * and so special care has to taken to ensure that they appear in the
 * journal file in a coherent order.
 */
int copy_results_file(res_fp)
FILE *res_fp;
{
    int n, rc = 0;
    char buf[JNL_LINE_MAX], *fg_val;
    int return_val = 0; /* will be set to tp end status */


#ifdef DBG
    (void) fprintf(stderr,"copy_results_file(<FILE *>)\n");
#endif

    /*
     * We want to find out the tp end status so that it can be checked for
     * a build tool failure - any failures in formatting will be picked up
     * in the full analysis of the file below.
     */

    do
    {
        if ((( fg_val = fgets(buf,sizeof(buf),res_fp)) == NULL) &&
                        (feof(res_fp) == 0))
                break;
        if (fg_val != NULL)
        {
	    /* check line is not too big */
	    if(strlen(buf) == (JNL_LINE_MAX - 1))
	    {
		(void) sprintf(error_mesg,"Warning result file line exceeded %d bytes - line begins \"%s\"\n", JNL_LINE_MAX, buf);
		ERROR(error_mesg);
	    }

            rc = tpend(buf,1); /* Look for the TET_JNL_TP_END */
            if(rc == FAILURE)
                break;
            if(rc != 0) /* this line is the tp end result */
                (void)sscanf(buf,"%*d | %*d %*d %d",&return_val);
        }
    }
    while ( (!feof(res_fp)) && rc == 0 );

    /* Get back to the beginning of the file so we do the proper analysis */
    rewind(res_fp);


    /* Now we do the proper analysis */

    while (!feof(res_fp)) /* for all test purposes */
    {
        /* While we don't encounter a TP start write the lines to the jnl */
        do
        {
            if ((( fg_val = fgets(buf,sizeof(buf),res_fp)) == NULL) &&
                    (feof(res_fp) == 0))
            {
                BAIL_OUT("fgets from results file\n"); 
            }

            n = strlen(buf);

            if (fg_val != NULL)
            {
                rc = tpstart(buf); /* Look for the TET_JNL_TP_START */
                if(rc == FAILURE)
                {
                    
                    (void) sprintf(error_mesg,"Bad format line in results file: %s\n",buf);
                    BAIL_OUT2(error_mesg);
                }
                else if (rc == 0)
                    BAIL_OUT_ON(write(jnl_fd,(void *)buf,(size_t)n), "write results to jnl file");
            }
        } 
        while ( (!feof(res_fp)) && rc == 0 );

        if ((!feof(res_fp)) && (fg_val != NULL))
        {
            /* We should have just encountered a TP start, now sort the rest */
            parse_tprs_list( make_tprs_list(res_fp,buf));
        }
    }
    return(return_val); /* The tp end status or the default of 0 (PASS) */
}


/*
 *  Does the line buf correspond to a TP start line ?
 */
int tpstart(buf)
char *buf;
{
    int f, n;

    n = sscanf(buf,"%d",&f);

    if (n != 1)
    {
        (void) sprintf(error_mesg,"results file entry has bad format: %s\n", buf);
        ERROR(error_mesg);
        return(FAILURE);
    }

    return (f == TET_JNL_TP_START);
}


/*
 *  Does the line buf correspond to a Invocable component start line ?
 */
int icstart(buf)
char *buf;
{
    int f, n;

    n = sscanf(buf,"%d",&f);

    if (n != 1)
    {
        (void) sprintf(error_mesg,"results file entry has bad format: %s\n", buf);
        ERROR(error_mesg);
        return(FAILURE);
    }

    return (f == TET_JNL_IC_START);
}


/*
 *  Does the line buf correspond to a Invocable component start line ?
 */
int icend(buf)
char *buf;
{
    int f, n;

    n = sscanf(buf,"%d",&f);

    if (n != 1)
    {
        (void) sprintf(error_mesg,"results file entry has bad format: %s\n", buf);
        ERROR(error_mesg);
        return(FAILURE);
    }

    return (f == TET_JNL_IC_END);
}


/*
 *  Does the line buf correspond to a Test Purpose end line, and if so does
 *  the corresponding result code indicate that the user wishes to Abort ?
 */
int tpend(buf, check_for_abort)
char *buf;
int  check_for_abort;
{
    int f, rescode, n;

    n = sscanf(buf,"%d",&f);

    if (n != 1)
    {
        (void) sprintf(error_mesg,"results file entry has bad format: %s\n", buf);
        ERROR(error_mesg);
        return(FAILURE);
    }

    if (f == TET_JNL_TP_RESULT)
    {
        /* Check to see if an abort was requested */
        n = sscanf(buf,"%*d | %*d %*d %d",&rescode);
        if (n != 1)
        {
            (void) sprintf(error_mesg,"results file entry has bad format: %s\n", buf);
            ERROR(error_mesg);
            return(FAILURE);
        }

	if (check_for_abort)
            if (*rescode_num_to_action(rescode) == 'A')
                abort_requested = TRUE;
    }

    return (f == TET_JNL_TP_RESULT);
}


/*
 * Does the line buf correspond to a start line, and if so what type of mode
 * does the start line represent returns the context which is used in do_again.
 */
int modestart(buf)
char *buf;
{
    int f, f2, n, rc;

    n = sscanf(buf,"%d",&f);

    if (n != 1)
    {
        (void) sprintf(error_mesg,"Old journal file entry has bad format: %s\n", buf);
        FATAL_ERROR(error_mesg);
    }
    else
    {
        /* determine the type of mode and return a corresponding value */
        switch(f)
        {
        case TET_JNL_BUILD_START:
	    rc = 1;
	    break;
        case TET_JNL_INVOKE_TC:
	    rc = 2;
	    break;
        case TET_JNL_CLEAN_START:
	    rc = 3;
	    break;
        default:
	    rc = 0;
	    break;
        }
    }

    if(rc >= 1 && rc <= 3)
    {
	/* "mode_num|context tc_name time|mode_name Start, scenario line X" */
        n = sscanf(buf,"%*d|%*d %*[^|]|%*s %*[^,],%*s %*s %d-%d",&f, &f2);

        if (n != 2)
        {
	    (void) fprintf(stderr,"%d\n",n);
            (void) sprintf(error_mesg,"Old journal file entry has bad format: \"%s\" expected : \"%s\"\n", buf, "<mode_start_num>|<activity> <tc_name> <time>|<mode_name> Start, scenario ref <num>-<element>");
            FATAL_ERROR(error_mesg);
        }
	else
	{
	    old_line = f;
	    old_element = f2;
	}
    }

    return(rc);
}


/*
 * Does the line buf correspond to an end line, and if so what type of mode
 * does the start line represent returns the context which is used in rerun.
 */
int modeend(buf)
char *buf;
{
    int f, n;

    n = sscanf(buf,"%d",&f);

    if (n != 1)
    {
        (void) sprintf(error_mesg,"results file entry has bad format: %s\n", buf);
        ERROR(error_mesg);
        return(FAILURE);
    }

    /* determine the type of mode and return a corresponding value */
    switch(f)
    {
    case TET_JNL_BUILD_END:
        return(1);
    case TET_JNL_TC_END:
        return(2);
    case TET_JNL_CLEAN_END:
        return(3);
    default:
        return(0);
    }
}


/* 
 * This function processes the results file from and including the TP start
 * line. It generates a doubly linked list of the lines that may need
 * sorting into order, and if need be passes them onto a collate and sort
 * routine. The sorted lines eventually end up in the journal.
 * The Spec details the way in which the lines should be sorted.
 */
int make_tprs_list(res_fp,inbuf)
FILE *res_fp;
char *inbuf;
{
    int buf_len, rc = 0, count = 0;
    char buf[JNL_LINE_MAX], *data_p;
    TPRS *node;

#ifdef DBG
    {
    char *neat_inbuf;
    neat_inbuf = (char *) TET_MALLOC(strlen(inbuf) + 1);
    (void) strcpy(neat_inbuf, inbuf);
    neat_inbuf[strlen(neat_inbuf) - 1] = '\0';
    (void) fprintf(stderr,"make_tprs_list(<FILE *>, %s)\n", neat_inbuf);
    TET_FREE((void *) neat_inbuf);
    }
#endif

    /* 
     * At this point,  we've copied out up to (not incl.) TP start,  
     * and we have the TP start in inbuf.
     */

    buf_len = strlen(inbuf);

    /* rootp is the root pointer in a doubly linked list */
    rootp = (TPRS *)(TET_MALLOC(sizeof(TPRS)));
    count++;

    rootp->prev = rootp->next = NULL;

    rootp->data = (char *)TET_MALLOC((size_t)buf_len+1);

    /* node is the pointer to the current node in the doubly linked list */
    node = rootp;

    /* copy the contents of inbuf into the data field of rootp */
    (void) strncpy(rootp->data,inbuf,(size_t)buf_len+1);

    /*
     * Identify the current jnl entry and set the appropiate fields of
     * the structure pointed to by rootp.
     * This is a bit silly coz the entry should be the TP start - see comment
     * above ...
     */
    rc = analyze_jnl_entry(rootp->data,&(rootp->tpnum),
            &(rootp->block), &(rootp->num),&(rootp->context));
    if(rc == FAILURE)
    {
        /* Failed to sucessfull classify a line - free() everything & exit */
        (void) sprintf(error_mesg,"Bad format line in results file: %s\n",rootp->data);
        TET_FREE((void *) rootp->data);
        TET_FREE((void *) rootp);
        BAIL_OUT2(error_mesg);
    }
    else
        rootp->type = rc;

    /* As long as we can get lines from the file add them to the list */
    do
    {
        if ((data_p = fgets(buf,sizeof(buf),res_fp)) != NULL)
        {
            if (feof(res_fp))
                break;

            buf_len = strlen(buf);

            /*
             * Create a new node. Ensure the "next" pointer of the current
             * node points to the new node. Ensure the "prev" pointer of 
             * the new node points to the current node. Then advance the
             * current node pointer to this new node.
             */

            node->next = (TPRS *)(TET_MALLOC(sizeof(TPRS)));
            (node->next)->prev = node;
            node = node->next;
            count++;

            /*
             * Identify the current jnl entry and set the appropiate fields of
             * the structure pointed to by node 
             */
            rc = analyze_jnl_entry(data_p,&(node->tpnum),&(node->block),&(node->num), &(node->context));
            if(rc == FAILURE)
            {
                /*
                 * Failed to sucessfull classify a line
                 * free all the allocated nodes and exit.
                 */

		/* This the new node just allocated - no data associated yet */
	        node = node->prev;
                TET_FREE((void *) node->next);

                while(node->prev != NULL)
                {
                    TET_FREE((void *) node->data);
                    node = node->prev;
                    TET_FREE((void *) node->next);
                }

		/* node should be pointing to the rootp at this stage */
                TET_FREE((void *) node->data);
                TET_FREE((void *) node);

                (void) sprintf(error_mesg,"Bad format line in results file: %s\n",data_p);
                BAIL_OUT2(error_mesg);
            }
            else
                node->type = rc;

            node->data = (char *)TET_MALLOC((size_t)buf_len+1);
            (void) strncpy(node->data,buf,(size_t)buf_len+1);
            node->next = NULL;
        }
    } 
    while ( (data_p != NULL) && (feof(res_fp) == 0) );

    return(count);
}


/*
 * Take the list generated by make_tprs_list() and wade through it, sorting
 * and making entries into the journal as we go along
 */
void parse_tprs_list( count)
int count;
{
    int found_tc_info, this_tpnum = -1;
    TPRS *node;

#ifdef DBG
    (void) fprintf(stderr, "parse_tprs_list(%d)\n", count);
#endif

    /*
     * OK - At this point we have a doubly linked list of all the
     * remaining entries in the tmp results file from and including
     * TET_JNL_TP_START 
     */

    while( rootp != NULL)
    {
        node = rootp;
        found_tc_info = 0;

        /*
         * Look for a Test Case Info line - these are the ones that might need
         * sorting. 
         */
	for( ; node->next != NULL; node = node->next)
        {
            if(node->type == TET_JNL_TC_INFO)
            {
                found_tc_info = 1;
                break;
            }

	    if(node->type == TET_JNL_TP_RESULT)
	        this_tpnum = -1;
	    else if(node->type == TET_JNL_TP_START)
	    {
	        if (this_tpnum == -1)
	            this_tpnum = node->type;
	        else                  /* ie. missing a TP End Line */
		    break;
	    }
	    else if(node->type == TET_JNL_IC_END)
	    {
	        if (this_tpnum != -1) /* ie. missing a TP End Line */
		    break;
	    }
        }

        if(! found_tc_info)
        {
	    /* Missing a TP End Line */
	    if ( node->next == NULL)
            {
		/* End of list */
                pr_list_to_jnl(rootp);
		rootp = NULL;
            }
	    else /* TET_JNL_TP_START || TET_JNL_IC_END */
            {
                (node->prev)->next = NULL;
                node->prev = NULL;
                /* Print the list and append a TP result line */
                pr_list_to_jnl(rootp);
	        jnl_tp_result(exec_ctr,this_tpnum,7,"NORESULT, Added by TCC");
		this_tpnum = -1;
                rootp = node;
            }
        }
        else
        {
	    if (node != rootp) /* ie for interleaved info lines */
	    {
	        /* Flush the accumalated lines to the journal */
                (node->prev)->next = NULL;
                node->prev = NULL;
                pr_list_to_jnl(rootp);
	    }

            /* Collect more lines: same context and block - then sort them */
            node = collate_and_sort(node,count+1);
            if(node != (TPRS *) NULL) 
            {
                /* change of Test Purpose Num - Not good */
		if(node == rootp)
		{
        	    jnl_tp_result(exec_ctr,this_tpnum,7,"NORESULT, Added by TCC");
		    this_tpnum = -1;
		}
            }
        }
    }
}


/*
 * Take a doubly linked list and print it to the journal file, if the flag
 * is set and the last entry isn't a TET_JNL_TP_RESULT then add one of our
 * own. 
 */
void pr_list_to_jnl(node)
TPRS *node;
{
    TPRS *newnode;

#ifdef DBG
    (void) fprintf(stderr,"pr_list_to_jnl(TPRS *)\n");
#endif
    /* put in the journal,and free */
    while(node != NULL)
    {
        jnl_entry(node->data);
#ifdef DBG2
	(void) fprintf(stderr,"\t%s", node->data);
#endif
        TET_FREE((void *)node->data);
        newnode = node->next;
        TET_FREE((void *)node);
	node = newnode;
    }
}


/*
 *  Analyze a line for the journal, setting values for the current node in
 *  the TPRS structure as appropiate. If the wrong number of field is read
 *  then a FAILURE is indicated
 */
int analyze_jnl_entry(data_p,tpnum_p,block_p,num_p,context_p)
char *data_p;
int *tpnum_p;
int *block_p;
int *num_p;
int *context_p;
{
    int type, rc;

    *tpnum_p = *block_p = *num_p = *context_p = -1;
    rc = sscanf(data_p,"%d|",&type);
    if (rc < 1)
        return(FAILURE);

    switch (type)
    {
    case TET_JNL_TP_START: /* 200 */
        if(sscanf(data_p,"%*d|%*d %d",tpnum_p) != 1)
            return(FAILURE);
        break;
    case TET_JNL_TC_INFO: /* 520 */
        if(sscanf(data_p,"%*d|%*d %d %d %d %d",tpnum_p,context_p,block_p,num_p) != 4)
            return(FAILURE);
        break;
    case TET_JNL_TP_RESULT: /* 220 */
        if(sscanf(data_p,"%*d|%*d %d",tpnum_p) != 1)
            return(FAILURE);
        break;
    default:
        break;
    }

    return(type);
}


/*
 * group together an array of entries with the same context
 * and block numbers. As each entry is found it is removed from the the
 * origianl list and added to a new one. The new list is then sorted and
 * then printed to the journal file 
 */
TPRS *collate_and_sort(node, count)
TPRS *node;
int count;
{
    TPRS **sortlist;
    int this_context, this_block;
    int list_pos = 0, rootpset = 0;

#ifdef DBG
    (void) fprintf(stderr,"collate_and_sort(<TPRS *>, %d)\n", count);
#endif

    this_context = node->context;
    this_block = node->block;


    sortlist = (TPRS **) TET_MALLOC(sizeof(TPRS) * count);

    while(node != NULL && node->type != TET_JNL_TP_RESULT)
    {
        /* Missing TP Result Line */
	if(node->type == TET_JNL_TP_START || node->type == TET_JNL_IC_END)
	{
	    break;
	}
        else if(node->context == this_context && node->block == this_block)
        {
            /*
             * this bit removes the entry from the original linked list
             * and adds the entry to the end of the sort list 
             */
            if(node->prev != NULL)
                (node->prev)->next = node->next;
            if(node->next != NULL)
                (node->next)->prev = node->prev;

            sortlist[list_pos++] = node;
        }
        else if( ! rootpset)
	{
	    rootp = node;
	    rootpset = 1;
	}
        node = node->next;
    }

    /* now we sort the list we have gathered according to sequence number */
    qsort((void *)sortlist,(size_t)list_pos,sizeof(sortlist[0]),
        compare_seq_num);

    /* Print the sorted list to the journal */
    pr_array_to_jnl(sortlist, list_pos);

    TET_FREE((void *) sortlist);

    if(! rootpset)
        rootp = node;

    if(node == NULL || node->type == TET_JNL_TP_RESULT)
    {
        /* This indicates full success */
        return((TPRS *) NULL);
    }
    else
    {
        /* Missing TP End Line */
        /* Return a pointer to the node that begins the change of TP num */
        return(node);
    }
}


/*
 *  Comparison function used for qsort()
 */
int compare_seq_num(node1, node2)
#if __STDC__
const void  *node1, *node2;
#else
TPRS **node1, **node2;
#endif
{
    return((*(TPRS **)node1)->num - (*(TPRS **)node2)->num);
}


/*
 * Print an array of journal type lines to the journal.
 */
void pr_array_to_jnl(list, list_len)
TPRS **list;
int list_len;
{
    int i = 0;
#ifdef DBG
    (void) fprintf(stderr,"pr_array_to_jnl(TPRS *list, %d)\n", list_len);
#endif

    /* put in the journal */
    while(i < list_len)
    {
        jnl_entry(list[i]->data);
#ifdef DBG2
	(void) fprintf(stderr,"\t%s", list[i]->data);
#endif
        TET_FREE((void *)list[i]->data);
        TET_FREE((void *)list[i]);
        i++;
    }
}
