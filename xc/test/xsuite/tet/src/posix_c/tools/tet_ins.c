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
static char sccsid[] = "@(#)TET_INS	1.9 03/09/92";
#endif

/************************************************************************

SCCS:           @(#)tet_ins.c	1.9 03/09/92
NAME:           tet_ins.c
PRODUCT:        TET (Test Environment Toolkit)
AUTHOR:         OSF Validation & SQA
DATE CREATED:   14 May 1991
CONTENTS:

MODIFICATIONS:

		Made the code lint-able.
		Put in checks for the setting of TET_ROOT.
		David G. Sawyer, UniSoft Ltd, July 1991

		Don't complain if tet_code doesn't exist (but
		still check its format if it does).
		Geoff Clare, UniSoft Ltd., 13 Nov 1991

************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>    /* strcpy, strcat */
#include <sys/wait.h>  /* waitpid */
#include <unistd.h>    /* access  */
#include <errno.h>     /* errno   */
#include <ctype.h>     /* isspace */
#include <tcc_mac.h>


#define MAXLEN 512
#define SCE_NAME_LEN 31

void tet_shutdown();
void print_error();
char *determine_name();

int main(argc, argv)
int argc;
char *argv[];
{

	char install_name[MAXLEN];
	char suite_name[MAXLEN];
	pid_t child;
	char *getenv();
	char *line_p, *tet_root = NULL;
	char config_exe_file[MAXLEN];  
	char config_build_file[MAXLEN];  
	char config_clean_file[MAXLEN];  
	char scen_file[MAXLEN];  
	char rescodes_file[MAXLEN];
	int  wstatus, ret_code, wait_code, error_ctr;

	if (argc != 2) {
		(void) fprintf(stderr, "Usage: tet_ins <suite name>\n");
		exit(1);
		/* NOTREACHED */
	}

	error_ctr = 0;

#ifdef TET_ROOT
    /* initialize 'tet_root' from defined value */
    tet_root = (char *)malloc(strlen(TET_ROOT)+2);
	if(tet_root == NULL)
	{
		(void) fprintf(stderr, "Failed malloc() for tet_root.\n");
		exit(-2);
		/* NOTREACHED */
	}
    (void) strcpy(tet_root,TET_ROOT);
#endif

    /* 
	 *  initialize 'tet_root' from environment var $TET_ROOT
     *  (this takes precedence over any compile-time value)
	 */
    if ((line_p = getenv("TET_ROOT")) != NULL)
    {
        if (tet_root == (char *) NULL)
		{
            tet_root = (char *)malloc(strlen(line_p)+1);
			if(tet_root == NULL)
			{
				(void) fprintf(stderr, "Failed malloc() for tet_root.\n");
				exit(-2);
				/* NOTREACHED */
			}
		}
        else
		{
            tet_root = (char *)realloc((void *)tet_root,strlen(line_p)+1);
			if(tet_root == NULL)
			{
				(void) fprintf(stderr, "Failed malloc() for tet_root.\n");
				exit(-2);
				/* NOTREACHED */
			}
		}

        (void) strcpy(tet_root,line_p);
    }

    /*
     * Stop if tet_root has not been defined from either source
     */
    if (tet_root == (char *) NULL)
    {
        (void) fprintf(stderr, "No TET_ROOT directory defined\n"); 
        exit(-2);
		/* NOTREACHED */
    }

	(void) strcat(strcat(strcpy(suite_name, tet_root), "/"), argv[1]);

	(void) strcpy(install_name, suite_name);
	(void) strcat(strcpy(config_build_file, suite_name), "/tetbuild.cfg");
	(void) strcat(strcpy(config_exe_file, suite_name), "/tetexec.cfg");
	(void) strcat(strcpy(config_clean_file, suite_name), "/tetclean.cfg");
	(void) strcat(strcpy(scen_file, suite_name), "/tet_scen");
	(void) strcat(strcpy(rescodes_file, suite_name), "/tet_code");  

	(void) strcat(install_name, "/bin/install");
	if (access(install_name, X_OK)) {
		(void) fprintf(stderr, "File %s could not be executed.\n", install_name);
		if (errno = EACCES)
			(void) fprintf(stderr, "Check the permissions of the executable.\n");
		else
			(void) fprintf(stderr, 
			"Check whether TET_ROOT is set in the environment\n");
		(void) fflush(stderr);
		exit(2);
	}
	if ((child = fork()) == (pid_t)0) {
		(void) fprintf(stdout, "\n\tInstall tool file = %s\n\n", install_name);
		(void) fflush(stdout);
		ret_code = execlp(install_name, install_name, NULL);
		if(ret_code == FAILURE)
			BAIL_OUT("execlp failed\n");
	}
	else {
		wait_code = waitpid(child, &wstatus, 0);
		BAIL_OUT_ON(wait_code, "waitpid child");
		if (check_config(config_build_file)) {
			(void) fprintf(stderr, "\t**Error in build configuration file**\n\n");
			(void) fflush(stderr);
			error_ctr++;
		}
		if (check_config(config_exe_file)) {
			(void) fprintf(stderr, 
			"\t**Error in execution configuration file**\n\n");
			(void) fflush(stderr);
			error_ctr++;
		}
		if (check_config(config_clean_file)) {
			(void) fprintf(stderr, "\t**Error in clean configuration file**\n\n");
			(void) fflush(stderr);
			error_ctr++;
		}

		if (check_sce(scen_file)) {
			(void) fprintf(stderr, "\t**Error in scenario file**\n\n");
			(void) fflush(stderr);
			error_ctr++;
		}

		if (check_rescode(rescodes_file)) {
			(void) fprintf(stderr, "\t**Error in suite result codes file**\n\n");
			(void) fflush(stderr);
			error_ctr++;
		}

		if (error_ctr)
			exit(3);
		else
			exit(0);
	}
	/*NOTREACHED*/
}


