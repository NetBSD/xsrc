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
 * session.c
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"
#include "greet.h"

#include <X11/Xlib.h>
#include <signal.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Error.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <grp.h>	/* for initgroups */

# ifdef HAVE_SETPROCTITLE
#  include <sys/types.h>
#  include <unistd.h>
# endif

#ifndef USE_PAM        /* PAM modules should handle these */
# ifdef SECURE_RPC
#  include <rpc/rpc.h>
#  include <rpc/key_prot.h>
#  if !HAVE_DECL_KEY_SETNET
extern int key_setnet(struct key_netstarg *arg);
#  endif
# endif
# ifdef K5AUTH
#  include <krb5/krb5.h>
# endif
#endif /* USE_PAM */

#ifdef USE_SELINUX
#include <selinux/selinux.h>
#include <selinux/get_context_list.h>
#endif /* USE_SELINUX */

# include <dlfcn.h>
# ifndef RTLD_NOW
#  define RTLD_NOW 1
# endif

#ifdef USE_SYSTEMD_DAEMON
#include <systemd/sd-daemon.h>
#endif

#ifdef USE_SELINUX
/* This should be run just before we exec the user session. */
static int
xdm_selinux_setup (const char *login)
  {
	security_context_t scontext;
	int ret = -1;
	char *seuser=NULL;
	char *level=NULL;

	/* If SELinux is not enabled, then we don't do anything. */
	if ( is_selinux_enabled () <= 0)
		return TRUE;

	if (getseuserbyname(login, &seuser, &level) == 0) {
		ret=get_default_context_with_level(seuser, level, 0, &scontext);
		free(seuser);
		free(level);
	}
	if (ret < 0 || scontext == NULL) {
		LogError ("SELinux: unable to obtain default security context for %s\n", login);
		return FALSE;
	}

	if (setexeccon (scontext) != 0) {
	freecon (scontext);
	LogError ("SELinux: unable to set executable context %s\n",
	      (char *)scontext);
	return FALSE;
	}

	freecon (scontext);
	return TRUE;
}
#endif /* USE_SELINUX */

static	int	runAndWait (char **args, char **environ);

#ifdef HAVE_GRP_H
# include <sys/types.h>
# include <grp.h>
#else
/* should be in <grp.h> */
extern	void	setgrent(void);
extern	struct group	*getgrent(void);
extern	void	endgrent(void);
#endif

#ifdef HAVE_GETSPNAM
# include <shadow.h>
#endif
#if defined(CSRG_BASED) || defined(__GLIBC__) || defined(__sun)
# include <pwd.h>
# include <unistd.h>
# if defined(__GLIBC__) && !defined(_XOPEN_CRYPT)
# include <crypt.h>
# endif
#else
extern	struct passwd	*getpwnam(GETPWNAM_ARGS);
# ifdef linux
extern  void	endpwent(void);
# endif
extern	char	*crypt(CRYPT_ARGS);
#endif

#ifdef USE_PAM
pam_handle_t **
thepamhp(void)
{
	static pam_handle_t *pamh = NULL;
	return &pamh;
}

pam_handle_t *
thepamh(void)
{
	pam_handle_t **pamhp;

	pamhp = thepamhp();
	if (pamhp)
		return *pamhp;
	else
		return NULL;
}
#endif

static	struct dlfuncs	dlfuncs = {
	PingServer,
	SessionPingFailed,
	Debug,
	RegisterCloseOnFork,
	SecureDisplay,
	UnsecureDisplay,
	ClearCloseOnFork,
	SetupDisplay,
	LogError,
	SessionExit,
	DeleteXloginResources,
	source,
	defaultEnv,
	setEnv,
	putEnv,
	parseArgs,
	printEnv,
	systemEnv,
	LogOutOfMem,
	setgrent,
	getgrent,
	endgrent,
#ifdef HAVE_GETSPNAM
	getspnam,
	endspent,
#endif
	getpwnam,
#if defined(linux) || defined(__GLIBC__)
	endpwent,
#endif
	crypt,
#ifdef USE_PAM
	thepamhp,
#endif
	};

