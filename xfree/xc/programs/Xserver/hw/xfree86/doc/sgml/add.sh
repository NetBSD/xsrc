#!/bin/sh
#
# $XFree86$
#
name=`basename $1 .sgml`
sgmlfmt -f index $name.sgml | \
	sed -e 's,<title>,<item><htmlurl name=",' \
	    -e 's,</title>," url="'$name.html'">,' \
	    -e 's,<author>,<!-- ,' \
	    -e 's,</author>, -->,' \
	    -e 's,<date>,<!-- ,' \
	    -e 's,</date>, -->,' >> index.sgml
exit 0
