All questions regarding this software should be directed at the
Xorg mailing list:

  https://lists.x.org/mailman/listinfo/xorg

The primary development code repository can be found at:

  https://gitlab.freedesktop.org/xorg/app/xfs

Please submit bug reports and requests to merge patches there.

For patch submission instructions, see:

  https://www.x.org/wiki/Development/Documentation/SubmittingPatches

------------------------------------------------------------------------------

[The rest of these notes come from the original X11R5 implementation
 in 1991, and have been updated slightly for the xfs 1.1.0 modular release,
 but are otherwise a bit stale.]

Installation instructions for fontserver

1 - If you don't want to use the default config file location,
run configure with the --with-default-config-file=path option to
point to the correct place.

An example config file is in ./config

2 - modify the config file so the 'catalogue'
parameter points to a set of valid font directories.

At this point the test programs should work.  Start the font server
(xfs &) and try some tests.  Most of the clients take
a command line switch of where to find the server, and
FSlib understands the environment variable FONTSERVER.
The format is the same as Xlib, (ie, hostname:server_number).

The doc directory contains various pieces of documentation on the font
server and associated software:

    xfs-design.xml	- DocBook source of fontserver design overview

Tested font formats:

The fontserver has been tested with PCF, Speedo, SNF and BDF formats.

Tested environments:

the fontserver and clients have been built & tested on SPARC running
4.x and a DECstation 3100 running V4.0.  it should work ok on any 32
bit UNIX w/ BSD sockets.

------------------------------------------------------------------------------

