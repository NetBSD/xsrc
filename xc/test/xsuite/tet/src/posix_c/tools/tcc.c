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

#ifndef lint
static char sccsid[] = "@(#)TCC 1.9 03/09/92";
#endif

/************************************************************************

SCCS:           @(#)tcc.c    1.9 03/09/92
NAME:           tcc.c
PRODUCT:        TET (Test Environment Toolkit)
AUTHOR:         OSF Validation & SQA
DATE CREATED:   14 May 1991
CONTENTS:

MODIFICATIONS:    
                Changed Phoenix References to TET / 14May91 / jbg

                "TET Rework"
                David G. Sawyer, UniSoft Ltd, July 1991

                Revised show_synopsis().
                Geoff Clare, UniSoft Ltd, 22 Oct 1991

                Added line to explicitly set the env to exe_env before the
                processing of TET_SAVE_FILES.
                David G. Sawyer, UniSoft Ltd, 6 Nov 1991

                Added code to check the validity of tet_root
                David G. Sawyer, UniSoft Ltd, 25 Nov 1991

		Rename getopt() to avoid clash on systems which have it
                Geoff Clare, Unisoft Ltd., 19 Dec 1991

		Error message buffer enlarged to cater for huge error
		messages.
		David G. Sawyer, UniSoft Ltd, 27 Jan 1992

		tc_search replaced by resume_status
		David G. Sawyer, UniSoft Ltd, 13 Feb 1992


		Rewrote tet_malloc() to take advantage of DBGMEM option.
		Added tet_realloc() and tet_free() to support DBGMEM.
		David G. Sawyer, UniSoft Ltd, 13-19 Feb 1992

		Only catch SIGHUP, SIGINT and SIGQUIT if inherited as
		SIG_DFL, so tcc can be run in background and under nohup.
		Geoff Clare, UniSoft Ltd, 4 Mar 1992

		    *** TET 1.9 for the MIT X Test suite ***
			Modified to allow compilation on systems with
		a poor understanding of the 'void' type. Made declarations
		of tet_realloc and tet_free arguement 'ptr' dependant
		on __STDC__ because some old compilers can't grok void *
		arguments properly.

			Stuart Boutell, UniSoft Ltd, 12 March 1992

************************************************************************/

#include <tcc_env.h>
#include <tcc_mac.h>
#include <tet_jrnl.h>
#include <tcc_prot.h>

/* Is the journal open */
bool jnl_open_flag = FALSE;

/* Different modes of operation */
bool build_mode = FALSE, exec_mode = FALSE, clean_mode = FALSE;

/* Options to the modes of operation */
bool resume_opt = FALSE, rerun_opt = FALSE;

int resume_status = 0;	      /* Resume option starts of non-active */
int jnl_fd = 0;               /* The journal file descriptor */
int context = 0;              /* The process that made an entry in the jnl */
int debug_suite = FALSE;      /* Has the -d option been used */
int lock_type = 0;            /* The type of lock cuurently used, 0 = None */
int cpid = 0;                 /* Child process id */
int g_timeout = 0;            /* Timeout in seconds, set using -t option */
int interrupted = 0;          /* Has the user generated an interrupt */
int *result_list = NULL;      /* The list of result code nums to look for */
int produce_output = FALSE;   /* Do you want to the tcc doing something */
int alt_exec_set = FALSE;     /* Has the alt_exec_dir been set */
int initialising = TRUE;      /* Used in tet_shutdown() to decide exit status */
int context_status;           /* Used in the tool and execute functions */
int oc_set = FALSE;           /* Is output capture set */
int eip_set = FALSE;          /* Is exec in place set */
int result_mode[3] = {0,0,0}; /* Modes specified in the result code pattern */
int abort_requested = FALSE;  /* Set if a result code requests an Abort */
int scenario_line = 0;        /* line number, used in mode start messages */
int old_line = 0;             /* the scenario line number to resume from */
int scenario_element = 0;     /* element number, used in mode start messages */
int old_element = 0;          /* the scenario element number to resume from */

char *result_pat = NULL;        /* result code pattern, for -m,-r options */
char *jnl_file_name = NULL;     /* journal file name */
char *bld_cfg_file_name = NULL; /* build configuration file */
char *cln_cfg_file_name = NULL; /* clean configuration file */
char *exe_cfg_file_name = NULL; /* execution configuration file */
char *scen_file_name = NULL;    /* scenario file name */
char *suite_name = NULL;        /* actual test suite name */
char *scenario_name = NULL;     /* the scene to look for */
char **yes_list = NULL;         /* Text to match before exeing a scen line */
char **no_list = NULL;          /* Text to match to not exec a scen line */
char **scen_lines = NULL;       /* Explicit scenario lines to be execed */
char **save_files = NULL;       /* Any files we want saving afterwards */

char error_mesg[BIG_BUFFER];    /* Error message buffer */

#ifdef DBGMEM
MEMCHECK *memtable = NULL;      /* Memory allocation checking table */
#endif

ENV_T * tet_env = NULL;         /* the current TET environment */
ENV_T * bld_env = NULL;         /* the build mode environment */
ENV_T * exe_env = NULL;         /* the exe mode environment */
ENV_T * cln_env = NULL;         /* the clean environment */

FILE * old_jnl_fp;              /* old journal file pointer */
struct sigaction sig;           /* signal handling structure */

char results_file[] = TMP_RES_FILE;       /* test case dir 'results' file */

char test_suite_root[_POSIX_PATH_MAX];    /* pretty obvious - I hope */
char results_dir[_POSIX_PATH_MAX];        /* results directory */
char alt_exec_dir[_POSIX_PATH_MAX];       /* alternate execution directory */
char tet_root[_POSIX_PATH_MAX];           /* for $TET_ROOT */
char temp_dir[_POSIX_PATH_MAX];           /* temp execution dir */
char old_journal[_POSIX_PATH_MAX];        /* old journal for resume/rerun  */
char start_dir[_POSIX_PATH_MAX];          /* The directory we start in */