static Bool StartClient(
    struct verify_info	*verify,
    struct display	*d,
    pid_t		*pidp,
    char		*name,
    char		*passwd);

static pid_t			clientPid;
static struct greet_info	greet;
static struct verify_info	verify;

static Jmp_buf	abortSession;

/* ARGSUSED */
_X_NORETURN
static void
catchTerm (int n)
{
    Longjmp (abortSession, 1);
}

static Jmp_buf	pingTime;

/* ARGSUSED */
_X_NORETURN
static void
catchAlrm (int n)
{
    Longjmp (pingTime, 1);
}

static Jmp_buf	tenaciousClient;

/* ARGSUSED */
_X_NORETURN
static void
waitAbort (int n)
{
	Longjmp (tenaciousClient, 1);
}

#if defined(_POSIX_SOURCE) || defined(SVR4)
# define killpg(pgrp, sig) kill(-(pgrp), sig)
#endif

static void
AbortClient (pid_t pid)
{
    int	sig = SIGTERM;
    volatile int	i;
    pid_t	retId;

    for (i = 0; i < 4; i++) {
	if (killpg (pid, sig) == -1) {
	    switch (errno) {
	    case EPERM:
		LogError ("xdm can't kill client\n");
	    case EINVAL:
	    case ESRCH:
		return;
	    }
	}
	if (!Setjmp (tenaciousClient)) {
	    (void) Signal (SIGALRM, waitAbort);
	    (void) alarm ((unsigned) 10);
	    retId = wait ((waitType *) 0);
	    (void) alarm ((unsigned) 0);
	    (void) Signal (SIGALRM, SIG_DFL);
	    if (retId == pid)
		break;
	} else
	    (void) Signal (SIGALRM, SIG_DFL);
	sig = SIGKILL;
    }
}

void
SessionPingFailed (struct display *d)
{
    if (clientPid > 1) {
	AbortClient (clientPid);
	source (verify.systemEnviron, d->reset);
    }
    SessionExit (d, RESERVER_DISPLAY, TRUE);
}

/*
 * We need our own error handlers because we can't be sure what exit code Xlib
 * will use, and our Xlib does exit(1) which matches REMANAGE_DISPLAY, which
 * can cause a race condition leaving the display wedged.  We need to use
 * RESERVER_DISPLAY for IO errors, to ensure that the manager waits for the
 * server to terminate.  For other X errors, we should give up.
 */

/*ARGSUSED*/
static int
IOErrorHandler (Display *dpy)
{
    LogError ("fatal IO error %d (%s)\n", errno, _SysErrorMsg(errno));
    exit(RESERVER_DISPLAY);
    /*NOTREACHED*/
    return 0;
}

static int
ErrorHandler(Display *dpy, XErrorEvent *event)
{
    LogError ("X error\n");
    if (XmuPrintDefaultErrorMessage (dpy, event, stderr) == 0) return 0;
    exit(UNMANAGE_DISPLAY);
    /*NOTREACHED*/
}

