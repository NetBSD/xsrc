/* oldHeader: INTERN.h,v 2.0 86/09/17 15:35:58 lwall Exp $
 * $Xorg: INTERN.h,v 1.3 2000/08/17 19:55:21 cpqbld Exp $
 *
 * Revision 2.0  86/09/17  15:35:58  lwall
 * Baseline for netwide release.
 * 
 */

#ifdef EXT
#undef EXT
#endif
#define EXT

#ifdef INIT
#undef INIT
#endif
#define INIT(x) = x

#define DOINIT
