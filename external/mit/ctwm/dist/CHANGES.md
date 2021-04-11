# CTWM Change History


## 4.0.3  (2019-07-21)

### Bugfixes

1. Perform various manipulations and overrides of `WM_HINTS` property
   when it gets reset during runtime, like we do when initially adopting
   the window.  The most visible effect of this was in windows that don't
   give a focus hint (which we override to give focus), but then reset
   `WM_HINTS` later and still don't give us a hint, where we wound up not
   re-overriding previously.  Reported for `xvile` by Wayne Cuddy.

1. The font height estimation changes in 4.0.0 were not applied correctly
   when UseThreeDMenus was set, leading to some odd vertical misalignment
   of the text with some fonts.  Reported by Wayne Cuddy.

1. A failure in OTP consistency checks caused by the handling of
   transients of fullscreen windows has been fixed.  This manifested as
   failures in OtpCheckConsistencyVS() assertions.



## 4.0.2  (2018-08-25)

### Backward-Incompatible Changes And Removed Features

1. The `UseThreeDIconBorders` config var has been removed.  It came in
   silently and undocumented in 3.4 and has never done anything.

1. The attempts to use DNS lookups for setting the `HOSTNAME` `m4` variable
   have been removed; it is now just a duplicate of `CLIENTHOST`.

### New Features

1. The EWMH `_NET_WM_NAME` property is now supported, and used for the
   window name in place of the ICCCM `WM_NAME` when set.  By default, we
   also accept `UTF8_STRING` encoded `WM_NAME` as a result of this
   change; see below for var to restore historical strictness.

1. The EWMH `_NET_WM_ICON_NAME` property is now supported, and used for
   the icon name in place of the ICCCM `WM_ICON_NAME` when set.  Similar
   comments as above apply to the encodings.

1. Support has been added for `CTWM_WM_NAME` and `CTWM_WM_ICON_NAME`
   properties, which will override any window/icon names otherwise
   specified.  This may be useful for applications that set unhelpful
   names themselves, or for manually adjusting labelling.  These
   properties can be set from the command line via `xprop`; as an
   example, `xprop -f CTWM_WM_NAME 8u -set CTWM_WM_NAME "awesome
   windowsauce"`.  See `xprop(1)` manual for details; the `s`, `t`, and
   `u` field type specifiers will all work.

1. When no icon name is set for a window, we've always used the window
   name for the icon name as well.  But that only happened the first time
   the window name is set; after that, the icon name is stuck at the
   first name.  It now updates along with the window name, if no icon
   name is set.

1. All icon manager windows will now have the `TwmIconManager` class set
   on them, so they can be addressed en mass by other config like
   `NoTitle` by that class name.

### New Config Options

1. Added `DontNameDecorations` config option to disable setting names on
   the X windows we create for window decoration (added in 4.0.0).  These
   have been reported to confuse `xwit`, and might do the same for other
   tools that don't expect to find them on non-end-app windows.  Reported
   by Frank Steiner.

1. Added `StrictWinNameEncoding` config option to enable historical
   behavior, where we're reject invalid property encoding for window
   naming properties (like a `UTF8_STRING` encoded `WM_NAME`).

### Bugfixes

1. Fix up broken parsing of `IconifyStyle "sweep"`.  Bug was introduced
   in 4.0.0.

1. When multiple X Screens are used, building the temporary file for m4
   definitions could fail with an error from `mkstemp()`.  Reported by
   Manfred Knick.

1. When multiple X Screens are used, the OTP code didn't recognize the
   difference, and kept everything in one list.  This caused the internal
   consistency checks to trip when it didn't find all the windows it
   expected.  Reported by Terran Melconian.

1. When `ReverseCurrentWorkspace` is set, mapping windows not on the
   current workspace (e.g., via restarting ctwm, or creating new windows
   with the desktop set via EWMH properties) could segfault.  Reported by
   Sean McAllister.

1. Fix some edge cases where we'd fight other apps' focus handling.  When
   an application moved focus itself to an unrelated (in X terms) window,
   our processing would often race and re-move the focus to the root
   ourselves.  This was visible with e.g. sub-windows in Firefox for
   context menu and urlbar dropdown, which would flash on and then
   disappear.

1. When creating a new transient window of an existing full-screen
   window, the OTP stacking may cause it to be stuck below the main
   window due to the special handling of full-screen focused windows in
   EWMH.  It should now be forced to the top.

1. Building ctwm since 4.0.0 in certain locales could misorder functions
   in the lookup table, leading to troubles parsing the config file.
   You'd get some loud "INTERNAL ERROR" lines from ctwm when running it
   if this were the case.  Now fixed.  Reported by Richard Levitte.



## 4.0.1  (2017-06-05)

### User Visible Changes

1. Fix a bug where fullscreen windows could sometimes wind up incorrectly
   stacked due to a focus-handling issue.  This would lead to ctwm
   aborting with an assertion failure in the OTP code, like `Assertion
   failed: (PRI(owl) >= priority), function OtpCheckConsistencyVS`.

1. Fix an edge case (probably only triggerable via manual work with EWMH
   messages) where a window could wind up resized down to nothing.

### Internals

1. Systems with the ctfconvert/ctfmerge tools available will now use them
   to include CTF info in the compiled binary.  This allows more detailed
   inspection of the running process via DTrace (e.g., the layout of the
   structs).

1. The initial rumblings of a Developer's Manual are now in
   `doc/devman/`.  This isn't tied into the main build, and there's no
   real reason it ever will be.  Things of interest to _users_ should
   wind up in the main manual; this should only have things of interest
   to people _developing_ ctwm.



## 4.0.0  (2017-05-24)

### Build System Change