void
ManageSession (struct display *d)
{
    static pid_t	pid = 0;
    Display		*dpy;
    greet_user_rtn	greet_stat;
    static GreetUserProc greet_user_proc = NULL;
    void		*greet_lib_handle;

    Debug ("ManageSession %s\n", d->name);
    (void)XSetIOErrorHandler(IOErrorHandler);
    (void)XSetErrorHandler(ErrorHandler);
#ifndef HAVE_SETPROCTITLE
    SetTitle(d->name, (char *) 0);
#else
    setproctitle("%s", d->name);
#endif
    /*
     * Load system default Resources
     */
    LoadXloginResources (d);

#ifndef STATIC_GREETER_LIB
    Debug ("ManageSession: loading greeter library %s\n", greeterLib);
    greet_lib_handle = dlopen(greeterLib, RTLD_NOW);
    if (greet_lib_handle != NULL)
	greet_user_proc = (GreetUserProc)dlsym(greet_lib_handle, "GreetUser");
#else
    greet_user_proc = (GreetUserProc)GreetUser;
#endif

    if (greet_user_proc == NULL) {
	LogError ("%s while loading %s\n", dlerror(), greeterLib);
	exit(UNMANAGE_DISPLAY);
	}

#ifdef USE_SYSTEMD_DAEMON
	/* Subsequent notifications will be ignored by systemd
	 * and calling this function will clean up the env */
	sd_notify(1, "READY=1");
#endif

    /* tell the possibly dynamically loaded greeter function
     * what data structure formats to expect.
     * These version numbers are registered with The Open Group. */
    verify.version = 1;
    greet.version = 1;
    greet_stat = (*greet_user_proc)(d, &dpy, &verify, &greet, &dlfuncs);

    if (greet_stat == Greet_Success) {
	clientPid = 0;
	if (!Setjmp (abortSession)) {
	    (void) Signal (SIGTERM, catchTerm);
	    /*
	     * Start the clients, changing uid/groups
	     *	   setting up environment and running the session
	     */
	    if (StartClient (&verify, d, &clientPid, greet.name, greet.password)) {
		Debug ("Client Started\n");

                /* Save memory; close library */
                dlclose(greet_lib_handle);

		/*
		 * Wait for session to end,
		 */
		for (;;) {
		    if (d->pingInterval) {
			if (!Setjmp (pingTime)) {
			    (void) Signal (SIGALRM, catchAlrm);
			    (void) alarm (d->pingInterval * 60);
			    pid = wait ((waitType *) 0);
			    (void) alarm (0);
			} else {
			    (void) alarm (0);
			    if (!PingServer (d, (Display *) NULL))
				SessionPingFailed (d);
			}
		    } else {
			pid = wait ((waitType *) 0);
		    }
		    if (pid == clientPid)
			break;
		}
	    } else {
		LogError ("session start failed\n");
	    }
	} else {
	    /*
	     * when terminating the session, nuke
	     * the child and then run the reset script
	     */
	    AbortClient (clientPid);
	}
    }
    /*
     * run system-wide reset file
     */
    Debug ("Source reset program %s\n", d->reset);
    source (verify.systemEnviron, d->reset);
    SessionExit (d, OBEYSESS_DISPLAY, TRUE);
}

void
LoadXloginResources (struct display *d)
{
    char	**args;
    char	**env = NULL;

    if (d->resources[0] && access (d->resources, 4) == 0) {
	env = systemEnv (d, (char *) 0, (char *) 0);
	args = parseArgs ((char **) 0, d->xrdb);
	args = parseArgs (args, d->resources);
	Debug ("Loading resource file: %s\n", d->resources);
	(void) runAndWait (args, env);
	freeArgs (args);
	freeEnv (env);
    }
}

void
SetupDisplay (struct display *d)
{
    char	**env = NULL;

    if (d->setup && d->setup[0]) {
	env = systemEnv (d, (char *) 0, (char *) 0);
	(void) source (env, d->setup);
	freeEnv (env);
    }
}

/*ARGSUSED*/
void
DeleteXloginResources (struct display *d, Display *dpy)
{
    int i;
    Atom prop = XInternAtom(dpy, "SCREEN_RESOURCES", True);

    XDeleteProperty(dpy, RootWindow (dpy, 0), XA_RESOURCE_MANAGER);
    if (prop) {
	for (i = ScreenCount(dpy); --i >= 0; )
	    XDeleteProperty(dpy, RootWindow (dpy, i), prop);
    }
}

static Jmp_buf syncJump;

/* ARGSUSED */
_X_NORETURN
static void
syncTimeout (int n)
{
    Longjmp (syncJump, 1);
}

