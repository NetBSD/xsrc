BASE=ctwm.1

HTML=${BASE}.html

html: ${HTML}
man: ${BASE}

all: html man

all_set_version: all
	sed -i '' -e "s/@ctwm_version_str@/`head -1 ../../VERSION`/" ${HTML} ${BASE}

clean:
	rm -f ${BASE} ${HTML}


# asciidoc vs asciidoctor notes:
#
# Note that asciidoctor has a slightly changed dialect, so there may be
# minor differences in the output.  Also, the manpage generation requires
# 1.5.3+.  At the moment, we're trying to work with both.  Current trends
# suggest that at some point asciidoctor will be the de facto
# asciidoc-world implementation, so we may drop the py-asciidoc at some
# point.  For the moment, asciidoctor is treated as the standard
# renderer, with asciidoc being a fallback we try to keep working well.


# Generating HTML output by various means
html-asciidoc: ${BASE}.adoc
	asciidoc -atoc -anumbered -o ${BASE}.html ${BASE}.adoc

html-asciidoctor: ${BASE}.adoc
	asciidoctor -atoc -anumbered -o ${BASE}.html ${BASE}.adoc


# Manpage output.  x-ref comment above about asciidoctor versions
man-asciidoc-a2x: ${BASE}.adoc
	a2x --doctype manpage --format manpage ${BASE}.adoc

man-asciidoc-adoc: ${BASE}.adoc
	asciidoc -b docbook -d manpage -o ${BASE}.xml ${BASE}.adoc
	xmlto man ${BASE}.xml

man-asciidoctor: ${BASE}.adoc
	asciidoctor -b manpage -o ${BASE} ${BASE}.adoc



# Set which are used by default.  Set to asciidoctor since it's massively
# faster.
${HTML}: ${BASE}.adoc
	@${MAKE} html-asciidoctor

${BASE}: ${BASE}.adoc
	@${MAKE} man-asciidoctor