/* recognised options */
char *optstring = "a:bcdef:g:hi:j:l:m:n:pr:s:t:v:x:y:?";

/* Ensure a sensible default value */
#ifndef TET_SIG_IGNORE
#define TET_SIG_IGNORE 0
#endif

/* Ensure a sensible default value */
#ifndef TET_SIG_LEAVE
#define TET_SIG_LEAVE 0
#endif

/* The list of signals that should be ignored or have their action unchanged */
int    sig_ignore[] = { 0, TET_SIG_IGNORE};
int    sig_leave[]  = { 0, TET_SIG_LEAVE};


/*
 *  TCC main routine, a "-?" flag option gives you the correct synopsis
 */
int main(argc,argv)
int argc;
char *argv[];
{
    int ch, ctr, pid, len, num, rc, sig_set, pat_len = 0;
    int exit_val = EXIT_OK, max_res = 0, user_results_dir = FALSE;
    char msg[256], tmp_dir[_POSIX_PATH_MAX];
    char *line_p, *eq_p, *jnl_dir, *tmp_ptr, *cp;
    char *save_pat_p, *pres_sep, *next_sep;

    /* check to see any options were supplied */
    if (argc == 1)
    {
        show_synopsis();
        tet_shutdown();
    }

    /* Sort out the signal handling */
    for( num = 1; num < NSIG; num++)
    {
        sig_set = 0;
        if (num == SIGKILL || num == SIGSTOP || num == SIGCHLD)
            continue;

        for(ctr = 0; (sizeof(sig_leave[0])*ctr) <= sizeof(sig_leave); ctr++)
            if(sig_leave[ctr] == num)
            {
                sig_set = 1;
                break;
            }

        if(sig_set)
            continue;

        for(ctr = 0; (sizeof(sig_ignore[0])*ctr) <= sizeof(sig_ignore); ctr++)
            if(sig_ignore[ctr] == num)
            {
                sig.sa_handler = SIG_IGN;
                sig_set = 1;
                break;
            }


        if( ! sig_set)
	{
	    if (num == SIGHUP || num == SIGINT || num == SIGQUIT)
	    {
		struct sigaction osig;
		if (sigaction(num, (struct sigaction *)NULL, &osig) != -1)
		{
		    if (osig.sa_handler == SIG_IGN)
		    {
			sig.sa_handler = SIG_IGN;
			sig_set = 1;
		    }
		}
	    }
	    if( ! sig_set)
	    {
		if (num == SIGINT) 
		{	
		    sig.sa_handler = siginthdlr;
		} else {
		    sig.sa_handler = sighdlr;
		}
	    }
	}

        sig.sa_flags = 0;
        (void) sigaction(num, &sig, (struct sigaction *)NULL);
    }
    
    /* so that the umask does not effect the creation of files etc */
    (void) umask(000); 

    /* sort out the dir we start off in */
    if( getcwd(start_dir, sizeof(start_dir)) == (char *) NULL)
    {
        perror("getcwd");
        tet_shutdown();
    }

#ifdef TET_ROOT
    /* initialize 'tet_root' from defined value */
    (void) strcpy(tet_root,TET_ROOT);
#endif

    /* 
     *  initialize 'tet_root' from environment var $TET_ROOT
     *  (this takes precedence over any compile-time value)
     */
    if ((line_p = getenv("TET_ROOT")) != NULL)
        (void) strcpy(tet_root,line_p);

    /*
     * Stop if tet_root has not been defined from either source
     */
    if (tet_root[0] == '\0')
    {
        (void) fprintf(stderr, "No TET_ROOT directory defined\n"); 
        tet_shutdown();
    }

    /*
     * Check the accessability of the tet_root...
     */
    if (chdir(tet_root) == -1)
    {
        BAIL_OUT("tet_root check failed ");
    }
    else
    {
        /* get back to where we were */
        BAIL_OUT_ON(chdir(start_dir),"changing back to starting directory.\n");
    }
#ifdef DBG2
    (void) fprintf(stderr,"tet_root = %s\n", tet_root);
#endif

    /* initialize 'temp_dir' from environment var $TET_TMP_DIR */
    if ((line_p = getenv("TET_TMP_DIR")) != NULL)
        (void) strcpy(temp_dir,line_p);

    /* initialize 'alt_exec_dir' from environment var $TET_EXECUTE */
    line_p = getenv("TET_EXECUTE");
    if ( ! ((line_p == NULL) || (*line_p == '\0')))
        (void) strcpy(alt_exec_dir,line_p);

    /*
     *  If a tet tmp directory was not specified in the environment then 
     *  create a directory under the tet root called "tet_tmp_dir".
     */
    if(temp_dir[0] == '\0')
    {
        /* The temporary directory will be created under tet_root. */
        (void) sprintf(temp_dir,"%s/%s", tet_root, "tet_tmp_dir");
    }
    if (access(temp_dir,F_OK) != 0)
    {
        if (do_mkdir(temp_dir) == FAILURE)
        {
            (void) fprintf(stderr, "Cannot make TET tmp directory\n"); 
            tet_shutdown();
        }
    }

    /*
     * generate a unique name sub-directory under the TET
     * temporary directory named $$c, where $$ is process id, and
     * 'c' is a single letter (if directory with 'a' exists,
     * a retry with 'b' is done and so on).
     */
    pid = (getpid() % 100000);
    ch = 'a';
    (void) sprintf(temp_dir, "%s/%d%c", temp_dir, pid, ch);
    len = strlen(temp_dir);
    for (num = 0; num < 10; num++)
    {
        errno = 0;
        rc = do_mkdir(temp_dir);
        if (rc == FAILURE && errno == EEXIST)
        {
            ch++;
            temp_dir[len - 1] = ch;
            continue;    /* Try again if dir already exists */
        }
        else
            break;    /* Succeeded, or failed for another reason */

    }
    if (rc == FAILURE)
    {
        (void) fprintf(stderr,"Cannot make tet tmp sub-directory\n"); 
        tet_shutdown();
    }

    /* 
     *  Initialise the mode environments.
     */
    bld_env = (ENV_T *) TET_MALLOC2((size_t)(ENV_BIGGER*sizeof(ENV_T)));
    for(ctr = 0; ctr < ENV_BIGGER; ctr++)
        bld_env[ctr].name = NULL;

    exe_env = (ENV_T *) TET_MALLOC2((size_t)(ENV_BIGGER*sizeof(ENV_T)));
    for(ctr = 0; ctr < ENV_BIGGER; ctr++)
        exe_env[ctr].name = NULL;

    cln_env = (ENV_T *) TET_MALLOC2((size_t)(ENV_BIGGER*sizeof(ENV_T)));
    for(ctr = 0; ctr < ENV_BIGGER; ctr++)
        cln_env[ctr].name = NULL;

    /*
     *  process all command line options.
     *  options used to date:
     *                        a,b,c,d,e,f,g,h,i,j,l,m,n,p,r,s,t,v,x,y,?
     */
    while (( ch = optget(argc,argv,optstring) ) != EOF)
    {
        switch ((char)ch)
        {
            /* The alternate execution directory */
        case 'a':
            /*
             * When the alt_exec_dir is specified by this option we 
             * must make sure it is stored as a full path.
             */
            if(optarg[0] != DIR_SEP_CHAR)
                (void) sprintf(alt_exec_dir,"%s/%s",start_dir,optarg);
            else
                (void) strcpy(alt_exec_dir,optarg);
            break;

            /* Build mode */
        case 'b':
            build_mode = TRUE;
            break;

            /* Clean mode */
        case 'c':
            clean_mode = TRUE;
            break;

            /* Enable debugging break points */
        case 'd':
            debug_suite = TRUE;
            break;

            /* Exec mode */
        case 'e':
            exec_mode = TRUE;
            break;

            /* Specific clean config file */
        case 'f':
            cln_cfg_file_name = (char *)TET_MALLOC(strlen(optarg) + 1);
            (void) strcpy(cln_cfg_file_name,optarg);
            break;

            /* Specific build config file */
        case 'g':
            bld_cfg_file_name = (char *)TET_MALLOC(strlen(optarg) + 1);
            (void) strcpy(bld_cfg_file_name,optarg);
            break;

            /* Intermediate Results Dir. (working dir.) */
        case 'i':
            (void) strcpy(results_dir,optarg);
            user_results_dir = TRUE;
            break;

            /* journal file name */
        case 'j':
            jnl_file_name = (char *)TET_MALLOC(strlen(optarg)+1);
            (void) strcpy(jnl_file_name,optarg);
            break;

            /* use this specific scenario line */
        case 'l':
            if( scen_lines == NULL)
            {
                scen_lines = (char **) TET_MALLOC((size_t)(sizeof(char *)*10));
                for(num = 0; num < 10; num++)
                    scen_lines[num] = NULL;
            }
            for(num = 0; scen_lines[num] != NULL; num++);
            if((num % 10) == 9)
            {
                scen_lines = (char **) TET_REALLOC((void *)scen_lines, (size_t)(sizeof(char *)*(num+11)));
                for(ctr = num; ctr < (num + 11); ctr++)
                    scen_lines[ctr] = NULL;
            }
            scen_lines[num] = (char *) TET_MALLOC(strlen(optarg)+1);
            (void) strcpy(scen_lines[num],optarg);
            break;

            /*  Resume option, record the result codes to look for and
             *  note that we need to look for a specific test case before
             *  processing the rest of the scenario file.
             */
        case 'm':
            resume_opt = TRUE;
            resume_status = 1; 					/* active */
            result_pat = (char *)TET_MALLOC2(strlen(optarg)+1);
            (void) strcpy(result_pat,optarg);
            break;

            /* If this text occurs in a scenario line don't exec it */
        case 'n':
            if( no_list == NULL)
            {
                no_list = (char **) TET_MALLOC((size_t)(sizeof(char *)*10));
                for(num = 0; num < 10; num++)
                    no_list[num] = NULL;
            }
            for(num = 0; no_list[num] != NULL; num++);
            if((num % 10) == 9)
            {
                no_list = (char **) TET_REALLOC((void *)no_list, (size_t)(sizeof(char *)*(num+11)));
                for(ctr = num; ctr < (num + 11); ctr++)
                    no_list[ctr] = NULL;
            }
            no_list[num] = (char *) TET_MALLOC(strlen(optarg)+1);
            (void) strcpy(no_list[num],optarg);
            break;

            /* Produce minimal output to give the user something to look at */
        case 'p':
            produce_output = TRUE;
            break;

            /* Rerun option, record the result codes to look for */
        case 'r':
            rerun_opt = TRUE;
            result_pat = (char *)TET_MALLOC2(strlen(optarg)+1);
            (void) strcpy(result_pat,optarg);
            break;

            /* scenario file name */
        case 's':
            scen_file_name = (char *)TET_MALLOC(strlen(optarg)+1);
            (void) strcpy(scen_file_name,optarg);
            break;

            /* timeout, in seconds */
        case 't':
            g_timeout = atoi(optarg);
            break;

            /* Define a configuration variable on the command line */
        case 'v':
            line_p = (char *)TET_MALLOC(strlen(optarg)+1);
            (void) strcpy(line_p,optarg);
            eq_p = strchr(line_p,CFG_SEP_CHAR);
            if (eq_p == NULL)
            {
                (void) fprintf(stderr,"-v option requires %c\n",CFG_SEP_CHAR);
                TET_FREE((void *)line_p);
                continue;
            }
            /* delimit variable name */
            *eq_p = '\0'; 
            if( *(eq_p+1) != '\0')
            {
                /* put the variable into each of the environments */
                tet_env = bld_env;
                add_tetenv(line_p,eq_p+1,CFG_PERM_VAL);
                tet_env = exe_env;
                add_tetenv(line_p,eq_p+1,CFG_PERM_VAL);
                tet_env = cln_env;
                add_tetenv(line_p,eq_p+1,CFG_PERM_VAL);
            }
            TET_FREE((void *)line_p);
            break;

            /* Specific execution config file */
        case 'x':
            exe_cfg_file_name = (char *)TET_MALLOC(strlen(optarg) + 1);
            (void) strcpy(exe_cfg_file_name,optarg);
            break;

            /* Only exec a scenario line if this text occurs in it */
        case 'y':
            if( yes_list == NULL)
            {
                yes_list = (char **) TET_MALLOC((size_t)(sizeof(char *)*10));
                for(num = 0; num < 10; num++)
                    yes_list[num] = NULL;
            }
            for(num = 0; yes_list[num] != NULL; num++);
            if((num % 10) == 9)
            {
                yes_list = (char **) TET_REALLOC((void *)yes_list, (size_t)(sizeof(char *)*(num+11)));
                for(ctr = num; ctr < (num + 11); ctr++)
                    yes_list[ctr] = NULL;
            }
            yes_list[num] = (char *) TET_MALLOC(strlen(optarg)+1);
            (void) strcpy(yes_list[num],optarg);
            break;

            /* print a synopsis for the TCC */
        case '?':
	case 'h':
        default:
            show_synopsis();
            tet_shutdown();
        }
    }

    /* Has the alternative execution directory been set */
    if ( alt_exec_dir[0] != '\0')
        alt_exec_set = TRUE;

#ifdef DBG2
    (void) fprintf(stderr,"alt_exec_dir = %s\n",((alt_exec_set == TRUE) ? alt_exec_dir : "NULL"));
#endif

    /* Must specify at least one mode */
    if( exec_mode == FALSE && build_mode == FALSE && clean_mode == FALSE)
    {
        (void) fprintf(stderr,"Must specify at least one mode of operation.\n");
        tet_shutdown();
    }

    /* Not allowed to have both resume and rerun enabled */
    if ( (resume_opt == TRUE) && (rerun_opt == TRUE) )
    {
        (void) fprintf(stderr,"Can't have both rerun and resume options set\n");
        tet_shutdown();
    }

    /* If we have rerun or resume option we need to know the old_jnl name */
    if ( (resume_opt == TRUE) || (rerun_opt == TRUE) )
    {
        if ( (argc - optind) < 1 )
        {
            (void) fprintf(stderr,"old journal file is a required argument.\n");
            tet_shutdown();
        }

        (void) strcpy(old_journal,argv[optind]);

        old_jnl_fp = fopen(old_journal,"r");
        if (old_jnl_fp == NULL)
        {
            (void) fprintf(stderr,"Failed to open old journal %s.\n",old_journal);
            BAIL_OUT("fopen of old journal");
        }

        /* make sure the file descriptor is closed on an exec call */
        (void) fcntl(fileno(old_jnl_fp), F_SETFD, FD_CLOEXEC);
        
        optind++;
    }

    /*
     *  Determine whether the suite_name and the scene name have been given
     *  and if not try and calculate their defaults.
     */
    if ((argc - optind) < 2)
    {
        /* If -l is null or scen_file_name has been specified */
        if( (scen_lines == NULL) || (scen_file_name != NULL))
        {
            /* default scene name */
            scenario_name = (char *)TET_MALLOC2(strlen("all")+1);
            (void) strcpy(scenario_name,"all");
        }

        if((argc - optind) < 1)
        {
            /* Don't want to mess with start_dir */
            (void) strcpy(tmp_dir, start_dir);

            /* try to establish suite_name, as per spec. */
            if(strncmp(tet_root,tmp_dir,strlen(tet_root)) == 0)
            {
                if((tmp_ptr = strchr(&tmp_dir[strlen(tet_root)+1],DIR_SEP_CHAR))
                            != NULL)
                {
                    *tmp_ptr = '\0';
                    suite_name = (char *) TET_MALLOC(
                                     strlen(&tmp_dir[strlen(tet_root)+1]) + 2);
                    (void) strcpy(suite_name, &tmp_dir[strlen(tet_root)+1]);
                }
                else
                {
                    if((tmp_dir[strlen(tet_root)+1] != '\0') && 
                        (! isspace(tmp_dir[strlen(tet_root)+1])))
                    {
                        suite_name = (char *) TET_MALLOC2(strlen(&tmp_dir[strlen(tet_root)+1]) + 2);
                        (void) strcpy(suite_name, &tmp_dir[strlen(tet_root)+1]);
                    }
                    else
                    {
                        (void) fprintf(stderr,"No test suite name supplied, and unable to establish default.\n");
                        tet_shutdown();
                    }
                }

                /* build up the test suite root */
                (void) sprintf(test_suite_root,"%s/%s", tet_root, suite_name);
            }
            else
            {
                (void) fprintf(stderr, "No test suite name supplied, and unable to establish default.\n");
                tet_shutdown();
            }

        }
        else
        {
            suite_name = (char *)TET_MALLOC(strlen(argv[optind])+1);
            /* set the test suite name */
            (void) strcpy(suite_name,argv[optind]);      

            if(suite_name[0] != DIR_SEP_CHAR)
                (void) sprintf(test_suite_root,"%s/%s", tet_root, suite_name);
            else
                (void) strcpy(test_suite_root, suite_name);
        }
    }
    else
    {
        suite_name = (char *)TET_MALLOC(strlen(argv[optind])+1);
        /* set the test suite name */
        (void) strcpy(suite_name,argv[optind]);      

        if(suite_name[0] != DIR_SEP_CHAR)
            (void) sprintf(test_suite_root,"%s/%s", tet_root, suite_name);
        else
            (void) strcpy(test_suite_root, suite_name);

        optind++;
        scenario_name = (char *)TET_MALLOC(strlen(argv[optind])+1);
        /* set the scene name */
        (void) strcpy(scenario_name,argv[optind]);      
    }

#ifdef DBG2
    (void) fprintf(stderr,"test_suite_root = %s\n", test_suite_root);
#endif

    if(scen_lines != NULL)
    {
        if((scenario_name != NULL) && (scen_file_name == NULL))
        {
            (void) fprintf(stderr,"Need to set the scenario file name if you wish to use a \nscenario with specific scenario lines.\n");
            tet_shutdown();
        }
        /* Not allowed resume or rerun enabled with specific scen lines */
        if ( (resume_opt == TRUE) || (rerun_opt == TRUE) )
        {
            (void) fprintf(stderr,"Can't have rerun or resume options set, with specific scenario lines.\n");
            tet_shutdown();
        }
    }

    /*
     *  Build the path to the 'results' dir,  adding "/"
     *  chars between tet_root, suite_name, & "results" 
     */
    if (results_dir[0] == '\0')
        (void) sprintf(results_dir,"%s/results",test_suite_root);

    /* create results dir. if necessary */
    if (do_mkdir(results_dir) == FAILURE)
    {
        (void) fprintf(stderr,"Cannot make results directory %s\n",results_dir);
        tet_shutdown();
    }

    /*
     * If the user did not supply a results directory
     * on the command line then we need to sort out the sub-directory
     * beneath the "results" directory.
     */
    if( user_results_dir == FALSE)
    {
        /* get_jnl_dir() uses results_dir */
        jnl_dir = get_jnl_dir();
        if (jnl_dir == (char *) NULL)
        {
            (void) fprintf(stderr, "Failed to obtain dir for journal file\n");
            tet_shutdown();
        }

        (void) strcpy(results_dir, jnl_dir);
    }

#ifdef DBG2
    (void ) fprintf(stderr,"results_dir = %s\n", results_dir);
#endif

    /* determine journal file name, if none given on command line */
    if (jnl_file_name == NULL)
    {
        jnl_file_name = (char *) TET_MALLOC2(strlen(results_dir) + 3 + strlen("journal"));
        (void) strcpy(jnl_file_name,results_dir);
        (void) strcat(jnl_file_name,DIR_SEP_STR);
        (void) strcat(jnl_file_name,"journal");
    }

    /* Create the journal */
    if ((jnl_fd = open(jnl_file_name,O_WRONLY|O_CREAT|O_EXCL,
               (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH))) == FAILURE)
    {
        (void) fprintf(stderr,"Error opening journal file %s\n",jnl_file_name);
        perror("open");
        tet_shutdown();
    }
    else
    {
        /* display the journal name as per Spec. - well sort of... */
        (void) printf("journal file name is: %s\n", jnl_file_name);
        (void) fflush(stdout);
        jnl_open_flag = TRUE;

        /* make sure the file descriptor is closed on an exec call */
        (void) fcntl(jnl_fd, F_SETFD, FD_CLOEXEC);
    }

    /*
     * Put a copy of the command line into the "TCC Start" journal message
     */
    (void) strcpy(msg,"TCC Start, Command Line: ");
    ctr = 0;
    while ((ctr<argc) && ((int) (strlen(msg) + strlen(argv[ctr] + 1)) < 256))
    {
        (void) strcat(msg,argv[ctr++]); 
        (void) strcat(msg," ");
    }

    /* write the TCC start message to the journal file */
    (void) jnl_entry_tcc_start(msg);

    /*
     *  Now we sort out all the configuration file names.
     */

    /* default build cfg file */
    default_cfg_file(&bld_cfg_file_name,tet_root,suite_name,"tetbuild.cfg");

    /* default exec cfg file */
    default_cfg_file(&exe_cfg_file_name,tet_root,suite_name,"tetexec.cfg");

    /* default clean cfg file */
    default_cfg_file(&cln_cfg_file_name,tet_root,suite_name,"tetclean.cfg");

    /* default scenario file */
    default_cfg_file(&scen_file_name,tet_root,suite_name,"tet_scen");

    /* 
     *  Load in the environment for each of the modes selected, this is also
     *  the point where the configuration variables get written to the jnl.
     */
    if (build_mode == TRUE)
    {
        init_config(MODE_BLD);
        do_config_file(bld_cfg_file_name,MODE_BLD);
    }

    if (exec_mode == TRUE)
    {
        init_config(MODE_EXEC);
        do_config_file(exe_cfg_file_name,MODE_EXEC);
    }

    if (clean_mode == TRUE)
    {
        init_config(MODE_CLN);
        do_config_file(cln_cfg_file_name,MODE_CLN);
    }

    /*
     * The result code file name will be the same in each env so we leave
     * tet_env set for the time being
     */

    /* Get the result codes and their actions from the  appropiate file */
    if(get_rescode_file() == FAILURE)
    {
        (void) fprintf(stderr, "Failure to successfully process TET result code file\n");
        tet_shutdown();
    }

    /* 
     *  Now that we have the result codes, convert the result pattern names
     *  into numbers.
     */
    if( result_pat != NULL)
    {
        result_list = (int *) TET_MALLOC2((size_t)(sizeof(int)*10));
        for(ctr = 0; ctr < 10; ctr++)
	    result_list[ctr] = -1;

        /* Get an array of results codes from the comma seperated list */
        for( cp = result_pat; cp != NULL; )
        {
	    ch = 0;
            /* The results code name is stored internally with quotes round
             * it so we add our own to the word we're checking.
             */
            tmp_ptr = (char *) TET_MALLOC( strlen(cp) + 5);
            tmp_ptr[0] = '"';
            (void) strcpy(&tmp_ptr[1], cp);

            if((cp = strchr(cp,',')) != NULL)
            {
                tmp_ptr[strlen(tmp_ptr) - strlen(cp)] = '\0';
                /* advance the pointer to the next position after the comma */
                cp++;
            }
            (void) strcat(tmp_ptr, "\"");

	    /* check for mode specifiers in the list */
	    if(strlen(tmp_ptr) == 3) /* ie a single letter in quotes */
	    {
		switch((int) tmp_ptr[1])
		{
			case 'b':
				result_mode[0] = 1;
				ch = 1;
				break;
			case 'e':
				result_mode[1] = 1;
				ch = 1;
				break;
			case 'c':
				result_mode[2] = 1;
				ch = 1;
				break;
			default:
				break;
		}
		if( ch == 1)
		{
			TET_FREE((void *) tmp_ptr);
			continue;
		}
	    }

            rc = rescode_name_to_num(tmp_ptr);
            if(rc == FAILURE)
            {
                (void) fprintf(stderr,"Invalid result code name: %s\n",tmp_ptr);
                tet_shutdown();
            }
            else
            {
                if((max_res % 10) == 9)
                {
                    result_list = (int *) TET_REALLOC((void *)result_list, (size_t)(sizeof(int)*(max_res+11)));
                    for(ctr = max_res; ctr < (max_res + 11); ctr++)
                        result_list[ctr] = -1;
                }
                result_list[max_res] = rc;
                ++max_res;
            }

            TET_FREE((void *) tmp_ptr);
        }
        if(max_res == 0 && result_mode[0] == 0 && result_mode[1] == 0 &&
		result_mode[2] == 0)
        {
            (void) fprintf(stderr,"Couldn't get any result codes from command line.\n");
            tet_shutdown();
        }
        else
            result_list[max_res] = -1;
    }

    /* 
     * Sort out the save files - if there are any
     * Note: tet_env is still set at this stage !
     */

    /* Make sure we using the exe env for the save files setup code */
    tet_env = exe_env;

    /* get the comma seperated list of files that should be saved */
    save_pat_p = get_tetenv("TET_SAVE_FILES");

    if(save_pat_p != NULL)
        pat_len = strlen(save_pat_p);
    if(pat_len != 0)
    {
        save_files = (char **) TET_MALLOC((size_t)(sizeof(char *) * 10));
        for(num = 0; num < 10; num++)
            save_files[num] = NULL;
        pres_sep = save_pat_p;
        ctr = 0;

        do
        {
            if ((next_sep = strchr(pres_sep, NAME_SEP_CHAR)) != NULL)
                *next_sep = '\0';
            save_files[ctr] = (char *) TET_MALLOC((size_t)(strlen(pres_sep)+1));
            save_files[ctr++] = pres_sep;

            if((ctr % 10) == 9)
            {
                save_files = (char **) TET_REALLOC((void *)save_files, (size_t)(sizeof(char *) * (ctr + 11)));
                for(num = ctr; num < (ctr + 11); num++)
                    save_files[num] = NULL;
            }
            if (next_sep != NULL)
                pres_sep = next_sep + 1;
        }
        while ( next_sep != NULL);
    }

    /* tet_env is not set until explicitly needed - helps debugging */
    tet_env = NULL;

    /* Everything is now set up ! */
    initialising = FALSE;

    /*
     *  Depending on what modes and options were selected we now call
     *  some heavy duty functions.
     */
    if (rerun_opt == TRUE)
    {
        /*
         *  do_again(RERUN) looks in the old journal file and reruns any 
         *  invocation cases that match the right results code, mode, etc..
         */
        if(do_again(RERUN) == FAILURE)
            exit_val = EXIT_BAD_MISC;
    }
    else
    {
        if (resume_opt == TRUE)
        {
            /*
             *  do_gain(RESUME) discovers the test case line (tc_line) and
             *  the invocable component number (ic_num) from which to
             *  continue processing from in the scenario file.
             */
            if(do_again(RESUME) == FAILURE)
                exit_val = EXIT_BAD_MISC;
            else
            {
                /* Process the scenario file */
                if(perform_scen() == FAILURE)
                    exit_val = EXIT_BAD_MISC;
            }
        }
        else 
        {
                if( scen_lines != NULL)
                {
                    /* Process specific scenario lines, if any */
                    for(num = 0; scen_lines[num] != NULL; num++)
                    {
                        process_line(scen_lines[num]);
			if (abort_requested == TRUE)
			    break;
                    }
                }

                /* Process the scenario file */
		if(abort_requested != TRUE)
                    if(scenario_name != NULL)
                        if(perform_scen() == FAILURE)
                            exit_val = EXIT_BAD_MISC;
        }
    }

    /* write the TCC end message to the journal file */
    (void) strcpy(msg,"TCC End");
    (void) jnl_entry_tcc_end(msg);
    (void) close(jnl_fd);

    tet_cleanup();

#ifdef DBGMEM
#ifdef DBGMEM2
    memory_table(1);
#else
    memory_table(0);
#endif
#endif

    exit(exit_val);

    /* NOTREACHED */
}


