
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
 *
 * @return int TRUE passes, FALSE fails.
 */
static int
test_allocation(void) {

  Bolt * b;
  Boltlist * bl;
  
  b = bolt_new();
  bolt_delete(b);

  b = bolt_new_1(1,1,2,2);
  bolt_delete(b);

  bl = boltlist_new();
  boltlist_delete(bl);

  return TRUE;
}


int
test_bolt_endpoints(void) {

  int passed = TRUE;
  Bolt * b;
  double x1,y1,x2,y2;
  /** Test values */
  double a1,b1,a2,b2;

  x1 = 1;
  y1 = 1;
  x2 = 2;
  y2 = 2;

  b = bolt_new();
  bolt_set_endpoints(b,x1,y1,x2,y2);
  bolt_get_endpoints(b,&a1,&b1,&a2,&b2);

  if ( a1 != x1  ||
       b1 != y1  ||
       a2 != x2  ||
       b2 != y2) {
    passed = FALSE;
  } 
  
  bolt_delete(b);

  return passed;
}


int 
test_equals_true(void) {

  int passed = TRUE;

  Bolt * b1 = bolt_new_1(1,1,2,2);
  Bolt * b2 = bolt_new_1(1,1,2,2);

  if (bolt_equals(b1,b2) == 0) {
    passed = FALSE;
  }

  bolt_delete(b1);
  bolt_delete(b2);

  return passed;
}


int 
test_equals_false(void) {

  int passed = TRUE;

  Bolt * b1 = bolt_new_1(1,1,2,2);
  Bolt * b2 = bolt_new_1(2,2,1,1);

  if (bolt_equals(b1,b2) == TRUE) {
    passed = FALSE;
  }

  bolt_delete(b1);
  bolt_delete(b2);

  return passed;
}




int 
test_clone(void) {

  return FALSE;
}



int
test_boltlist_append(void) {

  int passed = TRUE;
  Bolt * b;
  /* extras for testing printing */
  Bolt * b1, * b2, * b3;
  Boltlist * bl;

  b = bolt_new();
  bl = boltlist_new();
  boltlist_append(bl,b);


  b1 = bolt_new();
  b2 = bolt_new();
  b3 = bolt_new();
  boltlist_append(bl,b1);
  boltlist_append(bl,b2);
  boltlist_append(bl,b3);

  boltlist_print(bl,(PrintFunc)fprintf,stdout);

  boltlist_delete(bl);

  /** FIXME: Find a way to "booleanize" this
   * unit test.
   */
  passed = FALSE;

  return passed;
}



/** @todo Set up a table to call these sequentially, it 
 * will save a lot of typing.
 */
int 
bolt_test(void) {

   int passed = TRUE;

   if (test_allocation()) {
      fprintf(stdout,"Passed test_allocation...\n");
   } else {
      fprintf(stdout,"Failed test_allocation...\n");
      passed = FALSE;
   }

   if (test_bolt_endpoints()) {
      fprintf(stdout,"Passed test_bolt_endpoints...\n");
   } else {
      fprintf(stdout,"Failed test_bolt_endpoints...\n");
      passed = FALSE;
   }

   if (test_boltlist_append()) {
      fprintf(stdout,"Passed test_boltlist_append...\n");
   } else {
      fprintf(stdout,"Failed test_boltlist_append...\n");
      passed = FALSE;
   }

   if (test_equals_true()) {
      fprintf(stdout,"Passed test_equals_true...\n");
   } else {
      fprintf(stdout,"Failed test_equals_true...\n");
      passed = FALSE;
   }

   if (test_equals_false()) {
      fprintf(stdout,"Passed test_equals_false...\n");
   } else {
      fprintf(stdout,"Failed test_equals_false...\n");
      passed = FALSE;
   }


   return passed;
}


#ifdef STANDALONE
int
main(int argc, char ** argv) {

   if (bolt_test()) {
      return 0; 
   } else {
      return -1;
   }
}
#endif
