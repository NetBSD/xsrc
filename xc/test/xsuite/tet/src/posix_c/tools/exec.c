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

SCCS:           @(#)exec.c    1.9 03/09/92
NAME:           exec.c
PRODUCT:        TET (Test Environment Toolkit)
AUTHOR:         OSF Validation & SQA
DATE CREATED:   14 May 1991
CONTENTS:

MODIFICATIONS:

                "TET Rework"
                David G. Sawyer, UniSoft Ltd,  July 1991.

                do_save(): changed str2 to str1 in strrchr() call in loop that
                             removes empty dirs - now works.
                David G. Sawyer, UniSoft Ltd,  31 Oct 1991.

                do_rm(): BAIL_OUT changed to ERROR.
                David G. Sawyer, UniSoft Ltd,  26 Nov 1991.

************************************************************************/

#include <tcc_env.h>
#include <tcc_mac.h>
#include <tet_jrnl.h>
#include <tcc_prot.h>


/*
 *  Create a directory with permissions = (S_IRWXU|S_IRWXG|S_IRWXO)
 */
int do_mkdir(path_p)
char *path_p;
{
    char *pn;
    int rc;

#ifdef DBG
    (void) fprintf(stderr,"do_mkdir(%s)\n", path_p);
#endif

    if (path_p[strlen(path_p)-1] == DIR_SEP_CHAR)
    {
        pn = (char *)TET_MALLOC(strlen(path_p) + 1);
        (void) strcpy(pn,path_p);
        pn[strlen(path_p)-1] = '\0';
        rc = mkdir(pn,(S_IRWXU|S_IRWXG|S_IRWXO));
        if((rc == FAILURE) && (errno != EEXIST))
        {
            COMPLAIN("mkdir failed");
            return(FAILURE);
        }
        TET_FREE((void *)pn);
    }
    else
    {
        rc = mkdir(path_p,(S_IRWXU|S_IRWXG|S_IRWXO));
        if ( (rc == FAILURE) && (errno != EEXIST))
        {
            COMPLAIN("mkdir failed");
            return(FAILURE);
        }
    }
    return(SUCCESS);
}

/*
 *  If EXEC_IN_PLACE is flase then we do a recursive copy of the normal
 *  execution directory to the temp directory
 */
int do_copy(object_path)
char object_path[];
{
    int rc, stat_loc, tries = 0;
    char *new_dir, *freenew_dir;

#ifdef DBG
    (void) fprintf(stderr,"do_copy(%s)\n", object_path);
#endif

    /* get the relative name of the dir we want to cp */
    if (object_path[strlen(object_path)-1] == DIR_SEP_CHAR)
        object_path[strlen(object_path)-1] = '\0';
    new_dir = (char *) TET_MALLOC(strlen(object_path)); /* overkill ! */
    freenew_dir = new_dir;
    new_dir = strrchr(object_path, DIR_SEP_CHAR);
    new_dir = &(new_dir[1]); /* don't want the first "/" */

#ifdef DBG
    (void) fprintf(stderr,"dir to be copied is %s\n", new_dir);
#endif

    /* need to obtain a shared lock for the duration of the copy */
    if( obtain_exe_shrd_lock() == FAILURE)
    {
        jnl_tc_end(exec_ctr++,TET_ESTAT_LOCK,"TC End");
        return(FAILURE);
    }

    rc = chdir(object_path); /* go back to where we were */
    BAIL_OUT_ON(rc,"changing to object_path.\n");

    /* something odd happens when you do a recursive copy of a full
       path name - hence the change up directory and relative path */
    (void) chdir("..");

#ifdef DBG2
    (void) fprintf(stderr,"** Copying **\n");
#endif

    /* Give fork several chances to work */
    while ((cpid = fork()) == -1 && tries <= 5)
    {
        tries++;
        (void) sleep(5);
    }
    if (cpid == 0)
    {
        rc = execlp("cp","cp","-r",new_dir,temp_dir,NULL);
        if (rc == FAILURE)
        {
            perror("execlp");
            ERROR("error in cp exec (obj dir->tmp dir)\n");
            exit(EXIT_BAD_CHILD);
        }
        exit(EXIT_OK);
    }
    else
    {
        BAIL_OUT_ON(cpid,"Failure of fork() for exec out of place file copying.\n");

        /* Wait for the execed child to complete */
        (void) waitpid((pid_t)cpid,&stat_loc,0);

        cpid = 0;

        /* Check to see that the child completed successfully */
        if ( (WIFEXITED(stat_loc) == 0) || (WEXITSTATUS(stat_loc) == EXIT_BAD_CHILD) )
        {
            (void) sprintf(error_mesg,"cp IFEXITED %d exit status %d\n",WIFEXITED(stat_loc), WEXITSTATUS(stat_loc)); 
            ERROR(error_mesg);
            return(FAILURE);
        }
        else
        {
            /* free shared lock */
            (void) chdir(new_dir);
            (void) release_exe_shrd_lock();

            /* change dir to the newly created dir */
#ifdef DBG
    (void) fprintf(stderr,"About to change dir to %s/%s\n",temp_dir, new_dir);
#endif
            rc = chdir(temp_dir);
            BAIL_OUT_ON(rc,"changing to temp_dir.\n");
            rc = chdir(new_dir);
            BAIL_OUT_ON(rc,"changing to tmp dir created for EXEC_OUT_OF_PLACE");
        }
    }

    TET_FREE((void *) freenew_dir);

    return(SUCCESS);
}


/*
 *  Use a find and cp exec() call to save any files in the comma seperated list,
 *  TET_SAVE_FILES, that are in the execution directory hierarchy; and to do
 *  a recursive copy of them to the results directory
 */
void do_save(path_p)
char * path_p;
{
    int rc, stat_loc, cmd_ctr, ctr = 0;
    char **find_command, *str1, *str2, *str3, *new_dir, *rm_dir;

#ifdef DBG
    (void) fprintf(stderr,"do_save(%s)\n", path_p);
#endif

    /* if there are no save file patterns */
    if ( save_files == NULL)
        return;

    for (cmd_ctr = 0; save_files[cmd_ctr] != NULL; cmd_ctr++);

    /* 3 * the number of names ( -name <name> -o) plus all the other bits */
    find_command = (char **) TET_MALLOC((size_t) (sizeof(char *)*(cmd_ctr*3 + 12)));
    /* new_dir will be the directory that we want to copy the files to */
    new_dir = (char *) TET_MALLOC(strlen(results_dir) + strlen(path_p) + 2);
    (void) strcpy(new_dir, results_dir);

    /* Get a copy of the path_p so that we can play with it */
    str1 = str2 = str3 = (char *) TET_MALLOC(strlen(path_p) + 1);
    /* miss the first DIR_SEP_CHAR */
    (void) strcpy(str1, &path_p[1]);
    str2 = strrchr(str1,DIR_SEP_CHAR);
    if(str2 != NULL)
    {
        /* Strip the TC name off the end, but leave the DIR_SEP_CHAR */
        *(str2+1) = '\0';
        str2 = strtok(str1,DIR_SEP_STR);
        while(str2 != NULL)
        {
            (void) sprintf(new_dir,"%s/%s", new_dir, str2);
#ifdef DBG2
            (void) fprintf(stderr,"About to create %s\n", new_dir);
#endif
            /* Create any needed dirs to complete the path */
            /* They may already exist ... */
            (void) mkdir(new_dir,(S_IRWXU|S_IRWXG|S_IRWXO));
            str2 = strtok((char *) NULL, DIR_SEP_STR);
        }
    }

    if(str3 != NULL)
        TET_FREE((void *) str3);

    /*
     * We now build up the command we wish to exec(). It will take the form
     * of: `find . (-name expresion -o -name expression ...) -exec 
     *      cp -r {} results_dir;`
     */
    cmd_ctr = 0;
    find_command[cmd_ctr++] = "find";
    find_command[cmd_ctr++] = ".";
    find_command[cmd_ctr++] = "(";

    do
    {
        find_command[cmd_ctr++] = "-name";
        find_command[cmd_ctr++] = save_files[ctr++];
        if( save_files[ctr] != NULL)
            find_command[cmd_ctr++] = "-o";

    }
    while ( save_files[ctr] != NULL);

    find_command[cmd_ctr++] = ")";
    find_command[cmd_ctr++] = "-exec";
    find_command[cmd_ctr++] = "cp";
    find_command[cmd_ctr++] = "-r";
    find_command[cmd_ctr++] = "{}";
    find_command[cmd_ctr++] = new_dir;
    find_command[cmd_ctr++] = ";";
    find_command[cmd_ctr++] = NULL;

#ifdef DBG2
    {
    int i;
    (void) fprintf(stderr,"About to: ");
    for( i = 0; find_command[i] != NULL; i++)
        (void) fprintf(stderr,"%s ", find_command[i]);
    (void) fprintf(stderr,"\n");
    }
#endif

    /* Now we fork and then exec() the find command in the child */
    if ((cpid = fork()) == 0)
    {
        rc = execvp("find",find_command);
        if (rc == FAILURE)
        {
            perror("execvp");
            (void) ERROR("error in find/cp exec\n");
            exit(EXIT_BAD_CHILD);
        }
        exit(EXIT_OK);
    }
    else
    {
        /* Wait to see what happened to the child */
        (void) waitpid((pid_t)cpid,&stat_loc,0);

        cpid = 0;

        /* Check to see that the child exited correctly */
        if ((WIFEXITED(stat_loc) == 0) || (WEXITSTATUS(stat_loc) == EXIT_BAD_CHILD) )
        {
            (void) sprintf(error_mesg,"find IFEXITED %d exit status %d\n",WIFEXITED(stat_loc), WEXITSTATUS(stat_loc)); 
            ERROR(error_mesg);
        }
    }

    /* Now we try and remove any empty directories */

    /* rm_dir will be the backward recursive removal directory !?! */
    rm_dir = (char *) TET_MALLOC(strlen(results_dir) + strlen(path_p) + 2);
    (void) strcpy(rm_dir, results_dir);

    /* Get a copy of the path_p so that we can play with it */
    str1 = (char *) TET_MALLOC(strlen(path_p) + 1);
    /* miss the first DIR_SEP_CHAR */
    (void) strcpy(str1, &path_p[1]);

    /* get rid of the final field ( test case exe name ) */
    str2 = strrchr(str1,DIR_SEP_CHAR);

    while(str2 != NULL)
    {
        *(str2) = '\0';
        (void) sprintf(rm_dir,"%s/%s", results_dir, str1);
#ifdef DBG2
        (void) fprintf(stderr,"About to try and remove %s\n", rm_dir);
#endif
        (void) rmdir(rm_dir);
        str2 = strrchr(str1,DIR_SEP_CHAR);
    }

    if( str1 != (char *) NULL)
        TET_FREE((void *) str1);

    return;
}


/*
 *  Do a recursive remove of the unique temporary directory, and then attempt
 *  to remove the directory that holds the unique temp dirs 
 *  NOTE: No BAIL_OUTS are permitted in this function !
 */
void do_rm(dir_to_rm)
char * dir_to_rm;
{
    int rc, stat_loc;

#ifdef DBG
    (void) fprintf(stderr,"do_rm(%s)\n",dir_to_rm);
#endif

    /* Make sure we are not in a directory that we are about to remove */
    rc = chdir(tet_root);
    if(rc == FAILURE)
    {
        (void) sprintf(error_mesg,"Couldn't chdir() to tet_root.\n%s not removed.\n", dir_to_rm);
        ERROR(error_mesg);
    }
    else
    {
        /* delete all the stuff in the dir */
        if ((cpid = fork()) == 0)
        {
            rc = execlp("rm","rm","-rf",dir_to_rm,NULL);
            if (rc == FAILURE)
            {
                perror("execlp of rm");
                (void) ERROR("error in rm dir exec\n");
                exit(EXIT_BAD_CHILD);
            }
            exit(EXIT_OK);
        }
        else
        {
            (void) waitpid((pid_t)cpid,&stat_loc,0);

            cpid = 0;

            if ( (WIFEXITED(stat_loc) == 0) || (WEXITSTATUS(stat_loc) == EXIT_BAD_CHILD) )
            {
                (void) sprintf(error_mesg,"rm IFEXITED %d exit status %d\n",WIFEXITED(stat_loc), WEXITSTATUS(stat_loc)); 
                ERROR(error_mesg);
            }
        }
    }
}
