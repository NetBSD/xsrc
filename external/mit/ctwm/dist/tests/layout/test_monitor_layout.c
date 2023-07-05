/**
 * Test that MonitorLayout config works
 */

#include "ctwm.h"

#include <stdio.h>

#include "ctwm_main.h"
#include "ctwm_test.h"
#include "r_layout.h"
#include "screen.h"


/**
 * Callback: after the config file gets parsed, make sure we got the info
 * we expected out of it.
 */
static int
check_monitor_layout(void)
{
	// Guard against things that shouldn't happen
	if(Scr == NULL) {
		fprintf(stderr, "BUG: Scr == NULL\n");
		return 1;
	}
	if(Scr->Layout == NULL) {
		fprintf(stderr, "BUG: Scr->Layout == NULL\n");
		return 1;
	}

	// Debug/dev
	if(0) {
		RLayoutPrint(Scr->Layout);
		return 1;
	}


	// We should have 2 monitors...
	const int nmons = RLayoutNumMonitors(Scr->Layout);
	if(nmons != 3) {
		fprintf(stderr, "Expected 3 monitors, got %d\n", nmons);
		return 1;
	}


	// Check the names and sizes.
	// XXX We maybe should have better accessors for this, than grubbing
	// in the structs, but...
	char **names = Scr->Layout->names;

	if(strcmp(names[0], "One") != 0) {
		fprintf(stderr, "First monitor should be 'One', not '%s'\n", names[0]);
		return 1;
	}
	if(strcmp(names[1], "Two") != 0) {
		fprintf(stderr, "Second monitor should be 'Two', not '%s'\n", names[1]);
		return 1;
	}
	if(names[2] != NULL) {
		fprintf(stderr, "Third monitor should be unnamed, not '%s'\n", names[2]);
		return 1;
	}


	RAreaList *mons = Scr->Layout->monitors;

#define CHK_MON_VAL(mon, fld, val) do { \
                if(mons[0].areas[mon].fld != val) { \
                        fprintf(stderr, "Monitor %d %s should be %d, not %d\n", mon, \
                                #fld, val, mons[0].areas[mon].fld); \
                        return 1; \
                } \
        } while(0)

	CHK_MON_VAL(0, x, 0);
	CHK_MON_VAL(0, y, 0);
	CHK_MON_VAL(0, width,  1024);
	CHK_MON_VAL(0, height, 768);

	CHK_MON_VAL(1, x, 1024);
	CHK_MON_VAL(1, y, 0);
	CHK_MON_VAL(1, width,  768);
	CHK_MON_VAL(1, height, 1024);

	CHK_MON_VAL(2, x, 1792);
	CHK_MON_VAL(2, y, 0);
	CHK_MON_VAL(2, width,  800);
	CHK_MON_VAL(2, height, 600);



	// OK, everything was good.
	fprintf(stdout, "OK\n");
	return 0;
}



/*
 * Connect up our callback and kick off ctwm.
 *
 * XXX Args should be more locally controlled; x-ref test_m4.
 */
int
main(int argc, char *argv[])
{
	// Connect up
	TEST_POSTPARSE(check_monitor_layout);

	return ctwm_main(argc, argv);
}