/*
 * This routine determines the name of the config. file to open.  This
 * is based on the "priority" scheme in the spec.
 *
 * The "priorities" are:
 * 1) user specified file name  - use it (the name was already stored
 *     in *var by the command arg. parser)
 * 2) config. file in alternate exec. dir. - if it exists,  use it
 * 3) config. file in test suite directory
 *
 * This routine just concerns itself with stuffing the name of the file
 * to parse in *var.
 */

void default_cfg_file(var,root,suite,name)
char **var;
char *root;
char *suite;
char *name;
{
#ifdef DBG
    (void) fprintf(stderr, "default_cfg_file(<** file_name>, %s, %s, %s)\n", root, suite, name);
#endif

    if (*var != NULL)  /* if user specified file name */
        return;

    if (alt_exec_set == TRUE)  /* if there is an alt. exec. dir. */
    {
        *var = (char *)TET_MALLOC(strlen(alt_exec_dir)+strlen(name)+4);
        (void) strcpy(*var,alt_exec_dir);

        if ((*var)[strlen(*var)-1] != DIR_SEP_CHAR)  
            (void) strcat(*var,DIR_SEP_STR);

        (void) strcat(*var,name);

        /* check to see that the file is there */
        if (access(*var,F_OK) == 0)
            return;
        else
            TET_FREE((void *)*var);
    }

    /*
       build up the name of the file in the suite dir, 
       +4 covers the null char. at end,  plus 3 potentially missing
       "dir. sep. char's." (only 2 of which are inserted here)
    */

    *var = (char *)TET_MALLOC2(strlen(root)+strlen(suite)+strlen(name)+4);
    (void) strcpy(*var,root);

    if ((*var)[strlen(*var)-1] != DIR_SEP_CHAR)  
        (void) strcat(*var,DIR_SEP_STR);
    (void) strcat(*var,suite);

    if ((*var)[strlen(*var)-1] != DIR_SEP_CHAR)
        (void) strcat(*var,DIR_SEP_STR);
    (void) strcat(*var,name);

    return;
}


