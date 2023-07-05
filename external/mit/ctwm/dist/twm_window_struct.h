/**
 * \file
 * TwmWindow struct definition.
 *
 * This previously lived in ctwm.h, but was moved out here to make it a
 * bit easier to scan either this struct or all the other stuff in
 * ctwm.h, without so much rooting around.  It's \#include'd in ctwm.h,
 * and shouldn't be included elsewhere; it's split out purely for
 * dev ease.
 */
#ifndef _CTWM_TWM_WINDOW_STRUCT_H
#define _CTWM_TWM_WINDOW_STRUCT_H

/* Needed for doxygen to get at the #define's for config (like EMWH) */
#ifdef DOXYGEN
# include "ctwm_config.h"
#endif


/**
 * Info and control for every X Window we take over.
 *
 * As a window manager, our job is to...  y'know.  Manage windows.  Every
 * other window on the screen we wrap and control (as well as a few of
 * our internal windows) gets one of these structs put around it to hold
 * the various config and state info we track about it.  They get put
 * into various linked lists for each screen and workspace, and
 * references get stashed in X Contexts so we can find the window that
 * events happen on.
 *
 * Much of this is initially setup in AddWindow() when we find out about
 * and take over a window.
 */
struct TwmWindow {
	struct TwmWindow *next;  ///< Next TwmWindow on the Screen
	struct TwmWindow *prev;  ///< Previous TwmWindow on the Screen

	/// OTP control info for stacking.  Created in OtpAdd().
	OtpWinList *otp;

	/// The actual X Window handle
	Window w;

	/// Original window border width before we took it over and made our
	/// own bordering.  This comes from the XWindowAttributes we get from
	/// XGetWindowAttributes().
	int old_bw;

	/**
	 * \defgroup win_frame Window frame bits
	 * These fields are related to the "frame" window; the decoration we
	 * put around the application's own window (the thing in TwmWindow.w
	 * above) to display borders, titlebars, etc.
	 * @{
	 */
	Window frame;      ///< The X window for the overall frame
	Window title_w;    ///< The title bar Window
	Window hilite_wl;  ///< Left hilite window in titlebar
	Window hilite_wr;  ///< Right hilite window in titlebar
	Window lolite_wl;  ///< Left lolite window in titlebar
	Window lolite_wr;  ///< Right lolite window in titlebar

	/// Current resize cursor.  This changes depending on where on the
	/// frame you are, if we're making them.  \sa
	/// ScreenInfo.BorderCursors
	Cursor curcurs;

	/// Pixmap to which the border is set to when window isn't focused.
	/// \sa TwmWindow.borderC  \sa SetFocusVisualAttributes()
	/// \todo See the XXX in SetFocusVisualAttributes()
	Pixmap gray;

	/// @}

	struct Icon *icon;     ///< The current icon.  \sa CreateIconWindow()
	name_list *iconslist;  ///< The current list of potential icons

	/// \addtogroup win_frame Window frame bits
	/// @{
	int frame_x;                ///< X position on screen of frame
	int frame_y;                ///< Y position on screen of frame
	unsigned int frame_width;   ///< Width of frame
	unsigned int frame_height;  ///< Height of frame

	/// 2d border width.  \sa ScreenInfo.BorderWidth
	int frame_bw;
	/// 3d border width.  \sa ScreenInfo.ThreeDBorderWidth
	int frame_bw3D;

	int actual_frame_x;         ///< Saved frame_x when squeezed
	int actual_frame_y;         ///< Saved frame_y when squeezed
	unsigned int actual_frame_width;  ///< Saved frame_width when squeezed
	unsigned int actual_frame_height; ///< Saved frame_height when squeezed

	/// X coord of window title relative to title_w.
	/// \sa ComputeTitleLocation()
	int title_x;
	/// Y coord of window title relative to title_w.
	/// \sa ComputeTitleLocation()
	int title_y;

	unsigned int title_height;  ///< Height of the full title bar
	unsigned int title_width;   ///< Width of the full title bar

	/// @}

