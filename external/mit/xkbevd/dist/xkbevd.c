/************************************************************
 Copyright (c) 1995 by Silicon Graphics Computer Systems, Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of Silicon Graphics not be
 used in advertising or publicity pertaining to distribution
 of the software without specific prior written permission.
 Silicon Graphics makes no representation about the suitability
 of this software for any purpose. It is provided "as is"
 without any express or implied warranty.

 SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 THE USE OR PERFORMANCE OF THIS SOFTWARE.

 ********************************************************/

#include <X11/Xosdefs.h>
#include <stdlib.h>
#include "xkbevd.h"

/***====================================================================***/

#ifndef DFLT_XKBEVD_CONFIG
#define DFLT_XKBEVD_CONFIG "%s/.xkb/xkbevd.cf"
#endif /* DFLT_XKBEVD_CONFIG */

#ifndef DFLT_XKB_CONFIG_ROOT
#define	DFLT_XKB_CONFIG_ROOT "/usr/share/X11/xkb"
#endif

#ifndef DFLT_SYS_XKBEVD_CONFIG
#define DFLT_SYS_XKBEVD_CONFIG "%s/xkbevd.cf"
#endif /* DFLT_SYS_XKBEVD_CONFIG */

#ifndef DFLT_SOUND_CMD
#define	DFLT_SOUND_CMD "/usr/sbin/sfplay -q"
#endif /* DFLT_SOUND_CMD */

#ifndef DFLT_SOUND_DIR
#define	DFLT_SOUND_DIR "/usr/share/data/sounds/prosonus/"
#endif /* DFLT_SOUND_DIR */

/***====================================================================***/

static char *   dpyName = NULL;
Display *       dpy = NULL;
static const char *cfgFileName = NULL;
int             xkbOpcode = 0;
int             xkbEventCode = 0;
Bool            detectableRepeat = False;

static CfgEntryPtr config = NULL;
static unsigned long eventMask = 0;

static Bool     synch = False;
static int      verbose = 0;
static Bool     background = False;

static const char *soundCmd = NULL;
static const char *soundDir = NULL;

XkbDescPtr      xkb = NULL;

/***====================================================================***/

static void
Usage(int argc, char *argv[])
{
    fprintf(stderr, "Usage: %s [options]...\n%s", argv[0],
            "Legal options:\n"
            "-?, -help            Print this message\n"
            "-cfg <file>          Specify a config file\n"
            "-sc <cmd>            Specify the command to play sounds\n"
            "-sd <dir>            Specify the root directory for sound files\n"
            "-d[isplay] <dpy>     Specify the display to watch\n"
            "-bg                  Run in background\n"
            "-synch               Force synchronization\n"
            "-v                   Print verbose messages\n"
            "-version             Print program version\n"
        );
    return;
}

/***====================================================================***/

