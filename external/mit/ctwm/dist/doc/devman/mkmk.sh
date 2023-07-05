#!/bin/sh
cd `dirname $0`

echo "Generating Makefile"
exec 4<&1
exec 1>Makefile

# Will need to be smarter if/when we add a 'combined'.
FILES="*.adoc"


# Heading
cat << EOF
## GENERATED FILE

BDIR=build
ADOC=asciidoctor -atoc -anumbered

# Default target
all: \${BDIR}

\${BDIR}:
	mkdir -p \${BDIR}
	(cd \${BDIR} && ln -s ../static .)

clean:
	rm -rf \${BDIR}

allclean: clean
	rm -f Makefile


# Individual files

EOF


# Now list the files
for i in ${FILES}; do
	in="${i}"
	out="\${BDIR}/`echo ${i} | sed -E -e 's/(.*)\.adoc/\1.html/'`"
	echo "all: ${out}"
	echo "${out}: \${BDIR} ${in}"
	printf "\t\${ADOC} -o ${out} ${in}\n"
	echo
done



exec 1<&4

echo "Done."
