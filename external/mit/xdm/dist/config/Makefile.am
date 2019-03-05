#

xdmscriptdir = $(XDMSCRIPTDIR)
xdmconfigdir = $(XDMCONFIGDIR)

dist_xdmconfig_DATA = \
	Xaccess

xdmconfig_DATA = \
	xdm-config \
	Xresources \
	Xservers

xdmscript_SCRIPTS = \
	Xreset \
	Xsession \
	Xstartup

dist_xdmscript_SCRIPTS = \
	GiveConsole \
	TakeConsole \
	Xsetup_0 \
	Xwilling

pixmapdir = $(XDM_PIXMAPDIR)

dist_pixmap_DATA = xorg-bw.xpm xorg.xpm

BUILT_SOURCES = Xservers.ws xdm-config Xreset Xresources Xsession Xstartup

CLEANFILES = $(BUILT_SOURCES) Xservers

EXTRA_DIST = Xservers.ws.in Xservers.fs \
	xdm-config.in Xreset.in Xresources.in Xsession.in Xstartup.in

Xservers: Xservers.$(SERVERSTYPE)
	ln -s Xservers.$(SERVERSTYPE) Xservers

# Defines used when sed'ing config files to replace settings in the files

if USE_XPM
XPM_SED_FLAGS  = -e '/ifdef XPM/d' -e '/endif \/\* XPM \*\//d'
XPM_SED_FLAGS += -e 's|BITMAPDIR|$(XDM_PIXMAPDIR)|'
XPM_SED_FLAGS += -e 's|XDM_PIXMAP|$(XDM_PIXMAP)|'
XPM_SED_FLAGS += -e 's|XDM_BWPIXMAP|$(XDM_BWPIXMAP)|'
else
XPM_SED_FLAGS  = -e '/ifdef XPM/,/endif \/\* XPM \*\//d'
endif

#if HAVE_MKTEMP_COMMAND
MKTEMP_SED_FLAGS = -e 's|MKTEMP_COMMAND|$(MKTEMP_COMMAND)|'
#endif

if SET_UTMP_FILE
UTMP_SED_FLAGS  = -e 's|UTMP_FILE|$(UTMP_FILE)|'
UTMP_SED_FLAGS += -e 's|UTMP_FLAG|-u $(UTMP_FILE)|'
else
UTMP_SED_FLAGS  = -e 's|UTMP_FILE||' -e 's|UTMP_FLAG||'
endif

if SET_WTMP_FILE
WTMP_SED_FLAGS  = -e 's|WTMP_FILE|$(WTMP_FILE)|'
WTMP_SED_FLAGS += -e 's|WTMP_FLAG|-w $(WTMP_FILE)|'
else
WTMP_SED_FLAGS  = -e 's|WTMP_FILE||' -e 's|WTMP_FLAG||'
endif

SED_FILES_FLAGS  = -e 's|BINDIR|$(bindir)|'
SED_FILES_FLAGS += -e 's|DEFAULTVT|$(DEFAULTVT)|'
SED_FILES_FLAGS += $(UTMP_SED_FLAGS) $(WTMP_SED_FLAGS)
SED_FILES_FLAGS += -e 's|XDMDIR|$(XDMLIBDIR)|'
SED_FILES_FLAGS += -e 's|XDMLOGDIR|$(XDMLOGDIR)|'
SED_FILES_FLAGS += -e 's|XDMXAUTHDIR|$(XDMXAUTHDIR)|'
SED_FILES_FLAGS += -e 's|XDMPIDDIR|$(XDMPIDDIR)|'
SED_FILES_FLAGS += -e 's|XDMCONFIGDIR|$(XDMCONFIGDIR)|'
SED_FILES_FLAGS += -e 's|XDMSCRIPTDIR|$(XDMSCRIPTDIR)|'
SED_FILES_FLAGS += -e 's|SU|$(SU)|'
SED_FILES_FLAGS += -e 's|CHOOSERPATH|$(XDMLIBDIR)/chooser|'
SED_FILES_FLAGS += $(XPM_SED_FLAGS)
SED_FILES_FLAGS += -e 's|SHELL_CMD|$(SHELL_CMD)|'
SED_FILES_FLAGS += $(MKTEMP_SED_FLAGS)

# Substitutions previously needed to work around the C pre-processor
SED_CPP_FLAGS = -e 's|XCOMM|\#|' -e 's|XHASH|\#|' \
        -e 's|NLBS|\\n\\|' -e 's|BS|\\|' -e 's|/\*\*/||g'

SUFFIXES = .in

.in:
	$(AM_V_GEN)$(SED) $(SED_FILES_FLAGS) $(SED_CPP_FLAGS) $< > $@
