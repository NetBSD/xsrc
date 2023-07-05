/*
 * Compile-time config settings.  This gets processed by cmake into the
 * file that's actually used.
 */

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
/* #undef USE_RPLAY */
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

/* Is usable xrandr available? */
#ifdef USE_XRANDR
# define XRANDR
#endif

/* Enable captive mode (ctwm -w) related bits */
/* #undef USE_CAPTIVE */
#ifdef USE_CAPTIVE
# define CAPTIVE
#endif

/* Fragments of remaining VirtualScreens support */
/* #undef USE_VSCREEN */
#ifdef USE_VSCREEN
# define VSCREEN
#endif

/* WindowBox support */
/* #undef USE_WINBOX */
#ifdef USE_WINBOX
# define WINBOX
#endif

/* Session support */
#ifdef USE_SESSION
# define SESSION
#endif
