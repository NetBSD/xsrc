/* xtermcfg.h.  Generated automatically by configure.  */
/* $XTermId: xtermcfg.hin,v 1.228 2023/01/26 00:54:10 tom Exp $ */

/*
 * Copyright 1997-2023,2024 by Thomas E. Dickey
 *
 *                         All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization.
 */

#ifndef included_xtermcfg_h
#define included_xtermcfg_h 1

/* This is a template for <xtermcfg.h> */

/*
 * There are no configure options for these features:
 * ALLOWLOGFILECHANGES
 * ALLOWLOGFILEONOFF
 * CANT_OPEN_DEV_TTY
 * DEBUG* (any debug-option)
 * DUMP_* (mostly in the ReGIS/SIXEL code)
 * HAS_LTCHARS
 * PUCC_PTYD
 * USE_LOGIN_DASH_P
 * USE_X11TERM
 */

/* #undef ALLOWLOGFILEEXEC */		/* CF_ARG_ENABLE(enable-logfile-exec) */
/* #undef ALLOWLOGGING */		/* CF_ARG_ENABLE(enable-logging) */
#define CC_HAS_PROTOS 1		/* CF_ANSI_CC */
/* #undef CSRG_BASED */		/* CF_TYPE_FD_MASK */
/* #undef DECL_ERRNO */		/* CF_ERRNO */
#define DEFDELETE_DEL Maybe		/* AC_ARG_ENABLE(delete-is-del) */
#define DEF_ALT_SENDS_ESC False	/* CF_ARG_ENABLE(alt-sends-esc) */
#define DEF_BACKARO_BS True		/* CF_ARG_DISABLE(backarrow-key) */
#define DEF_BACKARO_ERASE False	/* CF_ARG_ENABLE(backarrow-is-erase) */
#define DEF_INITIAL_ERASE False	/* CF_ARG_DISABLE(initial-erase) */
#define DEF_META_SENDS_ESC False	/* CF_ARG_ENABLE(meta-sends-esc) */
/* #undef DFT_COLORMODE */		/* AC_ARG_WITH(default-color-mode) */
#define DFT_DECID "420"		/* AC_ARG_WITH(default-terminal-id) */
#define DFT_TERMTYPE "xterm"		/* AC_ARG_WITH(default-term-type) */
/* #undef DISABLE_SETGID */		/* CF_ARG_DISABLE(setgid) */
/* #undef DISABLE_SETUID */		/* CF_ARG_DISABLE(setuid) */
#define HAVE_CLOCK_GETTIME 1	/* CF_FUNC_GETTIME */
#define HAVE_ENDUSERSHELL 1	/* AC_CHECK_FUNCS(endusershell) */
#define HAVE_GETHOSTNAME 1		/* AC_CHECK_FUNCS(gethostname) */
#define HAVE_GETLOGIN 1		/* AC_CHECK_FUNCS(getlogin) */
#define HAVE_GETTIMEOFDAY 1	/* CF_FUNC_GETTIME */
#define HAVE_GETUSERSHELL 1	/* AC_CHECK_FUNCS(getusershell) */
#define HAVE_GRANTPT 1		/* CF_FUNC_GRANTPT */
/* #undef HAVE_GRANTPT_PTY_ISATTY */	/* CF_FUNC_GRANTPT */
#define HAVE_INITGROUPS 1		/* AC_CHECK_FUNCS(initgroups) */
#define HAVE_LANGINFO_CODESET 1	/* AM_LANGINFO_CODESET */
/* #undef HAVE_LASTLOG_H */		/* CF_LASTLOG */
/* #undef HAVE_LIBUTIL_H */		/* AC_CHECK_HEADERS(libutil.h) */
#define HAVE_LIBXPM 1		/* CF_WITH_XPM */
/* #undef HAVE_LIB_NEXTAW */		/* CF_X_ATHENA(--with-neXtaw) */
/* #undef HAVE_LIB_PCRE */		/* CF_WITH_PCRE */
/* #undef HAVE_LIB_PCRE2 */		/* CF_WITH_PCRE2 */
#define HAVE_LIB_XAW 1		/* CF_X_ATHENA */
/* #undef HAVE_LIB_XAW3D */		/* CF_X_ATHENA(--with-Xaw3d) */
/* #undef HAVE_LIB_XAW3DXFT */	/* CF_X_ATHENA(--with-Xaw3dxft) */
/* #undef HAVE_LIB_XAWPLUS */		/* CF_X_ATHENA(--with-XawPlus) */
#define HAVE_LIB_XCURSOR 1		/* AC_CHECK_LIB(Xcursor) */
#define HAVE_MKDTEMP 1		/* AC_CHECK_FUNCS(mkdtemp) */
/* #undef HAVE_NCURSES_CURSES_H */	/* AC_CHECK_HEADERS(ncurses/curses.h) */
/* #undef HAVE_NCURSES_TERM_H */	/* AC_CHECK_HEADERS(ncurses/term.h) */
/* #undef HAVE_PATHS_H */		/* AC_CHECK_HEADERS(paths.h) */
#define HAVE_PATHS_H 1		/* CF_LASTLOG */
/* #undef HAVE_PCRE2POSIX_H */	/* AC_CHECK_HEADERS(pcre2posix.h) */
/* #undef HAVE_PCRE2POSIX_H */	/* CF_WITH_PCRE2 */
/* #undef HAVE_PCRE2REGCOMP */	/* CF_WITH_PCRE2 */
/* #undef HAVE_PCREPOSIX_H */		/* AC_CHECK_HEADERS(pcreposix.h) */
/* #undef HAVE_PCREPOSIX_H */		/* CF_WITH_PCRE */
#define HAVE_POSIX_OPENPT 1	/* CF_FUNC_GRANTPT */
#define HAVE_POSIX_SAVED_IDS 1	/* CF_POSIX_SAVED_IDS */
#define HAVE_PTSNAME 1		/* CF_FUNC_GRANTPT */
#define HAVE_PTY_H 1		/* AC_CHECK_HEADERS(pty.h) */
#define HAVE_PUTENV 1		/* AC_CHECK_FUNCS(putenv) */
#define HAVE_SCHED_YIELD 1		/* AC_CHECK_FUNCS(sched_yield) */
#define HAVE_SETPGID 1		/* AC_CHECK_FUNCS(setpgid) */
#define HAVE_STDINT_H 1		/* AC_PROG_CC_STDC */
#define HAVE_STDLIB_H 1		/* AC_CHECK_HEADERS(stdlib.h) */
#if 0
/*
 * clang treats _Noreturn and __attribute__((__noreturn__)) differently,
 * so we end up labeling xt_error() with _Noreturn and using it in
 * XtSetErrorHandler which is marked with the attribute noreturn, and clang
 * complains. IMHO this is a bug in clang. We tell everyone that we don't
 * have the header, so we use the attribute consistently and everyone is happy.
 */
