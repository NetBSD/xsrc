/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * Copyright 1992 Claude Lecommandeur.
 */

#include "ctwm.h"
#include "version.h"

#define VERSION_MAJOR "4"
#define VERSION_MINOR "1"
#define VERSION_PATCH "0"
#define VERSION_ADDL  ""

#define VERSION_ID VERSION_MAJOR "." VERSION_MINOR "." VERSION_PATCH
#define VERSION_ID_FULL VERSION_ID VERSION_ADDL

const char *TwmVersion = "MIT X Consortium, R6, ctwm " VERSION_ID_FULL;
const char *VersionNumber = VERSION_ID;
const char *VersionNumberFull = VERSION_ID_FULL;
const char *VersionNumber_major = VERSION_MAJOR;
const char *VersionNumber_minor = VERSION_MINOR;
const char *VersionNumber_patch = VERSION_PATCH;
const char *VersionNumber_addl  = VERSION_ADDL;
const char *VCSType     = "bzr";
const char *VCSRevision = "fullermd@over-yonder.net-20230326223622-nydwio13vncluzwi";

#ifdef BUILD_VERSION_BIN
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	int ch;

	if((ch = getopt(argc, argv, "Mmpa")) != -1) {
		switch(ch) {
			case 'M':
				printf("%s\n", VersionNumber_major);
				exit(0);
			case 'm':
				printf("%s\n", VersionNumber_minor);
				exit(0);
			case 'p':
				printf("%s\n", VersionNumber_patch);
				exit(0);
			case 'a':
				printf("%s\n", VersionNumber_addl);
				exit(0);
			default:
				printf("Dunno.\n");
				exit(1);
		}
	}

	printf("%s\n", VersionNumberFull);
	exit(0);
}
#endif /* BUILD_VERSION_BIN */
