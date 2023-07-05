/*
 * Parser -- M4 specific routines.  Some additional stuff from parse.c
 * should probably migrate here over time.
 */

#include "ctwm.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <pwd.h>

#include "screen.h"
#include "parse.h"
#include "parse_int.h"
#include "version.h"


static char *m4_defs(Display *display, const char *host);


/*
 * Primary entry point to do m4 parsing of a startup file
 */
FILE *
start_m4(FILE *fraw)
{
	int fids[2];
	int fres;
	char *defs_file;

	/* Write our our standard definitions into a temp file */
	defs_file = m4_defs(dpy, CLarg.display_name);

	/* We'll read back m4's output over a pipe */
	pipe(fids);

	/* Fork off m4 as a child */
	fres = fork();
	if(fres < 0) {
		perror("Fork for " M4CMD " failed");
		unlink(defs_file);
		free(defs_file);
		exit(23);
	}

	/*
	 * Child: setup and spawn m4, and have it write its output into one
	 * end of our pipe.
	 */
	if(fres == 0) {
		/* Setup file descriptors */
		close(0);               /* stdin */
		close(1);               /* stdout */
		dup2(fileno(fraw), 0);  /* stdin = fraw */
		dup2(fids[1], 1);       /* stdout = pipe to parent */

		/*
		 * Kick off m4, telling it both our file of definitions, and
		 * stdin (dup of the .[c]twmrc file descriptor above) as input.
		 * It writes to stdout (one end of our pipe).
		 */
		execlp(M4CMD, M4CMD, "-s", defs_file, "-", NULL);

		/* If we get here we are screwed... */
		perror("Can't execlp() " M4CMD);
		unlink(defs_file);
		free(defs_file);
		exit(124);
	}

	/*
	 * Else we're the parent; hand back our reading end of the pipe.
	 */
	close(fids[1]);
	free(defs_file);
	return (fdopen(fids[0], "r"));
}


/* Technically should sysconf() this, but good enough for our purposes */
#define MAXHOSTNAME 255

/*
 * Writes out a temp file of all the m4 defs appropriate for this run,
 * and returns the file name
 */
