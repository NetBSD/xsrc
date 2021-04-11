## Options
OPTDEFS=

# Currently non-optional but still needed
OPTDEFS+=USE_SYS_REGEX

# XPM
OPTDEFS+=XPM
_LFLAGS+=-lXpm
OFILES+=${BDIR}/image_xpm.o
STDSRC+=${RTDIR}/image_xpm.c

# JPEG
OPTDEFS+=JPEG
_LFLAGS+=-ljpeg
OFILES+=${BDIR}/image_jpeg.o
STDSRC+=${RTDIR}/image_jpeg.c

# m4
OPTDEFS+=USEM4
OFILES+=${BDIR}/parse_m4.o
STDSRC+=${RTDIR}/parse_m4.c

# EWMH
OPTDEFS+=EWMH
OFILES+=${BDIR}/ewmh.o ${BDIR}/ewmh_atoms.o
STDSRC+=${RTDIR}/ewmh.c
GENSRC+=${BDIR}/ewmh_atoms.c



### Rules for generating various files

## Autogen'd files

# Stand-in ctwm_config.h
GENXTRA+=${BDIR}/ctwm_config.h
${BDIR}/ctwm_config.h:
	( \
		echo '#define SYSTEM_INIT_FILE "/not/yet/set/system.ctwmrc"' ; \
		echo '#define PIXMAP_DIRECTORY "/not/yet/set/pixmaps"' ; \
		echo '#define M4CMD "m4"' ; \
		for i in ${OPTDEFS}; do \
			echo "#define $${i}" ; \
		done ; \
	) > ${BDIR}/ctwm_config.h


# Atom lists are script-generated
GENSRC+=${BDIR}/ctwm_atoms.c
${BDIR}/ctwm_atoms.o: ${BDIR}/ctwm_atoms.c
${BDIR}/ctwm_atoms.c: ${RTDIR}/ctwm_atoms.in ${TOOLS}/mk_atoms.sh
	${TOOLS}/mk_atoms.sh ${RTDIR}/ctwm_atoms.in ${BDIR}/ctwm_atoms CTWM

# Only needed when EWMH (but doesn't hurt anything to have around if not)
${BDIR}/ewmh_atoms.o: ${BDIR}/ewmh_atoms.c
${BDIR}/ewmh_atoms.c: ${RTDIR}/ewmh_atoms.in ${TOOLS}/mk_atoms.sh
	${TOOLS}/mk_atoms.sh ${RTDIR}/ewmh_atoms.in ${BDIR}/ewmh_atoms EWMH


# Just make null version file
GENSRC+=${BDIR}/version.c
${BDIR}/version.o: ${BDIR}/version.c
${BDIR}/version.c.in: ${RTDIR}/version.c.in ${RTDIR}/VERSION ${TOOLS}/mk_version_in.sh
	${TOOLS}/mk_version_in.sh ${RTDIR}/version.c.in > ${BDIR}/version.c.in
${BDIR}/version.c: ${BDIR}/version.c.in
	sed -e "s/%%[A-Z]*%%/NULL/" \
		${BDIR}/version.c.in > ${BDIR}/version.c


# Table of event names
GENXTRA+=${BDIR}/event_names_table.h
${BDIR}/event_names_table.h: ${RTDIR}/event_names.list ${TOOLS}/mk_event_names.sh
	${TOOLS}/mk_event_names.sh ${RTDIR}/event_names.list \
		> ${BDIR}/event_names_table.h


# Function generated bits
_FUNC_GEN=${BDIR}/functions_defs.h
_FUNC_GEN+=${BDIR}/functions_deferral.h
_FUNC_GEN+=${BDIR}/functions_parse_table.h
_FUNC_GEN+=${BDIR}/functions_dispatch_execution.h
GENXTRA+=${_FUNC_GEN}
${_FUNC_GEN}: ${RTDIR}/functions_defs.list ${TOOLS}/mk_function_bits.sh
	${TOOLS}/mk_function_bits.sh ${RTDIR}/functions_defs.list ${BDIR}


# Default config
GENSRC+=${BDIR}/deftwmrc.c
${BDIR}/deftwmrc.c: ${RTDIR}/system.ctwmrc ${TOOLS}/mk_deftwmrc.sh
	${TOOLS}/mk_deftwmrc.sh ${RTDIR}/system.ctwmrc > ${BDIR}/deftwmrc.c


# lex/yacc inputs
GENSRC+=${BDIR}/lex.c
${BDIR}/lex.o: ${BDIR}/lex.c
${BDIR}/lex.c: ${RTDIR}/lex.l
	${FLEX} -o ${BDIR}/lex.c ${RTDIR}/lex.l

GENSRC+=${BDIR}/gram.tab.c
${BDIR}/gram.tab.o: ${BDIR}/gram.tab.c
${BDIR}/gram.tab.c ${BDIR}/gram.tab.h: ${BDIR}/gram.y
	(cd ${BDIR} && ${YACC_CMD} gram.y)
${BDIR}/gram.y: ${RTDIR}/gram.y
	cp ${RTDIR}/gram.y ${BDIR}/gram.y



## Main build

# Rewriting dependancy info.  The various 'make depend' implementations
# don't know where the .o file is going to go, and there's no portable
# way to tell them, so just give a target to rewrite the Makefile with
# them.
#
# gccmakedep always just calls it "whatever.o", while makedepend assumes
# the .o will be at the same path as the .c, so we need to deal with both
# variants.  Fortunately for us, all the .o's are always in ${BDIR}.
rwdepend:
	sed -E \
		-e 's#^([a-z0-9_./-]*/)?([a-z0-9_./-]*\.o): #$${BDIR}/\2: #' \
		Makefile > .Makefile.tmp
	mv .Makefile.tmp Makefile

# Generated files
GENFILES=${GENSRC} ${GENXTRA}
gen: ${GENFILES}

# Finalize LFLAGS like CFLAGS
_LFLAGS+=${LFLAGS}

# Build final output
ctwm: ${BDIR} ${GENFILES} ${OFILES}
	cc -o ctwm ${OFILES} ${_LFLAGS}

# Need extra transform rule for the generated .c files
.c.o:
	${CC} ${_CFLAGS} -c -o ${@} ${<}
