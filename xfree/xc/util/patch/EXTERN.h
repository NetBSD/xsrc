/* oldHeader: EXTERN.h,v 2.0 86/09/17 15:35:37 lwall Exp $
 * $Xorg: EXTERN.h,v 1.3 2000/08/17 19:55:21 cpqbld Exp $
 *
 * Revision 2.0  86/09/17  15:35:37  lwall
 * Baseline for netwide release.
 * 
 */

#ifdef EXT
#undef EXT
#endif
#define EXT extern

#ifdef INIT
#undef INIT
#endif
#define INIT(x)

#ifdef DOINIT
#undef DOINIT
#endif
