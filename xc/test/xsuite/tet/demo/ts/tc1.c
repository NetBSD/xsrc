#include <stdlib.h>
#include <tet_api.h>

void (*tet_startup)() = NULL, (*tet_cleanup)() = NULL;
void tp1();

struct tet_testlist tet_testlist[] = { {tp1,1}, {NULL,0} };

void tp1()
{
	tet_infoline("This is the first test case (tc1)");
	tet_result(TET_PASS);
}

