
#include <stdio.h>

#include "bolt.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE 
#define TRUE (!FALSE)
#endif


/** The purpose of this function is to test that allocation and deallocation 
 * of the bare struct does not leak any memory.  This function should 
 * always pass unless we are out of memory, which is an error that isn't yet 
 * handled.  
 */
int
test_allocation() {

  Bolt * b = bolt_new();
  bolt_delete(b);

  return TRUE;
}


int 
bolttest_test() {

   int passed = TRUE;

   if (test_allocation()) {
      fprintf(stdout,"Passed test_allocation...\n");
   } else {
      fprintf(stdout,"Failed test_allocation...\n");
      passed = FALSE;
   }

   return passed;

}


#ifdef STANDALONE
int
main(int argc, char ** argv) {

   if (bolttest_test()) {
      return 0; 
   } else {
      return -1;
   }
}
#endif
