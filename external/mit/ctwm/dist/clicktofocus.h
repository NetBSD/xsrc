#ifndef CLICKTOFOCUS_H
#define CLICKTOFOCUS_H

#include "twm.h"
#include "iconmgr.h"
#include "workmgr.h"

TwmWindow * get_last_window(WorkSpace *current);
void set_last_window(WorkSpace *current);

#endif