static Bool
parseArgs(int argc, char *argv[])
{
    register int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-bg") == 0) {
            background = True;
        }
        else if (strcmp(argv[i], "-cfg") == 0) {
            if (i >= (argc - 1)) {
                uError("No configuration file specified on command line\n");
                uAction("Trailing %s argument ignored\n", argv[i]);
            }
            else {
                char *name = argv[++i];

                if (cfgFileName != NULL) {
                    if (uStringEqual(cfgFileName, name))
                        uWarning("Config file \"%s\" specified twice!\n", name);
                    else {
                        uWarning("Multiple config files on command line\n");
                        uAction("Using \"%s\", ignoring \"%s\"\n", name,
                                cfgFileName);
                    }
                }
                cfgFileName = name;
            }
        }
        else if ((strcmp(argv[i], "-d") == 0) ||
                 (strcmp(argv[i], "-display") == 0)) {
            if (i >= (argc - 1)) {
                uError("No display specified on command line\n");
                uAction("Trailing %s argument ignored\n", argv[i]);
            }
            else {
                char *name = argv[++i];

                if (dpyName != NULL) {
                    if (uStringEqual(dpyName, name))
                        uWarning("Display \"%s\" specified twice!\n", name);
                    else {
                        uWarning("Multiple displays on command line\n");
                        uAction("Using \"%s\", ignoring \"%s\"\n", name,
                                dpyName);
                    }
                }
                dpyName = name;
            }
        }
        else if (strcmp(argv[i], "-sc") == 0) {
            if (i >= (argc - 1)) {
                uError("No sound command specified on command line\n");
                uAction("Trailing %s argument ignored\n", argv[i]);
            }
            else {
                char *name = argv[++i];

                if (soundCmd != NULL) {
                    if (uStringEqual(soundCmd, name))
                        uWarning("Sound command \"%s\" specified twice!\n",
                                 name);
                    else {
                        uWarning("Multiple sound commands on command line\n");
                        uAction("Using \"%s\", ignoring \"%s\"\n", name,
                                soundCmd);
                    }
                }
                soundCmd = name;
            }
        }
        else if (strcmp(argv[i], "-sd") == 0) {
            if (i >= (argc - 1)) {
                uError("No sound directory specified on command line\n");
                uAction("Trailing %s argument ignored\n", argv[i]);
            }
            else {
                char *name = argv[++i];

                if (soundDir != NULL) {
                    if (uStringEqual(soundDir, name))
                        uWarning("Sound directory \"%s\" specified twice!\n",
                                 name);
                    else {
                        uWarning("Multiple sound dirs on command line\n");
                        uAction("Using \"%s\", ignoring \"%s\"\n", name,
                                soundDir);
                    }
                }
                soundDir = name;
            }
        }
        else if ((strcmp(argv[i], "-synch") == 0) ||
                 (strcmp(argv[i], "-s") == 0)) {
            synch = True;
        }
        else if (strcmp(argv[i], "-v") == 0) {
            verbose++;
        }
        else if (strcmp(argv[i], "-version") == 0) {
            puts(PACKAGE_STRING);
            exit(0);
        }
        else if ((strcmp(argv[i], "-?") == 0) ||
                 (strcmp(argv[i], "-help") == 0)) {
            Usage(argc, argv);
            exit(0);
        }
        else {
            uError("Unknown flag \"%s\" on command line\n", argv[i]);
            Usage(argc, argv);
            return False;
        }
    }
    if (background == False) {
        eventMask = XkbAllEventsMask;
        verbose++;
    }

    return True;
}

static Display *
GetDisplay(const char *program, const char *displayName,
           int *opcodeRtrn, int *evBaseRtrn)
{
    int mjr, mnr, error;
    Display *display;

    mjr = XkbMajorVersion;
    mnr = XkbMinorVersion;
    display = XkbOpenDisplay(displayName, evBaseRtrn, NULL, &mjr, &mnr, &error);
    if (display == NULL) {
        switch (error) {
        case XkbOD_BadLibraryVersion:
            uInformation("%s was compiled with XKB version %d.%02d\n",
                         program, XkbMajorVersion, XkbMinorVersion);
            uError("X library supports incompatible version %d.%02d\n",
                   mjr, mnr);
            break;
        case XkbOD_ConnectionRefused:
            uError("Cannot open display \"%s\"\n", displayName);
            break;
        case XkbOD_NonXkbServer:
            uError("XKB extension not present on %s\n", displayName);
            break;
        case XkbOD_BadServerVersion:
            uInformation("%s was compiled with XKB version %d.%02d\n",
                         program, XkbMajorVersion, XkbMinorVersion);
            uError("Server %s uses incompatible version %d.%02d\n",
                   displayName, mjr, mnr);
            break;
        default:
            uInternalError("Unknown error %d from XkbOpenDisplay\n", error);
        }
    }
    else if (synch)
        XSynchronize(display, True);
    if (opcodeRtrn)
        XkbQueryExtension(display, opcodeRtrn, evBaseRtrn, NULL, &mjr, &mnr);
    return display;
}

/***====================================================================***/