/*
 *  Something has gone wrong - we now attempt to exit gracefully. 
 *  NOTE: No BAIL_OUTS are permitted in this function !
 */
void tet_shutdown()
{
#ifdef DBG
    (void) fprintf(stderr, "tet_shutdown()\n");
#endif

    tet_cleanup();
    if(initialising)
        exit(EXIT_BAD_INIT);
    else
        exit(EXIT_BAD_MISC);
}


/*
 *  Timeout during a Test Case - SIGALRM has been generated.
 */
/*ARGSUSED*/
void exec_sig_hdlr(throw_away)
int throw_away;
{
    int statloc,rc;

#ifdef DBG
    (void) fprintf(stderr, "exec_sig_hdlr(%d)\n", throw_away);
#endif

    if (cpid != 0)
    {
        (void) kill((pid_t)cpid,SIGTERM);

        rc = waitpid((pid_t)cpid,&statloc,0);
        COMPLAIN_ON(rc,"waitpid child in timeout handler ");

        cpid = 0;

        siglongjmp(jmpbuf,0);
    }

}


/*
 *  Timeout during a Tool Execution - SIGALRM has been generated.
 */
/* ARGSUSED */
void tool_sig_hdlr(throw_away)
int throw_away;
{
    int statloc;
    int rc;

#ifdef DBG
    (void) fprintf(stderr, "tool_sig_hdlr(%d)\n", throw_away);
#endif

    if (cpid != 0)
    {
         (void) kill((pid_t)cpid,SIGTERM);

         rc = waitpid((pid_t)cpid,&statloc,0);
         COMPLAIN_ON(rc,"waitpid child in timeout handler ");

         cpid = 0;

         siglongjmp(jmpbuf,0);
    }
}


