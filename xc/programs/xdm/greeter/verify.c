/* $XConsortium: verify.c /main/36 1996/04/18 14:56:15 gildea $ */
/* $XFree86: xc/programs/xdm/greeter/verify.c,v 3.4.2.4 1999/07/29 09:23:06 hohndel Exp $ */
/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * verify.c
 *
 * typical unix verification routine.
 */

# include	"dm.h"
# include	<pwd.h>
#ifdef USE_PAM
# include	<security/pam_appl.h>
#else /* ! USE_PAM */
#ifdef USESHADOW
# include	<shadow.h>
#if defined(SHADOWSUITE) && defined(linux)
#define crypt pw_encrypt
#endif
# include	<errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif
#endif
#endif /* USE_PAM */

# include	"greet.h"

#ifdef X_NOT_STDC_ENV
char *getenv();
#endif

static char *envvars[] = {
    "TZ",			/* SYSV and SVR4, but never hurts */
#if defined(sony) && !defined(SYSTYPE_SYSV) && !defined(_SYSTYPE_SYSV)
    "bootdev",
    "boothowto",
    "cputype",
    "ioptype",
    "machine",
    "model",
    "CONSDEVTYPE",
    "SYS_LANGUAGE",
    "SYS_CODE",
#endif
#if (defined(SVR4) || defined(SYSV)) && defined(i386) && !defined(sun)
    "XLOCAL",
#endif
    NULL
};

static char **
userEnv (d, useSystemPath, user, home, shell)
struct display	*d;
int	useSystemPath;
char	*user, *home, *shell;
{
    char	**env;
    char	**envvar;
    char	*str;
    extern char **defaultEnv (), **setEnv ();

    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    env = setEnv (env, "HOME", home);
    env = setEnv (env, "LOGNAME", user); /* POSIX, System V */
    env = setEnv (env, "USER", user);    /* BSD */
    env = setEnv (env, "PATH", useSystemPath ? d->systemPath : d->userPath);
    env = setEnv (env, "SHELL", shell);
    for (envvar = envvars; *envvar; envvar++)
    {
	str = getenv(*envvar);
	if (str)
	    env = setEnv (env, *envvar, str);
    }
    return env;
}

#ifdef USE_PAM
static char *PAM_password;

pam_handle_t *pamh;
static int pam_error;

static int PAM_conv (int num_msg,
		     const struct pam_message **msg,
		     struct pam_response **resp,
		     void *appdata_ptr) {
	int replies = 0;
	struct pam_response *reply = NULL;

	reply = malloc(sizeof(struct pam_response));
	if (!reply) return PAM_CONV_ERR;
	#define COPY_STRING(s) (s) ? strdup(s) : NULL

	for (replies = 0; replies < num_msg; replies++) {
		switch (msg[replies]->msg_style) {
		case PAM_PROMPT_ECHO_OFF:
			/* wants password */
			reply[replies].resp_retcode = PAM_SUCCESS;
			reply[replies].resp = COPY_STRING(PAM_password);
			break;
		case PAM_TEXT_INFO:
			/* ignore the informational mesage */
			break;
		case PAM_PROMPT_ECHO_ON:
			/* user name given to PAM already */
			/* fall through */
		default:
			/* unknown or PAM_ERROR_MSG */
			free (reply);
			return PAM_CONV_ERR;
		}
	}
	*resp = reply;
	return PAM_SUCCESS;
}

static struct pam_conv PAM_conversation = {
	&PAM_conv,
	NULL
};
#endif

int
Verify (d, greet, verify)
struct display		*d;
struct greet_info	*greet;
struct verify_info	*verify;
{
	struct passwd	*p;
#ifdef USESHADOW
	struct spwd	*sp;
#endif
	char		*user_pass = NULL;
#if !defined(SVR4) || !defined(GREET_LIB) /* shared lib decls handle this */
	char		*crypt ();
	char		**systemEnv (), **parseArgs ();
#endif
	char		*shell, *home;
	char		**argv;

	Debug ("Verify %s ...\n", greet->name);
	p = getpwnam (greet->name);
	endpwent();

	if (!p || strlen (greet->name) == 0) {
		Debug ("getpwnam() failed.\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
#ifndef USESHADOW
	} else {
	    user_pass = p->pw_passwd;
#endif
	}
#ifndef USE_PAM
#ifdef USESHADOW
	errno = 0;
	sp = getspnam(greet->name);
#if !(defined(__QNX__) && !defined(__QNXNTO__))
  	endspent();
#endif  /* QNX doesn't use endspent() to end shadow passwd ops */

	if (sp) {
	    user_pass = sp->sp_pwdp;		/* local shadow file	*/
	} else {
	    user_pass = p->pw_passwd;		/* NIS expanded passwd? */
	}

	if (!user_pass) {			/* should not happen	*/
	    Debug ("getspnam() failed, errno=%d.  Are you root?\n", errno);
	    bzero(greet->password, strlen(greet->password));
	    return 0;
	}
#endif
#if defined(ultrix) || defined(__ultrix__)
	if (authenticate_user(p, greet->password, NULL) < 0)
#else
	if (strcmp (crypt (greet->password, user_pass), user_pass))
#endif
	{
		if(!greet->allow_null_passwd || strlen(p->pw_passwd) > 0) {
			Debug ("password verify failed\n");
			bzero(greet->password, strlen(greet->password));
			return 0;
		} /* else: null passwd okay */
	}

	/* If the encrypted password begins with a "!", the account
	   is locked and the user cannot login, even if they have
	   been "pre-authenticated." */

	if (p->pw_passwd[0] == '!' || p->pw_passwd[0] == '*') {
	  Debug ("The account is locked, no login allowed.\n");
	  bzero(user_pass, strlen(user_pass));	     /* in case shadow password */
	  bzero(greet->password, strlen(greet->password)); /* clear plain password    */
	  return 0;
	}

	/* The password is passed to StartClient() for use by user-based
	   authorization schemes.  It is zeroed there. */
	bzero(user_pass, strlen(user_pass)); /* in case shadow password */

#else /* USE_PAM */
	#define PAM_BAIL if (pam_error != PAM_SUCCESS) { \
	   pam_end(pamh, 0); return 0; \
	 }
	PAM_password = greet->password;
	pam_error = pam_start("xdm", p->pw_name, &PAM_conversation, &pamh);
	PAM_BAIL;
	pam_error = pam_set_item(pamh, PAM_TTY, d->name);
	PAM_BAIL;
	pam_error = pam_authenticate(pamh, 0);
	PAM_BAIL;
	pam_error = pam_acct_mgmt(pamh, 0);
	/* really should do password changing, but it doesn't fit well */
	PAM_BAIL;
	pam_error = pam_setcred(pamh, 0);
	PAM_BAIL;
#endif /* USE_PAM */
	Debug ("verify succeeded\n");

	verify->uid = p->pw_uid;
	verify->gid = p->pw_gid;
	home = p->pw_dir;
	shell = p->pw_shell;
	argv = 0;
	if (d->session)
		argv = parseArgs (argv, d->session);
	if (greet->string)
		argv = parseArgs (argv, greet->string);
	if (!argv)
		argv = parseArgs (argv, "xsession");
	verify->argv = argv;
	verify->userEnviron = userEnv (d, p->pw_uid == 0,
				       greet->name, home, shell);
	Debug ("user environment:\n");
	printEnv (verify->userEnviron);
	verify->systemEnviron = systemEnv (d, greet->name, home);
	Debug ("system environment:\n");
	printEnv (verify->systemEnviron);
	Debug ("end of environments\n");
	return 1;
}
