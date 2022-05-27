#!/bin/sh
# this script written by daniel stone <daniel@freedesktop.org>, placed in the
# public domain.

# Shell function to replace old Imake CursorLinkTarget macro
CursorLinkTarget() {
    CURSORLINKS="${CURSORLINKS} $1"
    MAKE_LINKS="$(printf '%s && \\\n\t$(LN_S) %s %s' "${MAKE_LINKS}" $2 $1)"
}

# Default srcdir variable, overridden by Makefile.cfg in handhelds directory
srcdir='$(srcdir)'

test "x$1" = "x" || . "$1"

printf '# this is a generated file -- do not edit.\n'
printf '\n'
printf 'CURSORFILES = %s\n' "${CURSORS}"
printf 'CURSORLINKS =%s\n' "${CURSORLINKS}"
printf 'CLEANFILES = $(CURSORFILES)\n'
printf 'cursor_DATA = $(CURSORFILES)\n'
printf '\n'
printf 'EXTRA_DIST = %s\n' "${DIST}"
printf '\n'

for i in $CURSORS; do
	EXTRA_DIST=''
	printf '%s:' "${i}"
	for png in $(cut -d" " -f4 ${i}.cfg); do
		if test "x${srcdir}" = 'x$(srcdir)' ; then
			EXTRA_DIST="${EXTRA_DIST} ${png}"
		fi
		printf ' %s/%s' "${srcdir}" "${png}"
	done
	printf '\n'
	printf '\t$(XCURSORGEN) -p %s $(srcdir)/%s.cfg %s\n' \
	    "${srcdir}" "${i}" "${i}"
	printf '\n'
	EXTRA_DIST="${EXTRA_DIST} ${i}.cfg"
	if test "x${srcdir}" = 'x$(srcdir)' ; then
		EXTRA_DIST="${EXTRA_DIST} ${i}.xcf"
	fi
	# the lack of space is intentional.
	printf 'EXTRA_DIST +=%s\n\n' "${EXTRA_DIST}"
done

if test "x${MAKE_LINKS}" != "x" ; then
	printf 'install-data-hook:\n'
	printf '\tcd $(DESTDIR)$(cursordir) && rm -f $(CURSORLINKS)\n'
	printf '\tcd $(DESTDIR)$(cursordir)%s\n\n' "${MAKE_LINKS}"
	printf 'uninstall-hook:\n'
	printf '\tcd $(DESTDIR)$(cursordir) && rm -f $(CURSORLINKS)\n\n'
fi