	char *name;       ///< Current window name.  Points into TwmWindow::names.
	char *icon_name;  ///< Current icon name. Points into TwmWindow::names.

	/// Various sources of window/icon names.  These are the values from
	/// the various window properties we look at to get the results.  The
	/// TwmWindow::name and TwmWindow::icon_name point to the currently
	/// active element in here.
	struct _names {
		char *ctwm_wm_name; ///< Name from override CTWM_WM_NAME property
#ifdef EWMH
		char *net_wm_name;  ///< Name from EWMH _NET_WM_NAME property
#endif
		char *wm_name;      ///< Name from ICCCM WM_NAME property

		/// Icon name from override CTWM_WM_ICON_NAME property
		char *ctwm_wm_icon_name;
#ifdef EWMH
		/// Icon name from EWMH _NET_WM_ICON_NAME property
		char *net_wm_icon_name;
#endif
		char *wm_icon_name; ///< Icon name from WM_ICON_NAME property

		/// Whether an icon name property has been set.  Since we default
		/// the icon name to the window name when nothing is given, this
		/// flag allows the window-name-setting code to know when it
		/// needs to re-kick the icon-name-setting.
		bool icon_set;
	} names; ///< \copydoc TwmWindow::_names

	/// \addtogroup win_frame Window frame bits
	/// @{

	/// Position of window title text, relative to title_w.  Starts from
	/// \ref title_x, but may be pushed over due to TitleJustification
	/// config.
	int name_x;
	unsigned int name_width; ///< width of name text
	int highlightxl;         ///< Position of \ref hilite_wl and \ref lolite_wl
	int highlightxr;         ///< Position of \ref hilite_wr and \ref lolite_wr
	int rightx;              ///< Position of of right titlebar buttons
	/// @}

	/// Window attributes from XGetWindowAttributes()
	XWindowAttributes attr;
	/// Window size hints.  From WM_NORMAL_HINTS property.
	/// \sa GetWindowSizeHints()
	XSizeHints hints;
	/// Window manager hints.  From WM_HINTS property, filled in via
	/// XGetWMHints().
	XWMHints *wmhints;
	Window group;      ///< Window group, from WM hints.
	XClassHint class;  ///< Window class info.  From XGetClassHint().

	/// List of the icon managers the window is in.  \sa AddIconManager()
	struct WList *iconmanagerlist;

	ColorPair borderC;     ///< ColorPair for focused window borders
	ColorPair border_tile; ///< ColorPair for non-focused window borders
	ColorPair title;       ///< ColorPair for various other titlebar bits

	/// Has the window ever been iconified?
	/// \todo This is almost write-only, and the one reader seems bogus
	/// in light of what it does.  Investigate further and possibly
	/// remove.
	bool iconified;

	bool isicon;     ///< Is the window an icon now ?
	bool icon_on;    ///< Is the icon visible
	bool mapped;     ///< Is the window mapped ?
	bool squeezed;   ///< Is the window squeezed ?
	bool auto_raise; ///< Should we auto-raise this window ?
	bool auto_lower; ///< Should we auto-lower this window ?
	bool forced;     ///< Has had an icon forced upon it
	bool icon_moved; ///< User explicitly moved the icon
	bool highlight;  ///< Should highlight this window
	bool stackmode;  ///< Honor stackmode requests.  \sa ScreenInfo.StackMode
	bool iconify_by_unmapping;  ///< Unmap window to iconify it
	bool isiconmgr;  ///< This is an icon manager window
	bool iswspmgr;   ///< This is a workspace manager window
	bool isoccupy;   ///< This is an Occupy window

	bool istransient;    ///< This is a transient window
	/// What window it's transient for.  From XGetTransientForHint() and
	/// XM_TRANSIENT_FOR property.
	Window transientfor;

	bool titlehighlight;      ///< Should I highlight the title bar?

	/// Pointer to the icon manager structure, for windows that are icon
	/// managers.  Currently also set for some other window types to
	/// various things, but is only ever used for icon manager windows
	/// (\ref isiconmgr = true).
	struct IconMgr *iconmgrp;