/*
 *  What do we do in the event of a SIGINT - we don't panic thats what we do
 */
/* ARGSUSED */
void siginthdlr(throw_away)
int throw_away;
{
    int rc;

#ifdef DBG
    (void) fprintf(stderr, "siginthdlr(%d)\n", throw_away);
#endif

    /*
     * The interrupted flag is needed to check that a child recived a
     * SIGTERM due to the TCC and not some other source.
     */

    if(cpid != 0)
    {
        rc = kill((pid_t) cpid, SIGTERM);
        if(rc == -1 && errno == ESRCH)
            interrupted = 1;
        else
            interrupted = 2;
    }

    (void) jnl_user_abort("User Abort");
    (void) fprintf(stderr,"\n**** TCC USER INTERRUPT ****\n\n");
}

/* 
 *  What we do when we recieve any signal other than a SIGINT or SIGALRM
 */
void sighdlr(throw_away)
int throw_away;
{
#ifdef DBG
    (void) fprintf(stderr, "sighdlr(%d)\n", throw_away);
#endif

    if(cpid != 0)
        (void) kill((pid_t)cpid,SIGTERM);

    (void) sprintf(error_mesg,"**** TCC ABORT DUE TO SIGNAL %d ****\n",throw_away);
    ERROR(error_mesg);
    tet_shutdown();
}


