#include <stdlib.h>
#include <tet_api.h>

void (*tet_startup)() = NULL, (*tet_cleanup)() = NULL;
void tp1();

struct tet_testlist tet_testlist[] = { {tp1,1}, {NULL,0} };

void tp1()
{
	tet_infoline("This is the second test case (tc2)");
	printf("We have not set TET_OUTPUT_CAPTURE=True ");
	printf("so all normal stdin/stdout/stderr\nfiles are available.  ");
	printf("\nIf we had set output capture,  the results logged by");
	printf(" the API would not be in the journal.\n");
	printf("But these lines would.\n");
	tet_result(TET_PASS);
}

