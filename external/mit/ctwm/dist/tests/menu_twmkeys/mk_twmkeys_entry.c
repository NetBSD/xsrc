/*
 * Test mk_twmkeys_entry()
 */

#include "ctwm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "menus.h"

int
main(int argc, char *argv[])
{
	FuncKey key;

	// Init
	memset(&key, 0, sizeof(key));
	key.name   = "KEY";
	key.action = "ACT";

#define TST(expect) do { \
                const char *ret = mk_twmkeys_entry(&key); \
                if(strcmp(ret, expect) != 0) { \
                        fprintf(stderr, "Expected '%s', got '%s'\n", \
                                        expect, ret); \
                        exit(1); \
                } \
        } while(0)

	// Simple
	key.mods = ShiftMask;
	TST("[S+KEY] ACT");

	key.mods = ControlMask;
	TST("[C+KEY] ACT");

	// Combo
	key.mods = Mod1Mask | Alt1Mask;
	TST("[M+A1+KEY] ACT");

	key.mods = Alt1Mask | Alt2Mask | Alt3Mask | Alt4Mask | Alt5Mask;
	TST("[A1+A2+A3+A4+A5+KEY] ACT");

	// All the mods!
	key.mods = ShiftMask | ControlMask
	           | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask
	           | Alt1Mask | Alt2Mask | Alt3Mask | Alt4Mask | Alt5Mask;
	TST("[M+S+C+M2+M3+M4+M5+A1+A2+A3+A4+A5+KEY] ACT");


	// Magic value used to test overflow
	{
		// Overflow by 1: inherit mods from above, and add a bit
		key.mods |= 1 << 30;

		const char *ret = mk_twmkeys_entry(&key);
		if(ret != NULL) {
			fprintf(stderr, "Should have blown up for Over1, instead "
			        "got '%s'.\n", ret);
			exit(1);
		}
	}
	{
		// Overflow by itself
		key.mods = 1 << 31;

		const char *ret = mk_twmkeys_entry(&key);
		if(ret != NULL) {
			fprintf(stderr, "Should have blown up for OverAll, instead "
			        "got '%s'.\n", ret);
			exit(1);
		}
	}


	// OK then
	exit(0);
}
