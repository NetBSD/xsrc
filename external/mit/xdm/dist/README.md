The default installation settings of xdm match those used for most platforms
in the previous X.Org releases:

    Scripts & modules:          $(libdir)/X11/xdm (aka $(prefix)/lib/X11/xdm)
    Configuration files:        $(libdir)/X11/xdm
    Pixmap files:               $(libdir)/X11/xdm/pixmaps
    Log files:                  /var/log
    Process id/lock files:      /var/run
    xauth cookie files:         /var/lib/xdm

These may be overridden with the following options to configure:

    Loadable modules:           --with-xdmlibdir
    Scripts:                    --with-xdmscriptdir (or --with-xdmlibdir)
    Configuration files:        --with-xdmconfigdir (or --with-xdmlibdir)
    Pixmap files:               --with-xdmpixmapdir (or --with-xdmlibdir)
    Log files:                  --with-logdir
    Process id/lock files:      --with-piddir
    xauth cookie files:         --with-authdir

For instance, some packagers/sites may prefer:

    --with-xdmconfigdir=/etc/X11/xdm
    --with-xdmlibdir=$(prefix)/lib/xdm
    --with-xdmscriptdir=/etc/X11/xdm

The handling of --with-utmp-file & --with-wtmp-file have also changed
slightly since previous versions of xdm:

    --with-{u,w}tmp-file
        [default] write records to utmp/wtmp files, but allow sessreg to
        use its builtin default paths. Omits -u/-w flag entirely from
        sessreg command in Xstartup & Xreset files.
    --with-{u,w}tmp-file=<filename>
        write records to utmp/wtmp files at specified filename.
        Passes filename as argument to sessreg -u/-w flag in Xstartup/Xreset.
    --without-{u,w}tmp-file   or    --with-{u,w}tmp-file=none
        Do not write records to utmp/wtmp files at all.
        Passes "none" as argument to sessreg -u/-w flag in Xstartup/Xreset.

  ------------------------------------------------------------------------

All questions regarding this software should be directed at the
Xorg mailing list:

  https://lists.x.org/mailman/listinfo/xorg

The master development code repository can be found at:

  https://gitlab.freedesktop.org/xorg/app/xdm

Please submit bug reports and requests to merge patches there.

For patch submission instructions, see:

  https://www.x.org/wiki/Development/Documentation/SubmittingPatches