int getline(s, fp)
char s[];
FILE *fp;
{
	int c, i;

	i = 0;
	while ((c = getc(fp)) != '\n' && c != EOF)
		s[i++] = c;

	if (c == '\n') s[i++] = c;
	s[i] = '\0';
	return(i);

}

int check_config(file)
char file[];
{
	FILE *fp;
	char tmp[MAXLEN];
	int line_ctr;
	int err_ctr;
	int i;

	if ((fp = fopen(file, "r")) == NULL) {
		(void) fprintf(stderr, "File %s could not be opened for reading\n", file);
		(void) fflush(stderr);
		return(1);
	}
	line_ctr = err_ctr = 0;
	while (getline(tmp, fp) > 0) {
		line_ctr++;
		if (tmp[0] == '#')
			;
		else {
			for (i = 0; (size_t)i < strlen(tmp) && isspace(tmp[i]); i++) ;
			if (i != strlen(tmp)) {
				if (strchr(tmp, '=') == (char *)NULL ||
				    isspace(tmp[strlen(tmp) - strlen(strchr(tmp, '=')) - 1]) ||
				    isspace(tmp[strlen(tmp) - strlen(strchr(tmp, '=')) + 1])) {
					print_error(file, line_ctr, "Incorrect format", tmp);
					err_ctr++;
				}
			}
		}
	}
	if (err_ctr)
		return(1);
	else
		return(0);
}

check_sce(file)
char file[];
{
	FILE *fp;
	int line_ctr;
	int err_ctr;
	char tmp[MAXLEN];
	int in_block;
	int all_var;
	int i;
	char *name;

	if ((fp = fopen(file, "r")) == NULL) {
		(void) fprintf(stderr, "File %s could not be opened for reading\n", file);
		(void) fflush(stderr);
		return(1);
	}
	line_ctr = err_ctr = in_block = all_var = 0;
	while (getline(tmp, fp) > 0) {
		line_ctr++;
		for (i = 0; (size_t)i < strlen(tmp) && isspace(tmp[i]); i++) ;
		if (i == strlen(tmp))
			continue;
		if (tmp[0] == '#')
			;
		else {
			if (!in_block) {
				if (isspace(tmp[0])) {
					print_error(file, line_ctr, "Bad line format. Not in data block",
					tmp);
					err_ctr++;
				}
				else { 
					if ((name = determine_name(tmp)) == (char *)NULL) {
						print_error(file, line_ctr, "Bad format outside data block", tmp);
						err_ctr++;
					}
					else if (!strncmp(name, "all", 3)) {
						all_var++;
						in_block++;
					}
					else
						in_block++;
				}
			}
			else {
				if (isspace(tmp[0])) {
					if (determine_type(tmp)) {
						print_error(file, line_ctr, "Bad format in data block", tmp);
						err_ctr++;
					}
				}
				else {
					if ((name = determine_name(tmp)) == (char *)NULL) {
						print_error(file, line_ctr, "Bad format in data block", tmp);
						err_ctr++;
					}
					else if (!strncmp(name, "all", 3))
						all_var++;
				}
			}
		}
	}
	if (!all_var) {
		(void) fprintf(stderr, "Scenario file %s does not include an \"all\" scenario\n",
		file);
		(void) fflush(stderr);
		return(3);
	}
	else if (err_ctr)
		return(2);
	else
		return(0);
}

