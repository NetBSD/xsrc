/*
 * Copyright 2004 Richard Levitte
 */

#ifndef _CTWM_SESSION_H
#define _CTWM_SESSION_H

#include <stdio.h>  // For FILE

#include <X11/SM/SMlib.h>


/* Used in stashing session info */
struct TWMWinConfigEntry {
	struct TWMWinConfigEntry *next;
	int tag;
	char *client_id;
	char *window_role;
	XClassHint class;
	char *wm_name;
	int wm_command_count;
	char **wm_command;
	short x, y;
	unsigned short width, height;
	short icon_x, icon_y;
	bool iconified;
	bool icon_info_present;
	bool width_ever_changed_by_user;
	bool height_ever_changed_by_user;
	/* ===================[ Matthew McNeill Feb 1997 ]======================= *
	 * Added this property to facilitate restoration of workspaces when
	 * restarting a session.
	 */
	int occupation;
	/* ====================================================================== */

};


/* XXX Only used in one place, should convert to a func? */
extern SmcConn smcConn;

char *GetClientID(Window window);
char *GetWindowRole(Window window);
int WriteWinConfigEntry(FILE *configFile, TwmWindow *theWindow,
                        char *clientId, char *windowRole);
int ReadWinConfigEntry(FILE *configFile, unsigned short version,
                       TWMWinConfigEntry **pentry);
void ReadWinConfigFile(char *filename);
int GetWindowConfig(TwmWindow *theWindow,
                    short *x, short *y,
                    unsigned short *width, unsigned short *height,
                    bool *iconified,
                    bool *icon_info_present,
                    short *icon_x, short *icon_y,
                    bool *width_ever_changed_by_user,
                    bool *height_ever_changed_by_user,
                    int *occupation /* <== [ Matthew McNeill Feb 1997 ] == */
                   );
void SaveYourselfPhase2CB(SmcConn smcCon, SmPointer clientData);
void DieCB(SmcConn smcCon, SmPointer clientData);
void SaveCompleteCB(SmcConn smcCon, SmPointer clientData);
void ShutdownCancelledCB(SmcConn smcCon, SmPointer clientData);
void ProcessIceMsgProc(XtPointer client_data, int *source, XtInputId *id);
void ConnectToSessionManager(char *previous_id);

#endif /* _CTWM_SESSION_H */
