# 
#  Copyright 2005  Red Hat, Inc.
# 
#  Permission to use, copy, modify, distribute, and sell this software and its
#  documentation for any purpose is hereby granted without fee, provided that
#  the above copyright notice appear in all copies and that both that
#  copyright notice and this permission notice appear in supporting
#  documentation, and that the name of Red Hat not be used in
#  advertising or publicity pertaining to distribution of the software without
#  specific, written prior permission.  Red Hat makes no
#  representations about the suitability of this software for any purpose.  It
#  is provided "as is" without express or implied warranty.
# 
#  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
#  EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
#  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.


bin_PROGRAMS = \
	xtrapchar \
	xtrapproto \
	xtrapin \
	xtrapout \
	xtrapstats \
	xtrapreset \
	xtrapinfo

AM_CFLAGS = $(CWARNFLAGS) $(XTRAP_APP_CFLAGS)

# 
# xtrapchar
#
xtrapchar_LDADD = $(XTRAP_APP_LIBS)

xtrapchar_SOURCES = \
        xtrapchar.c \
	chparse.c \
	chparse.h \
	XEKeybCtrl.c \
	XEKeybCtrl.h 

#
# xtrapin
# 
xtrapin_CFLAGS = -D_DEFAULT_SOURCE $(AM_CFLAGS)
xtrapin_LDADD = $(XTRAP_APP_LIBS)

xtrapin_SOURCES = \
	xtrapin.c \
	XEKeybCtrl.c \
	XEKeybCtrl.h 

#
# xtrapout
#
xtrapout_CFLAGS = -D_DEFAULT_SOURCE $(AM_CFLAGS)
xtrapout_LDADD = $(XTRAP_APP_LIBS)

xtrapout_SOURCES = \
	xtrapout.c \
	XEKeybCtrl.c \
	XEKeybCtrl.h 

#
# xtrapproto
#
xtrapproto_LDADD = $(XTRAP_APP_LIBS)

xtrapproto_SOURCES = \
	xtrapproto.c

#
# xtrapreset
#
xtrapreset_LDADD = $(XTRAP_APP_LIBS)

xtrapreset_SOURCES = \
	xtrapreset.c

#
# xtrapstats
#
xtrapstats_LDADD = $(XTRAP_APP_LIBS)

xtrapstats_SOURCES = \
	xtrapstats.c

#
# xtrapinfo
#
xtrapinfo_LDADD = $(XTRAP_APP_LIBS)

xtrapinfo_SOURCES = \
	xtrapinfo.c \
        XEKeybCtrl.c \
	XEKeybCtrl.h 

appman_PRE = \
        xtrap.man


APP_MAN_DIR_SUFFIX = $(APP_MAN_DIR:@mandir@/man%=%)

appmandir = $(APP_MAN_DIR)

appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@) \
	      $(man_shadows:=.@APP_MAN_SUFFIX@)

EXTRA_DIST = $(appman_PRE)
CLEANFILES = $(appman_DATA) $(BUILT_SOURCES)

SED = sed

# Strings to replace in man pages
XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11

MAN_SUBSTS = \
	-e 's|__vendorversion__|"$(XORGRELSTRING)" "$(XORGMANNAME)"|' \
	-e 's|__xorgversion__|"$(XORGRELSTRING)" "$(XORGMANNAME)"|' \
	-e 's|__xservername__|Xorg|g' \
	-e 's|__xconfigfile__|xorg.conf|g' \
	-e 's|__projectroot__|$(prefix)|g' \
	-e 's|__apploaddir__|$(appdefaultdir)|' \
	-e 's|__appmansuffix__|$(APP_MAN_SUFFIX)|g' \
	-e 's|__libmansuffix__|$(LIB_MAN_SUFFIX)|g' \
	-e 's|__adminmansuffix__|$(ADMIN_MAN_SUFFIX)|g' \
	-e 's|__miscmansuffix__|$(MISC_MAN_SUFFIX)|g' \
	-e 's|__filemansuffix__|$(FILE_MAN_SUFFIX)|g'

SUFFIXES = .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@

man_shadows =    \
    xtrapreset   \
    xtrapinfo    \
    xtrapstats   \
    xtrapout     \
    xtrapin      \
    xtrapchar    \
    xtrapproto

shadowmen = $(man_shadows:=.@APP_MAN_SUFFIX@)

$(shadowmen):
	 $(AM_V_GEN)echo .so man$(APP_MAN_DIR_SUFFIX)/xtrap.$(APP_MAN_SUFFIX) > $@

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
