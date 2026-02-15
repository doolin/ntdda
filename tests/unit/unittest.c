
#include <stdio.h>

#include "unittest.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



void
unittest_print_header(FILE * ofp, const char * tag) {

  char header[] = {"\n\n=================  %s  =================\n"};

  fprintf(ofp,header,tag);

}


int
unittest(TestFunc * testfunc) {

    int i = 0;
    int passed = TRUE;

    while (testfunc[i].test != NULL) {

        unittest_print_header(stdout,testfunc[i].testname);

	if (testfunc[i].test()) {
	    fprintf(stdout, "Passed test_%s.\n", testfunc[i].testname);
	} else {
	    fprintf(stdout, "Failed test_%s.\n", testfunc[i].testname);
	    passed = FALSE;
	}
	i++;
    }

    return passed;
}



/** @todo Figure out how to tweak the Makefile.am to handle
 * including with various preprocessor #defines.  The following
 * doesn't work.
 */
#ifdef UNITTEST
int
main(int argc, char ** argv) {

  fixedpoint_test();

  return 0;

}

#endif  /* UNITTEST */

#ifdef __cplusplus
}
#endif
