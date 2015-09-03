#include "twm.h"

#ifndef _GNOME_
#define _GNOME_

typedef struct {
  Window *ws;
  int wsSize;
  int numWins;
} GnomeData;

void InitGnome (void);
void GnomeAddClientWindow (TwmWindow *new_win);
void GnomeDeleteClientWindow (TwmWindow *new_win);

#endif /* _GNOME_ */