void
SecureDisplay (struct display *d, Display *dpy)
{
    Debug ("SecureDisplay %s\n", d->name);
    (void) Signal (SIGALRM, syncTimeout);
    if (Setjmp (syncJump)) {
	LogError ("WARNING: display %s could not be secured\n",
		   d->name);
	SessionExit (d, RESERVER_DISPLAY, FALSE);
    }
    (void) alarm ((unsigned) d->grabTimeout);
    Debug ("Before XGrabServer %s\n", d->name);
    XGrabServer (dpy);
    if (XGrabKeyboard (dpy, DefaultRootWindow (dpy), True, GrabModeAsync,
		       GrabModeAsync, CurrentTime) != GrabSuccess) {
	(void) alarm (0);
	(void) Signal (SIGALRM, SIG_DFL);
	LogError ("WARNING: keyboard on display %s could not be secured\n",
		  d->name);
	SessionExit (d, RESERVER_DISPLAY, FALSE);
    }
    Debug ("XGrabKeyboard succeeded %s\n", d->name);
    (void) alarm (0);
    (void) Signal (SIGALRM, SIG_DFL);
    pseudoReset (dpy);
    if (!d->grabServer) {
	XUngrabServer (dpy);
	XSync (dpy, 0);
    }
    Debug ("done secure %s\n", d->name);
}

void
UnsecureDisplay (struct display *d, Display *dpy)
{
    Debug ("Unsecure display %s\n", d->name);
    if (d->grabServer) {
	XUngrabServer (dpy);
	XSync (dpy, 0);
    }
}

void
SessionExit (struct display *d, int status, int removeAuth)
{
#ifdef USE_PAM
    pam_handle_t *pamh = thepamh();

    if (pamh) {
        /* shutdown PAM session */
	pam_close_session(pamh, 0);
	pam_end(pamh, PAM_SUCCESS);
	pamh = NULL;
    }
#endif

    /* make sure the server gets reset after the session is over */
    if (d->serverPid >= 2 && d->resetSignal)
	kill (d->serverPid, d->resetSignal);
    else
	ResetServer (d);
    if (removeAuth) {
	if (setgid (verify.gid) == -1) {
	    LogError( "SessionExit: setgid: %s\n", strerror(errno));
	    exit(status);
	}
	if (setuid (verify.uid) == -1) {
	    LogError( "SessionExit: setuid: %s\n", strerror(errno));
	    exit(status);
	}
	RemoveUserAuthorization (d, &verify);
#if defined(K5AUTH) && !defined(USE_PAM)   /* PAM modules should handle this */
	/* do like "kdestroy" program */
        {
	    krb5_error_code code;
	    krb5_ccache ccache;

	    code = Krb5DisplayCCache(d->name, &ccache);
	    if (code)
		LogError ("%s while getting Krb5 ccache to destroy\n",
			 error_message(code));
	    else {
		code = krb5_cc_destroy(ccache);
		if (code) {
		    if (code == KRB5_FCC_NOFILE) {
			Debug ("No Kerberos ccache file found to destroy\n");
		    } else
			LogError ("%s while destroying Krb5 credentials cache\n",
				 error_message(code));
		} else
		    Debug ("Kerberos ccache destroyed\n");
		krb5_cc_close(ccache);
	    }
	}
#endif /* K5AUTH */
    }
    Debug ("Display %s exiting with status %d\n", d->name, status);
    exit (status);
}

