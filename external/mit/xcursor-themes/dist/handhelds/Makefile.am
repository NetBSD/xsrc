cursordir = @cursordir@/handhelds/cursors

all: Makefile.cursors

Makefile.cursors: $(srcdir)/Makefile.cfg
	$(top_srcdir)/genmakefile.sh $(srcdir)/Makefile.cfg > $@

if HANDHELDS
include Makefile.cursors
endif