/*
 * A function to return the basename from a pathname, being careful
 * never to return a null pointer.
 * Arguments:
 * pointer to pathname string
 * Returns:
 * pointer to 'leafname' of the string
 */
char *basename(path)
char *path;
{
    char *sp;

    if ((sp = strrchr(path, DIR_SEP_CHAR)) != (char *) NULL)
    {
        if (*(sp + 1) != '\0')
            return(++sp);
        else
            return(sp);
    }
    else
        return(path);
}


/*
 * Try and remove any lock files and any temporary files.
 *  NOTE: No BAIL_OUTS are permitted in this function !
 */
void tet_cleanup()
{
#ifdef DBG
    (void) fprintf(stderr, "tet_cleanup()\n");
#endif

    switch(lock_type)
    {
    case 0:
    default:
        /* no lock set */
        break;
    case 1:
        release_exe_shrd_lock();
        break;
    case 2:
        release_exe_excl_lock();
        break;
    case 3:
        release_tool_lock();
        break;
    }
    if(temp_dir[0] != '\0')
        do_rm(temp_dir);

    /* Get back to where we started */
    (void) chdir(start_dir);
}


/*
 * Pretty obvious from the name I hope ....
 */
void show_synopsis()
{
#ifdef DBG
    (void) fprintf(stderr, "show_synopsis()\n");
#endif

    (void) fprintf(stderr, "\n%s\n%s\n%s\n%s\n%s\n\n",
"Usage: tcc [ -b ] [ -e ] [ -c ] [ -a Alt_exec_dir ] [ -f Clean_cfg_file ]",
"    [ -g Build_cfg_file ] [ -i Int_results_dir ] [ -j Journal_file]",
"    [ -l Scenario_line ] [ -p ] [ -n Search_string ] [ -s Scenario_file]",
"    [ -t Timeout ] [ -v Variable=value ] [ -x Exec_cfg_file ]",
"    [ -y Search_string ] [ Test_suite [ Scenario ] ]");

    (void) fprintf(stderr, "%s\n\n%s\n",
"OR: tcc -m Code_list [ options ] Old_journal_file [ Test_suite [ Scenario ] ]",
"OR: tcc -r Code_list [ options ] Old_journal_file [ Test_suite [ Scenario ] ]"
	);
}


