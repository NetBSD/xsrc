/*
 * Window decoration bits -- initialization
 */

#ifndef _CTWM_DECORATIONS_INIT_H
#define _CTWM_DECORATIONS_INIT_H


void InitTitlebarButtons(void);
void SetCurrentTBAction(int button, int mods, int func, char *action,
                        MenuRoot *menuroot);
bool CreateTitleButton(char *name, int func, char *action,
                       MenuRoot *menuroot, bool rightside,
                       bool append);



#endif /* _CTWM_DECORATIONS_INIT_H */
