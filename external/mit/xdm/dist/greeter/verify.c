/*

Copyright 1988, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

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

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * verify.c
 *
 * typical unix verification routine.
 */

#include	"dm.h"
#include	"dm_error.h"

#include	<pwd.h>

#if defined(USE_PAM)
# include	<stdlib.h>
#elif defined(HAVE_GETSPNAM)
# include	<shadow.h>
# include	<errno.h>
#elif defined(USE_BSDAUTH)
# include	<login_cap.h>
# include	<stdarg.h>
# include	<bsd_auth.h>
#endif

#include	"greet.h"


static const char *envvars[] = {
    "TZ",
    NULL
};

#ifdef KERBEROS
# include <sys/param.h>
# include <kerberosIV/krb.h>
/* OpenBSD 2.8 needs this. */
# if defined(OpenBSD) && (OpenBSD <= 200012)
#  include <kerberosIV/kafs.h>
# endif
static char krbtkfile[MAXPATHLEN];
#endif

static char **
userEnv (struct display *d, int useSystemPath, char *user, char *home, char *shell)
{
    char	**env;
    const char	**envvar;
    const char	*str;

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

#ifdef USE_BSDAUTH
_X_INTERNAL
int
Verify (struct display *d, struct greet_info *greet, struct verify_info *verify)
{
	struct passwd	*p;
	login_cap_t	*lc;
	auth_session_t	*as;
	char		*style, *shell, *home, *s, **argv;
	char		path[MAXPATHLEN];
	int		authok;

	/* User may have specified an authentication style. */
	if ((style = strchr(greet->name, ':')) != NULL)
		*style++ = '\0';

	Debug ("Verify %s, style %s ...\n", greet->name,
	    style ? style : "default");

	p = getpwnam (greet->name);
	endpwent();

	if (!p || strlen (greet->name) == 0) {
		Debug("getpwnam() failed.\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}

	if ((lc = login_getclass(p->pw_class)) == NULL) {
		Debug("login_getclass() failed.\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
	if ((style = login_getstyle(lc, style, "xdm")) == NULL) {
		Debug("login_getstyle() failed.\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
	if ((as = auth_open()) == NULL) {
		Debug("auth_open() failed.\n");
		login_close(lc);
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
	if (auth_setoption(as, "login", "yes") == -1) {
		Debug("auth_setoption() failed.\n");
		login_close(lc);
		bzero(greet->password, strlen(greet->password));
		return 0;
	}

	/* Set up state for no challenge, just check a response. */
	auth_setstate(as, 0);
	auth_setdata(as, "", 1);
	auth_setdata(as, greet->password, strlen(greet->password) + 1);

	/* Build path of the auth script and call it */
	snprintf(path, sizeof(path), _PATH_AUTHPROG "%s", style);
	auth_call(as, path, style, "-s", "response", greet->name,
		  lc->lc_class, (void *)NULL);
	authok = auth_getstate(as);

	if ((authok & AUTH_ALLOW) == 0) {
		Debug("password verify failed\n");
		bzero(greet->password, strlen(greet->password));
		auth_close(as);
		login_close(lc);
		return 0;
	}
	/* Run the approval script */
	if (!auth_approval(as, lc, greet->name, "auth-xdm")) {
		Debug("login not approved\n");
		bzero(greet->password, strlen(greet->password));
		auth_close(as);
		login_close(lc);
		return 0;
	}
	auth_close(as);
	login_close(lc);
	/* Check empty passwords against allowNullPasswd */
	if (!greet->allow_null_passwd && strlen(greet->password) == 0) {
		Debug("empty password not allowed\n");
		return 0;
	}
	/* Only accept root logins if allowRootLogin resource is set */
	if (p->pw_uid == 0 && !greet->allow_root_login) {
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
#else /* !USE_BSDAUTH */
_X_INTERNAL
int
Verify (struct display *d, struct greet_info *greet, struct verify_info *verify)
{
	struct passwd	*p;
# ifndef USE_PAM
#  ifdef HAVE_GETSPNAM
	struct spwd	*sp;
#  endif
	char		*user_pass = NULL;
	char		*crypted_pass = NULL;
# endif
# ifdef __OpenBSD__
	char            *s;
	struct timeval  tp;
# endif
	char		*shell, *home;
	char		**argv;

	Debug ("Verify %s ...\n", greet->name);

	p = getpwnam (greet->name);
	endpwent();

	if (!p || strlen (greet->name) == 0) {
		Debug ("getpwnam() failed.\n");
		if (greet->password != NULL)
		    bzero(greet->password, strlen(greet->password));
		return 0;
	}

	/*
	 * Only accept root logins if allowRootLogin resource is not false
	 */
	if ((p->pw_uid == 0) && !greet->allow_root_login) {
		Debug("root logins not allowed\n");
		if (greet->password != NULL)
		    bzero(greet->password, strlen(greet->password));
		return 0;
	}

# if defined(sun) && defined(SVR4)
	/* Solaris: If CONSOLE is set to /dev/console in /etc/default/login,
	   then root can only login on system console */

#  define SOLARIS_LOGIN_DEFAULTS "/etc/default/login"

	if (p->pw_uid == 0) {
	    char *console = NULL, *tmp = NULL;
	    FILE *fs;

	    if ((fs= fopen(SOLARIS_LOGIN_DEFAULTS, "r")) != NULL)
	    {
		char str[120];
		while (!feof(fs))
		{
		    fgets(str, 120, fs);
		    if(str[0] == '#' || strlen(str) < 8)
			continue;
		    if((tmp = strstr(str, "CONSOLE=")) != NULL)
			console = strdup((tmp+8));
		}
		fclose(fs);
                if ( console != NULL &&
		  (strncmp(console, "/dev/console", 12) == 0) &&
		  (strncmp(d->name,":0",2) != 0) )
		{
                        Debug("Not on system console\n");
			if (greet->password != NULL)
			    bzero(greet->password, strlen(greet->password));
			free(console);
	                return 0;
                }
		free(console);
	    }
	    else
	    {
		Debug("Could not open %s\n", SOLARIS_LOGIN_DEFAULTS);
	    }
	}
# endif

# ifndef USE_PAM /* PAM authentication happened in GreetUser already */
#  ifdef linux
	if (!strcmp(p->pw_passwd, "!") || !strcmp(p->pw_passwd, "*")) {
	    Debug ("The account is locked, no login allowed.\n");
	    bzero(greet->password, strlen(greet->password));
	    return 0;
	}
#  endif
	user_pass = p->pw_passwd;
#  ifdef KERBEROS
	if(strcmp(greet->name, "root") != 0){
		char name[ANAME_SZ];
		char realm[REALM_SZ];
		char *q;
		int ret;

		if(krb_get_lrealm(realm, 1)){
			Debug ("Can't get Kerberos realm.\n");
		} else {

		    snprintf(krbtkfile, sizeof(krbktfile), "%s.%s",
			     TKT_ROOT, d->name);
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
#  endif
#  ifdef HAVE_GETSPNAM
	errno = 0;
	sp = getspnam(greet->name);
	if (sp == NULL) {
	    Debug ("getspnam() failed: %s\n", _SysErrorMsg (errno));
	} else {
	    user_pass = sp->sp_pwdp;
	}
	endspent();
#  endif /* HAVE_GETSPNAM */
	crypted_pass = crypt (greet->password, user_pass);
	if ((crypted_pass == NULL)
	    || (strcmp (crypted_pass, user_pass)))
	{
		if(!greet->allow_null_passwd || strlen(p->pw_passwd) > 0) {
			Debug ("password verify failed\n");
			bzero(greet->password, strlen(greet->password));
			return 0;
		} /* else: null passwd okay */
	}
#  ifdef KERBEROS
done:
#  endif
	/*
	 * Only accept root logins if allowRootLogin resource is set
	 */
	if ((p->pw_uid == 0) && !greet->allow_root_login) {
		Debug("root logins not allowed\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
#  ifdef __OpenBSD__
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
#  endif /* __OpenBSD__ */
	bzero(user_pass, strlen(user_pass)); /* in case shadow password */

# endif /* USE_PAM */
#endif /* USE_BSDAUTH */

	Debug ("verify succeeded\n");
	/* The password is passed to StartClient() for use by user-based
	   authorization schemes.  It is zeroed there. */
	verify->uid = p->pw_uid;
	verify->gid = p->pw_gid;
	home = p->pw_dir;
	shell = p->pw_shell;
	argv = NULL;
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