	int save_frame_x;        ///< x position of frame  (saved from zoom)
	int save_frame_y;        ///< y position of frame  (saved from zoom)
	unsigned int save_frame_width;  ///< width of frame   (saved from zoom)
	unsigned int save_frame_height; ///< height of frame  (saved from zoom)
	int zoomed;                ///< ZOOM_NONE || function causing zoom
	bool wShaped;              ///< This window is Shape'd
	/// Which protocols this window handles.  From WM_PROTOCOLS property
	/// via XGetWMProtocols()
	unsigned long protocols;
	Colormaps cmaps;           ///< colormaps for this application
	/// Button windows in the titlebar.  \ingroup win_frame
	TBWindow *titlebuttons;
	SqueezeInfo *squeeze_info;  ///< Control info for title squeezing
	bool squeeze_info_copied;   ///< Should ->squeeze_info be free()'d?

	/// Window ring connectivity
	struct _ring {
		struct TwmWindow *next; ///< Next window in the ring
		struct TwmWindow *prev; ///< Previous window in the ring
		bool cursor_valid;  ///< Whether curs_x and curs_y are usable
		int curs_x;         ///< Stored cursor position in the window
		int curs_y;         ///< Stored cursor position in the window
	} ring; ///< \copydoc TwmWindow::_ring
	// x-ref ScreenInfo.InfoWindow about doxygen hackery

	// Many of these are just the window's particular casing of various
	// config params, inherited from the screen's info.  In most cases,
	// they're essentially a read-only cache.
	bool OpaqueMove;      ///< Move opaquely.  \sa ScreenInfo.DoOpaqueMove
	bool OpaqueResize;    ///< Resize opaquely.  \sa ScreenInfo.DoOpaqueResize
	bool UnmapByMovingFarAway;  ///< \sa ScreenInfo.UnmapByMovingFarAway
	bool AutoSqueeze;     ///< \sa ScreenInfo.AutoSqueeze
	bool StartSqueezed;   ///< \sa ScreenInfo.StartSqueezed
	bool AlwaysSqueezeToGravity; ///< \sa ScreenInfo.AlwaysSqueezeToGravity
	bool DontSetInactive; ///< \sa ScreenInfo.DontSetInactive

	bool hasfocusvisible; ///< The window visibly has focus

	int occupation;  ///< Workspaces the window is in (bitmap)

	Image *HiliteImage; ///< Titlebar hilite backround.  \ingroup win_frame
	Image *LoliteImage; ///< Titlebar lolite backround.  \ingroup win_frame

	/// WindowRegion containing this window.  \todo Write-only?  Reap?
	WindowRegion *wr;
#ifdef WINBOX
	WindowBox *winbox; ///< WindowBox containing this window.
	bool iswinbox;     ///< This is a WindowBox window.
#endif

	/// Saved window geometry.  Used in f.savegeometry and
	/// f.restoregeometry.
	struct _savegeometry {
		int x;  ///< Saved x coord
		int y;  ///< Saved y coord
		unsigned int width;  ///< Saved width
		unsigned int height; ///< Saved height
	} savegeometry; ///< \copydoc TwmWindow::_savegeometry

	/// Where the window is currently mapped (may be NULL)
	struct VirtualScreen *vs;
	/// Where the window is parented.  Always set.
	struct VirtualScreen *parent_vs;

	/// Where the window would be.  Used only by f.showbackground.
	/// \sa ShowBackground()
	struct VirtualScreen *savevs;

	/// Has \ref TwmWindow::name ever changed?  Used only in session saving.
	bool nameChanged;
	/// Has \ref TwmWindow::attr width ever changed?  Used only in sessions.
	bool widthEverChangedByUser;
	/// Has \ref TwmWindow::attr height ever changed?  Used only in sessions.
	bool heightEverChangedByUser;

#ifdef EWMH
	EwmhWindowType ewmhWindowType; ///< EWMH-defined window type
	int ewmhFlags; ///< EWMH-defined window stats. Mostly from _NET_WM_STATE.
#endif /* EWMH */
};

#endif /* _CTWM_TWM_WINDOW_STRUCT_H */
