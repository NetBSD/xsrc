# $XConsortium: Makefile,v 1.1 92/06/11 15:29:09 rws Exp $


CFILES = getcwd.c putenv.c strspn.c strtok.c waitpid.c signals.c \
	toupper.c strchr.c strftime.c getopt.c vsprintf.c strpbrk.c \
	memcmp.c

OFILES = getcwd.o putenv.o strspn.o strtok.o waitpid.o signals.o \
	toupper.o strchr.o strftime.o getopt.o vsprintf.o strpbrk.o \
	memcmp.o

all: libport.a

libport.a:$P $(OFILES)
	$(AR) $@ `$(LORDER) $(OFILES) | $(TSORT)`
	$(RANLIB) $@

clean:
	$(RM) $(OFILES) libport.a

install: all
	echo "portlib is used from this directory at present"

LINT:
	$(LINT) $(LINTFLAGS) $(CFILES) $(LINTTCM) $(LINTLIBS)