char *determine_name(line)
char *line;
{
	int i;
	int j;
	char tmp[MAXLEN];

	for (i = 0; (size_t)i < strlen(line) && isalnum(line[i]); i++) ;
	if (i == (strlen(line) - 1))
		return(line);
	else {
		if (!isspace(line[i]))
			return((char *)NULL);
		else {
			for (j = i; (size_t)j < strlen(line) && isspace(line[j]); j++) ;
			if (j == strlen(line))
				return(line);
			else {
				for (i = 0; (size_t)i < (strlen(line) - (size_t)j); i++)
					tmp[i] = line[j + i];
				switch(line[j]) {
				case '/':
					if (!bad_pathname(tmp))
						return(line);
					break;
				case '"':
					if (!bad_quotation(tmp))
						return(line);
					break;
				case ':':
					if (!bad_include(tmp))
						return(line);
					else if (!bad_iteration(tmp))
						return(line);
					else if (!bad_group(tmp))
						return(line);
					break;
				default:
					return((char*)NULL);
					/* NOTREACHED */
					break;
				}
				return((char *)NULL);
			}
		}
	}
}

int bad_pathname(line)
char *line;
{
	int i;
	int prev_slash;
	int ic_list;
	int j;
	char tmp[MAXLEN];
	int prev_comma;

	prev_slash = ic_list = 0;
	if (line[0] != '/')
		return(1);
	else {
		for (i = 1; (size_t)i < strlen(line); i++) {
			if (isspace(line[i])) {
				for (j = i; (size_t)j < strlen(line) && isspace(line[j]); j++) ;
				if ((size_t)j != strlen(line))
					return(1);
			}
			else {
				if (!ic_list) {
					if (prev_slash) {
						if (!isalnum(line[i]) && (line[i] != '_') && (line[i] != '.'))
							return(1);
						else
							prev_slash = 0;
					}
					else {
						if (line[i] == '/')
							prev_slash++;
						else if (line[i] == '{') {
							for (j = 0; (size_t)j <= (strlen(line) - i); j++)
								tmp[j] = line[i + j];
							ic_list++;
						}
						else if (!isalnum(line[i]) && (line[i] != '_') && (line[i] != '.'))
							return(1);
					}
				}
				else {
					prev_comma = 0;
					for (j = 1; (size_t)j < strlen(tmp); j++) {
						if (prev_comma) {
							if (!isalnum(tmp[j]) && (tmp[j] != '_') && (tmp[j] != '.'))
								return(1);
							else
								prev_comma = 0;
						}
						else {
							if (tmp[j] == '}') {
								if (j != (strlen(tmp) - 2)) {
									j++;
									for (; (size_t)j < strlen(tmp) && isspace(tmp[j]); j++) ;
									if ((size_t)j != strlen(tmp))
										return(1);
									else
										return(0);
								}
								else
									return(0);
							}
							else if (tmp[j] == ',') {
								if (j == 1)
									return(1);
								else
									prev_comma++;
							}
							else if (!isalnum(tmp[j]) && (tmp[j] != '_') && (tmp[j] != '.'))
								return(1);
						}
					}
				}
			}
		}
		return(0);
	}
}

int bad_quotation(line)
char *line;
{
	int i;
	int j;

	if (line[0] != '"')
		return(1);
	else {
		for (i = 1; (size_t)i < strlen(line) && (line[i] != '"'); i++);
		if ((size_t)i == strlen(line))
			return(1);
		else {
			for (j = (i + 1); (size_t)j < strlen(line) && isspace(line[j]); j++);
			if ((size_t)j != strlen(line))
				return(1);
		}
	}
	return(0);
}

int bad_include(line)
char *line;
{
	int i;
	char tmp[MAXLEN];

	if (strncmp(line, ":include:", 9))
		return(1);
	else {
		for (i = 9; (size_t)i <= strlen(line); i++)
			tmp[i - 9] = line[i];
		if (!bad_pathname(tmp))
			return(0);
		else
			return(1);
	}
}

