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

SCCS:           @(#)tool.c    1.9 03/09/92
NAME:           tool.c
PRODUCT:        TET (Test Environment Toolkit)
AUTHOR:         OSF Validation & SQA
DATE CREATED:   14 May 1991
CONTENTS:

MODIFICATIONS:

               "TET Rework"
               David G. Sawyer, UniSoft Ltd,  July 1991.

               do_tool(): Added FD_CLOEXEC for exec_pipe[WRT_SIDE] in child.
                          Read from OC pipe first rather than exec_pipe.
                          Use basename() to get TC leaf name
                          Now states which tool failed the exec() call...
               Made alt_exec_lock global.
               obtain_tool_lock(): Added tc_dir parameter and made 
                                     alt_exec_lock include it.
               release_tool_lock: Now tries simply to unlink alt_exec_lock.
               David G. Sawyer, UniSoft Ltd,  30 - 31 Oct 1991

	       Release source lock on failure to obtain exec lock.
	       Geoff Clare, UniSoft Ltd, 7 Nov 1991

	       Added error message for case when lock file exists but no
	       timeout has been specified to each of the obtain lock functions
	       David G, Sawyer, UniSoft Ltd, 28 Jan 1991.

	       Restore old SIGINT handler instead of always setting to
	       siginthdlr after execution of build fail tool.
	       Geoff Clare, UniSoft Ltd, 4 Mar 1992

************************************************************************/

#include <tcc_env.h>
#include <tcc_mac.h>
#include <tet_jrnl.h>
#include <tcc_prot.h>


char line[257];            /* line buffer used in printing var jnl entries */
char linein[JNL_LINE_MAX]; /* line buffer used to get data from pipe */
char *temp_file;           /* temporary file created in shared lock directory */
int  tmode;                /* tool mode: BUILD, or CLEAN */
char *where_lock;          /* The dir to be in, in order to free any locks */
char zeetime[18];          /* Big enough to contain "HH:MM:YY YYYYMMDD" */

char *alt_exec_lock = NULL;   /* pathname of lock file in alt_exec_dir */

/*
 *  Execute the specified tool.
 */
int tool_tc(testcase_p,what_mode)
char *testcase_p;
int what_mode;
{
    char *ic_start_mark_p, *output_capture_p, *tc_p = NULL;
    char tc_dir[_POSIX_PATH_MAX], curr_dir[_POSIX_PATH_MAX];
    char source_path[_POSIX_PATH_MAX];
    int rc, b_rc;
    struct sigaction on_alarm;

#ifdef DBG
    (void) fprintf(stderr,"tool_tc(%s, %d)\n", testcase_p, what_mode);
#endif

    /* check to see whether this line should be processed */
    if(check_line(testcase_p) == FAILURE)
    {
#ifdef DBG2
        (void) fprintf(stderr,"line not to be processed.\n");
#endif
        /* Not a failure - just don't want it thats all */
        return(SUCCESS);
    }

    tc_p = (char *) TET_MALLOC((size_t)(strlen(testcase_p)+1));
    (void) strcpy(tc_p, testcase_p);

    if ((ic_start_mark_p = strchr(tc_p,IC_START_MARK)) != NULL)
	tc_p[(ic_start_mark_p - tc_p)] = '\0';

    if(produce_output)
    {
        (void) get_time();
        (void) fprintf(stdout,"%s  %s  %s\n", zeetime, ((what_mode == BUILD) ? "Build  " : "Clean  "), tc_p);
    }

    /* Is output capture set for this mode */
    output_capture_p = get_tetenv("TET_OUTPUT_CAPTURE");
    if ( (output_capture_p != NULL) && ((*output_capture_p == 'T') || 
                         (*output_capture_p == 't')) )
        oc_set = TRUE;
    else
        oc_set = FALSE;

    /* tmode is used in the event of a timeout */
    tmode = what_mode;

    if (getcwd(curr_dir,sizeof(curr_dir)) == NULL)
    {
        perror("getcwd current dir");
	if(tc_p != NULL)
	    TET_FREE((void *) tc_p);
        return(FAILURE);
    }

    /*
     *  build up the dir we want to be in, and then we change directory to it.
     */

    (void) strcpy(tc_dir, tc_p);
    /* make tc_dir the right length */
    *(strrchr(tc_dir, DIR_SEP_CHAR)) = '\0';

    (void) sprintf(source_path,"%s/%s", test_suite_root, tc_dir);

#ifdef DBG2
    (void) fprintf(stderr,"chdir(%s)\n", source_path);
#endif
    if(chdir(source_path) == FAILURE)
    {
        (void) sprintf(error_mesg,"chdir to source dir: %s\n", source_path);
        ERROR(error_mesg);
	if(tc_p != NULL)
	    TET_FREE((void *) tc_p);
        return(FAILURE);
    }

    /* Put a start message into the journal file - for the tool */
    if(tmode == BUILD)
        jnl_tool_start(exec_ctr,tc_p, "Build Start", tmode);
    else
        jnl_tool_start(exec_ctr,tc_p, "Clean Start", tmode);

    /* Obtain an exclusive lock in order to execute the tool */
    if(obtain_tool_lock(tc_dir) == FAILURE)
    {
        jnl_tool_end(exec_ctr, TET_ESTAT_LOCK, ((tmode == BUILD) ? "Build End" : "Clean End"), tmode);
	if(tc_p != NULL)
	    TET_FREE((void *) tc_p);
        return(FAILURE);
    }

    /* Set the alrm call for the timeout */
    on_alarm.sa_handler = tool_sig_hdlr;
    on_alarm.sa_flags = 0;
    (void) sigaction(SIGALRM, &on_alarm, (struct sigaction *)NULL);
    (void) alarm((unsigned int)g_timeout);

    /*
     * Pass the full test-case name & let do_tool()
     * itself strip off directory prefix where necessary.
     */
    b_rc = do_tool(tc_p);

    /* If they haven't already been cleared; clear the cpid and timer */
    if(cpid != 0)
    {
        cpid = 0;
        (void) alarm(0);
    }
    on_alarm.sa_handler = SIG_DFL;
    on_alarm.sa_flags = 0; 
    (void) sigaction(SIGALRM, &on_alarm, (struct sigaction *)NULL);

    /* remove the lock */
    release_tool_lock();

    /* Put a end message into the journal file - for the tool */
    if(tmode == BUILD)
        (void) strcpy(linein, "Build End");
    else
        (void) strcpy(linein, "Clean End");
    if(line[0] != '\0')
        (void) strcat(linein, line);

    jnl_tool_end(exec_ctr++, context_status, linein, tmode);

    /* Get back to the dir we were in before we started */
    rc = chdir(curr_dir);
    BAIL_OUT_ON(rc,"chdir to current dir");

    /* Completed another operation - time to note the change of context */
    context++;

    if(tc_p != NULL)
	TET_FREE((void *) tc_p);
    return(b_rc); 
}


/*
 * Execute the tool
 * Arguments:
 * pointer to full test-case name
 * file descriptor of journal file
 * array of environment structures
 * timeout period (in seconds)
 */
int do_tool(pathname)
char *pathname;
{
    int up_pipe[2], exec_pipe[2], rc, tries = 0, tp_end_status;
    int statloc, statloc2, argc = 0, waitstatus, bft_pid;
    int user_interrupt = FALSE, argvfree;
    char *argv[50], *tool_file_p, *bft_p;
    char *str1, *str2, *str3, *this_tool, *tc_name;
    static int timed_out, return_val;
    FILE *res_fp, *fp, *exec_fp;
    struct sigaction on_int, old_int;

#ifdef DBG
    (void) fprintf(stderr,"do_tool(%s)\n", pathname);
#endif

    /* These two are static because they would be used after a longjmp */
    timed_out = 0;
    return_val = SUCCESS;

    /* Get the tool name from the tet environment */
    if (tmode == BUILD)
        this_tool = get_tetenv("TET_BUILD_TOOL");
    else
        this_tool = get_tetenv("TET_CLEAN_TOOL");

    if ((this_tool == NULL) || (*this_tool == '\0'))
    {
        if (tmode == BUILD)
        {
            BAIL_OUT2("TET_BUILD_TOOL has NULL value.\n");
        }
        else
        {
            BAIL_OUT2("TET_CLEAN_TOOL has NULL value.\n");
        }

        return(FAILURE);
    }
    else
    {
        /* set str2 to the same so that we can free the space afterwards */
        str1 = str2 = (char *)TET_MALLOC(strlen(this_tool) + 1);
        (void) strcpy(str1,this_tool);

        /*
         * This bit gets any parameters supplied with the tool
         * and makes sure they are used during the exec() call.
         */

        str3 = strtok(str1," \t");

        argv[argc] = (char *) TET_MALLOC (strlen(str3) + 1);
	argvfree = argc;
        (void) strcpy(argv[argc++], str3);

        /* Dont want to change the env variable so... */
        this_tool = (char *)TET_MALLOC(strlen(str3) + 1);
        (void) strcpy(this_tool, str3);

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
    tc_name = basename(pathname);

    if (oc_set == TRUE)
    {
        /*
         *  Establish whether there is a tool file and if so make sure that
         *  it is included in the list of parameters that is passed to the
         *  tool at execution time.
         */
        if(tmode == BUILD)
            tool_file_p = get_tetenv("TET_BUILD_FILE");
        else
            tool_file_p = get_tetenv("TET_CLEAN_FILE");

        if (tool_file_p != NULL)
        {
            /* set str2 to the same so that we can free the space afterwards */
            str1 = str2 = (char *)TET_MALLOC(strlen(tool_file_p) + 1);
            (void) strcpy(str1,tool_file_p);

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

        /* The leaf name of the TC comes last in the list of argv */
	argv[argc] = (char *) TET_MALLOC (strlen(tc_name) + 1);
        (void) strcpy(argv[argc++], tc_name);
    }

    /* Make sure that argv is NULL terminated, and that argc is correct */
    argv[argc++] = NULL;

#ifdef DBG2
    for(tries = 0; argv[tries] != NULL; tries++)
        (void) fprintf(stderr,"argv[%d] = %s\n", tries, argv[tries]);
#endif

    /* If output capture is enabled set up a pipe from the tool to tcc */
    if (oc_set == TRUE)
    {
        /* upward: tool to tcc */
        rc = pipe(up_pipe);
        BAIL_OUT_ON(rc,"up_pipe");
    }

    /* Set up the pipe that informs the parent if the exec call failed */
    rc = pipe(exec_pipe);
    BAIL_OUT_ON(rc,"exec_pipe");

    (void) unlink(TMP_RES_FILE); /* get rid of the old tet_xres file */

    /* Export the tet environment */
    rc = export(tet_env);
        ERROR_ON(rc,"Failure in export().\n");

    /* this #if is to allow debugging in the child up to the point of exec */
#ifdef ONE_PROC
    if ((cpid = 0) == 0)
#else
    /* Have up five goes at getting a successful exec before giving up */
    tries = 0;
    while((cpid = fork()) == FAILURE)
    {
        tries++;
        if(tries >= 5)
            break;
        (void) sleep(5);
    }
    if (cpid == 0)
#endif
    {
#ifdef DBG2
        (void) fprintf(stderr,"About to exec %s\n", this_tool);
#endif

        /* in child */

        /* 
         * ensure the exec pipe is closed upon a sucessful execution so that
         * the parent is not left waiting to read the pipe if the IC should
         * hang for some reason
         */
        (void) fcntl(exec_pipe[PIPE_WRT_SIDE], F_SETFD, FD_CLOEXEC);

        if ( oc_set == TRUE)
        {
            /*
             *  Redirect the stdout and stderr of the tool so that go into
             *  the pipe that we set up.
             */
            ERROR_ON(close(up_pipe[PIPE_RD_SIDE]), "close up pipe / rd side\n");
            ERROR_ON(dup2(up_pipe[PIPE_WRT_SIDE],FILENO_STDOUT), "dup2 up pipe -> STDOUT\n");
            ERROR_ON(dup2(up_pipe[PIPE_WRT_SIDE],FILENO_STDERR), "dup2 up pipe -> stderr\n");
            ERROR_ON(close(up_pipe[PIPE_WRT_SIDE]), "close up pipe orig. fd.\n");
        }

        /* Execute the tool, over-writing the child process if successful */
        (void) execvp(this_tool,argv);

        perror("execvp");
        (void) sprintf(error_mesg,"Failed execv of tool %s\n", this_tool);
        ERROR(error_mesg);
        if ((exec_fp = fdopen( exec_pipe[PIPE_WRT_SIDE], "w")) != NULL)
            (void) fprintf(exec_fp,"TET_EXEC_FAILED");
        exit(EXIT_BAD_CHILD);
    }
    else
    {
        /* parent */
	for ( ; argv[argvfree] != NULL; argvfree++)
	    TET_FREE((void *) argv[argvfree]);

	TET_FREE((void *) this_tool);

        /* If fork() failed and return -1 we now bail out */
        BAIL_OUT_ON(cpid, "fork");

        /* In the event of a timeout control will return to this point and
         * timed_out will be set.
         */
        timed_out = sigsetjmp(jmpbuf,1);

        if (timed_out)
        {
#ifdef DBG2
            (void) fprintf(stderr,"Back in do_tool after time out.\n");
#endif
            (void) alarm(0);
            context_status = TET_ESTAT_TIMEOUT;
            return_val = FAILURE;
        }
        else
        {
            /*
             * If output_capture is set read info from the tool and enter
             * it into the journal file.
             */
            if ( oc_set == TRUE)
            {
                fp = fdopen(up_pipe[PIPE_RD_SIDE],"r");
                if (fp == NULL)
                    BAIL_OUT("fdopen up_pipe[PIPE_RD_SIDE]");

                (void) close(up_pipe[PIPE_WRT_SIDE]);

                while ( fgets(linein,sizeof(linein),fp) != NULL )
                    (void) jnl_entry_captured(exec_ctr,linein);

                (void) fclose(fp);
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
                    /* Note this jnl_tool_end is here coz of the BAIL_OUT */
                    if( tmode == BUILD)
                    {
                        jnl_tool_end(exec_ctr++,TET_ESTAT_EXEC_FAILED,"Build End, Exec() of build tool", BUILD);
                        BAIL_OUT2("Build tool failure.\n");
                    }
                    else
                    {
                        jnl_tool_end(exec_ctr++,TET_ESTAT_EXEC_FAILED,"Clean End, Exec() of clean tool", CLEAN);
                        BAIL_OUT2("Clean tool failure.\n");
                    }
                }
            }
            (void) fclose(exec_fp);

            /* Wait for the child to complete */
            while((rc = waitpid((pid_t)cpid,&statloc,0)) == FAILURE)
            {
                /* waitpid recieved a signal */
                if(errno == EINTR)
                    continue;

                ERROR("waitpid child\n");
                return(FAILURE);
            }
            /* If the child recieved a signal */
            if(WIFSIGNALED(statloc))
            {
                waitstatus = WTERMSIG(statloc);

                /* if user interrupt */
                if((waitstatus == SIGTERM && interrupted == 2) ||
                        (waitstatus == SIGINT && interrupted == 1))
                    user_interrupt = TRUE;

                /* Sort out the status for the jnl_tool_end entry */
                context_status = waitstatus + 1000;
            }
            else
            {
                /* Sort out the status for the jnl_tool_end entry */
                context_status = WEXITSTATUS(statloc);
            }

            /* make sure user interrupt flag is reset */
            interrupted = 0;

            /* No more Build/Clean Tool and no more timeout */
            if( cpid != 0)
            {
                cpid = 0;
                (void) alarm(0);
            }

            /*
             * If output capture wasn't enabled then get the results 
             * from the results file.
             */
            if ( oc_set == FALSE)
            {
                res_fp = fopen(results_file,"r");
                if (res_fp == NULL) 
                {
                    (void) fprintf(stderr,"resfile open err = %d\n", errno);
                    (void) sprintf(error_mesg,"do_tool(): can't open results file (%s)\n", results_file);
                    ERROR(error_mesg);
                }
                else
                {
                    /* A return 1 here indicates the build tool failed */
                    tp_end_status = copy_results_file(res_fp);
                    (void) fclose(res_fp);
                }
            }

            if ( (tp_end_status == 1) || (WIFEXITED(statloc) == 0) || 
                                                 (WEXITSTATUS(statloc) != 0))
            {
                if(((bft_p = get_tetenv("TET_BUILD_FAIL_TOOL")) != NULL)
                                                        && tmode == BUILD)
                {
                    /*
                     * The build tool executed but failed - so see if we can 
                     * execute a build fail tool.
                     */

                    /* No interruptions whilst invoking the Build Fail Tool */
                    on_int.sa_handler = SIG_IGN;
                    on_int.sa_flags = 0;
                    if (sigaction(SIGINT, &on_int, &old_int) == -1)
		    {
			old_int.sa_handler = siginthdlr;
			old_int.sa_flags = 0;
		    }
                    /* 
                     * Set up the pipe that informs the parent if the exec 
                     * call failed. 
                     */
                    rc = pipe(exec_pipe);
                    BAIL_OUT_ON(rc,"exec_pipe");

                    /* Give fork several chances to work */
                    tries = 0;
                    while ((bft_pid = fork()) == -1 && tries <= 5)
                    {
                        (void) sleep(5);
                        tries++;
                    }
                    if (bft_pid == 0)
                    {  /* child */
#ifdef DBG2
                        (void) fprintf(stderr,"About to exec %s\n", bft_p);
#endif
                        if (execlp(bft_p,bft_p, tc_name, NULL) == FAILURE)
                        {
                            (void) perror("execlp");
                            if ((exec_fp = fdopen( exec_pipe[PIPE_WRT_SIDE], "w")) != NULL)
                                (void) fprintf(exec_fp,"TET_EXEC_FAILED");
                            exit(EXIT_BAD_CHILD);
                        }
                    }
                    else
                    {
                        BAIL_OUT_ON(bft_pid,"Failure of fork() for build fail tool.\n");
                        /* Check to see if exec() call failed */
                        exec_fp = fdopen(exec_pipe[PIPE_RD_SIDE], "r");
                        if (exec_fp == NULL)
                            BAIL_OUT("fdopen exec_pipe[PIPE_RD_SIDE]");
                        (void) close (exec_pipe[PIPE_WRT_SIDE]);
                        while(fgets(linein, sizeof(linein), exec_fp) != NULL)
                        {
                            if( !strcmp(linein, "TET_EXEC_FAILED"))
                            {
                                /* Note this jnl_tool_end is here coz of the BAIL_OUT */
                                jnl_tool_end(exec_ctr++,TET_ESTAT_EXEC_FAILED,"Build End, Exec() call of build fail tool failed.", tmode);
                                BAIL_OUT2("Tool failure.\n");
                            }
                        }
                        (void) fclose(exec_fp);

                        /* Wait for the child to complete */
                        rc = waitpid((pid_t)bft_pid,&statloc2,0);
                        if(rc == FAILURE || WIFEXITED(statloc2) == 0 || 
                                                    WEXITSTATUS(statloc2) != 0)
                                return_val = FAILURE;
                        else
                                return_val = SUCCESS;
                    }

                    /* Reset SIGINT handling */
                    (void) sigaction(SIGINT,&old_int,(struct sigaction *)NULL);
                }
                else
                {
                    return_val = FAILURE;
                }
            }
        }
    }

    line[0] = '\0';
    if(context_status >= 1000) /* ie signalled */
    {
        (void) sprintf(line,", Tool terminated by signal %d", waitstatus);
        if( user_interrupt == TRUE)
            (void) strcat(line," (User Interrupt)");
    }

    return(return_val);
}


/*
 *  Obtain an execlusive lock in the source directory, and also in the
 *  alternative execution directory if need be.
 */
int obtain_tool_lock(tc_dir)
char *tc_dir;
{
    int rc, time_left = g_timeout, mesg_flag = 0;
    char pd[_POSIX_PATH_MAX];

#ifdef DBG
    (void) fprintf(stderr,"obtain_tool_lock()\n");
#endif

    if (getcwd(pd,sizeof(pd)) == NULL)
    {
        perror("getcwd current dir");
        BAIL_OUT("getcwd");
    }
    where_lock = (char *) TET_MALLOC(strlen(pd) + 2);
    (void) strcpy( where_lock, pd);

    /* We try to obtain all the necessary locks and we don't get a particular
     * lock then we wait for WAIT_INTERVAL seconds before trying again. If
     * we reach the specified time out limit then any locks that have been
     * gained so far as freed and we return a FAILURE 
     */
    time_left += WAIT_INTERVAL; /* the first try is free of charge */

    while(time_left >= WAIT_INTERVAL)
    {
        rc = open(LOCK_FILE,O_CREAT|O_EXCL,
                            (S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH));
        if (rc == FAILURE)
        {
            if (errno == EEXIST || errno == EISDIR)
            {
                if((time_left == g_timeout+WAIT_INTERVAL) && (g_timeout != 0))
                    (void) fprintf(stderr,"Lock encountered, retrying until timeout.. \n");
                if(g_timeout != 0)
		{
                    (void) sleep(WAIT_INTERVAL);
		}
		else
		{
			/* no timeout specified but lock still exists */
			ERROR("Lock encountered in src dir, test case abandoned.\n");
		}
                time_left -= WAIT_INTERVAL;
            }
            else
            {
                ERROR("creat of lock file in src dir\n");
                return(FAILURE);
            }
        }
        else 
        {
            (void) close(rc);
            if(time_left != g_timeout + WAIT_INTERVAL) /* ie been waiting */
                (void) fprintf(stderr,"Lock released,  continuing.\n");
            if(alt_exec_set)
            {
                if(access(alt_exec_dir,F_OK) == -1)
                {
                    ERROR("Cannot access alt_exec_dir.\n");
                    (void) unlink(LOCK_FILE);
                    time_left = 0;
                    break;
                }
                alt_exec_lock = (char *) TET_MALLOC(strlen(alt_exec_dir)+3+strlen(LOCK_FILE)+strlen(tc_dir));
                (void) strcpy(alt_exec_lock, alt_exec_dir);

                if (alt_exec_lock[strlen(alt_exec_dir)-1] != DIR_SEP_CHAR)  
                    (void) strcat(alt_exec_lock,DIR_SEP_STR);
                (void) strcat(alt_exec_lock,tc_dir);

                if (alt_exec_lock[strlen(alt_exec_dir)-1] != DIR_SEP_CHAR)  
                    (void) strcat(alt_exec_lock,DIR_SEP_STR);
                (void) strcat(alt_exec_lock,LOCK_FILE);

                while(time_left >= WAIT_INTERVAL)
                {
                    rc = open(alt_exec_lock,O_CREAT|O_EXCL,
                            (S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH));
                    if (rc == FAILURE)
                    {
                        if (errno == EEXIST || errno == EISDIR)
                        {
                            if(time_left == g_timeout + WAIT_INTERVAL) 
                            {
                                /* ie first time */
                                (void) fprintf(stderr,"Lock encountered, retrying until timeout.. \n");
                                mesg_flag = 1;
                            }
                            if(g_timeout != 0)
			    {
                                (void) sleep(WAIT_INTERVAL);
			    }
			    else
			    {
				/* no timeout specified but lock still exists */
				ERROR("Lock encountered in exec dir, test case abandoned.\n");
		  	    }
                            time_left -= WAIT_INTERVAL;
                        }
                        else
                        {
                            ERROR("creat of lock file in exec dir\n");
                    	    (void) unlink(LOCK_FILE);
                            return(FAILURE);
                        }
                    }
                    else
                    {
                        (void) close(rc);
                        if( mesg_flag) 
                            /* ie been waiting */
                            (void) fprintf(stderr,"  lock released,  continuing.\n");
                        break;
                    }
                }
                if(time_left < WAIT_INTERVAL)
                    (void) unlink(LOCK_FILE);
            }
            break;
        }
    }
    if(time_left < WAIT_INTERVAL)
    {
        if( g_timeout != 0)
            ERROR("Timeout on obtaining lock.\n");
        return(FAILURE);
    }

    /* This variable is used in the tet_cleanup() function */
    lock_type = 3;

    return(SUCCESS);
}


/*
 *  Release the locks. If there is an alternate exe dir the one in there is
 *  freed first. Then the lock in source directory is freed 
 *  NOTE: No BAIL_OUTS are permitted in this function !
 */
void release_tool_lock()
{
    int rc;

#ifdef DBG
    (void) fprintf(stderr,"release_tool_lock()\n");
#endif

    (void) chdir(where_lock);
    TET_FREE((void *) where_lock);

    if(alt_exec_set)
    {
        if(alt_exec_lock != NULL)
        {
            rc = unlink(alt_exec_lock);
            ERROR_ON(rc,"unlock of alt_exec_dir lock file\n");
            TET_FREE((void *)alt_exec_lock);
            alt_exec_lock = NULL;
        }
    }
    rc = unlink(LOCK_FILE);
    ERROR_ON(rc,"unlock of src dir lock file\n");

    /* No locks in place now */
    lock_type = 0;
}


/*  
 *  Obtain an exclusive lock in the current directory. If at first you don't
 *  succeed keep trying every WAIT_INTERVAL seconds until you time out.
 */
int obtain_exe_excl_lock()
{
    int rc, time_left = g_timeout;
    char pd[_POSIX_PATH_MAX];

#ifdef DBG
    (void) fprintf(stderr,"obtain_exe_excl_lock()\n");
#endif

    if (getcwd(pd,sizeof(pd)) == NULL)
    {
        perror("get current dir");
        BAIL_OUT("getcwd");
    }
    where_lock = (char *) TET_MALLOC(strlen(pd) + 2);
    (void) strcpy(where_lock, pd);

    time_left += WAIT_INTERVAL; /* the first try is free of charge */

    while(time_left >= WAIT_INTERVAL)
    {
        rc = open(LOCK_FILE,O_CREAT|O_EXCL,
                            (S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH));
        if (rc == FAILURE)
        {
            if (errno == EEXIST || errno == EISDIR)
            {
                if((time_left == g_timeout+WAIT_INTERVAL) && (g_timeout != 0))
                    (void) fprintf(stderr,"Lock encountered, retrying until timeout.. \n");
                if(g_timeout != 0)
		{
                    (void) sleep(WAIT_INTERVAL);
		}
		else
		{
			/* no timeout specified but lock still exists */
			ERROR("Lock encountered, test case abandoned.\n");
		}
                time_left -= WAIT_INTERVAL;
            }
            else
            {
                ERROR("creat of exclusive lock file in exe dir\n");
                return(FAILURE);
            }
        }
        else 
        {
            (void) close(rc);
            if(time_left != g_timeout + WAIT_INTERVAL) /* ie been waiting */
                (void) fprintf(stderr, "Lock released,  continuing.\n");
            break;
        }
    }
    if(time_left < WAIT_INTERVAL)
    {
        if( g_timeout != 0)
            ERROR("Timeout on obtaining lock.\n");
        return(FAILURE);
    }

    /* This variable is used in the tet_cleanup() function */
    lock_type = 2;

    return(SUCCESS);
}



/*
 *  Remove the lock file in the current directory.
 *  NOTE: No BAIL_OUTS are permitted in this function !
 */
void release_exe_excl_lock()
{
    int rc;

#ifdef DBG
    (void) fprintf(stderr,"release_exe_excl_lock()\n");
#endif

    (void) chdir(where_lock);
    TET_FREE((void *) where_lock);

    rc = unlink(LOCK_FILE);
    ERROR_ON(rc,"unlock of exe dir exclusive lock file\n");
    lock_type = 0;
}


/*
 *  Obtain a shared lock for the duration of the recursive copy when
 *  TET_EXEC_IN_PLACE is false.
 *  Firstly create a lock_file dir and then create a uniquie file in the
 *  dir. Once again if we're unsuccessful first go we keep trying every
 *  WAIT_INTERVAL seconds until we time out.
 */
int obtain_exe_shrd_lock()
{
    int rc, pid, ch, len, num;
    int time_left = g_timeout;
    char pd[_POSIX_PATH_MAX];

#ifdef DBG
    (void) fprintf(stderr,"obtain_exe_shrd_lock()\n");
#endif

    if (getcwd(pd,sizeof(pd)) == NULL)
    {
        perror("get current dir");
        BAIL_OUT("getcwd");
    }
    where_lock = (char *) TET_MALLOC(strlen(pd) + 2);
    (void) strcpy(where_lock, pd);

    time_left += WAIT_INTERVAL; /* the first try is free of charge */

    while(time_left >= WAIT_INTERVAL)
    {
        rc = mkdir(LOCK_FILE,(S_IRWXU|S_IRWXG|S_IRWXO));
        if (rc == FAILURE)
        {
            if (errno == EEXIST)
            {
                if(chdir(LOCK_FILE) == 0)
                    break;
                if((time_left == g_timeout+WAIT_INTERVAL) && (g_timeout != 0))
                    (void) fprintf(stderr,"Lock encountered, retrying until timeout.. \n");
                if(g_timeout != 0)
		{
                    (void) sleep(WAIT_INTERVAL);
		}
		else
		{
		    /* Lock exists but no timeout specified */
		    ERROR("Lock encountered, test case abandoned\n");
		}

                time_left -= WAIT_INTERVAL;
            }
            else
            {
                ERROR("creat of lock dir in exe dir\n");
                return(FAILURE);
            }
        }
        else 
        {
            (void) chdir(LOCK_FILE);
            if(time_left != g_timeout + WAIT_INTERVAL) /* ie been waiting */
                (void) fprintf(stderr,"Lock released,  continuing.\n");
            break;
        }
    }
    if(time_left < WAIT_INTERVAL)
    {
        if( g_timeout != 0)
             ERROR("Timeout on obtaining lock.\n");
        return(FAILURE);
    }

    /* generate a unique name file under tet_lock/tmp */
    temp_file =  (char *)TET_MALLOC(8);
    pid = (getpid() % 100000);
    ch = 'a';
    (void) sprintf(temp_file, "%d%c", pid, ch);
    len = strlen(temp_file);
    for (num = 0; num < 5; num++)
    {
        errno = 0;
        rc = open(temp_file, O_CREAT|O_EXCL, 
                            (S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH));
        if (rc == FAILURE && errno == EEXIST)
        {
            ch++;
            temp_file[len - 1] = ch;
            continue;    /* Try again if file already exists */
        }
        else
            break;    /* Succeeded, or failed for another reason */

    }
    if (rc == FAILURE)
    {
        (void) ERROR("Unable to create unique file.\n");
        (void) chdir("..");
        (void)rmdir(LOCK_FILE);
        return(FAILURE);
    }
    else
        (void) close( rc);

    /* This variable is used in the tet_cleanup() function */
    lock_type = 1;

    return(SUCCESS);
}

/*
 *  Remove the shared lock files. We don't worry if we can't remove the
 *  shared lock dir as this should mean that there is another shared lock
 *  in place.
 *  NOTE: No BAIL_OUTS are permitted in this function !
 */
void release_exe_shrd_lock()
{
    int rc;

#ifdef DBG
    (void) fprintf(stderr,"release_exe_shrd_lock()\n");
#endif

    (void) chdir(where_lock);
    TET_FREE((void *) where_lock);

    (void) chdir(LOCK_FILE);
    rc = unlink(temp_file);
    ERROR_ON(rc,"unlink of unique file in shared lock dir\n");
    if(temp_file != NULL)
        TET_FREE((void *) temp_file);
    (void) chdir("..");
    /* try to remove lock dir - don't worry if we can't */
    (void)rmdir(LOCK_FILE);
    lock_type = 0;
}