static Bool
StartClient (
    struct verify_info	*verify,
    struct display	*d,
    pid_t		*pidp,
    char		*name,
    char		*passwd)
{
    char	**f, *home;
    char	*failsafeArgv[2];
    pid_t	pid;
#ifdef HAVE_SETUSERCONTEXT
    struct passwd* pwd;
#endif
#ifdef USE_PAM
    pam_handle_t *pamh = thepamh ();
    int	pam_error;
#endif

    if (verify->argv) {
	Debug ("StartSession %s: ", verify->argv[0]);
	for (f = verify->argv; *f; f++)
		Debug ("%s ", *f);
	Debug ("; ");
    }
    if (verify->userEnviron) {
	for (f = verify->userEnviron; *f; f++)
		Debug ("%s ", *f);
	Debug ("\n");
    }
#ifdef USE_PAM
    if (pamh) pam_open_session(pamh, 0);
#endif
    switch (pid = fork ()) {
    case 0:
	CleanUpChild ();
#ifdef XDMCP
	/* The chooser socket is not closed by CleanUpChild() */
	DestroyWellKnownSockets();
#endif

	/* Do system-dependent login setup here */

#ifndef HAVE_SETUSERCONTEXT
	if (setgid (verify->gid) < 0) {
	    LogError ("setgid %d (user \"%s\") failed: %s\n",
		      verify->gid, name, _SysErrorMsg (errno));
	    return (0);
	}
# if defined(BSD) && (BSD >= 199103)
	if (setlogin (name) < 0) {
	    LogError ("setlogin for \"%s\" failed: %s\n",
		      name, _SysErrorMsg (errno));
	    return (0);
	}
# endif
	if (initgroups (name, verify->gid) < 0) {
	    LogError ("initgroups for \"%s\" failed: %s\n",
		      name, _SysErrorMsg (errno));
	    return (0);
	}
#endif /* !HAVE_SETUSERCONTEXT */

#ifdef USE_PAM
	if (pamh) {
	    long i;
	    char **pam_env;

	    pam_error = pam_setcred (pamh, PAM_ESTABLISH_CRED);
	    if (pam_error != PAM_SUCCESS) {
		LogError ("pam_setcred for \"%s\" failed: %s\n",
			 name, pam_strerror(pamh, pam_error));
		return(0);
	    }

	    /* pass in environment variables set by libpam and modules it called */
	    pam_env = pam_getenvlist(pamh);
	    for(i = 0; pam_env && pam_env[i]; i++) {
		verify->userEnviron = putEnv(pam_env[i], verify->userEnviron);
	    }

	}
#endif

#ifndef HAVE_SETUSERCONTEXT
	if (setuid(verify->uid) < 0) {
	    LogError ("setuid %d (user \"%s\") failed: %s\n",
		      verify->uid, name, _SysErrorMsg (errno));
	    return (0);
	}
#else /* HAVE_SETUSERCONTEXT */
	/*
	 * Set the user's credentials: uid, gid, groups,
	 * environment variables, resource limits, and umask.
	 */
	pwd = getpwnam(name);
	if (pwd) {
	    if (setusercontext(NULL, pwd, pwd->pw_uid, LOGIN_SETALL) < 0) {
		LogError ("setusercontext for \"%s\" failed: %s\n",
			  name, _SysErrorMsg (errno));
		return (0);
	    }
	    endpwent();
	} else {
	    LogError ("getpwnam for \"%s\" failed: %s\n",
		      name, _SysErrorMsg (errno));
	    return (0);
	}
#endif /* HAVE_SETUSERCONTEXT */

#ifndef USE_PAM		/* PAM modules should handle these */
	/*
	 * for user-based authorization schemes,
	 * use the password to get the user's credentials.
	 */
# ifdef SECURE_RPC
	/* do like "keylogin" program */
	{
	    char    netname[MAXNETNAMELEN+1], secretkey[HEXKEYBYTES+1];
	    int	    nameret, keyret;
	    int	    len;
	    struct  key_netstarg netst;
	    int     key_set_ok = 0;

	    nameret = getnetname (netname);
	    Debug ("User netname: %s\n", netname);
	    len = strlen (passwd);
	    if (len > 8)
		bzero (passwd + 8, len - 8);
	    keyret = getsecretkey(netname,secretkey,passwd);
	    Debug ("getsecretkey returns %d, key length %lu\n",
		    keyret, strlen (secretkey));
	    memcpy(&(netst.st_priv_key), secretkey, HEXKEYBYTES);
	    netst.st_netname = strdup(netname);
	    memset(netst.st_pub_key, 0, HEXKEYBYTES);
            if (key_setnet(&netst) < 0) {
		Debug ("Could not set secret key.\n");
            }
	    free(netst.st_netname);
	    /* is there a key, and do we have the right password? */
	    if (keyret == 1) {
		if (*secretkey) {
		    keyret = key_setsecret(secretkey);
		    Debug ("key_setsecret returns %d\n", keyret);
		    if (keyret == -1)
			LogError ("failed to set NIS secret key\n");
		    else
			key_set_ok = 1;
		} else {
		    /* found a key, but couldn't interpret it */
		    LogError ("password incorrect for NIS principal \"%s\"\n",
			      nameret ? netname : name);
		}
	    }
	    if (!key_set_ok) {
		/* remove SUN-DES-1 from authorizations list */
		int i, j;
		for (i = 0; i < d->authNum; i++) {
		    if (d->authorizations[i]->name_length == 9 &&
			memcmp(d->authorizations[i]->name, "SUN-DES-1", 9) == 0) {
			for (j = i+1; j < d->authNum; j++)
			    d->authorizations[j-1] = d->authorizations[j];
			d->authNum--;
			break;
		    }
		}
	    }
	    bzero(secretkey, strlen(secretkey));
	}
# endif
# ifdef K5AUTH
	/* do like "kinit" program */
	{
	    int i, j;
	    int result;
	    extern char *Krb5CCacheName();

	    result = Krb5Init(name, passwd, d);
	    if (result == 0) {
		/* point session clients at the Kerberos credentials cache */
		verify->userEnviron =
		    setEnv(verify->userEnviron,
			   "KRB5CCNAME", Krb5CCacheName(d->name));
	    } else {
		for (i = 0; i < d->authNum; i++) {
		    if (d->authorizations[i]->name_length == 14 &&
			memcmp(d->authorizations[i]->name, "MIT-KERBEROS-5", 14) == 0) {
			/* remove Kerberos from authorizations list */
			for (j = i+1; j < d->authNum; j++)
			    d->authorizations[j-1] = d->authorizations[j];
			d->authNum--;
			break;
		    }
		}
	    }
	}
# endif /* K5AUTH */
#endif /* !USE_PAM */

	if (d->windowPath)
		verify->userEnviron = setEnv(verify->userEnviron, "WINDOWPATH", d->windowPath);

	if (passwd != NULL)
	    bzero(passwd, strlen(passwd));

	SetUserAuthorization (d, verify);
#ifdef USE_SELINUX
   /*
    * For Security Enhanced Linux:
    * set the default security context for this user.
    */
   if ( ! xdm_selinux_setup (name)) {
      LogError ("failed to set security context\n");
       exit (UNMANAGE_DISPLAY);
       return (0);
   }
#endif /* USE_SELINUX */
	home = getEnv (verify->userEnviron, "HOME");
	if (home)
	    if (chdir (home) == -1) {
		LogError ("user \"%s\": cannot chdir to home \"%s\" (err %d), using \"/\"\n",
			  getEnv (verify->userEnviron, "USER"), home, errno);
		chdir ("/");
		verify->userEnviron = setEnv(verify->userEnviron, "HOME", "/");
	    }
	if (verify->argv) {
		LogInfo ("executing session %s\n", verify->argv[0]);
		execute (verify->argv, verify->userEnviron);
		LogError ("Session \"%s\" execution failed (err %d)\n", verify->argv[0], errno);
	} else {
		LogError ("Session has no command/arguments\n");
	}
	failsafeArgv[0] = d->failsafeClient;
	failsafeArgv[1] = NULL;
	execute (failsafeArgv, verify->userEnviron);
	exit (1);
    case -1:
	if (passwd != NULL)
	    bzero(passwd, strlen(passwd));
	Debug ("StartSession, fork failed\n");
	LogError ("can't start session on \"%s\", fork failed: %s\n",
		  d->name, _SysErrorMsg (errno));
	return 0;
    default:
	if (passwd != NULL)
	    bzero(passwd, strlen(passwd));
	Debug ("StartSession, fork succeeded %d\n", pid);
	*pidp = pid;
	return 1;
    }
}

