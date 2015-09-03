#!/bin/sh
#
# Setup and generate a release tarball

# Version
version=`grep '^\#define VERSION_ID' version.c | cut -d'"' -f2`

# If it's a non-release, append date
if echo $version | grep -q '[^0-9]$'; then
    version="$version.`date '+%Y%m%d'`"
fi

# Setup the dir
dir="ctwm-$version"
if [ -d $dir ] ; then
	echo "Dir '$dir' already exists!"
	exit;
fi
if [ -r $dir.tar ] ; then
	echo "Tarball '$dir.tar' already exists!"
	exit;
fi
mkdir -m755 $dir

# Copy the pertinent files in
find * \
	\! -name STATUS \! -name TABLE \
	\! -name Makefile \
	\! -name Imakefile.local \! -name descrip.local \
	\! -name '*.o.info' \! -name '*.flc' \! -name semantic.cache \
	\! -name y.output \
	| cpio -pmdu $dir

# Edit the set version file in-place
revid=`bzr revision-info --tree | cut -d' ' -f2`
sed -i '' -e "s/%%REVISION%%/$revid/" $dir/version.c

# Tar it up
tar \
	--uid 0 --uname ctwm --gid 0 --gname ctwm \
	-cvf $dir.tar $dir

# Cleanup
rm -rf $dir
ls -l $dir.tar
