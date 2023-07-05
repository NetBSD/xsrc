/*
 * Command-line arg handling
 */

#include "ctwm.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clargs.h"
#include "ctopts.h"
#include "deftwmrc.h"
#include "screen.h"
#include "version.h"

static void usage(void) __attribute__((noreturn));
static void print_version(void);
static void DisplayInfo(void);
static void dump_default_config(void);


/*
 * Command-line args.  Initialize with useful default values.
 */
ctwm_cl_args CLarg = {
	.MultiScreen     = true,
	.Monochrome      = false,
	.cfgchk          = false,
	.InitFile        = NULL,
	.display_name    = NULL,
	.PrintErrorMessages = false,
#ifdef DEBUG
	.ShowWelcomeWindow  = false,
#else
	.ShowWelcomeWindow  = true,
#endif
#ifdef CAPTIVE
	.is_captive      = false,
	.capwin          = (Window) 0,
	.captivename     = NULL,
#endif
#ifdef USEM4
	.KeepTmpFile     = false,
	.keepM4_filename = NULL,
	.GoThroughM4     = true,
#endif
#ifdef EWMH
	.ewmh_replace    = false,
#endif
	.client_id       = NULL,
	.restore_filename = NULL,
};



/*
 * Parse them out and setup CLargs.
 */
void
clargs_parse(int argc, char *argv[])
{
	int ch, optidx;

	/*
	 * Setup long options for arg parsing
	 */
	static struct option long_options[] = {
		/* Simple flags */
		{ "single",    no_argument,       NULL, 0 },
		{ "mono",      no_argument,       NULL, 0 },
		{ "verbose",   no_argument,       NULL, 'v' },
		{ "quiet",     no_argument,       NULL, 'q' },
		{ "nowelcome", no_argument,       NULL, 'W' },

		/* Config/file related */
		{ "file",      required_argument, NULL, 'f' },
		{ "cfgchk",    no_argument,       NULL, 0 },

		/* Show something and exit right away */
		{ "help",      no_argument,       NULL, 'h' },
		{ "version",   no_argument,       NULL, 0 },
		{ "info",      no_argument,       NULL, 0 },
		{ "dumpcfg",   no_argument,       NULL, 0 },

		/* Misc control bits */
		{ "display",   required_argument, NULL, 'd' },
		{ "xrm",       required_argument, NULL, 0 },
#ifdef CAPTIVE
		{ "window",    optional_argument, NULL, 'w' },
		{ "name",      required_argument, NULL, 0 },
#endif

#ifdef EWMH
		{ "replace",   no_argument,       NULL, 0 },
#endif

		/* M4 control params */
#ifdef USEM4
		{ "keep-defs", no_argument,       NULL, 'k' },
		{ "keep",      required_argument, NULL, 'K' },
		{ "nom4",      no_argument,       NULL, 'n' },
#endif

		/* Random session-related bits */
		{ "clientId",  required_argument, NULL, 0 },
		{ "restore",   required_argument, NULL, 0 },

		{ NULL,        0,                 NULL, 0 },
	};


	/*
	 * Short aliases for some
	 *
	 * I assume '::' for optional args is portable; getopt_long(3)
	 * doesn't describe it, but it's a GNU extension for getopt(3).
	 */
	const char *short_options = "vqWf:hd:"
#ifdef CAPTIVE
	                            "w::"
#endif
#ifdef USEM4
	                            "kK:n"
#endif
	                            ;


	/*
	 * Backward-compat cheat: accept a few old-style long args if they
	 * came first.  Of course, this assumed argv[x] is editable, which on
	 * most systems it is, and C99 requires it.
	 */
	if(argc > 1) {
#define CHK(x) else if(strcmp(argv[1], (x)) == 0)
		if(0) {
			/* nada */
		}
		CHK("-version") {
			print_version();
			exit(0);
		}
		CHK("-info") {
			DisplayInfo();
			exit(0);
		}
		CHK("-cfgchk") {
			CLarg.cfgchk = true;
			*argv[1] = '\0';
		}
		CHK("-display") {
			if(argc <= 2 || strlen(argv[2]) < 1) {
				usage();
			}
			CLarg.display_name = strdup(argv[2]);

			*argv[1] = '\0';
			*argv[2] = '\0';
		}
#undef CHK
	}


	/*
	 * Parse out the args
	 */
	optidx = 0;
	while((ch = getopt_long(argc, argv, short_options, long_options,
	                        &optidx)) != -1) {
		switch(ch) {
			/* First handle the simple cases that have short args */
			case 'v':
				CLarg.PrintErrorMessages = true;
				break;
			case 'q':
				CLarg.PrintErrorMessages = false;
				break;
			case 'W':
				CLarg.ShowWelcomeWindow = false;
				break;
			case 'f':
				CLarg.InitFile = optarg;
				break;
			case 'h':
				usage();
			case 'd':
				CLarg.display_name = optarg;
				break;
#ifdef CAPTIVE
			case 'w':
				CLarg.is_captive = true;
				CLarg.MultiScreen = false;
				if(optarg != NULL) {
					sscanf(optarg, "%x", (unsigned int *)&CLarg.capwin);
					/* Failure will just leave capwin as initialized */
				}
				break;
#endif

#ifdef USEM4
			/* Args that only mean anything if we're built with m4 */
			case 'k':
				CLarg.KeepTmpFile = true;
				break;
			case 'K':
				CLarg.keepM4_filename = optarg;
				break;
			case 'n':
				CLarg.GoThroughM4 = false;
				break;
#endif


			/*
			 * Now the stuff that doesn't have short variants.
			 */
			case 0:

#define IFIS(x) if(strcmp(long_options[optidx].name, (x)) == 0)
				/* Simple flag-setting */
				IFIS("single") {
					CLarg.MultiScreen = false;
					break;
				}
				IFIS("mono") {
					CLarg.Monochrome = true;
					break;
				}
				IFIS("cfgchk") {
					CLarg.cfgchk = true;
					break;
				}
#ifdef EWMH
				IFIS("replace") {
					CLarg.ewmh_replace = true;
					break;
				}
#endif

				/* Simple value-setting */
#ifdef CAPTIVE
				IFIS("name") {
					CLarg.captivename = optarg;
					break;
				}
#endif
				IFIS("clientId") {
					CLarg.client_id = optarg;
					break;
				}
				IFIS("restore") {
					CLarg.restore_filename = optarg;
					break;
				}

				/* Some immediate actions */
				IFIS("version") {
					print_version();
					exit(0);
				}
				IFIS("info") {
					DisplayInfo();
					exit(0);
				}
				IFIS("dumpcfg") {
					dump_default_config();
					exit(0);
				}

				/* Misc */
				IFIS("xrm") {
					/*
					 * Quietly ignored by us; Xlib processes it
					 * internally in XtToolkitInitialize();
					 */
					break;
				}
#undef IFIS

				/*
				 * Some choices may just be internally setting a flag.
				 * We have none right now, but leave this in case we grow
				 * more later.
				 */
				if(long_options[optidx].flag != NULL) {
					break;
				}

				/* Don't think it should be possible to get here... */
				fprintf(stderr, "Internal error in getopt: '%s' unhandled.\n",
				        long_options[optidx].name);
				usage();

			/* Something totally unexpected */
			case '?':
				/* getopt_long() already printed an error */
				usage();

			default:
				/* Uhhh...  */
				fprintf(stderr, "Internal error: getopt confused us.\n");
				usage();
		}
	}


	/* Should do it */
	return;
}


