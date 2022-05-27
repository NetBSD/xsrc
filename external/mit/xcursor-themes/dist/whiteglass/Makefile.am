cursordir = @cursordir@/whiteglass/cursors

all: Makefile.cursors

Makefile.cursors: $(srcdir)/Makefile.cfg
	$(top_srcdir)/genmakefile.sh $(srcdir)/Makefile.cfg > $@

if WHITEGLASS
include Makefile.cursors
endif
