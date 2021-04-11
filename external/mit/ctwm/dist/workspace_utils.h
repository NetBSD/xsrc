/*
 * General and utility routiens for workspace handling
 */
#ifndef _CTWM_WORKSPACE_UTILS_H
#define _CTWM_WORKSPACE_UTILS_H

void GotoWorkSpace(VirtualScreen *vs, WorkSpace *ws);
void GotoWorkSpaceByName(VirtualScreen *vs, const char *wname);
void GotoWorkSpaceByNumber(VirtualScreen *vs, int workspacenum);
void GotoPrevWorkSpace(VirtualScreen *vs);
void GotoNextWorkSpace(VirtualScreen *vs);
void GotoRightWorkSpace(VirtualScreen *vs);
void GotoLeftWorkSpace(VirtualScreen *vs);
void GotoUpWorkSpace(VirtualScreen *vs);
void GotoDownWorkSpace(VirtualScreen *vs);

void ShowBackground(VirtualScreen *vs, int state);

char *GetCurrentWorkSpaceName(VirtualScreen *vs);
WorkSpace *GetWorkspace(const char *wname);

#endif /* _CTWM_WORKSPACE_UTILS_H */
