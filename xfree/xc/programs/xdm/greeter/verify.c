/* $Xorg: verify.c,v 1.3 2000/08/17 19:54:17 cpqbld Exp $ */
/*

Copyright 1988, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/greeter/verify.c,v 3.13.2.1 2001/05/25 18:50:14 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * verify.c
 *
 * typical unix verification routine.
 */

# include	"dm.h"
# include	"dm_error.h"

# include	<pwd.h>
#ifdef USE_PAM
# include	<security/pam_appl.h>
#else
# ifdef USESHADOW
#  include	<shadow.h>
#  include	<errno.h>
#  ifdef X_NOT_STDC_ENV
extern int errno;
#  endif
# endif
#endif

# include	"greet.h"

#ifdef X_NOT_STDC_ENV
char *getenv();
#endif

#ifdef QNX4
extern char *crypt(const char *, const char *);
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

#ifdef KERBEROS
#include <sys/param.h>
#include <kerberosIV/krb.h>
#include <kerberosIV/kafs.h>
static char krbtkfile[MAXPATHLEN];
#endif

static char **
userEnv (struct display *d, int useSystemPath, char *user, char *home, char *shell)
{
    char	**env;
    char	**envvar;
    char	*str;

    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    env = setEnv (env, "HOME", home);
    env = setEnv (env, "LOGNAME", user); /* POSIX, System V */
    env = setEnv (env, "USER", user);    /* BSD */
    env = setEnv (env, "PATH", useSystemPath ? d->systemPath : d->userPath);
    env = setEnv (env, "SHELL", shell);
#ifdef KERBEROS
    if (krbtkfile[0] != '\0')
        env = setEnv (env, "KRBTKFILE", krbtkfile);
#endif
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
static int pam_error;

static int PAM_conv (int num_msg,
#ifdef sun
		     struct pam_message **msg,
#else
		     const struct pam_message **msg,
#endif
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

#undef COPY_STRING
	*resp = reply;
	return PAM_SUCCESS;
}

static struct pam_conv PAM_conversation = {
	PAM_conv,
	NULL
};
#endif /* USE_PAM */

int
Verify (struct display *d, struct greet_info *greet, struct verify_info *verify)
{
	struct passwd	*p;
#ifdef USE_PAM
	pam_handle_t **pamhp = thepamhp();
#else
#ifdef USESHADOW
	struct spwd	*sp;
#endif
#endif
#ifdef __OpenBSD__
	char            *s;
	struct timeval  tp;
#endif
	char		*user_pass = NULL;
	char		*shell, *home;
	char		**argv;

	Debug ("Verify %s ...\n", greet->name);
	p = getpwnam (greet->name);
	endpwent();

	if (!p || strlen (greet->name) == 0) {
		Debug ("getpwnam() failed.\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	} else {
#ifndef USE_PAM
#ifdef linux
	    if (p->pw_passwd[0] == '!' || p->pw_passwd[0] == '*') {
		Debug ("The account is locked, no login allowed.\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	    }
#endif
#endif
	    user_pass = p->pw_passwd;
	}
#ifdef KERBEROS
	if(strcmp(greet->name, "root") != 0){
		char name[ANAME_SZ];
		char realm[REALM_SZ];
		char *q;
		int ret;
	    
		if(krb_get_lrealm(realm, 1)){
			Debug ("Can't get Kerberos realm.\n");
		} else {

		    sprintf(krbtkfile, "%s.%s", TKT_ROOT, d->name);
		    krb_set_tkt_string(krbtkfile);
		    unlink(krbtkfile);
           
		    ret = krb_verify_user(greet->name, "", realm, 
				      greet->password, 1, "rcmd");
           
		    if(ret == KSUCCESS){
			    chown(krbtkfile, p->pw_uid, p->pw_gid);
			    Debug("kerberos verify succeeded\n");
			    if (k_hasafs()) {
				    if (k_setpag() == -1)
					    LogError ("setpag() failed for %s\n",
						      greet->name);
				    
				    if((ret = k_afsklog(NULL, NULL)) != KSUCCESS)
					    LogError("Warning %s\n", 
						     krb_get_err_text(ret));
			    }
			    goto done;
		    } else if(ret != KDC_PR_UNKNOWN && ret != SKDC_CANT){
			    /* failure */
			    Debug("kerberos verify failure %d\n", ret);
			    krbtkfile[0] = '\0';
		    }
		}
	}
#endif
#ifndef USE_PAM
#ifdef USESHADOW
	errno = 0;
	sp = getspnam(greet->name);
	if (sp == NULL) {
	    Debug ("getspnam() failed, errno=%d.  Are you root?\n", errno);
	} else {
	    user_pass = sp->sp_pwdp;
	}
#ifndef QNX4
	endspent();
#endif  /* QNX4 doesn't need endspent() to end shadow passwd ops */
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
done:
#ifdef __OpenBSD__
	/*
	 * Only accept root logins if allowRootLogin resource is set
	 */
	if ((p->pw_uid == 0) && !greet->allow_root_login) {
		Debug("root logins not allowed\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
	/*
	 * Shell must be in /etc/shells 
	 */
	for (;;) {
		s = getusershell();
		if (s == NULL) {
			/* did not found the shell in /etc/shells 
			   -> failure */
			Debug("shell not in /etc/shells\n");
			bzero(greet->password, strlen(greet->password));
			endusershell();
			return 0;
		}
		if (strcmp(s, p->pw_shell) == 0) {
			/* found the shell in /etc/shells */
			endusershell();
			break;
		}
	} 
	/*
	 * Test for expired password
	 */
	if (p->pw_change || p->pw_expire)
		(void)gettimeofday(&tp, (struct timezone *)NULL);
	if (p->pw_change) {
		if (tp.tv_sec >= p->pw_change) {
			Debug("Password has expired.\n");
			bzero(greet->password, strlen(greet->password));
			return 0;
		}
	}
	if (p->pw_expire) {
		if (tp.tv_sec >= p->pw_expire) {
			Debug("account has expired.\n");
			bzero(greet->password, strlen(greet->password));
			return 0;
		} 
	}
#endif /* __OpenBSD__ */
	bzero(user_pass, strlen(user_pass)); /* in case shadow password */

#else /* USE_PAM */
#define PAM_BAIL	\
	if (pam_error != PAM_SUCCESS) { pam_end(*pamhp, 0); return 0; }

	PAM_password = greet->password;
	pam_error = pam_start("xdm", p->pw_name, &PAM_conversation, pamhp);
	PAM_BAIL;
	pam_error = pam_set_item(*pamhp, PAM_TTY, d->name);
	PAM_BAIL;
	pam_error = pam_set_item(*pamhp, PAM_RHOST, "");
	PAM_BAIL;
	pam_error = pam_authenticate(*pamhp, 0);
	PAM_BAIL;
	pam_error = pam_acct_mgmt(*pamhp, 0);
	/* really should do password changing, but it doesn't fit well */
	PAM_BAIL;
	pam_error = pam_setcred(*pamhp, 0);
	PAM_BAIL;
#undef PAM_BAIL
#endif /* USE_PAM */

	Debug ("verify succeeded\n");
	/* The password is passed to StartClient() for use by user-based
	   authorization schemes.  It is zeroed there. */
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
