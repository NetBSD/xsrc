DISTCHECK_CONFIGURE_FLAGS = --with-cursordir=\$${datadir}/icons

SUBDIRS = handhelds redglass whiteglass

MAINTAINERCLEANFILES = ChangeLog INSTALL
EXTRA_DIST = genmakefile.sh README.md

.PHONY: ChangeLog INSTALL

INSTALL:
	$(INSTALL_CMD)

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog INSTALL