#define HAVE_STDNORETURN_H 1		/* CF_C11_NORETURN */
#endif
#define HAVE_STRFTIME 1		/* AC_CHECK_FUNCS(strftime) */
/* #undef HAVE_STROPTS_H */		/* AC_CHECK_HEADERS(stropts.h) */
#define HAVE_SYS_PARAM_H 1		/* AC_CHECK_HEADERS(sys/param.h) */
/* #undef HAVE_SYS_PTEM_H */		/* AC_CHECK_HEADERS(sys/ptem.h) */
/* #undef HAVE_SYS_TIME_H */		/* AC_HEADER_TIME */
#define HAVE_SYS_TTYDEFAULTS_H 1	/* AC_CHECK_HEADERS(sys/ttydefaults.h) */
#define HAVE_SYS_WAIT_H 1		/* AC_HEADER_SYS_WAIT */
#define HAVE_TCGETATTR 1		/* AC_CHECK_FUNCS(tcgetattr) */
#define HAVE_TERMCAP_H 1		/* AC_CHECK_HEADERS(termcap.h) */
#define HAVE_TERMIOS_H 1		/* AC_CHECK_HEADERS(termios.h) */
/* #undef HAVE_TERMIO_C_ISPEED */	/* CF_TERMIO_C_ISPEED */
#define HAVE_TERM_H 1		/* AC_CHECK_HEADERS(term.h) */
#define HAVE_TIGETSTR 1		/* AC_CHECK_FUNCS(tigetstr) */
#define HAVE_UNISTD_H 1		/* AC_CHECK_HEADERS(unistd.h) */
#define HAVE_UNSETENV 1		/* AC_CHECK_FUNCS(unsetenv) */
/* #undef HAVE_USE_EXTENDED_NAMES */	/* AC_CHECK_FUNCS(use_extended_names) */
#define HAVE_UTIL_H 1		/* AC_CHECK_HEADERS(util.h) */
#define HAVE_UTMP 1		/* CF_UTMP */
#define HAVE_UTMP_UT_HOST 1	/* CF_UTMP_UT_HOST */
#define HAVE_UTMP_UT_SESSION 1	/* CF_UTMP_UT_SESSION */
/* #undef HAVE_UTMP_UT_SYSLEN */	/* CF_UTMP_UT_SYSLEN */
#define HAVE_UTMP_UT_XSTATUS 1	/* CF_UTMP_UT_XSTATUS */
#define HAVE_UTMP_UT_XTIME 1	/* CF_UTMP_UT_XTIME */
#define HAVE_WAITPID 1		/* AC_CHECK_FUNCS(waitpid) */
#define HAVE_WCHAR_H 1		/* AC_CHECK_HEADERS(wchar.h) */
#define HAVE_WCSWIDTH 1		/* AC_CHECK_FUNCS(wcswidth) */
#define HAVE_WCWIDTH 1		/* AC_CHECK_FUNCS(wcwidth) */
#define HAVE_X11_DECKEYSYM_H 1	/* AC_CHECK_HEADERS(X11/DECkeysym.h) */
#define HAVE_X11_EXTENSIONS_XDBE_H 1 /* AC_CHECK_HEADERS(X11/extensions/Xdbe.h) */
#define HAVE_X11_EXTENSIONS_XINERAMA_H 1 /* AC_CHECK_HEADERS(X11/extensions/Xinerama.h) */
#define HAVE_X11_EXTENSIONS_XKB_H 1 /* AC_CHECK_HEADERS(X11/extensions/XKB.h) */
#define HAVE_X11_SUNKEYSYM_H 1	/* AC_CHECK_HEADERS(X11/Sunkeysym.h) */
#define HAVE_X11_TRANSLATEI_H 1	/* AC_CHECK_HEADERS(X11/TranslateI.h) */
#define HAVE_X11_XF86KEYSYM_H 1	/* AC_CHECK_HEADERS(X11/XF86keysym.h) */
#define HAVE_X11_XKBLIB_H 1	/* AC_CHECK_HEADERS(X11/XKBlib.h) */
#define HAVE_X11_XPOLL_H 1		/* AC_CHECK_HEADERS(X11/Xpoll.h) */
#define HAVE_XDBESWAPBUFFERS 1	/* AC_CHECK_FUNC(XdbeSwapBuffers) */
#define HAVE_XFTDRAWSETCLIP 1	/* CF_X_FREETYPE */
#define HAVE_XFTDRAWSETCLIPRECTANGLES 1 /* CF_X_FREETYPE */
#define HAVE_XKBKEYCODETOKEYSYM 1	/* AC_CHECK_FUNCS(XkbKeycodeToKeysym) */
#define HAVE_XKBQUERYEXTENSION 1	/* AC_CHECK_FUNCS(XkbQueryExtension) */
#define HAVE_XKB_BELL_EXT 1	/* CF_XKB_BELL_EXT */
#define LUIT_PATH "/usr/X11R7/bin/luit"		/* CF_ARG_ENABLE(luit) */
/* #undef NO_ACTIVE_ICON */		/* CF_ARG_DISABLE(active-icon) */
/* #undef NO_LEAKS */			/* CF_ARG_DISABLE(leaks) */
#define OPT_256_COLORS 1		/* CF_ARG_ENABLE(256-color) */
/* #undef OPT_88_COLORS */		/* CF_ARG_ENABLE(88-color) */
/* #undef OPT_AIX_COLORS */		/* CF_ARG_DISABLE(16-color) */
/* #undef OPT_BLINK_CURS */		/* CF_ARG_DISABLE(blink-cursor) */
/* #undef OPT_BLINK_TEXT */		/* CF_ARG_DISABLE(blink-text) */
/* #undef OPT_BOX_CHARS */		/* CF_ARG_DISABLE(boxchars) */
#define OPT_BROKEN_OSC 0		/* CF_ARG_ENABLE(broken-osc) */
/* #undef OPT_BROKEN_ST */		/* CF_ARG_DISABLE(broken-st) */
/* #undef OPT_BUILTIN_XPMS */		/* CF_ARG_ENABLE(builtin-xpms) */
/* #undef OPT_C1_PRINT */		/* CF_ARG_DISABLE(c1-print) */
/* #undef OPT_COLOR_CLASS */		/* CF_ARG_DISABLE(color-class) */
/* #undef OPT_DABBREV */		/* CF_ARG_ENABLE(dabbrev) */
/* #undef OPT_DEC_CHRSET */		/* CF_ARG_DISABLE(doublechars) */
/* #undef OPT_DEC_LOCATOR */		/* CF_ARG_ENABLE(dec-locator) */
#define OPT_DEC_RECTOPS 1		/* CF_ARG_DISABLE(rectangles) */
#define OPT_DIRECT_COLOR 1		/* CF_ARG_ENABLE(direct-color) */
/* #undef OPT_DOUBLE_BUFFER */	/* CF_ARG_ENABLE(double-buffer) */
/* #undef OPT_EXEC_XTERM */		/* CF_ARG_ENABLE(exec-xterm) */
#define OPT_GRAPHICS 1		/* CF_ARG_ENABLE(graphics) */
/* #undef OPT_HIGHLIGHT_COLOR */	/* CF_ARG_DISABLE(highlighting) */
/* #undef OPT_HP_FUNC_KEYS */		/* CF_ARG_ENABLE(hp-fkeys) */
/* #undef OPT_I18N_SUPPORT */		/* CF_ARG_DISABLE(i18n) */
/* #undef OPT_INITIAL_ERASE */	/* CF_ARG_DISABLE(initial-erase) */
/* #undef OPT_INPUT_METHOD */		/* CF_ARG_DISABLE(input-method) */
/* #undef OPT_ISO_COLORS */		/* CF_ARG_DISABLE(ansi-color) */
/* #undef OPT_LOAD_VTFONTS */		/* CF_ARG_ENABLE(load-vt-fonts) */
#define OPT_LUIT_PROG 1		/* CF_ARG_ENABLE(luit) */
/* #undef OPT_MAXIMIZE */		/* CF_ARG_DISABLE(maximize) */
#define OPT_MINI_LUIT 1		/* CF_ARG_ENABLE(mini-luit) */
/* #undef OPT_NUM_LOCK */		/* CF_ARG_DISABLE(num-lock) */
#define OPT_PASTE64 1		/* CF_ARG_ENABLE(past64) */
/* #undef OPT_PC_COLORS */		/* CF_ARG_DISABLE(pc-color) */
/* #undef OPT_PRINT_GRAPHICS */	/* CF_ARG_ENABLE(print-graphics) */
#define OPT_PTY_HANDSHAKE 1	/* CF_ARG_ENABLE(pty-handshake) */
#define OPT_READLINE 1		/* CF_ARG_ENABLE(readline-mouse) */
/* #undef OPT_REGIS_GRAPHICS */	/* CF_ARG_ENABLE(regis-graphics) */
/* #undef OPT_SAME_NAME */		/* CF_ARG_DISABLE(samename) */
/* #undef OPT_SCO_FUNC_KEYS */	/* CF_ARG_ENABLE(sco-fkeys) */
/* #undef OPT_SCREEN_DUMPS */		/* CF_ARG_ENABLE(screen-dumps) */
/* #undef OPT_SELECTION_OPS */	/* CF_ARG_DISABLE(selection-ops) */
#define OPT_SELECT_REGEX 1		/* CF_ARG_DISABLE(regex) */
/* #undef OPT_SESSION_MGT */		/* CF_ARG_DISABLE(session-mgt) */
#define OPT_SIXEL_GRAPHICS 1	/* CF_ARG_ENABLE(sixel-graphics) */
#define OPT_STATUS_LINE 1	/* CF_ARG_ENABLE(status-line) */
/* #undef OPT_SUN_FUNC_KEYS */	/* CF_ARG_ENABLE(sun-fkeys) */
#define OPT_TCAP_FKEYS 1		/* CF_ARG_ENABLE(tcap-fkeys) */
#define OPT_TCAP_QUERY 1		/* CF_ARG_ENABLE(tcap-query) */
/* #undef OPT_TEK4014 */		/* CF_ARG_DISABLE(tek4014) */
/* #undef OPT_TOOLBAR */		/* CF_ARG_ENABLE(toolbar) */
/* #undef OPT_VT52_MODE */		/* CF_ARG_DISABLE(vt52) */
/* #undef OPT_WIDER_ICHAR */		/* CF_ARG_ENABLE(16bit-chars) */
#define OPT_WIDE_ATTRS 1		/* CF_ARG_DISABLE(wide-attrs) */
#define OPT_WIDE_CHARS 1		/* CF_ARG_DISABLE(wide-chars) */
/* #undef OPT_XMC_GLITCH */		/* CF_ARG_ENABLE(xmc-glitch) */
/* #undef OPT_ZICONBEEP */		/* CF_ARG_DISABLE(ziconbeep) */
/* #undef OWN_TERMINFO_DIR */		/* AC_ARG_WITH(own-terminfo) */
/* #undef OWN_TERMINFO_ENV */		/* AC_ARG_ENABLE(env-terminfo) */
/* #undef PROCFS_ROOT */		/* CF_ARG_ENABLE(exec-xterm) */
#define SCROLLBAR_RIGHT 1		/* CF_ARG_ENABLE(rightbar) */
#define SIG_ATOMIC_T volatile sig_atomic_t		/* CF_SIG_ATOMIC_T */
#define STDC_NORETURN _Noreturn			/* CF_C11_NORETURN */
/* #undef SVR4 */			/* CF_SVR4, imake */
/* #undef SYSV */			/* CF_SYSV, imake */
#define TIME_WITH_SYS_TIME 1	/* AC_HEADER_TIME */
#define TTY_GROUP_NAME "tty"		/* CF_TTY_GROUP */
/* #undef USE_LASTLOG */		/* CF_LASTLOG */
#define USE_POSIX_WAIT 1		/* CF_POSIX_WAIT */
/* #undef USE_STRUCT_LASTLOG */	/* CF_STRUCT_LASTLOG */
#define USE_SYSV_UTMP 1		/* CF_UTMP */
/* #undef USE_SYS_SELECT_H */		/* CF_TYPE_FD_SET */
/* #undef USE_TERMCAP */		/* CF_FUNC_TGETENT */
#define USE_TERMINFO 1		/* CF_FUNC_TGETENT */
#define USE_TTY_GROUP 1		/* CF_TTY_GROUP */
/* #undef USE_UTEMPTER */		/* CF_UTEMPTER */
/* #undef USE_UTMP_SETGID */		/* AC_ARG_WITH(utmp-setgid) */
#define UTMPX_FOR_UTMP 1		/* CF_UTMP */
#define XRENDERFONT 1		/* CF_X_FREETYPE */
/* #undef cc_t */			/* CF_TYPE_CC_T */
/* #undef gid_t */			/* AC_TYPE_UID_T */
/* #undef mode_t */			/* AC_TYPE_MODE_T */
/* #undef off_t */			/* AC_TYPE_OFF_T */
/* #undef pid_t */			/* AC_TYPE_PID_T */
/* #undef time_t */			/* AC_CHECK_TYPE(time_t, long) */
/* #undef uid_t */			/* AC_TYPE_UID_T */
/* #undef ut_name */			/* CF_UTMP */
#define ut_xstatus ut_exit.e_exit		/* CF_UTMP_UT_XSTATUS */
/* #undef ut_xtime */			/* CF_UTMP_UT_XTIME */

/*
 * Ifdef'd to make it simple to override.
 */
#ifndef OPT_TRACE
/* #undef OPT_TRACE */		/* CF_ARG_ENABLE(trace) */
/* #undef OPT_TRACE_FLAGS */		/* ...no option */
#endif

/*
 * g++ support for __attribute__() is haphazard.
 */
#ifndef __cplusplus
#define GCC_PRINTF	1
#define GCC_PRINTFLIKE(fmt,var)	__attribute__((__format__(__printf__,fmt,var)))
#define GCC_NORETURN	__attribute__((__noreturn__))
#define GCC_UNUSED	__attribute__((__unused__))
#endif

#ifndef HAVE_X11_XPOLL_H
#define NO_XPOLL_H	/* X11R6.1 & up use Xpoll.h for select() definitions */
#endif

/* vile:cmode
 */
#endif /* included_xtermcfg_h */
