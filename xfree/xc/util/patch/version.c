/* oldHeader: version.c,v 2.0 86/09/17 15:40:11 lwall Exp $
 * $Xorg: version.c,v 1.3 2000/08/17 19:55:22 cpqbld Exp $
 *
 * Revision 2.0  86/09/17  15:40:11  lwall
 * Baseline for netwide release.
 * 
 */

#include "EXTERN.h"
#include "common.h"
#include "util.h"
#include "INTERN.h"
#include "patchlevel.h"
#include "version.h"

void my_exit();

/* Print out the version number and die. */

void
version()
{
    fprintf(stderr, "Patch version 2.0, patch level %s\n", PATCHLEVEL);
    my_exit(0);
}
