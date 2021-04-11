#!/bin/sh
#
# Rewrite version.c.in (well, technically, stdin) with bzr revision info.
# We assume if we're getting called, bzr is available.


# "revno revid" of the working tree
REVID=`bzr revision-info --tree | cut -d ' ' -f2-`
if [ $? -ne 0 ]; then
	# Failed somehow
	REVID="???"
fi

# Test inserting some bad chars, to be sure it gets cleaned up right
#REVID="xy\"z${REVID}x\\yz"

# Do a little sanitization.  We have to double up \'s in the first two
# subs to get past sh's quoting, and then double up the \'s in the
# resulting string again to not get eaten in the substitution when we
# interpolate it below.  Jeez.
REVID=`echo "$REVID" | sed -e 's/\\\\/\\\\\\\\/g' -e 's/"/\\\\"/g' \
	-e 's/\\\\/\\\\\\\\/g'`

# That's all we need; just pass stdin through and sub
sed -e "s/%%VCSTYPE%%/\"bzr\"/" -e "s/%%REVISION%%/\"${REVID}\"/"