static char *
m4_defs(Display *display, const char *host)
{
	char client[MAXHOSTNAME];
	char *vc, *color;
	char *tmp_name;
	FILE *tmpf;
	char *user;

	/* Create temp file */
	{
		char *td = getenv("TMPDIR");
		if(!td || strlen(td) < 2 || *td != '/') {
			td = "/tmp";
		}
		asprintf(&tmp_name, "%s/ctwmrc.XXXXXXXX", td);
		if(!tmp_name) {
			perror("asprintf failed in m4_defs");
			exit(1);
		}

		int fd = mkstemp(tmp_name);
		if(fd < 0) {
			perror("mkstemp failed in m4_defs");
			exit(377);
		}
		tmpf = fdopen(fd, "w+");
	}


	/*
	 * Now start writing the defs into it.
	 */
#define WR_DEF(k, v) fprintf(tmpf, "define(`%s', `%s')\n", (k), (v))
#define WR_NUM(k, v) fprintf(tmpf, "define(`%s', `%d')\n", (k), (v))

	/*
	 * The machine running the window manager process (and, presumably,
	 * most of the other clients the user is running)
	 */
	if(gethostname(client, MAXHOSTNAME) < 0) {
		perror("gethostname failed in m4_defs");
		exit(1);
	}
	WR_DEF("CLIENTHOST", client);

	/*
	 * A guess at the machine running the X server.  We take the full
	 * $DISPLAY and chop off the screen specification.
	 */
	{
		char *server, *colon;

		server = strdup(XDisplayName(host));
		if(!server) {
			server = strdup("unknown");
		}
		colon = strchr(server, ':');
		if(colon != NULL) {
			*colon = '\0';
		}

		/* :0 or unix socket connection means it's the same as CLIENTHOST */
		if((server[0] == '\0') || (!strcmp(server, "unix"))) {
			free(server);
			server = strdup(client);
		}
		WR_DEF("SERVERHOST", server);

		free(server);
	}

#ifdef HISTORICAL_HOSTNAME_IMPL
	/*
	 * Historical attempt to use DNS to figure a canonical name.  This is
	 * left inside this #ifdef for easy restoration if somebody finds a
	 * need; enabling it is not supported or documented.  Unless somebody
	 * comes up with a good reason to revive it, it will be removed after
	 * 4.0.2.
	 */
	{
		struct hostent *hostname = gethostbyname(client);
		if(hostname) {
			WR_DEF("HOSTNAME", hostname->h_name);
		}
		else {
			WR_DEF("HOSTNAME", client);
		}
	}
#else
	/*
	 * Just leave HOSTNAME as a copy of CLIENTHOST for backward
	 * compat.
	 */
	WR_DEF("HOSTNAME", client);
#endif

	/*
	 * Info about the user and their environment
	 */
	if(!(user = getenv("USER")) && !(user = getenv("LOGNAME"))) {
		struct passwd *pwd = getpwuid(getuid());
		if(pwd) {
			user = pwd->pw_name;
		}
	}
	if(!user) {
		user = "unknown";
	}
	WR_DEF("USER", user);
	WR_DEF("HOME", Home);

	/*
	 * ctwm meta
	 */
	WR_DEF("TWM_TYPE", "ctwm");
	WR_DEF("TWM_VERSION", VersionNumber);
	WR_DEF("CTWM_VERSION_MAJOR", VersionNumber_major);
	WR_DEF("CTWM_VERSION_MINOR", VersionNumber_minor);
	WR_DEF("CTWM_VERSION_PATCH", VersionNumber_patch);
	WR_DEF("CTWM_VERSION_ADDL",  VersionNumber_addl);

	/*
	 * X server meta
	 */
	if(display) {
		WR_NUM("VERSION", ProtocolVersion(display));
		WR_NUM("REVISION", ProtocolRevision(display));
		WR_DEF("VENDOR", ServerVendor(display));
		WR_NUM("RELEASE", VendorRelease(display));
	}
	else {
		// Standin numbers
		WR_NUM("VERSION", 11);
		WR_NUM("REVISION", 0);
		WR_DEF("VENDOR", "Your Friendly Local Ctwm");
		WR_NUM("RELEASE", 123456789);
	}

	/*
	 * Information about the display
	 */
	WR_NUM("WIDTH", Scr->rootw);
	WR_NUM("HEIGHT", Scr->rooth);
#define Resolution(pixels, mm)  ((((pixels) * 100000 / (mm)) + 50) / 100)
	WR_NUM("X_RESOLUTION", Resolution(Scr->rootw, Scr->mm_w));
	WR_NUM("Y_RESOLUTION", Resolution(Scr->rooth, Scr->mm_h));
#undef Resolution
	WR_NUM("PLANES", Scr->d_depth);
	WR_NUM("BITS_PER_RGB", Scr->d_visual->bits_per_rgb);
	color = "Yes";
	switch(Scr->d_visual->class) {
		case(StaticGray):
			vc = "StaticGray";
			color = "No";
			break;
		case(GrayScale):
			vc = "GrayScale";
			color = "No";
			break;
		case(StaticColor):
			vc = "StaticColor";
			break;
		case(PseudoColor):
			vc = "PseudoColor";
			break;
		case(TrueColor):
			vc = "TrueColor";
			break;
		case(DirectColor):
			vc = "DirectColor";
			break;
		default:
			vc = "NonStandard";
			break;
	}
	WR_DEF("CLASS", vc);
	WR_DEF("COLOR", color);

	/*
	 * Bits of "how this ctwm invocation is being run" data
	 */
	if(0) {
		// Dummy
	}
#ifdef CAPTIVE
	else if(CLarg.is_captive && Scr->captivename) {
		WR_DEF("TWM_CAPTIVE", "Yes");
		WR_DEF("TWM_CAPTIVE_NAME", Scr->captivename);
	}
#endif
	else {
		WR_DEF("TWM_CAPTIVE", "No");
	}

	/*
	 * Various compile-time options.
	 */
#ifdef PIXMAP_DIRECTORY
	WR_DEF("PIXMAP_DIRECTORY", PIXMAP_DIRECTORY);
#endif
#ifdef XPM
	WR_DEF("XPM", "Yes");
#endif
#ifdef JPEG
	WR_DEF("JPEG", "Yes");
#endif
#ifdef SOUNDS
	WR_DEF("SOUNDS", "Yes");
#endif
#ifdef EWMH
	WR_DEF("EWMH", "Yes");
#endif
#ifdef XRANDR
	WR_DEF("XRANDR", "Yes");
#endif
	/* Since this is no longer an option, it should be removed in the future */
	WR_DEF("I18N", "Yes");

#undef WR_NUM
#undef WR_DEF


	/*
	 * We might be keeping it, in which case tell the user where it is;
	 * this is mostly a debugging option.  Otherwise, delete it by
	 * telling m4 to do so when it reads it; this is fairly fugly, and I
	 * have more than half a mind to dike it out and properly clean up
	 * ourselves.
	 */
	if(CLarg.KeepTmpFile) {
		fprintf(stderr, "Left file: %s\n", tmp_name);
	}
	else {
		fprintf(tmpf, "syscmd(/bin/rm %s)\n", tmp_name);
	}


	/* Close out and hand it back */
	fclose(tmpf);
	return(tmp_name);
}
