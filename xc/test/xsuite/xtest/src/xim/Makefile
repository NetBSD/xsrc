# $XConsortium: Makefile,v 1.3 94/01/29 17:39:08 rws Exp $
##########################################################################
#
#
##########################################################################
# Default directory locations
#
# The location of TET_ROOT.  This must not contain variable expansions.
#
#
# Location of Test Suite
#
SUITE_ROOT = ${TET_ROOT}/xtest
SUITE_LIB = ${SUITE_ROOT}/lib
SUITE_BIN = ${SUITE_ROOT}/bin
SUITE_INC = ${SUITE_ROOT}/include

INSTLIB = $(SUITE_LIB)
INSTBIN = $(SUITE_BIN)

################################
##### C compiler Directives Section
################################

CFILES = linklocale.c clnlocale.c ic.c nextlocale.c nextlclmod.c \
         nxtfontset.c parse.c xim_cb.c xim_comp.c response.c \
         xim_save.c stimulus.c
OFILES = linklocale.o clnlocale.o ic.o nextlocale.o nextlclmod.o \
         nxtfontset.o parse.o xim_cb.o xim_comp.o response.o \
         xim_save.o stimulus.o

LIBRARY = libximtest.a

##############################################################################

install: all
	$(CP) $(LIBRARY) $(INSTLIB)/$(LIBRARY)
	$(RANLIB) $(INSTLIB)/$(LIBRARY)

all: $(LIBRARY)
	
$(LIBRARY): $P 	$(OFILES)
	$(AR) $@ `$(LORDER) $(OFILES) | $(TSORT)`
	$(RANLIB) $@

FORCE: CLOBBER all

LINT:
	$(LINT) $(LINTFLAGS) $(CFILES) $(LINTTCM) $(LINTLIBS)

LINTLIB: $(LINTSUNTEST)

$(LINTXTEST): llib-lsuntest.ln
	cp $? $@
	@echo "	$? made and installed"

llib-lsuntest.ln: $(CFILES)
	$(LINT) -Csuite $(LINTFLAGS) -u $?

CLEAN:
	$(RM) $(OFILES) $(LIBRARY) makefile.bak 

CLOBBER: CLEAN
	$(RM) $(INSTLIB)/$(LIBRARY)

clean: CLEAN

shar:
	shar Makefile $(SUITE_INC)/*.h $(CFILES) > libsuntest.shar
