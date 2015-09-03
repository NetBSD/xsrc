/* The hints we recognize */
#define XA_WIN_PROTOCOLS           "_WIN_PROTOCOLS"
#define XA_WIN_ICONS               "_WIN_ICONS"
#define XA_WIN_WORKSPACE           "_WIN_WORKSPACE"
#define XA_WIN_WORKSPACE_COUNT     "_WIN_WORKSPACE_COUNT"
#define XA_WIN_WORKSPACE_NAMES     "_WIN_WORKSPACE_NAMES"    
#define XA_WIN_LAYER               "_WIN_LAYER"
#define XA_WIN_STATE               "_WIN_STATE"
#define XA_WIN_HINTS               "_WIN_HINTS"
#define XA_WIN_WORKAREA            "_WIN_WORKAREA"
#define XA_WIN_CLIENT_LIST         "_WIN_CLIENT_LIST"
#define XA_WIN_APP_STATE           "_WIN_APP_STATE"
#define XA_WIN_EXPANDED_SIZE       "_WIN_EXPANDED_SIZE"
#define XA_WIN_CLIENT_MOVING       "_WIN_CLIENT_MOVING"
#define XA_WIN_SUPPORTING_WM_CHECK "_WIN_SUPPORTING_WM_CHECK"

#define  WIN_LAYER_DESKTOP      0
#define  WIN_LAYER_BELOW        2
#define  WIN_LAYER_NORMAL       4
#define  WIN_LAYER_ONTOP        6
#define  WIN_LAYER_DOCK         8
#define  WIN_LAYER_ABOVE_DOCK   10

#define  WIN_STATE_STICKY           (1<<0) /* everyone knows sticky */
#define  WIN_STATE_MINIMIZED        (1<<1) /* ??? */
#define  WIN_STATE_MAXIMIZED_VERT   (1<<2) /* window in maximized V state */
#define  WIN_STATE_MAXIMIZED_HORIZ  (1<<3) /* window in maximized H state */
#define  WIN_STATE_HIDDEN           (1<<4) /* not on taskbar but window visible */
#define  WIN_STATE_SHADED           (1<<5) /* shaded (NeXT style) */
#define  WIN_STATE_HID_WORKSPACE    (1<<6) /* not on current desktop */
#define  WIN_STATE_HID_TRANSIENT    (1<<7) /* owner of transient is hidden */
#define  WIN_STATE_FIXED_POSITION   (1<<8) /* window is fixed in position even */
#define  WIN_STATE_ARRANGE_IGNORE   (1<<9)  /* ignore for auto arranging */

#define  WIN_HINTS_SKIP_FOCUS       (1<<0) /* "alt-tab" skips this win */
#define  WIN_HINTS_SKIP_WINLIST     (1<<1) /* not in win list */
#define  WIN_HINTS_SKIP_TASKBAR     (1<<2) /* not on taskbar */
#define  WIN_HINTS_GROUP_TRANSIENT  (1<<3) /* ??????? */
#define  WIN_HINTS_FOCUS_ON_CLICK   (1<<4) /* app only accepts focus when clicked */
#define  WIN_HINTS_DO_NOT_COVER     (1<<5)  /* attempt to not cover this window */
