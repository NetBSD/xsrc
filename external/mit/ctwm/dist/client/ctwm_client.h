/*
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_CLIENT_H
#define _CTWM_CLIENT_H

#include <stdbool.h>

bool    CtwmIsRunning(Display *display, int scrnum);
char    **CtwmListWorkspaces(Display *display, int scrnum);
char    *CtwmCurrentWorkspace(Display *display, int scrnum);
int     CtwmChangeWorkspace(Display *display, int scrnum,
                            char   *workspace);
char    **CtwmCurrentOccupation(Display *display, Window window);
int     CtwmSetOccupation(Display *display, Window window,
                          char **occupation);
int     CtwmAddToCurrentWorkspace(Display *display, Window window);

#endif /* _CTWM_CLIENT_H */