/*
 * A routine that allows more than one go at mallocing something
 */
void * tet_malloc(size_wanted, file_name, line_no, flag)
size_t size_wanted;
char * file_name;
int    line_no;
int    flag;
{
    char *mall_ptr;
    int tries = 0;

#ifdef DBGMEM
    mem_setup();
#endif

    while(tries < 5)
    {
        mall_ptr = (char *) malloc((size_t) size_wanted);
        if(mall_ptr != NULL)
            break;
        tries++;
        (void) sleep(2);
    }
    if(tries >= 5)
    {
        (void) fprintf(stderr,"Malloc failed. source file %s line %d\n", file_name, line_no);
        tet_shutdown();
    }

#ifdef DBGMEM
    {
        int memctr;

        for(memctr=0; memtable[memctr].line != -1; memctr++)
        {
            if (memtable[memctr].line == 0)
            {
                memtable[memctr].ptr = (long) mall_ptr;
                memtable[memctr].vol = (int) size_wanted;
                memtable[memctr].line = line_no;
		if (flag)
		{
                     (void) strcpy(memtable[memctr].file, "* ");
                     (void) strcat(memtable[memctr].file, file_name);
		}
		else
                     (void) strcpy(memtable[memctr].file, file_name);
                break;
            }
        }
#ifdef DBGMEM3
	(void) fprintf(stderr, "%-11s(%12ld %8d %20s %15d)\n", "tet_malloc",memtable[memctr].ptr, memtable[memctr].vol, memtable[memctr].file, memtable[memctr].line);
#endif
    }
#else
    tries = flag; /* silly little assignment to shut lint up */
#endif

    return((void *)mall_ptr);
}


