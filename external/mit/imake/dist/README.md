The imake package contains the imake utility, plus the following support
programs:

   - ccmakedep
   - mergelib
   - revpath
   - mkdirhier
   - makeg
   - cleanlinks
   - mkhtmlindex
   - xmkmf

Most usage will also require installing the Xorg util/cf files which contain
the platform-specific configuration data for known platforms.

The X Window System used imake extensively up through the X11R6.9 release,
for both full builds within the source tree and external software.
X moved to GNU autoconf and automake for its build system in 2005 for
X11R7.0 and later releases, but still provides imake for building existing
external software programs that have not yet converted, though we are not
actively maintaining it for new OS or platform releases.

More information about Imake and its usage may be found in the resources at:

 - https://www.snake.net/software/imake-stuff/
 - https://www.kitebird.com/imake-book/

All questions regarding this software should be directed at the
Xorg mailing list:

  https://lists.x.org/mailman/listinfo/xorg

The primary development code repository can be found at:

  https://gitlab.freedesktop.org/xorg/util/imake

Please submit bug reports and requests to merge patches there.

For patch submission instructions, see:

  https://www.x.org/wiki/Development/Documentation/SubmittingPatches