void
InterpretConfigs(CfgEntryPtr cfg)
{
    config = cfg;
    while (cfg != NULL) {
        char *name = cfg->name.str;
        if (cfg->entry_type == VariableDef) {
            if (uStrCaseEqual(name, "sounddirectory") ||
                uStrCaseEqual(name, "sounddir")) {
                if (soundDir == NULL) {
                    soundDir = cfg->action.text;
                    cfg->name.str = NULL;
                    cfg->action.text = NULL;
                }
            }
            else if (uStrCaseEqual(name, "soundcommand") ||
                     uStrCaseEqual(name, "soundcmd")) {
                if (soundCmd == NULL) {
                    soundCmd = cfg->action.text;
                    cfg->name.str = NULL;
                    cfg->action.text = NULL;
                }
            }
            else {
                uWarning("Assignment to unknown variable \"%s\"\n", name);
                uAction("Ignored\n");
            }
        }
        else if (cfg->entry_type == EventDef) {
            unsigned int priv;

            switch (cfg->event_type) {
            case XkbBellNotify:
                if (name != NULL)
                    cfg->name.atom = XInternAtom(dpy, name, False);
                else
                    cfg->name.atom = None;
                if (name)
                    free(name);
                break;
            case XkbAccessXNotify:
                priv = 0;
                if (name == NULL)
                    priv = XkbAllNewKeyboardEventsMask;
                else if (uStrCaseEqual(name, "skpress"))
                    priv = XkbAXN_SKPressMask;
                else if (uStrCaseEqual(name, "skaccept"))
                    priv = XkbAXN_SKAcceptMask;
                else if (uStrCaseEqual(name, "skreject"))
                    priv = XkbAXN_SKRejectMask;
                else if (uStrCaseEqual(name, "skrelease"))
                    priv = XkbAXN_SKReleaseMask;
                else if (uStrCaseEqual(name, "bkaccept"))
                    priv = XkbAXN_BKAcceptMask;
                else if (uStrCaseEqual(name, "bkreject"))
                    priv = XkbAXN_BKRejectMask;
                else if (uStrCaseEqual(name, "warning"))
                    priv = XkbAXN_AXKWarningMask;
                if (name)
                    free(name);
                cfg->name.priv = priv;
                break;
            case XkbActionMessage:
                /* nothing to do */
                break;
            }
        }
        eventMask |= (1L << cfg->event_type);
        cfg = cfg->next;
    }
    while ((config) && (config->entry_type != EventDef)) {
        CfgEntryPtr next;

        if (config->name.str)
            free(config->name.str);
        if (config->action.text)
            free(config->action.text);
        config->name.str = NULL;
        config->action.text = NULL;
        next = config->next;
        free(config);
        config = next;
    }
    cfg = config;
    while ((cfg != NULL) && (cfg->next != NULL)) {
        CfgEntryPtr next;

        next = cfg->next;
        if (next->entry_type != EventDef) {
            if (next->name.str)
                free(config->name.str);
            if (next->action.text)
                free(config->action.text);
            next->name.str = NULL;
            next->action.text = NULL;
            cfg->next = next->next;
            next->next = NULL;
            free(next);
        }
        else
            cfg = next;
    }
    return;
}

static CfgEntryPtr
FindMatchingConfig(XkbEvent * ev)
{
    CfgEntryPtr cfg, dflt;

    dflt = NULL;
    for (cfg = config; (cfg != NULL); cfg = cfg->next) {
        if ((ev->type != xkbEventCode) || (cfg->event_type != ev->any.xkb_type))
            continue;
        switch (ev->any.xkb_type) {
        case XkbBellNotify:
            if (ev->bell.name == cfg->name.atom)
                return cfg;
            else if ((cfg->name.atom == None) && (dflt == NULL))
                dflt = cfg;
            break;
        case XkbAccessXNotify:
            if (cfg->name.priv & (1L << ev->accessx.detail))
                return cfg;
            break;
        case XkbActionMessage:
            if (cfg->name.str == NULL)
                dflt = cfg;
            else if (strncmp(cfg->name.str, ev->message.message,
                             XkbActionMessageLength) == 0)
                return cfg;
            break;
        default:
            uInternalError("Can't handle type %d XKB events yet, Sorry.\n",
                           ev->any.xkb_type);
            break;
        }
    }
    return dflt;
}

static Bool
ProcessMatchingConfig(XkbEvent * ev)
{
    CfgEntryPtr cfg;
    char buf[1024], *cmd;
    int ok;

    cfg = FindMatchingConfig(ev);
    if (!cfg) {
        if (verbose)
            PrintXkbEvent(stdout, ev);
        return False;
    }
    if (cfg->action.type == UnknownAction) {
        if (cfg->action.text == NULL)
            cfg->action.type = NoAction;
        else if (cfg->action.text[0] == '!') {
            char *tmp;

            cfg->action.type = ShellAction;
            tmp = uStringDup(&cfg->action.text[1]);
            free(cfg->action.text);
            cfg->action.text = tmp;
        }
        else
            cfg->action.type = SoundAction;
    }
    switch (cfg->action.type) {
    case NoAction:
        return True;
    case EchoAction:
        if (cfg->action.text != NULL) {
            snprintf(buf, sizeof(buf), "%s", cfg->action.text);
            cmd = SubstituteEventArgs(buf, ev);
            printf("%s", cmd);
        }
        return True;
    case PrintEvAction:
        PrintXkbEvent(stdout, ev);
        return True;
    case ShellAction:
        if (cfg->action.text == NULL) {
            uWarning("Empty shell command!\n");
            uAction("Ignored\n");
            return True;
        }
        cmd = cfg->action.text;
        break;
    case SoundAction:
        if (cfg->action.text == NULL) {
            uWarning("Empty sound command!\n");
            uAction("Ignored\n");
            return True;
        }
        snprintf(buf, sizeof(buf), "%s %s%s", soundCmd, soundDir,
                 cfg->action.text);
        cmd = buf;
        break;
    default:
        uInternalError("Unknown error action type %d\n", cfg->action.type);
        return False;
    }
    cmd = SubstituteEventArgs(cmd, ev);
    if (verbose)
        uInformation("Executing shell command \"%s\"\n", cmd);
    ok = (system(cmd) == 0);
    return ok;
}

