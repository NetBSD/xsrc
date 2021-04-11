/*
 * Test ExpandFilePath()
 */

#include "ctwm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	char *in = strdup("foo:~/bar");
	Home = "FOO"; // Just overwrite global
#define EXPECT "foo:FOO/bar"

	char *ret = ExpandFilePath(in);
	if(strcmp(ret, EXPECT) == 0)
		exit(0);
	fprintf(stderr, "'%s' != expected '%s'\n", ret, EXPECT);
	exit(1);
}
