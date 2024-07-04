#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "xload.h"

/* Not all OS supports get_rload
   steal the STUB idea from get_load
 */
#ifndef HAVE_PROTOCOLS_RWHOD_H
#define RLOADSTUB
#endif

#ifdef RLOADSTUB
void GetRLoadPoint(
    Widget	w,		/* unused */
    XtPointer	closure,	/* unused */
    XtPointer	call_data)	/* pointer to (double) return value */

{
  *(double *)call_data = 1.0;
}
#else  /* RLOADSTUB */

#include <protocols/rwhod.h>
#ifndef _PATH_RWHODIR
#define _PATH_RWHODIR "/var/spool/rwho"
#endif

#define WHDRSIZE        ((int)(sizeof (buf) - sizeof (buf.wd_we)))

void GetRLoadPoint(
    Widget	w,		/* unused */
    XtPointer	closure,	/* unused */
    XtPointer	call_data)	/* pointer to (double) return value */
{
  int f;
  static char *fname = NULL;
  static struct whod buf;
  int cc;

  *(double *)call_data = 0.0; /* to be on the safe side */

  if (fname == NULL) {
#ifdef HAVE_ASPRINTF
    if (asprintf(&fname, "%s/whod.%s", _PATH_RWHODIR, resources.remote) < 0) {
      perror("GetRLoadPoint: asprintf() failed");
      exit(1);
    }
#else
    if ((fname = malloc(strlen(_PATH_RWHODIR)+strlen("/whod.")+strlen(resources.remote)+1)) == NULL) {
      fprintf(stderr,"GetRLoadPoint: malloc() failed\n");
      exit(1);
    }
    strcpy(fname,_PATH_RWHODIR);
    strcat(fname,"/whod.");
    strcat(fname,resources.remote);
#endif
  }
  if ((f = open(fname, O_RDONLY, 0)) < 0)
    return;

  cc = read(f, &buf, sizeof(buf));
  close(f);
  if (cc < WHDRSIZE)
    return;

  *(double *)call_data = buf.wd_loadav[0] / 100.0;
}

#endif  /* RLOADSTUB */
