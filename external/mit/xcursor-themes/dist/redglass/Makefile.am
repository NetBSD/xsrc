cursordir = @cursordir@/redglass/cursors

all: Makefile.cursors

Makefile.cursors: $(srcdir)/Makefile.cfg
	$(top_srcdir)/genmakefile.sh $(srcdir)/Makefile.cfg > $@

if REDGLASS
include Makefile.cursors
endif
