/* $XFree86: xc/config/util/mktemp-dgux.c,v 1.1.2.2 1999/08/18 14:42:29 hohndel Exp $ */
/*****************************************************************************
 * DGUX Release 4.20MU04 (ix86)
 * <takis@dpmms.cam.ac.uk>
 * mktemp for XFree86 3.3.5
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#define DG_PROGNAME "mktemp"

void err(int somecode, char * message, ...) {
    va_list arglist;

    va_start(arglist, message);
    vfprintf(stderr, message, arglist);
    fprintf(stderr, "\n");
    va_end(arglist);

    exit(somecode);
}

void errx(int somecode, char * message) {
    err(somecode, message);
}

void
usage()
{
   (void) fprintf(stderr, "UX:%s: TO FIX: usage: %s [-q] [-u] template\n",
				DG_PROGNAME, DG_PROGNAME);
   exit(1);
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	char *template;
	int c, uflag = 0, qflag = 0, makedir = 0;

	while ((c = getopt(argc, argv, "dqu")) != -1)
		switch(c) {
		case 'd':
			makedir = 1;
			break;
		case 'q':
			qflag = 1;
			break;
		case 'u':
			uflag = 1;
			break;
		case '?':
		default:
			usage();
	}

	if (argc - optind != 1)
		usage();

	if ((template = strdup(argv[optind])) == NULL) {
		if (qflag)
			exit(1);
		else
			errx(1, "Cannot allocate memory");
	}

	if (makedir)
	{
	   fprintf(stderr, "UX:%s: ERROR: Illegal option: -d is not supported on DG/ux\n",
					DG_PROGNAME);
 	   exit(1);
	}
	else
	{
		if (mkstemp(template) < 0)
		{
			if (qflag)
				exit(1);
			else
				err(1, "Cannot create temp file %s", template);
		}

		if (uflag)
			(void) unlink(template);
	}

	(void) puts(template);
	free(template);

	exit(0);
}
