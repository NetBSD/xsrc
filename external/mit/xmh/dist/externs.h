/*
 * $XConsortium: externs.h /main/36 1996/01/14 16:51:37 kaleb $
 *
 *
 *		       COPYRIGHT 1987, 1989
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */
/* $XFree86: xc/programs/xmh/externs.h,v 1.4 2001/10/28 03:34:38 tsi Exp $ */

#include <X11/Intrinsic.h>
#include <errno.h>
#include <stdlib.h>

/* Action routines are declared in actions.h */
/* Functions which begin with `Do' are the corresponding callbacks. */

	/* from command.c */

extern int	DoCommand		(char * const *, const char *, const char *);
extern char *	DoCommandToFile		(char * const *);
extern char *	DoCommandToString	(char * const *);

	/* from compfuncs.c */

extern void 	DoResetCompose		(XMH_CB_ARGS);
extern void	CreateForward		(MsgList, String *, Cardinal);

	/* from folder.c */

extern void	DoClose			(XMH_CB_ARGS);
extern void	DoComposeMessage	(XMH_CB_ARGS);
extern void	DoOpenFolder		(XMH_CB_ARGS);
extern void 	DoOpenFolderInNewWindow	(XMH_CB_ARGS);
extern void	DoCreateFolder		(XMH_CB_ARGS);
extern void 	DoDeleteFolder		(XMH_CB_ARGS);
extern void	DoSaveYourself		(XMH_CB_ARGS);
extern void	Push			(Stack *, const char *);
extern const char *	Pop		(Stack *);

	/* from init.c */

extern void	InitializeWorld		(int, char **);

	/* from menu.c */

extern void	AttachMenuToButton	(Button, Widget, const char *);
extern void	DoRememberMenuSelection (XMH_CB_ARGS);
extern void	SendMenuEntryEnableMsg	(Button, const char *, int);
extern void	ToggleMenuItem		(Widget, Boolean);

	/* from miscfuncs.c */

extern int	ScanDir			(const char *, char ***,
                                         int (*)(char *));

	/* from msg.c */

extern Widget   CreateFileSource	(Widget, String, Boolean);
extern char*	MsgName			(Msg);

	/* from pick.c */

extern void	InitPick		(void);
extern void	AddPick			(Scrn, Toc, const char *, const char *);

	/* from popup.c */

extern void	DestroyPopup		(XMH_CB_ARGS);
extern void	WMDeletePopup		(Widget, XEvent*);
extern void	PopupPrompt		(Widget, String, XtCallbackProc);
extern void	PopupConfirm		(Widget, String,
					 XtCallbackList, XtCallbackList);
extern void	PopupNotice		(String, XtCallbackProc, XtPointer);
extern void 	PopupError		(Widget, String);
extern void	PopupWarningHandler(String, String, String, String, String *, Cardinal *);

	/* from screen.c */

extern void	EnableProperButtons	(Scrn);
extern Scrn	CreateNewScrn		(ScrnKind);
extern Scrn	NewViewScrn		(void);
extern Scrn	NewCompScrn		(void);
extern void	ScreenSetAssocMsg	(Scrn, Msg);
extern void	DestroyScrn		(Scrn);
extern void	MapScrn			(Scrn);
extern Scrn	ScrnFromWidget		(Widget);

	/* from toc.c */

extern int	TocFolderExists		(Toc);
extern Boolean	TocHasChanges		(Toc);

	/* from tocfuncs.c */

extern Boolean	UserWantsAction		(Widget, Scrn);
extern void 	DoIncorporateNewMail	(XMH_CB_ARGS);
extern void 	DoCommit		(XMH_CB_ARGS);
extern void	DoPack			(XMH_CB_ARGS);
extern void	DoSort			(XMH_CB_ARGS);
extern void 	DoForceRescan		(XMH_CB_ARGS);
extern void 	DoReverseReadOrder	(XMH_CB_ARGS);
extern void	DoNextView		(XMH_CB_ARGS);
extern void	DoPrevView		(XMH_CB_ARGS);
extern void	DoDelete		(XMH_CB_ARGS);
extern void	DoMove			(XMH_CB_ARGS);
extern void	DoCopy			(XMH_CB_ARGS);
extern void	DoUnmark		(XMH_CB_ARGS);
extern void	DoViewNew		(XMH_CB_ARGS);
extern void	DoReply			(XMH_CB_ARGS);
extern void	DoForward		(XMH_CB_ARGS);
extern void	DoTocUseAsComp		(XMH_CB_ARGS);
extern void	DoPrint			(XMH_CB_ARGS);
extern void	DoPickMessages		(XMH_CB_ARGS);
extern void	DoSelectSequence	(XMH_CB_ARGS);
extern void	DoOpenSeq		(XMH_CB_ARGS);
extern void 	DoAddToSeq		(XMH_CB_ARGS);
extern void 	DoRemoveFromSeq		(XMH_CB_ARGS);
extern void	DoDeleteSeq		(XMH_CB_ARGS);

	/* from util.c */

extern void	Punt			(const char *) _X_NORETURN;
extern int	myopen			(const char *, int, int);
extern FILE *	myfopen			(const char *, const char *);
extern void	myclose			(int);
extern void	myfclose		(FILE *);
extern char *	MakeNewTempFileName	(void);
extern char **	MakeArgv		(int);
extern char **	ResizeArgv		(char **, int);
extern FILEPTR	FOpenAndCheck		(const char *, const char *);
extern char *	ReadLine		(FILE *);
extern char *	ReadLineWithCR		(FILE *);
extern void	DeleteFileAndCheck	(const char *);
extern void	CopyFileAndCheck	(const char *, const char *);
extern void	RenameAndCheck		(const char *, const char *);
extern char *	CreateGeometry		(int, int, int, int, int);
extern int	FileExists		(const char *);
extern long	LastModifyDate		(const char *);
extern int	GetFileLength		(const char *);
extern Boolean	IsSubfolder		(const char *);
extern void 	SetCurrentFolderName	(Scrn, const char *);
extern void	ChangeLabel		(Widget, const char *);
extern Widget	CreateTextSW		(Scrn, const char *, ArgList, Cardinal);
extern Widget	CreateTitleBar		(Scrn, const char *);
extern void	Feep			(int, int, Window);
extern MsgList	CurMsgListOrCurMsg	(Toc);
extern int	GetWidth		(Widget);
extern int	GetHeight		(Widget);
extern Toc	SelectedToc		(Scrn);
extern Toc	CurrentToc		(Scrn);
extern int	strncmpIgnoringCase	(const char *, const char *, int);
extern void 	StoreWindowName		(Scrn, const char *);
extern void	InitBusyCursor		(Scrn);
extern void	ShowBusyCursor		(void);
extern void 	UnshowBusyCursor	(void);
extern void 	SetCursorColor		(Widget, Cursor, unsigned long);

	/* from viewfuncs.c */

extern void	DoCloseView		(XMH_CB_ARGS);
extern void	DoViewReply		(XMH_CB_ARGS);
extern void 	DoViewForward		(XMH_CB_ARGS);
extern void	DoViewUseAsComposition	(XMH_CB_ARGS);
extern void	DoEditView		(XMH_CB_ARGS);
extern void	DoSaveView		(XMH_CB_ARGS);
extern void	DoPrintView		(XMH_CB_ARGS);
