#!/bin/sh
# This spits to stdout; we expect to be redirected.

src=$1

# This turns the soruce version.c.in into a version.c.in with the version
# numbers from VERSION sub'd in.  The destination is still a .in because
# it [potentially] gets VCS info sub'd in as well.

# Assume VERSION is in the dir above us
vfile="$(dirname ${0})/../VERSION"

# Split the version bits out
vstr=`sed -E -e 's/([0-9]+)\.([0-9]+)\.([0-9]+)(.*)/\1 \2 \3 \4/' ${vfile}`
set -- junk $vstr
shift

maj=$1
min=$2
pat=$3
add=$4


# And sub
sed \
	-e "s/@ctwm_version_major@/$maj/" \
	-e "s/@ctwm_version_minor@/$min/" \
	-e "s/@ctwm_version_patch@/$pat/" \
	-e "s/@ctwm_version_addl@/$add/" \
	${src}
