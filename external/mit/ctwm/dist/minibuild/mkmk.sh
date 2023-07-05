#!/bin/sh
cd `dirname $0`
rtdir=".."
build="build"
MAKE=${MAKE-"make"}

# Output into Makefile
echo "Generating Makefile"
exec 4<&1
exec 1>Makefile

# Get the list of base non-generated and generated sources
ngfiles=`sed \
	-e '1,/##STDSRC-START/d' -e '/##STDSRC-END/,$d' \
	-e 's/#.*//' -e 's/[[:space:]]*//' -e '/^$/d' \
	${rtdir}/cmake_files/basic_vars.cmake`
gfiles=`sed \
	-e '1,/##GENSRC-START/d' -e '/##GENSRC-END/,$d' \
	-e 's/#.*//' -e 's/[[:space:]]*//' -e '/^$/d' \
	${rtdir}/cmake_files/basic_vars.cmake`

# Extra file needed to build the binary
ngfiles="${ngfiles} ctwm_wrap.c"


# Non-generated files that are optional; separate so they don't end up in
# OFILES in the main run
ongfiles=""
ongfiles="${ongfiles} image_xpm.c"
ongfiles="${ongfiles} image_jpeg.c"
ongfiles="${ongfiles} parse_m4.c"
ongfiles="${ongfiles} ewmh.c"
ongfiles="${ongfiles} xrandr.c"


# Top boilerplate
cat << EOF
## GENERATED FILE

RTDIR=${rtdir}
BDIR=${build}

EOF


# Defs (external)
cat defs.mk
echo


# Will need this for setting the name of the obj file for a given .c
mkobj()
{
	# /bin/sh on Slowaris is ksh93, which doesn't support 'local'.
	# Thanks, guys.  Well, I guess I'll just be sure not to use $_o
	# anywhere else...
	_o=$1
	_o=${_o##*/}
	_o="${_o%.c}.o"

	eval "$2=$_o"
}



# List the core files first
echo "## Core files"
echo "OFILES = \\"
for i in ${ngfiles}; do
	mkobj $i oret
	echo "    \${BDIR}/${oret} \\"
done
echo

# Have to manually write these; transform rules won't work across dirs
for i in ${ngfiles} ${ongfiles}; do
	src="\${RTDIR}/${i}"
	mkobj $i dst
	dst="\${BDIR}/${dst}"
	echo "${dst}: ${src}"
	echo "	\${CC} \${_CFLAGS} -c -o ${dst} ${src}"
done
echo


# Generated files
echo "## Generated files"
echo "OFILES += \\"
for i in ${gfiles}; do
	echo "    \${BDIR}/${i%.c}.o \\"
done
echo
for i in ${gfiles}; do
	echo "gen: \${BDIR}/${i}"
done
echo
echo


# Build up STDSRC for depend state
echo "## For make depend"
echo "STDSRC = \\"
for i in ${ngfiles}; do
	echo "	\${RTDIR}/${i} \\"
done
echo


# The rest of the file
cat main.mk



# Figure out how we can run 'make depend', if we can.  'gccmakedep' and
# 'makedepend' are both programs from X.org that do it, and imake uses
# them, so probably anyone running X has one of them...
mkdep=""
for i in gccmakedep makedepend; do
	mkdep=`command -v ${i}`
	if [ "X" != "X${mkdep}" ]; then
		echo
		echo "depend: \${BDIR} \${GENFILES}"
		echo "	${mkdep} -- \${_CFLAGS} -- \${STDSRC} \${GENSRC}"
		echo "	${MAKE} rwdepend"
		echo
		echo
		break
	fi
done



# Done
exec 1<&4



# Run make depend if we have it.  Else, run make gen so the generated
# files are already there, since otherwise we don't have the detailed
# dependencies to know when we need various headers.
if [ "X" != "X${mkdep}" ]; then
	echo "Running make depend"
	${MAKE} depend
	rm Makefile.bak
else
	echo "Running make gen"
	${MAKE} gen
fi