int
source (char **environ, char *file)
{
    char	**args, *args_safe[2];
    int		ret = 0;
    FILE	*f;

    if (file && file[0]) {
	f = fopen (file, "r");
	if (!f)
	    LogInfo ("not sourcing %s (%s)\n", file, _SysErrorMsg (errno));
	else {
	    fclose (f);
	    LogInfo ("sourcing %s\n", file);
	    args = parseArgs ((char **) 0, file);
	    if (!args) {
		args = args_safe;
		args[0] = file;
		args[1] = NULL;
	    }
	    ret = runAndWait (args, environ);
	    freeArgs (args);
	}
    } else
	Debug ("source() given null pointer in file argument\n");
    return ret;
}

static int
runAndWait (char **args, char **environ)
{
    pid_t	pid;
    waitType	result;

    switch (pid = fork ()) {
    case 0:
	CleanUpChild ();
#ifdef XDMCP
	/* The chooser socket is not closed by CleanUpChild() */
	DestroyWellKnownSockets();
#endif
	execute (args, environ);
	LogError ("can't execute \"%s\" (err %d)\n", args[0], errno);
	exit (1);
    case -1:
	Debug ("fork failed\n");
	LogError ("can't fork to execute \"%s\" (err %d)\n", args[0], errno);
	return 1;
    default:
	while (wait (&result) != pid)
		/* SUPPRESS 530 */
		;
	break;
    }
    return waitVal (result);
}

