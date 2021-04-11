/*
 * Event handler functions (internal to event code)
 */
#ifndef _CTWM_EVENT_HANDLERS_H
#define _CTWM_EVENT_HANDLERS_H

void HandleExpose(void);
void HandleDestroyNotify(void);
void HandleMapRequest(void);
void HandleMapNotify(void);
void HandleUnmapNotify(void);
void HandleMotionNotify(void);
void HandleButtonRelease(void);
void HandleButtonPress(void);
void HandleEnterNotify(void);
void HandleLeaveNotify(void);
void HandleConfigureRequest(void);
void HandleClientMessage(void);
void HandlePropertyNotify(void);
void HandleKeyPress(void);
void HandleKeyRelease(void);
void HandleColormapNotify(void);
void HandleVisibilityNotify(void);
void HandleUnknown(void);
void HandleCirculateNotify(void);

void HandleFocusChange(void);
void HandleCreateNotify(void);
void HandleShapeNotify(void);
#ifdef EWMH
void HandleSelectionClear(void);
#endif
#endif /* _CTWM_EVENT_HANDLERS_H */
