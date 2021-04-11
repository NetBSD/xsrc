# Minibuild

## Overview

This is a minimal buildchain for ctwm.  It doesn't attempt to do anything
but build the binary, has virtually no autoconfiguration, and may well
require a little manual adjustment to build on any given system.  You are
strongly encouraged to use the main build system with CMake instead.
However, if you can't go that way, this provides a fallback that can at
least get you something you can run.


## Using

    % ./mkmk.sh
    % make


## Per-system notes

### GNU libc

Systems using GNU libc and its headers (e.g., almost any Linux system)
will require some extra -D flags to build properly.  Uncomment the
appropriate line in the Makefile (search for 'glibc') to turn them on.

### Linux

Linux systems are fairly likely to have bison installed, but not yacc.
You can set `YACC=bison` in the environment or on the make command line.
(env is the only way to trigger it properly when running mkmk.sh)

### Solarish

Solaris / OpenSolaris / Illumos / OpenIndiana and whatever other named
derivatives are out there today need more tweaking.  If you're using the
Sun Studio compiler, a different flag is needed to set C99; uncomment the
alternate `C99_FLAG` setting.  `gethostbyname()` isn't in libc, so an
extra library has to be linked in; see the -lnsl line in the Makefile.

Also, its make is dmake(1), which fails utterly at handling something in
the seemingly straightforward Makefile.  A little poking around shed no
light on the matter.  It's probably reasonably likely that a given system
has gmake around, so using that instead (invoked manually for build, and
in the mkmk.sh via `MAKE=gmake` in the environment for the setup) gets
you through.  If this annoys someone enough to find a fix that isn't
terribly invasive, I wouldn't be unhappy.