int bad_iteration(line)
char *line;
{
	int i;
	char tmp[MAXLEN];
	char tmp2[MAXLEN];
	int j;

	if (strncmp(line, ":repeat,", 8))
		return(1);
	else {
		for (i = 8; (size_t)i <= strlen(line); i++)
			tmp[i - 8] = line[i];
		for (i = 0; i < (int) strlen(tmp) && isdigit(tmp[i]); i++) ;
		if (i == 0 || (size_t)i == strlen(tmp))
			return(1);
		else if (tmp[i] != ':')
			return(1);
		else {
			i++;
			for (j = 0; (size_t)i < strlen(tmp); j++, i++)
				tmp2[j] = tmp[i];
			if (bad_pathname(tmp2))
				return(1);
			else
				return(0);
		}
	}
}

int bad_group(line)
char *line;
{
	int i;
	char tmp[MAXLEN];
	int j;

	if (strncmp(line, ":group", 6))
		return(1);
	else if (line[6] == ',') {
		for (i = 7; i < (int) strlen(line) && isdigit(line[i]); i++) ;
		if ((size_t)i == (strlen(line) - 1) || i == 7)
			return(1);
		else {
			if (line[i] != ':')
				return(1);
			else {
				i++;
				for (j = 0; (size_t)i < strlen(line); i++, j++)
					tmp[j] = line[i];
				if (bad_pathname(tmp))
					return(1);
				else
					return(0);
			}
		}
	}
	else if (line[6] == ':') {
		for (j = 0; (size_t)j <= (strlen(line) - 7); j++)
			tmp[j] = line[j + 7];
		if (bad_pathname(tmp))
			return(1);
		else
			return(0);
	}
	else
		return(1);
}

int determine_type(line)
char *line;
{
	int i;
	char tmp[MAXLEN];
	int j;

	for (i = 0; (size_t)i < strlen(line) && isspace(line[i]); i++) ;
	for (j = 0; (size_t)i <= strlen(line); i++, j++)
		tmp[j] = line[i];
	if (!bad_pathname(tmp))
		return(0);
	else if (!bad_quotation(tmp))
		return(0);
	else if (!bad_include(tmp))
		return(0);
	else if (!bad_iteration(tmp))
		return(0);
	else if (!bad_group(tmp))
		return(0);
	else
		return(1);
}

int check_rescode(file)
char file[];
{
	FILE *fp;
	char tmp[MAXLEN];
	int line_ctr;
	int err_ctr;
	int i;
	int j;
	int code;
	char str1[MAXLEN];
	char str2[MAXLEN];

	if ((fp = fopen(file, "r")) == NULL) {
		/* If it doesn't exist, don't complain */
		if (errno == ENOENT)
			return 0;

		(void) fprintf(stderr, "File %s could not be opened for reading\n", file);
		(void) fflush(stderr);
		return(1);
	}
	line_ctr = err_ctr = 0;
	while (getline(tmp, fp) > 0) {
		line_ctr++;
		if (tmp[0] == '#')
			;
		else {
			for (j = 0; j < (int) strlen(tmp) && isspace(tmp[j]); j++) ;
			if ((size_t)j != strlen(tmp)) {
				if (((i = sscanf(tmp, "%d%s%s", &code, str1, str2)) != 2 && i != 3) || 
				    (str1[0] != '"') || (str1[strlen(str1) - 1] != '"')) {
					print_error(file, line_ctr, "Incorrect format", tmp);
					err_ctr++; 
				}
				else {
					if (i == 3 && (strcmp(str2, "Continue") && strcmp(str2, "Abort"))) {
						print_error(file, line_ctr, "Incorrect Action field", tmp);
						err_ctr++;
					}
				}
			}
		}
		code = -1;
		(void) strcpy(str1, "");
		(void) strcpy(str2, "");
	}
	if (err_ctr)
		return(1);
	else
		return(0);
}

void print_error(file, d, msg, line)
char *file;
int d;
char *msg;
char *line;
{
	(void) fprintf(stderr, "File: %s Line: %d %s\n", file, d, msg);
	(void) fprintf(stderr, "Line = %s", line);
	(void) fflush(stderr);
}


/*
 *	fatal error routine
 */
void tet_shutdown()
{
	/* if we're FATAL close down and exit */
	exit(-1);
}
