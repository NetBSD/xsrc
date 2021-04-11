/*
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_WINDOWBOX_H
#define _CTWM_WINDOWBOX_H

name_list **addWindowBox(char *boxname, char *geometry);
void createWindowBoxes(void);
WindowBox *findWindowBox(TwmWindow *twmwin);

void ConstrainedToWinBox(TwmWindow *twmwin,
                         int x, int y, int *nx, int *ny);
void fittocontent(TwmWindow *twmwin);

#endif /* _CTWM_WINDOWBOX_H */
