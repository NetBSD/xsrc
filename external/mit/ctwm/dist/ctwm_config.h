/* $NetBSD: ctwm_config.h,v 1.1 2021/04/11 10:11:57 nia Exp $ */

/*
 * Various on/off configs
 */

/* Using XPM? */
#ifdef USE_XPM
# define XPM
#endif

/* libjpeg */
#ifdef USE_JPEG
# define JPEG
#endif

/* m4 preprocessing of config */
#ifdef USE_M4
# define USEM4
#endif

/* rplay? */
#ifdef USE_RPLAY
/*
 * This mismatched naming is a historical remnant.  User-facing stuff
 * (build-time config, config file params, etc) has been moved as much as
 * possible to RPLAY-based to start building up compatibility for any
 * future where we support other sound methods.  I've left internal stuff
 * based around SOUNDS / sound.c / something_sound_something() / etc
 * since that would be a lot of churn, and there aren't the compat
 * concerns so we can just do those at the time they become necessary
 * without worrying further.
 */
# define SOUNDS
#endif

/* How about EWMH properties */
#ifdef USE_EWMH
# define EWMH
#endif

/* Does libc provide regex funcs we use? */
#ifdef USE_SREGEX
# define USE_SYS_REGEX
#endif