/***====================================================================***/

int
main(int argc, char *argv[])
{
    FILE *      file;
    static char buf[1024];
    XkbEvent    ev;
    Bool        ok;

    yyin = stdin;
    uSetErrorFile(NullString);

    if (!parseArgs(argc, argv))
        exit(1);
    file = NULL;
    XkbInitAtoms(NULL);
    if (cfgFileName == NULL) {
        char *home = getenv("HOME");
        snprintf(buf, sizeof(buf), DFLT_XKBEVD_CONFIG, (home ? home : ""));
        cfgFileName = buf;
    }
    if (uStringEqual(cfgFileName, "-")) {
        static const char *in = "stdin";

        file = stdin;
        cfgFileName = in;
    }
    else {
        file = fopen(cfgFileName, "r");
        if (file == NULL) {     /* no personal config, try for a system one */
            if (cfgFileName != buf) {   /* user specified a file.  bail */
                uError("Can't open config file \"%s\n", cfgFileName);
                uAction("Exiting\n");
                exit(1);
            }
            snprintf(buf, sizeof(buf), DFLT_SYS_XKBEVD_CONFIG,
                     DFLT_XKB_CONFIG_ROOT);
            file = fopen(cfgFileName, "r");
            if (file == NULL && !eventMask) {
                if (verbose) {
                    uError("Couldn't find a config file anywhere\n");
                    uAction("Exiting\n");
                    exit(1);
                }
                exit(0);
            }
        }
    }

    if (background) {
        if (fork() != 0) {
            if (verbose)
                uInformation("Running in the background\n");
            exit(0);
        }
    }
    dpy = GetDisplay(argv[0], dpyName, &xkbOpcode, &xkbEventCode);
    if (!dpy)
        goto BAILOUT;
    ok = True;
    setScanState(cfgFileName, 1);
    CFGParseFile(file);
    if (!config && !eventMask) {
        uError("No configuration specified in \"%s\"\n", cfgFileName);
        goto BAILOUT;
    }
    if (eventMask == 0) {
        uError("No events to watch in \"%s\"\n", cfgFileName);
        goto BAILOUT;
    }
    if (!XkbSelectEvents(dpy, XkbUseCoreKbd, eventMask, eventMask)) {
        uError("Couldn't select desired XKB events\n");
        goto BAILOUT;
    }
    xkb = XkbGetKeyboard(dpy, XkbGBN_AllComponentsMask, XkbUseCoreKbd);
    if (eventMask & XkbBellNotifyMask) {
        unsigned ctrls, vals;

        if (verbose)
            uInformation("Temporarily disabling the audible bell\n");
        if (!XkbChangeEnabledControls
            (dpy, XkbUseCoreKbd, XkbAudibleBellMask, 0)) {
            uError("Couldn't disable audible bell\n");
            goto BAILOUT;
        }
        ctrls = vals = XkbAudibleBellMask;
        if (!XkbSetAutoResetControls(dpy, XkbAudibleBellMask, &ctrls, &vals)) {
            uWarning("Couldn't configure audible bell to reset on exit\n");
            uAction("Audible bell might remain off\n");
        }
    }
    if (soundCmd == NULL)
        soundCmd = DFLT_SOUND_CMD;
    if (soundDir == NULL)
        soundDir = DFLT_SOUND_DIR;
    XkbStdBellEvent(dpy, None, 0, XkbBI_ImAlive);
    while (1) {
        XNextEvent(dpy, &ev.core);
        if ((!ProcessMatchingConfig(&ev)) && (ev.type == xkbEventCode) &&
            (ev.any.xkb_type == XkbBellNotify)) {
            XkbForceDeviceBell(dpy, ev.bell.device,
                               ev.bell.bell_class, ev.bell.bell_id,
                               ev.bell.percent);
        }
    }

    XCloseDisplay(dpy);
    return (ok == 0);
 BAILOUT:
    uAction("Exiting\n");
    if (dpy != NULL)
        XCloseDisplay(dpy);
    exit(1);
}