/* ARGSUSED */
void tet_free( ptr, file_name, line_no)
#if __STDC__
void *ptr;
#else
char *ptr;
#endif
char *file_name;
int line_no;
{
#ifdef DBGMEM
    int memctr;

    for(memctr=0; memtable[memctr].line != -1; memctr++)
    {
        if (memtable[memctr].ptr == (long) ptr)
        {
#ifdef DBGMEM3
	    (void) fprintf(stderr, "%-11s(%12ld %8d %20s %15d)\n", "tet_free",memtable[memctr].ptr, memtable[memctr].vol, memtable[memctr].file, memtable[memctr].line);
#endif
            memtable[memctr].line = 0;
            memtable[memctr].ptr = 0;
            break;
        }
    }
    if (memtable[memctr].line == -1)
        (void) fprintf(stderr,"Attempt to free pointer in source file %s line %d\nthat is not in memory table\n", file_name, line_no);
    else
        free((void *) ptr);
#else
        free((void *) ptr);
#endif
}


/*
 * A routine that allows more than one go at reallocing something
 */
void * tet_realloc(ptr, size_wanted, file_name, line_no, flag)
#if __STDC__
void * ptr;
#else
char * ptr;
#endif
size_t size_wanted;
char * file_name;
int    line_no;
int    flag;
{
    char *mall_ptr;
    int tries = 0;

#ifdef DBGMEM
    mem_setup();
#endif

    while(tries < 5)
    {
        mall_ptr = (char *) realloc((void *) ptr, (size_t) size_wanted);
        if(mall_ptr != NULL)
            break;
        tries++;
        (void) sleep(2);
    }
    if(tries >= 5)
    {
        (void) fprintf(stderr,"Realloc failed. source file %s line %d\n", file_name, line_no);
        tet_shutdown();
    }

#ifdef DBGMEM
    {
        int memctr;

        for(memctr=0; memtable[memctr].line != -1; memctr++)
        {
            if (memtable[memctr].ptr == (long) ptr)
                break;
        }

        if (memtable[memctr].line == -1)
        {
            for(memctr=0; memtable[memctr].line != -1; memctr++)
            {
                if (memtable[memctr].line == 0)
                    break;
            }
        }
        memtable[memctr].ptr = (long) mall_ptr;
        memtable[memctr].vol = (int) size_wanted;
        memtable[memctr].line = line_no;
	if (flag)
	{
            (void) strcpy(memtable[memctr].file, "* ");
            (void) strcat(memtable[memctr].file, file_name);
	}
	else
            (void) strcpy(memtable[memctr].file, file_name);
#ifdef DBGMEM3
	(void) fprintf(stderr, "%-11s(%12ld %8d %20s %15d)\n", "tet_realloc",memtable[memctr].ptr, memtable[memctr].vol, memtable[memctr].file, memtable[memctr].line);
#endif
    }
#else
    tries = flag; /* silly little assignment to shut lint up */
#endif
    return((void *)mall_ptr);
}


#ifdef DBGMEM
void memory_table( flag)
int flag;
{
    int memctr, found_one = 0;

    for(memctr = 0; memtable[memctr].line != -1; memctr++)
    {
        if (memtable[memctr].line != 0)
        {
	    if ((flag == 0 && *(memtable[memctr].file) != '*') || (flag == 1))
	    {
                if (! found_one++)
                    (void) fprintf(stderr, "\n%40s\n%40s\n\n%12s %8s %20s %15s\n", "Memory Table Contents", "~~~~~~~~~~~~~~~~~~~~~", "Pointer", "Size", "Source File", "Line Number");
                (void) fprintf(stderr, "%12ld %8d %20s %15d\n", memtable[memctr].ptr, memtable[memctr].vol, memtable[memctr].file, memtable[memctr].line);
	    }
        }
    }
}
#endif


#ifdef DBGMEM
void mem_setup()
{
    int memctr, newctr;

    if (memtable == NULL)
    {
        if ((memtable = (MEMCHECK *) malloc((size_t) (sizeof(MEMCHECK) * 50))) == NULL)
        {
            (void) fprintf(stderr,"Malloc failed. source file %s line %d\n", __FILE__, __LINE__);
            tet_shutdown();
        }
        for(memctr = 0; memctr < 49; memctr++)
            memtable[memctr].line = 0;
        memtable[memctr].line = -1;
    }
    else
    {
        for(memctr = 0; (memtable[memctr].line != 0) && (memtable[memctr].line != -1); memctr++);
        if (memtable[memctr].line == -1) /* full */
        {
            if ((memtable = (MEMCHECK *) realloc((void *) memtable, (size_t) (sizeof(MEMCHECK) * (50+memctr+1)))) == NULL)
            {
                 (void) fprintf(stderr,"Realloc failed. source file %s line %d\n", __FILE__, __LINE__);
                 tet_shutdown();
            }
            for(newctr = memctr; newctr < (memctr+49); newctr++)
                memtable[newctr].line = 0;
            memtable[newctr].line = -1;
        }
    }
}
#endif
