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

SCCS:           @(#)config.c    1.9 03/09/92
NAME:           config.c
PRODUCT:        TET (Test Environment Toolkit)
AUTHOR:         OSF Validation & SQA
DATE CREATED:    
CONTENTS:

MODIFICATIONS:  Changed Phoenix References to TET / 14May91 / jbg

                Alternative routine for non Posix function: getopt()
                Geoff Clare, Unisoft Ltd.

                "TET Rework"
                David G. Sawyer, UniSoft Ltd,  July 1991

		Fix off-by-one bug in add_tetenv()
                Geoff Clare, Unisoft Ltd., 21 Oct 1991

		Rename getopt() to avoid clash on systems which have it
                Geoff Clare, Unisoft Ltd., 19 Dec 1991

		Changed size of line buffer, added a line size check and
		changed an error message in do_config_file().
		Changed malloc to TET_MALLOC in tet_putenv().
		David G. Sawyer, UniSoft Ltd, 27 Jan 1992.

		Altered export().
		David G. Sawyer, UniSoft Ltd, 20 Feb 1992.

************************************************************************/

#include <tcc_env.h>
#include <tcc_mac.h>
#include <tet_jrnl.h>
#include <tcc_prot.h>


/* The environment pointer */
extern char ** environ;

/*  The table of "pre-initialized" configuration variables */
ENV_T CFG_INIT[] =
{
    { "TET_OUTPUT_CAPTURE", "False"},
    { "TET_RESCODES_FILE",  "tet_code"},
    { "TET_EXEC_IN_PLACE",  "False"},
    {  NULL,                 NULL    }
};


/*
 *  Initial the configuration variables in the tet env. - removing non-perm
 *  variables and making sure the hard wired ones are there.
 */
void init_config(which_mode)
int which_mode;
{
    int ctr = 0;

#ifdef DBG
    (void) fprintf(stderr,"init_config(%d)\n", which_mode);
#endif

    /* Use the correct env for the mode selected */
    switch(which_mode)
    {
    case MODE_BLD:
        tet_env = bld_env;
        break;
    case MODE_CLN:
        tet_env = cln_env;
        break;
    case MODE_EXEC:
    default:
        tet_env = exe_env;
        break;
    }

    add_tetenv("TET_VERSION", TET_VERSION, CFG_PERM_VAL);

    /* Put the hard wired configuration variables into the tet env. */
    for (ctr = 0; CFG_INIT[ctr].name != NULL; ctr++)
        add_tetenv(CFG_INIT[ctr].name,CFG_INIT[ctr].value,CFG_TEMP_VAL);
}


/*
 *  Open the specified configuration file and get the configuration variables
 *  and their values.
 */
void do_config_file(filename, mode)
char *filename;
int mode;
{
    FILE *fp;
    char *eq_p, *tmp_ptr = NULL;
    char linein[INPUT_LINE_MAX];
    int linectr = 0;

#ifdef DBG
    (void) fprintf(stderr,"do_config_file(%s, %d)\n", filename, mode);
#endif

    if((fp = fopen(filename,"r")) == (FILE *) NULL)
    {
        perror(filename);
        BAIL_OUT("file pointer in do_config_file is NULL.\n");
    }

    while (fgets(linein,sizeof(linein),fp) != NULL)
    {
	/* Check for a line that is too big */
	if(strlen(linein) == (INPUT_LINE_MAX - 1))
	{
            (void) sprintf(error_mesg,"File %s line %d. Line too long ! line begins \"%s\"\n", filename,linectr,linein);
            BAIL_OUT(error_mesg);
	}

        /* zap trailing new line */
        if (linein[strlen(linein)-1] == '\n')
            linein[strlen(linein)-1] = '\0'; 

        /* continue on blank or comment line */
        if ( (strlen(linein) == 0) || (linein[0] == CFG_CMNT_CHAR) )
            continue;

        /* if no '=' found, this is a bogus line */
        if ((eq_p = strchr(linein,CFG_SEP_CHAR)) == NULL)
        {
            (void) sprintf(error_mesg,"File %s line %d. No \"%c\", in line \"%s\"\n", filename,linectr,CFG_SEP_CHAR,linein);
            ERROR(error_mesg);
        }
        else
        {
            /* check to see that there is a text value */
            if( (*(eq_p+1) == '\0'))
                continue;
            else
            {
                if(tmp_ptr != NULL)
                    TET_FREE((void *) tmp_ptr);
                tmp_ptr = (char *) TET_MALLOC(strlen(eq_p+1) + 1);
                (void) strcpy(tmp_ptr, eq_p+1);
                if (strtok(tmp_ptr," \t") == NULL)
                    continue;
            }

            /* the line is in a local var.,  no problem with inserting \0
            to separate the var. name from the value */

            *eq_p = '\0';
            add_tetenv(linein,eq_p+1,CFG_TEMP_VAL);
        }
        linectr++;
    }

    (void) fclose(fp);

    /* dump config info to journal file */
    dump_config(filename,mode);

    if(tmp_ptr != NULL)
        TET_FREE((void *) tmp_ptr);

    return;
}


/*
 *  Look to see if the variable is already in the env. If so check if it has
 *  permanent status and if so issue a warning for trying to change it. If
 *  its not permanent change its value. If is not there then add it to the
 *  enviroment.
 */
void add_tetenv(name_p, value_p, perm)
char *name_p, *value_p;
int perm;
{
    int env_ctr = 0, tmp_ctr, which_mode;

#ifdef DBG
    (void) fprintf(stderr,"add_tetenv(%s, %s, %s)  ",name_p, value_p, (perm == 1 ? "PERM" : "TEMP"));
#endif

    if(tet_env == bld_env)
        which_mode = MODE_BLD;
    else if(tet_env == exe_env)
        which_mode = MODE_EXEC;
    else
        which_mode = MODE_CLN;

#ifdef DBG
    if(which_mode == MODE_BLD)
        (void) fprintf(stderr,"Build env\n");
    else
        (void) fprintf(stderr,"%s env\n",((which_mode == MODE_EXEC) ? "Execute" : "Clean"));
#endif

    /* Look for it in the env. */
    for (env_ctr = 0; !( (tet_env == NULL) || 
                         (tet_env[env_ctr].name == NULL)); env_ctr++)
    {
        /* replace the value of an existing entry */
        if (strcmp(tet_env[env_ctr].name,name_p) == 0)
        {
            if (tet_env[env_ctr].perm == CFG_PERM_VAL)
            {
                /* Naughty - tried to change a permanent value... */
                return;
            }
            else
            {
                tet_env[env_ctr].perm = perm;
                if (tet_env[env_ctr].value != NULL)
                    TET_FREE((void *)tet_env[env_ctr].value);

                if (value_p == NULL)
                    tet_env[env_ctr].value = NULL;
                else
                {
                    tet_env[env_ctr].value = (char *)TET_MALLOC2(strlen(value_p)+1);
                    (void) strcpy(tet_env[env_ctr].value,value_p);
                }
                return;
            }
        }
    }

    /* 
     *  check to see if the enviroment is nearly full.
     *  ... or if its exists at all.
     *  This method relies on the ENV_BIGGER constant.
     */
    if((tet_env == NULL) || ((env_ctr % ENV_BIGGER) == (ENV_BIGGER - 1)))
    {
#ifdef DBG2
        (void) fprintf(stderr,"About to expand the environment, new size = %d\n",env_ctr + ENV_BIGGER + 1);
#endif
        tet_env = (ENV_T *) TET_REALLOC((void *)tet_env,
                        (size_t)((env_ctr+ENV_BIGGER+1)*sizeof(ENV_T)));
        tmp_ctr = env_ctr;
        for( ++tmp_ctr; tmp_ctr <= (env_ctr + ENV_BIGGER); tmp_ctr++)
            tet_env[tmp_ctr].name = NULL;

        /* Re-assign the environment to the corresponding mode */
        switch(which_mode)
        {
        case MODE_BLD:
            bld_env = tet_env;
            break;
        case MODE_EXEC:
            exe_env = tet_env;
            break;
        default:
            cln_env = tet_env;
            break;
        }
    }

    /* add a new entry to the table */

    tet_env[env_ctr].perm = perm;

    tet_env[env_ctr].name  = (char *)TET_MALLOC2(strlen(name_p)+1);
    (void) strcpy(tet_env[env_ctr].name,name_p);

    if (value_p == NULL)
        tet_env[env_ctr].value = NULL;
    else
    {
        tet_env[env_ctr].value = (char *)TET_MALLOC2(strlen(value_p)+1);
        (void) strcpy(tet_env[env_ctr].value,value_p);
    }

    return;
}


/*
 *  Look for a variable in the env. If its there, return its value otherwise
 *  return NULL.
 */
char *get_tetenv(name_p)
char *name_p;
{
    char *val;
    int env_ctr;

#ifdef DBG
    (void) fprintf(stderr,"get_tetenv(%s)  ", name_p);
#endif

    for( env_ctr = 0; tet_env[env_ctr].name != NULL; env_ctr++)
    {
        if (strcmp(name_p,tet_env[env_ctr].name) == 0)
        {
            val = tet_env[env_ctr].value;
#ifdef DBG
            (void) fprintf(stderr,"value is: %s\n", val);
#endif
            return(val);
        }
    }

#ifdef DBG
    (void) fprintf(stderr,"value is: NULL\n");
#endif
    return((char *) NULL);
}


/*
 *  Write the configuration variables and their values into the journal file
 */
void dump_config(filename, mode)
char *filename;
int mode;
{
    char msg[INPUT_LINE_MAX];
    int ctr;

#ifdef DBG
    (void) fprintf(stderr,"dump_config(%s, %d)\n", filename, mode);
#endif

    (void) jnl_config_start(filename,mode);

    for (ctr = 0; tet_env[ctr].name != NULL; ctr++)
    {
            (void) sprintf(msg,"%s=%s",tet_env[ctr].name,
                (tet_env[ctr].value == NULL) ? "" : tet_env[ctr].value);

            (void) jnl_config_value(msg);
    }

    (void) jnl_config_end();
}


/* Special non Posix functions used in tcc */


int    opterr = 1;
int    optind = 1;
char   *optarg;

int optget(argc, argv, opts)
int    argc;
char   **argv, *opts;
{
    static int sp = 1;
    int c;
    char *cp;

    if (sp == 1)
    {
        if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
            return EOF;
        else if(strcmp(argv[optind], "--") == 0)
        {
            optind++;
            return EOF;
        }
    }

    c = argv[optind][sp];
    if (c == ':' || (cp=strchr(opts, c)) == NULL)
    {
        if (opterr)
            (void) fprintf(stderr, "%s: illegal option -- %c\n", argv[0], c);
        if (argv[optind][++sp] == '\0')
        {
            optind++;
            sp = 1;
        }
        return '?';
    }
    if (*++cp == ':')
    {
        if (argv[optind][sp+1] != '\0')
            optarg = &argv[optind++][sp+1];
        else if(++optind >= argc)
        {
            if (opterr)
                (void) fprintf(stderr,
                    "%s: option requires an argument -- %c\n", argv[0], c);
            sp = 1;
            return '?';
        } 
        else
            optarg = argv[optind++];
        sp = 1;
    }
    else
    {
        if (argv[optind][++sp] == '\0')
        {
            sp = 1;
            optind++;
        }
        optarg = NULL;
    }
    return c;
}


int tet_putenv(envstr)
char *envstr;
{
    /*
     * This routine mimics putenv(), and is provided purely
     * because putenv() is not in POSIX.1
     */

    char **newenv, **cur, *envname;
    int n, count = 0;
    static char **allocp = NULL;

    if (environ == NULL)
    {
        newenv = (char **)TET_MALLOC((size_t)(2*sizeof(char *)));

        newenv[0] = (char *) TET_MALLOC2((size_t) (strlen(envstr)+1));
        (void) strcpy(newenv[0] ,envstr);
        newenv[1] = NULL;
        environ = newenv;
        allocp = newenv;
        return(SUCCESS);
    }

    cur = environ;

    /* Look to substitute it in the current environment */
    while (*cur != NULL)
    {
        count++;
        envname = *cur;
        n = strcspn(envstr, "=");
        if (strncmp(envname, envstr, (size_t) n) || envname[n] != '=')
            cur++;
        else
        {
            *cur = (char *) TET_MALLOC2((size_t) (strlen(envstr)+1));
            (void) strcpy(*cur, envstr);
            return(SUCCESS);
        }
    }
    
    /*
     * If we previously allocated this environment enlarge it using
     * realloc(), otherwise allocate a new one and copy it over.
     * Note that only the last malloc()/realloc() pointer is saved, so
     * if environ has since been changed the old space will be wasted.
     */

    if (environ == allocp)
        newenv = (char **) TET_REALLOC2((void *) environ, 
                (size_t) ((count+2)*sizeof(char *)));
    else
        newenv = (char **) TET_MALLOC2((size_t) ((count+2)*sizeof(char *)));

    if (environ != allocp)
    {
        for (n = 0; environ[n] != NULL; n++)
            newenv[n] = environ[n];
        allocp = newenv;
    }
    newenv[count] = (char *) TET_MALLOC2((size_t) (strlen(envstr)+1));
    (void) strcpy(newenv[count], envstr);
    newenv[count+1] = NULL;
    environ = newenv;

    return(SUCCESS);
}


char *rescode_num_to_name(num)
int num;
{
    int ctr;
    static char no_quotes[128];

    for (ctr = 0 ; (ctr < MAX_RESCODES) ; ctr++)
        if (res_tbl[ctr].num == num)
        {
            (void) strcpy(no_quotes, res_tbl[ctr].name);
            *(strrchr(no_quotes,'"')) = '\0';;
            return(&(no_quotes[1]));
        }
    return((char *) NULL);
}