void
execute (char **argv, char **environ)
{
    int err;
    /* give /dev/null as stdin */
    (void) close (0);
    open ("/dev/null", O_RDONLY);
    /* make stdout follow stderr to the log file */
    dup2 (2,1);
    Debug ("attempting to execve() %s\n", argv[0]);
    execve (argv[0], argv, environ);
    err = errno;
    Debug ("execve() of %s failed: %s\n", argv[0], _SysErrorMsg (errno));
    /*
     * In case this is a shell script which hasn't been
     * made executable, do a reasonable thing.
     */
    if (err != ENOENT) {
	char	program[1024], *e, *p, *optarg;
	FILE	*f;
	char	**newargv, **av;
	int	argc;

	/*
	 * emulate BSD kernel behaviour -- read
	 * the first line; check if it starts
	 * with "#!", in which case it uses
	 * the rest of the line as the name of
	 * program to run.  Else use "/bin/sh".
	 */
	f = fopen (argv[0], "r");
	if (!f)
	    return;
	if (fgets (program, sizeof (program) - 1, f) == NULL) {
	    fclose (f);
	    return;
	}
	fclose (f);
	if (program[0] == '\0')
	    return;
	e = program + strlen (program) - 1;
	if (*e == '\n')
	    *e = '\0';
	if (!strncmp (program, "#!", 2)) {
	    p = program + 2;
	    while (*p && isspace (*p))
		++p;
	    optarg = p;
	    while (*optarg && !isspace (*optarg))
		++optarg;
	    if (*optarg) {
		*optarg = '\0';
		do
		    ++optarg;
		while (*optarg && isspace (*optarg));
	    } else
		optarg = NULL;
	} else {
	    p = "/bin/sh";
	    optarg = NULL;
	}
	Debug ("Shell script execution: %s (optarg %s)\n",
		p, optarg ? optarg : "(null)");
	for (av = argv, argc = 0; *av; av++, argc++)
	    /* SUPPRESS 530 */
	    ;
	newargv = malloc ((argc + (optarg ? 3 : 2)) * sizeof (char *));
	if (!newargv)
	    return;
	av = newargv;
	*av++ = p;
	if (optarg)
	    *av++ = optarg;
	/* SUPPRESS 560 */
	while ((*av++ = *argv++))
	    /* SUPPRESS 530 */
	    ;
	Debug ("Attempting to execve() %s\n", newargv[0]);
	execve (newargv[0], newargv, environ);
    }
}

char **
defaultEnv (void)
{
    char    **env, **exp, *value;

    env = NULL;
    for (exp = exportList; exp && *exp; ++exp) {
	value = getenv (*exp);
	if (value)
	    env = setEnv (env, *exp, value);
    }
    return env;
}

char **
systemEnv (struct display *d, char *user, char *home)
{
    char	**env;

    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    if (home)
	env = setEnv (env, "HOME", home);
    if (user) {
	env = setEnv (env, "USER", user);
	env = setEnv (env, "LOGNAME", user);
    }
    env = setEnv (env, "PATH", d->systemPath);
    env = setEnv (env, "SHELL", d->systemShell);
    if (d->authFile)
	    env = setEnv (env, "XAUTHORITY", d->authFile);
    if (d->windowPath)
	    env = setEnv (env, "WINDOWPATH", d->windowPath);
    return env;
}
