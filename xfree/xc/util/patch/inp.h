/* oldHeader: inp.h,v 2.0 86/09/17 15:37:25 lwall Exp $
 * $Xorg: inp.h,v 1.3 2000/08/17 19:55:22 cpqbld Exp $
 *
 * Revision 2.0  86/09/17  15:37:25  lwall
 * Baseline for netwide release.
 * 
 */

EXT LINENUM input_lines INIT(0);	/* how long is input file in lines */
EXT LINENUM last_frozen_line INIT(0);	/* how many input lines have been */
					/* irretractibly output */

bool rev_in_string();
void scan_input();
bool plan_a();			/* returns false if insufficient memory */
void plan_b();
char *ifetch();

