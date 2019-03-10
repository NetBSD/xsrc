SUBDIRS = doc src test trace

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = vdpau.pc

EXTRA_DIST = vdpau.pc.in autogen.sh ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
