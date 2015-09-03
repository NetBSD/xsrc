#include "clicktofocus.h"

#include "twm.h"
#include "util.h"
#include "screen.h"

TwmWindow * get_last_window(WorkSpace *current)
{
    TwmWindow		*t;
    TwmWindow  *first = NULL;

    if (! current) return NULL;

    for (t = Scr->FirstWindow; t != NULL; t = t->next) {
      if (!first && !t->iconmgr && OCCUPY (t, current) && t->mapped)
	first = t;
      if (t->hasfocusvisible && OCCUPY (t, current))
	return t;
    }

    return first;
}

void set_last_window(WorkSpace *current)
{
  TwmWindow * t;

  t = get_last_window(current);

  SetFocus(t, CurrentTime);
}