/*
 * Sanity check CLarg's
 */
void
clargs_check(void)
{

#ifdef USEM4
	/* If we're not doing m4, don't specify m4 options */
	if(!CLarg.GoThroughM4) {
		if(CLarg.KeepTmpFile) {
			fprintf(stderr, "--keep-defs is incompatible with --nom4.\n");
			usage();
		}
		if(CLarg.keepM4_filename) {
			fprintf(stderr, "--keep is incompatible with --nom4.\n");
			usage();
		}
	}
#endif

#ifdef CAPTIVE
	/* If we're not captive, captivename is meaningless too */
	if(CLarg.captivename && !CLarg.is_captive) {
		fprintf(stderr, "--name is meaningless without --window.\n");
		usage();
	}

	/*
	 * Being captive and --cfgchk'ing is kinda meaningless.  There's no
	 * reason to create a window just to destroy things, and it never
	 * adds anything.  And it's one more way we're forcing changes on the
	 * X side before we parse the actual config, so let's just disallow
	 * it.
	 */
	if(CLarg.is_captive && CLarg.cfgchk) {
		fprintf(stderr, "--window is incompatible with --cfgchk.\n");
		usage();
	}
#endif

	/* Guess that's it */
	return;
}


/*
 * Small utils only currently used in this file.  Over time they may need
 * to be exported, if we start using them from more places.
 */
static void
usage(void)
{
	/* How far to indent continuation lines */
	int llen = 10;

	fprintf(stderr, "usage: %s [(--display | -d) dpy]  "
#ifdef EWMH
	        "[--replace]  "
#endif
	        "[--single]\n", ProgramName);

	fprintf(stderr, "%*s[(--file | -f) initfile]  [--cfgchk]  [--dumpcfg]\n",
	        llen, "");

#ifdef USEM4
	fprintf(stderr, "%*s[--nom4 | -n]  [--keep-defs | -k]  "
	        "[(--keep | -K) m4file]\n", llen, "");
#endif

	fprintf(stderr, "%*s[--verbose | -v]  [--quiet | -q]  [--mono]  "
	        "[--xrm resource]\n", llen, "");

	fprintf(stderr, "%*s[--version]  [--info]  [--nowelcome | -W]\n",
	        llen, "");

#ifdef CAPTIVE
	fprintf(stderr, "%*s[(--window | -w) [win-id]]  [--name name]\n", llen, "");
#endif

	/* Semi-intentionally not documenting --clientId/--restore */

	fprintf(stderr, "%*s[--help]\n", llen, "");


	exit(1);
}


static void
print_version(void)
{
	printf("ctwm %s\n", VersionNumberFull);
	if(VCSType && VCSRevision) {
		printf(" (%s:%s)\n", VCSType, VCSRevision);
	}
}


static void
DisplayInfo(void)
{
	char *ctopts;

	printf("Twm version:  %s\n", TwmVersion);

	ctopts = ctopts_string(" ");
	printf("Compile time options : %s\n", ctopts);
	free(ctopts);
}


static void
dump_default_config(void)
{
	int i;

	for(i = 0 ; defTwmrc[i] != NULL ; i++) {
		printf("%s\n", defTwmrc[i]);
	}
}
