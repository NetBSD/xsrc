/*
 * Utils for translating X events between their names and numbers
 */

#include "ctwm.h"

#include <stddef.h>
#include <strings.h>

#include "event_names.h"

/* num->name lookup table (generated build-time) */
#include "event_names_table.h"



/* Need this for any iteration */
size_t
event_names_size(void)
{
	return(sizeof(event_names) / sizeof(*event_names));
}


/* Return the name for any event number */
const char *
event_name_by_num(int evt)
{
	/* Bounds */
	if(evt < 0 || evt >= event_names_size()) {
		return NULL;
	}

	/* Else it's whatever is[n't] there */
	return event_names[evt];
}


/*
 * Find the number for a name.  Technically, the index of the array is a
 * size_t.  We'd need a ssize_t to allow -1 to mean "not found".  But the
 * numbers in the X definition are ints, so that's what we'll return.
 */
int
event_num_by_name(const char *ename)
{
	int i;

	if(ename == NULL) {
		return -1;
	}

	for(i = 0 ; i < event_names_size() ; i++) {
		if(event_names[i] != NULL && strcasecmp(ename, event_names[i]) == 0) {
			return i;
		}
	}

	/* Not found */
	return -1;
}
