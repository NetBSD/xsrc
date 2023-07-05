/*
 * Compile-time options
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctopts.h"


/*
 * What options we're build with
 */
static char *ctopts[] = {
	"I18N",     // Used to be optional, now standard.  Remove?
#ifdef XPM
	"XPM",
#endif
#ifdef JPEG
	"JPEG",
#endif
#ifdef USEM4
	"USEM4",
#endif
#ifdef SOUNDS
	"SOUNDS",
#endif
#ifdef EWMH
	"EWMH",
#endif
#ifdef XRANDR
	"XRANDR",
#endif
#ifdef DEBUG
	"DEBUG",
#endif
	NULL
};


/*
 * Build a string of our compile-time opts
 */
char *
ctopts_string(char *sep)
{
	char *cto;
	size_t slen, tlen;
	int i;

	/* Figure out how long our string would be */
	slen = strlen(sep);
	tlen = 0;
	i = -1;
	while(ctopts[++i]) {
		tlen += strlen(ctopts[i]);
		if(i > 0) {
			tlen += slen;
		}
	}

	/* Now make it */
	cto = malloc(tlen + 1);
	*cto = '\0';
	i = -1;
	while(ctopts[++i]) {
		if(i > 0) {
			strcat(cto, sep);
		}
		strcat(cto, ctopts[i]);
	}

	return cto;
}
