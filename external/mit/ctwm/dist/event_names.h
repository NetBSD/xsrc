/*
 * X event name/number mapping utils
 */
#ifndef _CTWM_EVENT_NAMES_H
#define _CTWM_EVENT_NAMES_H

#include <stddef.h>    // for size_t

size_t event_names_size(void);
const char *event_name_by_num(int);
int event_num_by_name(const char *);

#endif /* _CTWM_EVENT_NAMES_H */
