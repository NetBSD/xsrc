#!/bin/sh
# $0 <object-file-dir> <ctwm-binary>

# Should be given env for ${CTFCONVERT} and ${CTFMERGE}

odir=$1
ctwmbin=$2

if [ ! -r "${odir}/ctwm_main.c.o" ]; then
	echo "No object files in ${odir}, bailing!  You'll get no CTF info."
	exit;
fi

if [ ! -w "${ctwmbin}" ]; then
	echo "${ctwmbin} isn't writable, bailing!  You'll get no CTF info."
	exit;
fi


# Convert the debug info in-place into CTF
for i in ${odir}/*.o; do
	${CTFCONVERT} -l 0 ${i}
done

# Yank it all over into the binary
${CTFMERGE} -l 0 -o ${ctwmbin} ${odir}/*.o