The old `imake` build system has been replaced by a new structure using
`cmake`.  This makes [cmake](https://cmake.org/) a requirement to build
ctwm.  See the `README.md` file for how to run it.

A fallback minimal build system is available in the `minibuild/`
directory for environments that can't use the main one.  This is likely
to need some manual adjustment on many systems, and the main build is
strongly preferred.  But it should suffice to get a runnable binary if
all else fails.

### Platform Support

Support for many non-current platforms has been dropped.  In particular,
remnants of special-case VMS support have been removed.  Many old and now
dead Unix variants have been similarly desupported.  Generally, platforms
without support for C99 and mid-2000's POSIX are increasingly less likely
to work.

### Backward-Incompatible Changes And Removed Features

1. Argument parsing has been rewritten to use `getopt_long()`.  All
   `-long` options are now `--long` instead.  `-version`, `-info`,
   `-cfgchk`, and `-display` are still accepted if they're the first
   option given, to make it easier for scripts to simultaneously support
   before/after versions; this shim will be removed in a later version.

1. Support for the SDSC imconv library, and the IMCONV options related to
    it, has been removed.  The last release is almost 20 years old, and
    doesn't support any remotely recent platforms.

1. The USE_SIGNALS code to use signal-driven animations has been removed.
    It's been non-default since 3.2 (more than 20 years ago), and not
    documented anywhere but in the code and a comment in this file.

1. The USE_GNOME option and code for GNOME1 support has been removed.

1. The old-style title button action specifications (without an `=` in
   them) deprecated since 3.8 are no longer supported.  Just replacing
   the "`:`" with "`= :`" should suffice to make it work right in 3.8+.
   If you need to share configs with older versions, you'll have to
   conditionalize the syntax with m4 or some other preprocessing.

1. The `f.cut` (and `^` alias for it), `f.cutfile`, and `f.file`
   functions have been removed.  These functions for messing with the
   clipboard were never visibly documented, and came into the manpage in
   3.0 already commented-out and saying they were obsolete.

1. The `f.source` function has been removed.  It's never done anything
   (except beep) as far back as 1.1 and has never been documented.

1. The `f.movemenu` function has been removed.  It was added silently in
   2.1, has never done anything, and has never been documented.

1. The `NoVersion` config parameter has been removed.  It's been
   undocumented, obsoleted, and done absolutely nothing since 1.1.

1. Support for non-flex versions of lex(1) is deprecated, and will take
   some manual work to build.  Note that release tarballs include
   prebuild lexers, so this probably only matters to people building from
   a development tree.  (And if you are, and really need AT&T or some
   other lex to work, talk to us!)

1. Support for building with internal regex implementation has been
   disabled; we now require regex support from libc.  It is still
   possible to enable by manually editing files, but this will be removed
   in the future.  If you have to mess with this, please bring it up on
   the mailing list so we can figure out a long-term solution.

1. Parsing of the `ctwm.workspaces` X resource (i.e., setting `-xrm
   "ctwm.workspaces: something"` on program command-lines) since 3.0 has
   collapsed doubled backslashes (`\\`) into a single (`\`).  However,
   there were no other escapes, so this didn't gain anything.  Using a
   single will work with both variants, unless you need multiple
   backslashes in a row in your workspace names.

1. The `IconRegion` and `WindowRegion` config params both take a `vgrav
   hgrav` pair of parameters to control layout.  Previous versions would
   accept a `hgrav vgrav` ordering in the parsing, and would mostly work
   by odd quirks of the code.  The parsing has been made stricter, so
   only the documented `vgrav hgrav` ordering is accepted now.

### User Visible Changes

1. The default install locations have been changed.  See the README for
    details about where things are installed and how to change them.

1. Several default settings have been changed.  ctwm now defaults to
   acting as though `RestartPreviousState`, `NoGrabServer`,
   `DecorateTransients`, `NoBackingStore`, `RandomPlacement`,
   `OpaqueMove`, `OpaqueResize`, `SortIconManager`, and `StartInMapState`
   have been set.  Those settings that didn't previously have an inverse
   (to get the behavior previously seen when they weren't specified) have
   such added; see below.

1. Added various config parameters as inverses of existing params.  New
   params (with existing param they invert in parens):
    * `BackingStore` (`NoBackingStore`)
    * `GrabServer` (`NoGrabServer`)
    * `StartInButtonState` (`StartInMapState`)
    * `NoSortIconManager` (`SortIconManager`)
    * `NoRestartPreviousState` (`RestartPreviousState`)
    * `NoDecorateTransients` (`DecorateTransients`)

1. Added `DontShowWelcomeWindow` config option to not show welcome
    splashscreen image.

1. Selected a number of cleanups from Stefan Monnier
    <<monnier@IRO.UMontreal.CA>>, including rate-limiting of animations
    using a new `_XA_WM_END_OF_ANIMATION` message.  Font height is
    estimated based on used characters only.  Added some similar changes,
    improved the prevention of placing windows off-screen, the
    `f.rescuewindows` function for emergencies, a hack-fix for
    `f.adoptwindow`. More virtual screen tweaks/fixes.

1. Added the remaining OnTopPriority changes from Stefan Monnier
    <<monnier@IRO.UMontreal.CA>>: `AutoPopup`, `AutoPriority`,
    `OnTopPriority`, `PrioritySwitching`, `f.changepriority`,
    `f.priorityswitching`, `f.setpriority`, `f.switchpriority`,
    `f.tinylower`, `f.tinyraise`.  Currently consistency checking code is
    enabled, which will terminate with an assertion failure if something
    unexpected happens. Smoothed out various inconsistencies that this
    check discovered when virtual screens are used.

1. Basic support for EWMH (Extended Window Manager Hints) added and
    enabled by default.  `EWMHIgnore {}` config option allows selectively
    disabling bits.
    [Olaf "Rhialto" Seibert, Matthew Fuller]

1. Icon manager windows are no longer included in the window ring
    (that had confusing effects on the focus sequence).

1. Added `--dumpcfg` command-line option to print out the compiled-in
    fallback config file.

1. The `Occupy {}` specification now accepts "ws:" as a prefix for
    workspaces.  This may break things if you have workspaces with names
    that differ only by that prefix (e.g., you have workspaces "abc" and
    "ws:abc", and your `Occupy {}` declarations affects both.

1. If ctwm is built with rplay support, sounds may now be configured with
    the RplaySounds {} parameter in the config file in place of the
    `~/.ctwm-sounds` file.  If so, ctwm will give a warning if
    `.ctwm-sounds` exists; support for the external file will be removed
    in a future version.  Also the `SoundHost` config parameter is
    replaced by `RplaySoundHost`; the old name is still accepted, but
    will be removed in a future version.

1. Added `MWMIgnore {}` config option to allow selectively disabling
    honoring of some Motif WM hints.

1. Warping to a window now explicitly sets focus on that window.  This
    would generally (but not always, in the presence of odd X server
    behavior) have already happened for users with focus following mouse,
    but now occurs for `ClickToFocus` users as well.
    [Alexander Klein]

1. Several bugs relating to the Occupy window were fixed.  Iconifying the
    Occupy window no longer loses it and leaves you unable to pull it up
    again.  Minor undersizing in some cases fixed.

1. Windows which fail to use the `WM_HINTS` property to tell us things like
    whether they want us to give them focus are now explicitly given
    focus anyway.  This should fix focus problems with some apps
    (Chromium is a common example).

1. Added `ForceFocus {}` config option to forcibly give focus to all (or
    specified) windows, whether they request it or not.  Previously the
    code did this unconditionally (except when no `WM_HINTS` were
    provided; x-ref previous), but this causes problems with at least
    some programs that tell us they don't want focus, and mean it
    (some Java GUI apps are common examples).

1. `OpaqueMoveThreshold` values >= 200 (the default) are now treated as
    infinite, and so will always cause opaque moving.

### Internals

1. A new code style has been chosen and the entire codebase reformatted
    into it.  Configs for
    [Artistic Style](http://astyle.sourceforge.net/)
    to generate the proper output are in the source tree.

1. The `full_name` element of the TwmWindow structure has been removed.
    Consumers should just use the `name` element instead.



## 3.8.2  (2014-05-18)

1. Various code cleanups.

    * Cleanup re: raising and warping to windows (previous location of
      pointer in windows), SaveWorkspaceFocus. A few extra NULL pointer
      checks.

    * Logical hasfocusvisible cleanup.

    * Rename TwmWindow.list to iconmanagerlist, and various smaller
      cleanups.

    * Eliminated TwmWindow TwmRoot from struct ScreenInfo. Mostly a
      mechanical change.  I found some cases where the dummy TwmWindow
      was apparently mistakenly included in a loop. Replaced `.next`
      with `TwmWindow *FirstWindow` and `.cmaps` with `Colormaps
      RootColormaps`.  Other members were not used.

1. Fix a bug where insufficient validation of the size hints
    resulted in a division by zero when considering aspect ratio.

1. Lots of minor compiler warnings and build fixes, a few of which
    were real current or latent bugs.  Leave warnings enabled by
    default.  A few of the build system adjustments may break very
    old systems (e.g., those with original AT&T `yacc`).

1. Fix incorrect inclusion of `$DESTDIR` in some paths.

1. Update for new website and mailing list at <http://www.ctwm.org>.

1. Look at `_MOTIF_WM_HINTS` for titlebar-less or border-less
    windows.



## 3.8.1  (2012-01-05)

1. Fix bug causing [de]iconified status of windows to not be
    maintained across workspaces.
    [Matthew Fuller]

1. Quiet a bunch of compiler warnings.
    [Matthew Fuller]

1. Make sure we fully initialize our WorkSpaceWindow structure so
    we don't try to dereference uninitialized pointers later on.
    [Matthew Fuller]

1. Increased the number of supported mouse buttons again, having
    just heard of a mouse with 9 possible buttons...
    [Richard Levitte]

1. Fix a bug in the warping "next" function, where if there is a
    single window and the cursor is not on it, invoking `f.warpring
    "next"` does nothing.
    [Martin Blais]

1. Introduce a new feature called "SaveWorkspaceFocus", which when
    enabled, makes ctwm remember which window has the focus within
    each virtual workspace. As you switch workspaces, the cursor is
    automatically warped to the window previous in focus in the
    workspace. This significantly reduces the amount of mouse use.
    [Martin Blais]

1. f.fill patch from Matthias Kretschmer <<kretschm@cs.uni-bonn.de>>.
    Without the patch, you might get windows which are increased by
    two times the border width more than it should be.  Additionally
    if you place a window with no/not much size contrainst like
    firefox in the upper left corner and perform `f.fill "top"` or
    `f.fill "left"` the size of the window will increase by two times
    the border width in width and height without changing the
    top-left coordinate without the patch.  Of course in such a
    situation the size should not change at all...
    [via Olaf Seibert]



## 3.8  (2007-02-16)

1. Global cleanup.  There were some variables shadowing others, things
    not being safely initialized, that sort of thing.
    [Richard Levitte]

1. Fixed several memory leaks found by
    "Nadav Har'El" <<nyh@math.technion.ac.il>>.
    [Olaf "Rhialto" Seibert]

1. Merged in the `f.movetitlebar` command. By default this is bound to
    alt-left-click in the titlebar.
    [Olaf "Rhialto" Seibert]

1. Fixed the following issue:

    * Poking at the code, it looks like InitVirtualScreens() is called
      before the configuration file is parsed which would explain what I
      see since there's no attempt to create them after the config file
      read.  Moving the call after the config parsing causes things to
      work.

    * I've run into a few other issues that I fixed with the attached
      patch:
        * shadow menus on the right screen open the shadow on the left
          screen
        * shadow menus on the left screen open on top of the window
        * windows on the right screen disappear after startup

    [Todd Kover]

1. Adjustments to ctwm.man.  I noticed a couple of small errors.  [Ross
    Combs]

    * One is that the window list arguments for the opaque keywords are
      now optional, are listed with square brackets in the man page.

    * The other is that the two Threshold keywords are shown in the man
      page as requiring curly-brackets, but they are not required or
      accepted in configuration files.

1. Improve algoritm to deal with mismatched geometry of virtual
    screens

    * allow windows to be dragged from one virtual screen to another and
      have them switch workspaces appropriately

    * handle restarts properly with virtual screens, including preserving
      where windows were placed within workspaces regardless of which
      virtual screen a window was on; preserve across restarts

    [Todd Kover]

1. `WMapCreateCurrentBackGround()` and `WMapCreateDefaultBackGround()`
    would skip remaining virtual screens if not all parameters are
    present.  Small type errors.
    [Olaf "Rhialto" Seibert]

1. There were some directives in the config file that wanted to set some
    setting for all virtual screens. However since that list is (now) only
    set up after parsing the config file, they failed to work.  Moreover,
    these settings were basically meant to be global to all virtual
    screens, so a better place for them is somewhere in `*Scr`.  They all
    related to the Workspace Manager, so I moved them from `struct
    WorkSpaceWindow` to `struct WorkSpaceMgr`.

    The affected directives are StartInMapState, WMgrVertButtonIndent,
    WMgrHorizButtonIndent, MapWindowCurrentWorkSpace,
    MapWindowDefaultWorkSpace.  The window and icon_name, even though not
    user-settable, were also moved.

    This is basically the previous change above done right.
    [Olaf "Rhialto" Seibert]

1. Re-introduced `TwmWindow.oldvs`, used to avoid calling
    `XReparentWindow()` when possibe (it messed up the stacking order of
    windows). However, maybe the use of `.vs` should be rethought a bit:
    in `Vanish()` it is now set to `NULL` with the old value kept in
    `.oldvs`.  However the window is still a child of the same vs.  Maybe
    it is better not to set it to `NULL` and then, when *really* changing
    the virtual screen, `.vs` can be used instead of `.oldvs`.

    This whole "virtual screen" thing is unexplained in the manual, which
    even uses it as a synonym for "workspace" already in the introduction
    paragraph. (There also does not seem to be a way now to test virtual
    screens in captive windows) I suspect that all this causes lots of
    confusion, and when cleared up, can simplify the code a lot.

    I also fixed up the horrible indentation in the functions
    where I changed something.
    [Olaf "Rhialto" Seibert]

1. Fixed interaction between "inner" and "outer" workspace
    selection with "captive" windows. This was because the Gnome
    `_WIN_WORKSPACE` property is used in 2 conflicting ways: for
    client windows it indicates which workspace they are in, for
    root windows it indicates which workspace they show. Captive
    windows are both. Also, the initially selected inner workspace
    is now the first, not the same as the outer workspace (this had
    a different cause).
    [Olaf "Rhialto" Seibert]

1. Introduce `Scr->XineramaRoot` to store the root window that
    encompasses all virtual screen rootwindows. This further reduces any
    need to use RealRoot and/or CaptiveRoot.  Add a schematic drawing
    that clarifies the relation between the various root-type windows.
    [Olaf "Rhialto" Seibert]

1. Get rid of all non-locale code and make I18N the silent default
    (doesn't have to be mentioned any more).  **THIS WILL BREAK CTWM ON
    OLDER (PRE-LOCALE) ENVIRONMENTS**.  I strongly recommend an upgrade
    to "post-locale" standards.
    [Richard Levitte]

1. Enhance RandomPlacement with a displacement argument, so the
    pseudo-radomness can be of displacements other than +30+30.  Here's
    an example for a pretty funky displacement:

        RandomPlacement "on" "-30-100"

    [Richard Levitte]

1. Extend the Info window with the geometry seen from the lower
    right corner as well.
    [Richard Levitte]

1. Extend the pointer button specification for title buttons to take
    modifiers.  As part of this change, the following title pointer
    button specification is deprecated:

        Button {j} : {function}

    in favor of the following, for consistency:

        Button {j} = {function}

    The old way still works, but is considered bad form and will
    be removed in a future version ("ctwm 4.0").
    [Richard Levitte]

1. Fix position of buttons in Occupy window, to make them centered.  (and
    spread the remaining space evenly in 4).
    [Olaf "Rhialto" Seibert]

1. `TwmWindow.group` was once apparently set to 0 when a window had no
    group leader but this was changed to pointing to its own window.
    This resulted however in many places checking for both conditions,
    and several checking only for 0 which could not occur anymore.
    Changed it back to 0 (so we can now distinguish again windows that
    really indicate themselves as group leader, if we wish) and this gave
    rise to some simplifications.

    Also, there were various loops through all TwmWindows looking for a
    group leader (or a `transientfor`), I replaced them with
    `GetTwmWindow()` which uses the Xlib function `XFindContext()` which
    uses a hash table lookup. This should be more efficient.

    When you change the occupation of a group member window, it is now
    applied to the group leader (which in turn affects all members).

    I tried this with ExMH, the only program that uses a real group
    leader that I could find.  Iconifying the leader unmaps the members.
    What should "squeezing" do?  ExMH also has an icon window (see ICCCM
    4.1.9, 3rd option) which behaves weirdly; this may be a bug in ExMH
    (see exmh-2.7.2/exmh.BUGS) even though fvwm somehow handles it
    better.
    [Olaf "Rhialto" Seibert]

1. When Squeezing a window group leader, unmap the member windows, just
    like happens with iconification.
    [Olaf "Rhialto" Seibert]

1. Simplifications c.q. de-duplications of code regarding the
    WorkSpaceManager and Occupation windows. This includes coding the
    layout of these windows only once instead of twice (at initialisation
    and when resizing). If it's wrong now at least it should be
    consistent.

    When changing occupation via functions like f.movetonextworkspace,
    also move complete window groups (just like when you do it via the
    Occupation window).  Also fixed changing the occupation of the
    Occupation window.  Documented (so far) undocumented possibility to
    edit the labels of workspaces on the fly (what use this is, I'm not
    sure).  Removed some unused variables.
    [Olaf "Rhialto" Seibert]

1. Get rid of the `USE_SESSION` and `X11R6` macros and make them the
    silent default.  Also cleaned out a few references to the macro
    `X11R4`, which hasn't been used for ages.  **THIS WILL BREAK CTWM ON
    OLDER (PRE-X11R6) ENVIRONMENTS**.  I strongly recommend an upgrade to a
    newer X11 release.
    [Richard Levitte]

1. Modified the random placement so a negative X displacement has the
    first "random" window start near the right edge instead of the right
    and a negative Y displacement has the first "random" window start
    near the bottom edge instead of the top.
    [Richard Levitte]



## 3.7  (2005-07-19)

1. Workspace context (bkctwmws.patch)

    Makes it possible to bind keys specific to the workspace manager
    (by Bj&ouml;rn Knutsson). Use the event context "workspace" for this.

1. New keyword : AlwaysSqueezeToGravity

    If it is enabled, window squeezing always follows window gravity
    (instead of northward when the window has a title).
    (by Rudolph T. Maceyko).

1. TwmKeys and TwmVisible menus (dlctwmmenu.patch)

    Adds TwmKeys (rootmenu listing all keybindings) and TWM Visible (rootmenu
    showing only deiconified windows) (by Dan Lilliehorn).

1. Preliminary GNOME compliance (see README.gnome and TODO.gnome)
    (by Nathan Dushman).

1. IconifyStyle : "normal" "mosaic" "zoomin" "zoomout" "sweep"

    A few "fancy" graphical effects when you iconify windows, just for fun.

1. JPEG images support : You can now use jpeg image files wherever you
    can use images. Use the `jpeg:imagename` syntax.

1. `f.showbackground`

    Since we can now use fancy jpeg image for root backgrounds, this function
    unmaps all windows in the current workspace. This is a toggle function,
    if all windows are unmapped, they are all remapped. Better bind this
    function in the root context.

1. Preliminary support for Xinerama extention. You can define "virtual"
    screens (it's better if they correspond to you actual screens). The
    thing is that you can see several workspaces at the sams time, one per
    virtual screen. Of course, you cannot view the same workspace (or the
    same window) in 2 vscreens at the same time. The syntax is:

        VirtualScreens {
            "1280x1024+0+0"
            "1600x1200+1280+0"
        }

    for 2 screens, the first one (on the left) is 1280x1024, the second one
    (on the right) is 1600x1200.

    This is preliminary, because this has not been extensively tested. I did
    this because I have now 2 screens, but I was unable to get them working
    properly, so I use only one.

1. **[ At this point, Claude has stopped working on CTWM, and the project
    is now in the hands of Richard Levitte <<richard@levitte.org>>. ]**

1. Changed Imakefile to support a distribution target.

1. Changed `:xpm:cross` to become a bit larger and have a slightly more
    3D appearance, and is visible even in very dark configurations.

1. Make AlwaysSqueezeToGravity to work for all windows (if no window
    list is given).

1. New keyword: `NoImagesInWorkSpaceManager`

    If it's enabled, background images aren't displayed in the workspace
    map.

    This was contributed by Thomas Linden.

1. New command line option: `-cfgchk`

    If used, CTWM will only parse the configuration file and indicate
    if it found errors or not.

    This was contributed by Matthew D. Fuller.

1. `DontMoveOff` patch (by Bj&ouml;rn Knutsson)

    Change the behavior of `DontMoveOff` / `MoveOffResistance` so that
    when you attempt to move a window off screen, it will not move at all
    until it's been moved `MoveOffResistance` pixels (as before), but at
    this time it will no longer "snap", but instead it will start moving
    off screen. This means that you still have the old behavior of
    DontMoveOff, but now with the ability to move a window off screen
    less that `MoveOffResistance` pixels.

1. Random placement and DontMoveOff patch (by Bj&ouml;rn Knutsson, changed)

    When random placement was used, DontMoveOff wasn't honored.
    This behavior has now changed so a window will be kept within
    the screen when at all possible.  When the window is too
    large, it's top or left edge (or both) will be placed in
    coordinate 0.
    This change differs a little bit from Bj&ouml;rns contribution by
    not using rand() at all.

1. `f.warpring` patch (by Bj&ouml;rn Knutsson)

    If `IconManagerFocus` is set, there's no reason why the icon
    manager should get enter and leave events.  This fixes some
    disturbing in the warpring that would otherwise happen.

1. `f.movetoprevworkspace`,
    `f.movetonextworkspace`,
    `f.movetoprevworkspaceandfollow`,
    `f.movetonextworkspaceandfollow` patch (by Daniel Holmstr&ouml;m)

    Makes it possible to move a window to the previous or next
    workspace and, if you like, go to that workspace and focus
    the moved window.

1. `f.fill` "vertical" patch (by Daniel Holmstr&ouml;m)

    Expands the window vertically without overlapping any other window,
    much like `{ f.fill "top" f.fill "bottom" }` but with the exception
    that it doesn't expand over window borders. It also sets the windows
    "zoomed" to `F_FULLZOOM`, so one can toggle between this size,
    original and maximized.

1. `RESIZEKEEPSFOCUS` bugfix patch (by Daniel Holmstr&ouml;m)

    If a window is maximized with `togglemaximize` and then restored it
    might loose focus if the cursor is outside the restored window.  This
    hack puts the cursor at the left-top corner of the window.

1. `f.zoom` bugfix patch (by Daniel Holmstr&ouml;m)

    `f.zoom` now doesn't move the window up (as it sometimes did before)

1. `IgnoreTransient` patch (by Peter Berg Larsen)

    New keyword with list of windows for which to ignore transients.

1. Workspace switch peformance optimization (by MC)

    Stops ctwm from redrawing windows that occupy all workspaces when
    switching from one workspace to another.

1. GTK "group leader" bugfix (by Olaf 'Rhialto' Seibert)

    Makes ctwm aware of the mysterious GTK group leader windows.

1. Resize cursor with non-3D-borders bugfix (by Olaf 'Rhialto' Seibert)

    BorderResizeCursors now works also for top and left borders when
    non-3D-borders are used.

1. Memory leak bugfix (by Simon Burge)

    `GetWMPropertyString` in `util.c` no longer leaks memory.

1. Warpring bugfix (by Takahashi Youichirou)

    Solves these two problems when warping the pointer to the
    next/previous mapped window:

    * Sometimes the pointer moved right too much and ended up outside the
      title bar.

    * When the active window was closed and the pointer ended up on the
      root window, the pointer wouldn't warp until moved with the mouse.

1. NoWarpToMenuTitle patch (by Julian Coleman)

    Fixes the sometimes annoying feature that the cursor is warped to the
    menu title if the menu won't fit on the screen below the current
    pointer position.

    This patch introduces a new keyword `NoWarpToMenuTitle` keyword to
    turn this off.

1. `Scr->workSpaceMgr.windowFont` font init bugfix (by Martin Stjernholm)

    `The Scr->workSpaceMgr.windowFont` in workmgr.c is now initialized.

1. Full GNU regex patch (by Claude Lecommandeur)

    It is now possible to use full GNU regex for window or class names by
    defining `USE_GNU_REGEX` in Imakefile. It is disabled in the default
    Imakefile.

1. DontToggleWorkSpaceManagerState patch (by Dan 'dl' Lilliehorn)

    New keyword to turn off the feature toggling the workspace manager
    state to/from map/button state when you press ctrl and the workspace
    manager window is in focus.

1. TWMAllIcons patch (by Dan 'dl' Lilliehorn)

    Adds the TWMAllIcons menu, listing all iconified windows on all
    workspaces.

1. `f.changesize` patch (by Dan 'dl' Lilliehorn)

    Adds the function `f.changesize` which allows you to change the size
    of the focused window via menus and keybindings.

    Examples:

        "Down"     = c|s: all           : f.changesize "bottom +10"
        "F1"       = c|s: all           : f.changesize "640x480"

1. When crashing, ctwm now refers to ctwm-bugs@free.lp.se instead of
    Claude.Lecommandeur@epfl.ch.
    **NOTE: This is historical information: neither of these addresses
    are the current contact.**

1. Changed all the code to use ANSI C prototypes instead of the old
    K&R style.
    [Richard Levitte]

1. Only use the DefaultFunction if no function was found.
    [Richard Levitte]

1. Correct DontMoveOff

    The DontMoveOff checks when calculating random placement wasn't
    satisfactory.  It ended up placing all windows that were small enough
    to fit in a random place at +50+50 with no exception.  The behavior
    has now been changed to only apply to very large windows (almost as
    large as or larger than the screen).  At the same time, the
    RandomPlacement algorithm and the DonMoveOff checks have been tweaked
    to keep the title height in mind, so centering and coordinates
    correspond to the realities of the rest of CTWM.
    [Richard Levitte]

1. Correct resizing from menu

    Choosing resize from the menu when not having 3D borders moved
    the target window down and right by a border width.  This was
    an error in window position calculations.
    [Richard Levitte]

1. Enhanced info window

    Added the outer geometry.  Added the 3D border width.
    [Richard Levitte]

1. Restart on subsequent SIGHUPs

    Reworked the code that catches a SIGHUP and has ctwm restart as
    a result.  The restarting code has moved from Restart() to the new
    DoRestart().  Restart() now only sets a flag, and CtwmNextEvent()
    has been changed to react to that flag and call DoRestart().  From
    now on, CtwmNextEvent() is always used to get the next event, even
    when no animations are going on.
    [Richard Levitte]

1. A number of VMS-related changes

    DEC/HP PC is a bit picky, the X11 environment is a little bit
    different, and there were some sign/unsigned conflicts and one
    too large symbol (the VMS linker truncates anything beyond the
    31 first characters of any symbol name), so some tweaks were
    needed to get CTWM to build cleanly on VMS.
    [Richard Levitte]

1. Allow gcc users to build with paranoia

    To make it easier to find possible problems, the Imakefile macro
    GCC_PEDANTIC can be defined in Imakefile.local.
    [Richard Levitte]

1. Allow spaces in sound files.

    The .ctwm-sounds file parser would clip sound files at the first
    spaces.  That won't do for sound libraries where file names may
    have spaces in them.  The parser now accepts spaces in file names,
    and will trim spaces from the beginning and the end of both file
    names and event tokens, allowing for a slightly more flexible
    format.
    [Richard Levitte]

1. ctwm.spec

    Added a specification file for RPM building.
    [Richard Levitte]

1. More info for m4

    The m4 subprocess now gets the variable PIXMAP_DIRECTORY, which
    is defined to the directory where the pixmaps are installed, and
    the new flags IMCONV, GNOME, SOUNDS, SESSION and I18N.
    [Richard Levitte]

1. Document sounds

    The sounds system is now documented in the man page.
    [Richard Levitte]

1. Build RPMs

    Added the target "rpm" to build an RPM directly from a distribution
    tarball.
    [Richard Levitte]

1. Make life easier for package builders

    Added the possibility to configure where some libraries can be found
    through the use of `USER_*` make variables in Imakefile.local.  Added
    a lot more commentary in Imakefile.local-template.
    [Richard Levitte]

1. Make it easier to configure on VMS

    Moved all the configuration definitions to descrip.local-template,
    and instruct the users to copy that file to descrip.local and make
    all needed changes there.
    [Richard Levitte]

1. Changed all relevant occurences of levitte@lp.se to
    richard@levitte.org.
    [Richard Levitte]



## 3.6  (2002-08-08 or earlier)

1. Fix line numbers for errors when using m4 preprocessor. Send thanks
    to Josh Wilmes <<josh@hitchhiker.org>>.

1. Fix the way menu entries are selected with the keyboard. Now
    when you type a letter, the pointer moves to the next entry
    whose first letter is this letter, but does not activate it.
    The new keyword IgnoreCaseInMenuSelection, can be used to
    ignore case for this delection.

1. New keyword: DontSave.  Takes a window list as argument. All listed
    windows won't have their characteristics saved for the session manager.
    Patch from Matthias Baake <<Matthias.Baake@gmx.de>>

1. Also from Matthias Baake <<Matthias.Baake@gmx.de>>:
    With the new keywords BorderLeft, BorderRight, BorderBottom and
    BorderTop (each of them is optional with a default value of 0 and
    takes a nonnegative integer as argument) you can declare a border
    "off limits" for f.move etc.. These borders act the same way as the
    real borders of the screen when moving windows; you can use
    f.forcemove to override them.

1. Sloppy Focus added with keyword "SloppyFocus" in configuration file
    (DINH V. Hoa <<dinh@enserb.fr>>).

1. The keyword "ClickToFocus" has been correctly implemented
    (DINH V. Hoa <<dinh@enserb.fr>>).

1. The keyword "IgnoreModifier" has been added, to use this feature, you
    have to add a line `IgnoreModifier { lock m2 }` in the configuration
    file.  All bindings (buttons and keys) will ignore the modifiers you
    specified. It is useful when you use caps locks or num locks. You
    don't need IgnoreLockModifier any more with this option.  (DINH V.
    Hoa <<dinh@enserb.fr>>).

1. New keyword: WindowBox.  Creates a new window called a box, where
    all the client windows that match the windows list are opened in,
    instead of the roor window. This is useful to group small windows
    in the same box (xload for instance) :

        WindowBox "xloadbox" "320x100+0-0" {
            "xload"
        }

1. New function: f.fittocontent.  Can be used only with window boxes.
    The result is to have the box have the minimal size that contains
    all its children windows.

1. New keyword: WindowGeometries.  Used to give a default geometry to some
    clients:

        WindowGeometries {
            "Mozilla*"       "1000x800+10+10"
            "jpilot*"        "800x600-0-0"
        }

1. New keyword: IconMenuDontShow.  Don't show the name of these windows
    in the TwmIcons menu.

1. And, as usual, a few bug fixes here and there.



## 3.5.2  (1999-09-10 or earlier)

1. `f.moveresize`: Takes one string argument which is a geometry with the
    standard X geometry syntax (e.g. `200x300+150-0`). Sets the current
    window to the specified geometry. The width and height are to be given
    in pixel, no base size or resize increment are used.

1. AutoLower et `f.autolower`: from Kai Gro&szlig;johann
    (Kai.Grossjohann@CS.Uni-Dortmund.DE). Same as autoraise but with lower.

1. `WindowRingExclude`: Takes a window list as argument. All listed windows
    will be excluded from the WarpRing.

1. A new menu: "TwmIcons" same as "TwmWindows", but shows only iconified
    windows. I did this when I got bored of having icons. Now I have no
    icons and no icon managers. I use this menu to deiconify windows.
    When I was young, I liked to have brightly colored icons, but now that
    I am getting old(er), I prefer a bare desktop.



## 3.5.1  (1999-05-02 or earlier)

1. `f.initsize`: resets a window to its initial size given by the
    `WM_NORMAL_HINTS` hints.

1. `f.ring`: Selects a window and adds it to the WarpRing, or removes it if
    it was already in the ring. This command makes f.warpring much more
    useful, by making its configuration dynamic (thanks to Philip Kizer
    <<pckizer@tamu.edu>>).

1. f.jumpleft, f.jumpright, f.jumpup, f.jumpdown : takes one integer
    argument (the step). These function are designed to be bound to keys,
    they move the current window (step * {X,Y}MoveGrid) pixels in the
    corresponding direction. stopping when the window encounters another
    window (ala f.pack).



## 3.5  (1997-11-27 or earlier)

1. `f.pack [direction]`.
    Where direction is one of: "right", "left", "top" or "bottom".
    The current window is moved in the specified direction until it reaches
    an obstacle (either another window, or the screen border). The pointer
    follows the window. Examples:

        "Right" = m   : window        : f.pack "right"
        "Left"  = m   : window        : f.pack "left"
        "Up"    = m   : window        : f.pack "top"
        "Down"  = m   : window        : f.pack "bottom"

1. `f.fill [direction]`.
    Where direction is either : "right", "left", "top" or "bottom".
    The current window is resized in the specified direction until it
    reaches an obstacle (either another window, or the screen border).

        "Right" = s|m   : window        : f.fill "right"
        "Left"  = s|m   : window        : f.fill "left"
        "Up"    = s|m   : window        : f.fill "top"
        "Down"  = s|m   : window        : f.fill "bottom"

1. `f.savegeometry`.
    The geometry of the current window is saved. The next call to
    `f.restoregeometry` will restore this window to this geometry.

1. `f.restoregeometry`
    Restore the current window geometry to what was saved in the last
    call to `f.savegeometry`.

1. ShortAllWindowsMenus
    Don't show WorkSpaceManager and IconManagers in the TwmWindows and
    TwmAllWindows menus.

1. f.toggleworkspacemgr
    Toggle the presence of the WorkSpaceManager. If it is mapped, it will
    be unmapped and vice verça.

1. OpenWindowTimeout number
    number is an integer representing a number of second. When a window
    tries to open on an unattended display, it will be automatically
    mapped after this number of seconds.

1. `DontSetInactive { win-list }`
    These windows won't be set to InactiveState when they become invisible
    due to a change workspace. This has been added because some ill-behaved
    clients (Frame5) don't like this.

1. `UnmapByMovingFarAway { win-list }`
    These windows will be moved out of the screen instead of being
    unmapped when they become invisible due to a change workspace. This has
    been added because some ill-behaved clients (Frame5) don't like to be
    unmapped. Use this if the previous doesn't work.

1. `AutoSqueeze { win-list }`
    These windows will be auto-squeezed. i.e. automatically unsqueezed
    when they get focus, and squeezed when they loose it. Useful for the
    workspace manager. (Note, it is not possible to AutoSqueeze icon
    managers).

1. `StartSqueezed  { win-list }`
    These windows will first show up squeezed.

1. RaiseWhenAutoUnSqueeze
    Windows are raised when auto-unsqueezed.

1. Now if the string "$currentworkspace" is present inside the string
    argument of f.exec, it will be substituated with the current workspace
    name. So it is possible to do something like :

        f.exec "someclient -xrm ctwm.workspace:$currentworkspace &"

    and the client will popus up in the workspace where the command was
    started even if you go elsewhere before it actually shows up.

1. Fixes for the VMS version. From Richard Levitte - VMS Whacker
    <<levitte@lp.se>>.

1. Better I18N. From Toshiya Yasukawa <<t-yasuka@dd.iij4u.or.jp>>. (Define
    I18N in Imakefile to activate it).

1. Better Session Management interface. Patches from Matthew McNeill
    <<M.R.McNeill@durham.ac.uk>>.

1. new flag : `-name`, useful only for captive Ctwm. Sets the name of the
    captive root window. Useful too for next point. If no name is
    specified ctwm-n is used, where n is a number automatically
    generated.

1. Two new client resources are now handled by Ctwm :

    * `ctwm.redirect: <captive_ctwm_name>`

        The new client window is open in the captive Ctwm with name
        `<captive_ctwm_name>`.

    * `ctwm.rootWindow: <window_id>`

        The new client window is reparented into `<window_id>` (whaa!!!).
        It is up to you to find any usefullness to this.

1. If the string "$redirect" is present inside the string
    argument of f.exec, it will be substituated with a redirection
    to the current captive Ctwm if any (or nothing if in a main Ctwm).
    So it is possible to do something like :

        f.exec "someclient $redirect &"

    and the client will popus up in the right captive Ctwm.

1. New function f.hypermove. With it, you can drag and drop a window
    between 2 captives Ctwm (or between a captive and the root Ctwm).

1. 2 new m4 variables defined in your startup file:

    * TWM_CAPTIVE
        : value "Yes" if Ctwm is captive, "No" else.

    * TWM_CAPTIVE_NAME
        : The name of the captive Ctwm, if captive.

1. `RaiseOnClick`: if present a window will be raised on top of others
    when clicked on, and the ButtonPress event will be correctly
    forwarded to the client that owns this window (if it asked to).

        RaiseOnClickButton <n> : <Button number to use for RaiseOnClick>

1. `IgnoreLockModifier`: if present, all bindings (buttons and keys) will
    ignore the LockMask. Useful if you often use caps lock, and don't
    want to define twice all your bindings.

1. AutoFocusToTransients
    Transient windows get focus automatically when created.  Useful with
    programs that have keyboard shortcuts that pop up windows.  (patch
    from Kai Grossjohann <<grossjohann@charly.cs.uni-dortmund.de>>).

1. PackNewWindows
    Use f.movepack algorithm instead of f.move when opening a new window.



## 3.4  (1996-09-14  or earlier)

1. 2 new keywords: XMoveGrid and YMoveGrid with an integer parameter.
    Constrains window moves so that its x and y coordinates are multiple
    of the specified values. Useful to align windows easily.

1. New function: f.deleteordestroy. First tries to delete the window
    (send it `WM_DELETE_WINDOW` message), or kills it, if the client
    doesn't accept such message.

1. New function : f.squeeze. It squeezes a window to a null vertical
    size. Works only for windows with either a title, or a 3D border
    (in order to have something left on the screen). If the window is
    already squeezed, it is unsqueezed.

1. New built-in title button: `:xpm:vbar` (a vertical bar).

1. CenterFeedbackWindow : The moving and resizing information window
    is centered in the middle of the screen instead of the top left
    corner.

1. 2 New options:

    * -version
        : Ctwm just prints its version number and exits.
    * -info
        : Ctwm prints its detailed version and compile time options.

1. WarpToDefaultMenuEntry (Useful only with StayUpMenus) : When using
    StayUpMenus, and a menu does stays up, the pointer is warped to
    the default entry of the menu. Try it. Can emulate double click.
    For example :

        Button2 =       : icon          : f.menu "iconmenu"
        menu "iconmenu" {
            "Actions"           f.title
            ""                  f.separator
            "*Restore"          f.iconify
            "Move"              f.move
            "Squeeze"           f.squeeze
            "Occupy ..."        f.occupy
            "Occupy All"        f.occupyall
            ""                  f.separator
            "Delete"            f.deleteordestroy
        }

    will result in DoubleButton2 on an icon uniconifies it.

1. When you popup a menu that is constrained by the border of the screen
    the pointer is warped to the first entry. (Avoid exiting ctwm when you
    just want to refresh the screen).

1. When compiled with `X11R6` defined, ctwm supports ICE session
    management.  (the code has been stolen directly from the X11R6 twm,
    it has not been thoroughly tested, humm... actually, not tested at
    all).

1. SchrinkIconTitles: A la Motif schrinking of icon titles, and expansion
    when mouse is inside icon.
    (Yes, it's misspelt.  Yes, the misspelling is accepted.)

1. AutoRaiseIcons: Icons are raised when the cursor enters it. Useful
    with SchrinkIconTitles.


1. XPM files for title bars or buttons may include the following symbolic
    colors. These symbolic colors allow the possiblity of using the same
    3d XPM file with different colors for different titlebars.

    Background
    : The main color to be used by the title bar

    HiShadow
    :   The color to be used as the highlight

    LoShadow
    :   The color to be used as the dark shadow.

    Using these colors, I have built some 3d XPM files for various
    titlebars while still keeping the ability to change titlebar colors.
    [Matt Wormley <<mwormley@airship.ardfa.calpoly.edu>>]

1. Added a keyword to the .ctwmrc file: "UseSunkTitlePixmap".  This
    makes it so the shadows are inversed for title pixmaps when focus is
    lost.  This is similar to having the SunkFocusWindowTitle, but it
    makes your 3d XPM sink instead of just the whole bar.
    [Matt Wormley <<mwormley@airship.ardfa.calpoly.edu>>]

1. Added 3 new builtin 3d buttons for "Iconify", "Resize" and "Box". They
    are available with the :xpm: identifier in the .ctwmrc file.
    [Matt Wormley <<mwormley@airship.ardfa.calpoly.edu>>]

1. Added another keyword to the .ctwmrc file: "WorkSpaceFont". This
    allows you to specify the font to use in the workspace manager.
    [Matt Wormley <<mwormley@airship.ardfa.calpoly.edu>>]

1. 8 new xpm pixmaps for buttons, title highlite, etc... :
    3dcircle.xpm 3ddimple.xpm 3ddot.xpm 3dfeet.xpm 3dleopard.xpm 3dpie.xpm
    3dpyramid.xpm 3dslant.xpm
    [Matt Wormley <<mwormley@airship.ardfa.calpoly.edu>>]

1. 2 new functions : f.forwmapiconmgr and f.backmapiconmgr, similar to
    f.forwiconmgr and f.backiconmgr but only stops on mapped windows.
    [Scott Bolte <<scottb@cirque.moneng.mei.com>>]

1. Last minute: PixmapDirectory now accept a colon separated list of
    directories.

1. If you use m4, ctwm now defines `TWM_VERSION` which is the version in
    the form of floating point (e.g. 3.4).

1. I forgot to tell that IconRegion has now 3 more optionnal parameters
    iconjust, iconregjust and iconregalign. That can be used to give
    special values to IconJustification, IconRegionJustification and
    IconRegionAlignement for this IconRegion. The new syntax is :

        IconRegion geomstring vgrav hgrav gridwidth gridheight \
            [iconjust] [iconregjust] [iconregalign] [{ win-list }]



## 3.3  (pre-1995-02-11 or 1995-05-04)

1. Better 3D borders with SqueezeTitle.

1. New keywords : BorderShadowDepth, TitleButtonShadowDepth,
    TitleShadowDepth, MenuShadowDepth and IconManagerShadowDepth. You can
    modify the depth of the 3D shadow of all the objects.

1. f.altcontext. a new context named "alter" is introduced. The next key
    or button event after a call to f.altcontext will be interpreted using
    the alternate context. To define bindings in the alternate context, use
    the keyword alter in the context field of the binding command.

1. f.altkeymap. Up to 5 alternate modifiers (a1 to a5). The next key
    or button event after a call to f.altkeymap will be interpreted with
    this alternate modifies set. To define bindings with an alternate
    modifier, use the keyword 'a' followed by the number of the modifier in
    the modifier field of the binding command. Only the root, window, icon
    and iconmgr context are allowed when an alternate modified is used.

1. Default menu entry : If a menu entry name begins with a "\*" (star),
    this star won't be displayed and the corresponding entry will be the
    default entry for this menu. When a menu has a default entry and is used
    as a pull-right in another menu, this default entry action will be executed
    automatically when this submenu is selected without being displayed.
    It's hard to explain, but easy to understand.

1. New keywords:

    `ReallyMoveInWorkspaceManager`
    : tells ctwm to move the actual window when the user is moving the
      small windows in the WorkSpaceMap window.

    `AlwaysShowWindowWhenMovingFromWorkspaceManager`
    : tells ctwm to always map the actual window during the move,
      regardless of whether it crosses the current workspace or not. The
      Shift key toggles this behaviour.

1. 4 new functions:

    * f.rightworkspace
    * f.leftworkspace
    * f.upworkspace
    * f.downworkspace

    Do what you expect.

1. The function f.raiseicons (from Rickard Westman <<ricwe@ida.liu.se>>).
    Raises all icons.

1. A new keyword: IconRegionAlignement. Like IconRegionJustification
    but align vertically. The parameter is "top", "center", "bottom" or
    "border".

1. f.addtoworkspace, f.removefromworkspace and f.toggleoccupation. (idea
    from Kai Grossjohann <<grossjoh@linus.informatik.uni-dortmund.de>>). They
    take one argument that is a workspace name. When applied to a window,
    they add to, remove from, or toggle the occupation of this window in
    this workspace.

1. AlwaysOnTop (from Stefan Monnier <<monnier@di.epfl.ch>>). Accept a list
    of windows as argument. Ctwm will do it's best to keep these windows
    on top of the screen. Not perfect.

1. Some moving stuff.

    f.movepack
    : is like f.move, but it tries to avoid overlapping of windows on the
      screen.  When the moving window begin to overlap with another
      window, the move is stopped.  If you go too far over the other
      window (more than MovePackResistance pixels), the move is resumed
      and the moving window can overlap with the other window. Useful to
      pack windows closely.

    f.movepush
    : Instead of stopping the move, tries to push the other window to
      avoid overlap.  f.movepush is here mainly because I found it
      amusing to do it. Is is not very useful.

1. `TitleJustification`: Takes one string argument : "left", "center", or
    "right". Tells ctwm how to justify the window titles.

1. `UseThreeDWMap`: Tells ctwm to use 3D decorations for the small windows
    in the workspace map.

1. `ReverseCurrentWorkspace`: Tells ctwm to reverse the background and
    foreground colors in the small windows in the workspace map for the
    current workspace.

1. `DontWarpCursorInWMap`: Tells ctwm not to warp the cursor to the
    corresponding actual window when you click in a small window in the
    workspace map.

1. If there is neither MapWindowBackground, nor MapWindowForeground in the
    config file,the window title colors are used for the small windows in the
    workspace map.



## 3.2  (1994-11-13  or earlier)

1. I have considerably reworked the focus handling. So I have probably
    introduced some problems.

1. New keyword: `NoIconManagerFocus`. Tells ctwm not to set focus on windows
    when the pointer is in an IconManager.

1. new option: `-W`. Tells ctwm not to display any welcome when starting.
    To be used on slow machines.

1. New keyword: `StayUpMenus`. Tells ctwm to use stayup menus. These
    menus will stay on the screen when ButtonUp, if either the menu has
    not yet been entered by the pointer, or the current item is a
    f.title.

1. Now ctwm tries to use welcome.xwd instead of welcome.xpm if it exists.
    On my machine the ctwm process size went from 2.3MB to 1MB when changing
    this. Xpm is very greedy.

1. New keyword: `IconRegionJustification`. Tells ctwm how to justify
    icons inside their place in the IconRegion. This keyword needs a
    string value. The acceptable values are : "left", "center", "right"
    and "border".  If "border" is given, the justification will be "left"
    if the icon region gravity is "west" and "right" if the icon region
    gravity is "east".  (clever, isn't it)

1. If you specify the `-f filename` option, ctwm will first try to load
    filename.scrnum, where scrnum is the screen number. If it fails, it
    will try to load filename as usual.

1. TitleButtons can now have different bindings for buttons with the
    following syntax :

        LeftTitleButton ":xpm:menu" {
            Button1 : f.menu "WindowMenu"
            Button2 : f.zoom
            Button3 : f.hzoom
        }

    The old syntax is of course accepted.
    Patch from Stefan Monnier <<Stefan_Monnier@NIAGARA.NECTAR.CS.CMU.EDU>>.

1. A lot of new animated title buttons : `%xpm:menu-up`, `%xpm:menu-down`,
    `%xpm:resize-out-top`, `%xpm:resize-in-top`, `%xpm:resize-out-bot`,
    `%xpm:resize-in-bot`, `%xpm:maze-out`, `%xpm:maze-in`, `%xpm:zoom-out`,
    `%xpm:zoom-in` and `%xpm:zoom-inout`. From Stefan Monnier
    <<Stefan_Monnier@NIAGARA.NECTAR.CS.CMU.EDU>>.

1. 2 new builtin menus: TwmAllWindows and TwmWorkspaces. Guess what they
    do.

1. You can now bind menus to keys. When a menu is visible, you can
    navigate in it with the arrow keys. "Down" or space goes down, "Up"
    goes up, "Left" pops down the menu, and "Right" activates the current
    entry. The first letter of an entry name activates this entry (the first
    one if several entries match). If the first letter is ~ then
    Meta-the-second-letter activates it, if this first letter is ^ then
    Control-the-second-letter activates it, and if this first letter is space,
    then the second letter activates it.

1. Support for VMS. Patch from Peter Chang <<peterc@v2.ph.man.ac.uk>>.
    Completely untested. If you have problems to build on VMS ask
    Peter Chang.

1. New keyword: `MoveOffResistance`.  Idea borrowed to fvwm.  If you set
    MoveOffResistance to a positive (n) value, dontmoveoff will only
    prevent you from going off the edge if you're within n pixels off the
    edge. If you go further, dontmoveoff gives up and lets you go as far
    as you wish.  f.forcemove still allows you to totally ignore
    dontmoveoff. A negative value puts you back into "never moveoff" mode
    (it's the default).

1. The files `background[1-7].xpm` and `background9.xpm` have been
    removed from the distribution. Someone tells me that they are
    copyrighted. I tried to contact him in order to join his copyright,
    but his mail address is invalid.  <<desktop-textures@avernus.com>>.
    Most of these backgrounds and much more can be obtained in the AIcons
    package on ftp.x.org. Particularly in cl-bgnd/Textures: bg_blu.gif,
    concrete.gif, marble1.gif, sharks.gif bg_grn.gif, granite_dark.gif,
    marble2.gif, snails.gif, coarse.gif, granite_light.gif and pool.gif.

1. New keyword: `BorderResizeCursors` with no parameter. If used ctwm
    will put nice cursors when the cursor in on the window borders.  To
    be used when you have bound a button to f.resize in the frame
    context.

1. The xpm files are now installed in `$(TWMDIR)/images` instead of
    `$(TWMDIR)`.

1. Due to the many problems I had with signals being slightly different
    on different systems, I rewrote the animation handling without using
    signals anymore. I hope it is more portable. The old code is still
    available if you define USE_SIGNALS.



## 3.1  (1994-01-28)

1. Ctwm is moving. You can now have animated images for icons, root
    backgrounds, title buttons and focus window title image. This adds
    one new keyword: `AnimationSpeed`, and 4 new function:
    `f.startanimation`, `f.stopanimation`, `f.speedupanimation` and
    `f.slowdownanimation`. An image name is considered an animation if it
    contains the percent (%) character. In which case ctwm replaces this
    character by numbers starting a 1, and will play an animation with
    all these images. There is only 2 examples : ball%.xpm suitable for
    icons, and supman%.xbm suitable for title highlight.  Another example
    (much more beautiful) can be found in the Mosaic distribution. There
    is also one built-in animation for title buttons : `%xpm:resize`, for
    example :

        RightTitleButton                "%xpm:resize" = f.resize

1. Add the WMgrButtonShadowDepth keyword to control the depth of the
    shadow of the workspace manager buttons.

1. The RandomPlacement command has now an optionnal parameter:
    "on", "off", "all" or "unmapped".

1. Three new keywords : ChangeWorkspaceFunction, IconifyFunction and
    DeIconifyFunction, the argument is the name of a function that is
    executed whenever the corresponding event occurs. Useful for sounds :

        ChangeWorkspaceFunction !"cat /users/lecom/sounds/bom.au 2>/dev/null 1>/dev/audio &"

1. A new keyword : IconJustification with 1 argument, either: "left",
    "center" or "right". Tells ctwm how to justify the icon image on the
    icon title (if any).

1. flex is now supported.

1. The IconRegion keyword now support an optionnal winlist argument.
    Thanks to Mike Hoswell <<hoswell@ncar.ucar.edu>> for adding this.

1. f.separator now works (does something) with 3D menus.

1. The format xwd is now accepted for images (icons, background, ...). You
    have to prefix the image file name with xwd: to use this format.
    If the first character of an image file name is |, the filename is
    supposed to be a command that output a xwd image, and it is executed.
    For example, to use a gif file, use :

        "|(giftoppm | pnmtoxwd) < /users/lecom/images/2010.gif"

1. A new keyword: MaxIconTitleWidth with an integer argument. If an icon
    title is larger than this integer, it is truncated.

1. A sound extension is supported. To use it you have to define
    `USE_SOUND` in the Imakefile (not defined by default). In order to
    use this option you need the rplay package. The documentation for
    this extension is in sounds.doc. Warning: this extension is not
    mine, and I don't use it, so don't expect a good support if you have
    problems with it.

1. A new keyword : NoBorder with a window list argument. These windows
    won't have borders. Thanks to J.P. Albers van der Linden
    <<albers@pasichva.serigate.philips.nl>> for this patch.

1. Ctwm has a new option selectable with the flag `-w`, if used, ctwm
    will not take over the whole screen(s), instead it will create a new
    window and manage it. The `-w` has an optional argument which is a
    window id of an existing window, if specified, ctwm will try to
    manage this window.  This is totally useless, but I like it. The
    `f.adoptwindow` function can be used to capture an existing window
    into such a captive ctwm. A possible use of such mode can be to test
    new configuration file without restarting ctwm.

1. Now the welcome file can be of any type understood by ctwm. So it must
    be prefixed with its type. The default is `xpm:welcome.xpm` if the
    XPM option is compiled in, else it is `xwd:welcome.xwd`. You use for
    example:

        setenv CTWM_WELCOME_FILE "|(giftoppm | pnmtoxwd) < ~/images/2010.gif"

1. You can now have 3D window borders with the keyword: UseThreeDBorders.
    In which case the 3D border width is given with: ThreeDBorderWidth.
    The default value is 6.  The color is BorderColor for the window that
    has focus and BorderTileBackground for all others windows. Note: The
    3D borders do not merge very well with squeezed titles, as the top
    edge of the window where the title is missing does not get a 3d
    border.

1. Now, WindowRing can be specified without argument, in this case all
    the windows are in the ring. (Alec Wolman
    <<wolman@blue.cs.washington.edu>>)

1. New keyword: WarpRingOnScreen, if present, tells ctwm that f.warpring
    should warp pointer only to windows visible in the current workspace.



## 3.0  (1993-07-21)

1. A few bugs fixes.

1. A 3D presentation of menus, titles and IconManagers can be selected
    with UseThreeDMenus, UseThreeDTitles and UseThreeDIconManagers. If
    UseThreeDTitles is set the default values for TitleButtonBorderWidth,
    FramePadding, TitlePadding, ButtonIndent are set to 0 pixels. I am
    not that proud of the appearance of 3D titles but 3D menus look nice.
    If UseThreeDTitles is set the flag SunkFocusWindowTitle tells ctwm to
    sunk the title of the window that the focus. 3D features look ugly on
    monochrome displays, but I have no such display for testing purpose.
    If a monochrome display owner can have a look, he is welcome. The
    contrast of the clear and dark shadows can be tuned via the
    ClearShadowContrast and DarkShadowContrast parameters. These
    parameters are percentages.  The formulas used are :

        clear.{RGB} = (65535 - color.{RGB}) * (ClearShadowContrast / 100)
        dark.{RGB}  = color.{RGB} * ((100 - DarkShadowContrast) / 100)

    If you choose UseThreeDIconManagers, icon titles are also 3D. By
    defaults new colors are allocated for shadows, but you can specify
    BeNiceToColormap to inform ctwm to use stipple instead of new colors,
    the effect is less beautiful, but acceptable.


1. A new keyword: NoIconTitle with an optionnal window list.

1. A new keyword: TransientOnTop with an integer parameter. This
    paramater is a percentage and tells ctwm to put transient (and
    non-group leader) windows always on top of their leader only if their
    surface is smaller than this fraction of the surface of their leader.

1. OpaqueMove and OpaqueResize now accept an optionnal list of windows
    as parameter. They also have their NoOpaqueMove and NoOpaqueResize
    counterpart with the same syntax.

1. Two new keywords: OpaqueMoveThreshold and OpaqueResizeThreshold with
    one integer parameter. The parameter represent a percentage of the
    screen surface. If Opaque{Move,Resize} is active for a window, (via
    point 4) the opaque {move, resize} is done only if the window surface
    is smaller than this percentage of the screen. The default is large
    enough.

1. Startup is optionally piped into `m4` before ctwm parse it, ypu can
    now have a common startup file for ctwm, tvtwm, etc ... It can be
    disabled at compile time by undefining USEM4 in Imakefile. It can be
    disabled at execution time by using the `-n` option. Take care if you
    have backquotes (`) in your .ctwmrc file. This character is special
    to m4. In that case, put something like :

        changequote(,)
        changequote(``,'')

    at the beginning of your .ctwmrc.

1. The startup looks nicer (I think). If you use XPM and the file
    welcome.xpm is present in your PixmapDirectory, it is displayed while
    the startup is in progress. Unfortunately, the PixmapDirectory is
    known only after the .ctwmrc is loaded, and this loading is a large
    part of the startup time. So you can define the environnement
    variable `CTWM_WELCOME_FILE` to point to an XPM file, in which case
    it will be displayed very quickly.

1. A new function: f.separator, valid only in menus. The effect is to add
    a line separator between the previous and the following entry.  The
    name selector part in the menu is not used. f.separator works only
    with conventionnal menus, not with 3D menus.

1. Thanks to <<bret@essex.ac.uk>>, the man page is integrated with the
    original twm one, and is of a much better quality.

1. While moving a window, the position is displayed in a similar way as
    the size when resizing.

1. The info window now display the compile time options of the current
    version of ctwm.

1. You can now specify xpm pixmap title buttons and TitleHighlight.
    There is 5 built-in scalable pixmap for buttons, `:xpm:menu`,
    `:xpm:dot`, `:xpm:resize`, `:xpm:zoom` and `:xpm:bar`.

1. Ctwm now restarts when receiving signal SIGHUP, so to restart it from
    a shell, use `kill -1 the_ctwm_pid`.

1. 2 New keywords: WMgrVertButtonIndent and WMgrHorizButtonIndent with 1
    parameter, specifying the vertical and horizontal space beetween
    buttons in the workspace manager.

1. Some more xpm files given. Among them several backgrounds.

1. Ctwm set the property `WM_WORKSPACELIST` (type STRING) on the root
    window, this property contains the null separated list of all the
    workspaces. Now the `WM_OCCUPATION` property on each window is a null
    separated list instead of a space separated list, it was wrong since
    workspace names can contain spaces. So, the first time you will start
    the this version, your windows will show up anywhere.

1. A new library libctwm.a and an include file ctwm.h are given. The
    library contains functions for an external program to have some
    control over ctwm. The functions are:

        Bool    CtwmIsRunning                   ();
        char    **CtwmWorkspaces                ();
        char    *CtwmCurrentWorkspace           ();
        int     CtwmChangeWorkspace             ();
        char    **CtwmCurrentOccupation         ();
        int     CtwmSetOccupation               ();
        int     CtwmAddToCurrentWorkspace       ();

    There is no documentation. A program demolib.c is given to help.



## 2.2  (1993-02-05)

1. Bugs:

    * Redraw small windows when icon name changes.
    * Kill window from the title bar menu
    * Partial geometry on Workspace manager can core dump.
    * AutoRaise and tiny windows in the Workspace Map.

1. Transient windows and non group leader windows are now always on the
    top of their leader.

1. When an icon name changes, the icon itself changes automatically
    according the Icons list in your .ctwmrc. This is very useful for
    clients that have several states. For example xrn or some X mail
    readers can have two differents icons for new mail (news) / no new
    mail (news).

1. A new keyword: TransientHasOccupation has been added for people
    annoyed by the fact that since ctwm-2.1, transient-for non
    group-leader windows have the same occupation that their leader. If
    you specify this, these windows have their own occupation.

1. A new keyword: AutoOccupy. If specified, the occupation of a client is
    changed automatically when it's name or icon name changes, according
    to the Occupy list in your .ctwmrc. For example a mail reader can
    popup instantly in the current workspace when mail arrives.

1. A new keyword: DontPaintRootWindow. If specified, the root window is
    not painted, whatever you told in the Workspaces specification. This
    is useful to have pixmaps in the Workspace Map but not on the root
    window.

1. You can use XPM pixmaps for your background root window. Use
    xpm:filename instead of @filename. The latter is still accepted. Of
    course if your XPM file has transparent parts, there are not
    transparent on the root window, i.e. you dont see the electron gun
    through it.

1. XPMIconDirectory is replaced by PixmapDirectory. (XPMIconDirectory is
    still accepted).

1. You can now use colored root background pixmap and icons in many
    formats.  Ctwm use the imconv library from the San Diego
    Supercomputer Center.  To use these formats, specify: "im:filename"
    for the pixmap name.

    * The following format are supported :

        bmp
        :    Microsoft Windows bitmap image file

        cur
        :    Microsoft Windows cursor image file

        eps
        :    Adobe Encapsulated PostScript file

        gif
        :    Compuserve Graphics image file

        hdf
        :    Hierarchical Data File

        ico
        :    Microsoft Windows icon image file

        icon
        :    Sun Icon and Cursor file

        iff
        :    Sun TAAC Image File Format

        mpnt
        :    Apple Macintosh MacPaint file

        pbm
        :    PBM Portable Bit Map file

        pcx
        :    ZSoft IBM PC Paintbrush file

        pgm
        :    PBM Portable Gray Map file

        pic
        :    PIXAR picture file

        pict
        :    Apple Macintosh QuickDraw/PICT file

        pix
        :    Alias image file

        ppm
        :    PBM Portable Pixel Map file

        pnm
        :    PBM Portable aNy Map file

        ps
        :    Adobe PostScript file

        ras
        :    Sun Rasterfile

        rgb
        :    SGI RGB image file

        rla
        :    Wavefront raster image file

        rle
        :    Utah Run length encoded image file

        synu
        :    SDSC Synu image file

        tga
        :    Truevision Targa image file

        tiff
        :    Tagged image file

        viff
        :    Khoros Visualization image file

        x
        :    AVS X image file

        xbm
        :    X11 bitmap file

        xwd
        :    X Window System window dump image file

    * You can find the imconv package at `ftp.sdsc.edu`. in the directory
      `/pub/sdsc/graphics/imtools`.

    * If `(width > screenwidth / 2) || (height > screenheight / 2)` the
      image is centered else it is tiled.

    * If you don't have the libim library or don't want to use it,
      undefine IMCONV in Imakefile.

    But take care :

    * It is very memory consuming (on the server side).
    * It is very color cells consuming.
    * The ctwm executable is much larger executable.
    * Startup is much much slower (but not the workspace swap).
    * It works only for 8 planes pixmaps and 8 planes screens. If there
      is an imconv specialist somewhere that can generelize this, he is
      welcome.

1. Two new functions : f.nextworkspace, f.prevworkspace.

1. Xpm examples files are now automatically installed in `$(TWMDIR)`

1. An example of .ctwmrc is given, showing some aspect of ctwm
    (example.ctwmrc). It is not a complete .ctwmrc, only the ctwm
    aspects are shown.

1. A new file PROBLEMS has been added that lists some problems you
    can have while using ctwm and some solutions.

Is there any good pixmap designer out there, that i can add beautiful
icons and background to the distribution. Don't use too many colors,
try to use the same few already used in the example icons.



## 2.1  (1992-12-22)

1. Cleanup code to make gcc happy.

1. Bugs fixed

    * IconMaskHint honored.

    * Workaround a bug on HP7xx/8.07 servers for RaiseLower in Map
        window. The stacking order in the MapWindow was not correct on
        those servers. Use

            EXTRA_DEFINES = -DBUGGY_HP700_SERVER

        in your Imakefile if you plan to use this server. It doesn't
        break on others servers.

    * No longer core dump if MapWindowCurrentWorkSpace or
        MapWindowDefaultWorkSpace are specified before WorkSpaces in
        .ctwmrc

    * Small windows handling in the WorkspaceMap window works even if the
        Workspace Manager window has a title (that was not the case with
        ctwm-2.0).

    * ForceIcon works for Xpm icons.

    * Occupation of "transient for" window is correct.

    * RestartPreviousState necessary to keep previous window occupation
        on restart.

    * If a window dies while Occupy Window is mapped, the Occupy Window
        is correctly unmapped.

1. Ctwm now maintains the `WM_CURRENTWORKSPACE` property on the root
    window and `WM_OCCUPATION` on every windows. They mean what you
    think.  These properties are string properties and are in clear text
    instead of an obscure mask. If an external application changes these
    properties ctwm respond with the correct actions, changing the
    current workspace or the occupation of a window. I give a small
    example (gtw.c). An application can manage its occupation and it is
    even possible to write an external workspace manager. It is of course
    not ICCCM compliant because ICCCM says nothing on multiple
    workspaces. The special names "all" and "current" can be used. And
    you can specify relative occupations if the workspace names list
    begin with a "+" or "-" (ex: "+current" adds a window to the current
    workspace).

1. 3 new functions:

    `f.pin`
    :   Pin/Unpin a menu on the screen. Only usable inside a root menu.

    `f.vanish`
    :   Remove a window from the current workspace.  Works only if the
        window occupies at least one other workspace.

    `f.warphere "win-name"`
    :   Adds the window whose name matches win-name to the current
        workspace and warps the pointer to it.

1. And a new keyword: `NoShowOccupyAll`.  Tells ctwm not to show
    OccupyAll windows in the WorkSpaceMap window.

1. All window names can now be specified as (shell-like) regular expressions.



## 2.0  (date unknown)

1. A few bugs fixed:

    * Resize at window creation with button2 works.

    * Some others i don't remember.

1. Better support of monochrome displays: video inverse instead of 3d
    buttons.

1. WorkSpaceManager and Occupy Window are now resizable.  Don't forget to
    verify you have a powerful server before resizing the workspace
    manager with OpaqueResize set.

1. X11R4 support with Imakefile.X11R4 (i didn't try so tell me).

1. The visibility of the workspace manager is now consistant with the
    visibility of the icon managers.  This mean that by default the
    workspace manager is *NOT* visible at startup.  Use the
    ShowWorkSpaceManager to make it visible at startup.

1. Two new functions: f.showworkspacemgr and f.hideworkspacemgr have been
    added. They do what you imagine.

1. And now, the cherry on the cake. The workspace manager has now 2
    states, the button state (the usual one) and the map state (the new
    one). In the map state  the buttons  are replaced  by windows
    displaying  a synthetic view of the corresponding workspaces.  All
    the non-iconified windows of the workspace are shown as small windows
    with the  icon name  written in it.  It looks like  the virtual
    screen of  [t]vtwm, but, of course, much nicer.

    * In this state, you can modify directly the occupation of your
        windows by manipulating these little windows.

        * Button1 move a window from a workspace to another.
        * Button2 copy a window from a workspace to another.
        * Button3 remove a window from a workspace.

    * Clicking in the "root" of these windows warps you to the
        corresponding workspace.  Clicking and releasing Button1 or
        Button2 quickly in a small window go to the corresponding
        workspace and warps the pointer to the corresponding window.

    * The Control-Key (Press and Release) in workspace manager toggles
        the buttons and map state.

    * Four variables and Three functions manipulates this:

        * StartInMapState: The map state is selected at startup, default is
          buttons state.

        * MapWindowCurrentWorkSpace: The aspect of the current workspace in
          the map window.

        * MapWindowDefaultWorkSpace:  Specify the aspect of the non-current
          workspaces in the map window.

        * MapWindowBackground:
        * MapWindowForeground:  Specify the aspect of the small  windows in
          the map window on a per-client basis.

        * f.setbuttonsstate: You can guess.

        * f.setmapstate: You can guess.

        * f.togglestate: You can guess.

1. AutoRaise with RaiseDelay.  Thanks to Johan Vromans <<jv@mh.nl>> who
    gave me this patch.  I think Warren Jessop <<whj@cs.washington.edu>>
    wrote it for twm.



## 1.3  (1992-09-16)

1. Many bugs fixed:

    * Partial geometry in the WorkSpaceManagerGeometry statement no
        longer cause ctwm to core dump.

    * The occupy window name now is "Occupy Window" instead of "Occupy WIndow"
        a typo on the uppercase I in window.

    * Several types problems that make good compilers to issue warnings.

    * The icons of the WorkSpaceManager and Occupy Window windows now
        behave correctly with ButtonPress.

    * UnknownIcon can now have Xpm icons specified.

    * f.showiconmgr no longer map empty icon managers.

    * The ctwm process is smaller (even smaller than twm).

1. Add the Occupy command in .ctwmrc, you can now specify at startup
    which window occupy which workspace.  Example:

        Occupy {
                      "xload"  {"all"}
            Window    "xterm"  {"here" "there" "elsewhere"}
                      "xv"     {"images"}
            WorkSpace "images" {"xloadimage"}
        }



## 1.2  (date unknown)

1. You can now directly edit workspace names in their buttons. only
    printable characters, delete and backspace keys are honored.

1. Ctwm now handle shaped colored icons in XPM format. This added the
    variable XPMIconDirectory, and slightly modified the syntax of the
    Icons command. The XPM icon file names should be prefixed by the
    character '@' to distinguished them from the ordinary bitmap files.

    Example:

        XPMIconDirectory  "/usr/lib/X11/X11/XPM"

        Icons {
            "Axe"    "@xedit.xpm"
            "xterm"  "@xterm.xpm"
            "xrn"    "@xrn.xpm"
            "HPterm" "@hpterm.xpm"
            "XAlarm" "@datebook.xpm"
            "Xman"   "@xman.xpm"
        }

    These above xpm pixmap are given.

1. Many bugs fixed:

    * Icon regions now works.
    * The absence of ShowIconManager is taken into account.
    * The `-iconic` flag is honored.
    * The `-xrm 'ctwm.workspace'` works as expected.
    * I think that f.warpto[to|ring] works correctly i.e warps.  Also to
        the correct workspace if the destination window doesn't occupy
        the current workspace.
    * A few minor bugs fixed.



## 1.1  (1992-06-24 or 26)

1. Correction of a few bugs

1. Add the OpaqueResize flag: similar to OpaqueMove, but redraw the
    window you are resizing at each motion event. Extremely resource
    consuming, but beautiful with fast server/client/network.

1. Now if you don't specify any background/foreground/pixmap indication
    for the root window, ctwm leave it alone so you can have your own
    root background pixmap.

1. You can now specify on the command line a list of workspaces in which a
    new client opens.  The syntax is:

        whatever_client -xrm 'ctwm.workspace: name1 name2 ... namen'

        or

        whatever_client -xrm 'ctwm.workspace: all'

    where 'name1', 'name2', ..., 'namen' are names of workspaces and
    'all' refers to all workspaces.  Example:

        xload -xrm 'ctwm.workspace: all'

1. Add the  OccupyAll command in .ctwmrc, you can now specify at startup
    a list of windows that occupy all the workspaces.  Example:

        OccupyAll {
            "xload"
            "xconsole"
            "xbiff"
        }

1. Add the f.gotoworkspace function. It goes the workspace specified by
    its name.  Example:

        "F1"      =    : root           : f.gotoworkspace "cognac"



## 1.0  (after 1992-04-22)





{>>
 vim:expandtab:ft=markdown:formatoptions-=q:formatoptions+=2
<<}
