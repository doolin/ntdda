
#include <stdio.h>
#include <math.h>


#include "bolt.h"


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE 
#define TRUE (!FALSE)
#endif



/**
 * Static arrays for testing array-based bolt
 *  functions.
 */

static double bolt1[17] = {
            0.0,
            1.0,
            1.0,
            2.0,
            2.0,
            1.0,
            2.0,
            100000.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0
          };

static double bolt2[17] = {
                            0.0,
                            10.0,
                            0.0,
                            15.0,
                            0.0,
                            0.0,
                            0.0,
                            100000.0,
                            0.0,
                            10000.0,
                            0.0,
                            0.0,
                            0.0,
                            0.0,
                            0.0,
                            0.0,
                            0.0
                          };

static double * bolts[2] = {0, 0};


void
print_header_string(FILE * fp, const char * tag) {

   char header[] = {"\n\n================  %s =================\n"};

   fprintf(fp,header,tag);
}


int
test_bolt_get_ref_length_a(void) {

   int passed = 1;

   double length;
   double testlength;

   FILE * ofp = stdout;

   bolts[0] = bolt1;
   bolts[1] = bolt2;

   print_header_string(ofp,"referential length test");

   bolt_set_length_a(bolts[0]);
   bolt_set_ref_length_a(bolts[0]);
   testlength = bolt_get_ref_length_a(bolts[0]);
   length = sqrt(2.0);  /* No pretension on this bolt. */

   if (testlength != length) {
      fprintf(ofp,"Failed bolt 1 ref length test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 1 ref length test.\n");
   }

   bolt_set_length_a(bolts[1]);
   bolt_set_ref_length_a(bolts[1]);
   testlength = bolt_get_ref_length_a(bolts[1]);
   length = 5 - 0.1;

   if (testlength != length) {
      fprintf(ofp,"Failed bolt 2 ref length test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 2 ref length test.\n");
   }

   return passed;
}



int
test_bolt_dir_cos_a(void) {

   int passed = 1;

   double lx,ly;
   double testlx,testly;

   FILE * ofp = stdout;

   print_header_string(ofp,"direction cosine test");

   bolt_get_dir_cosine_a (bolt1, &testlx, &testly);
   lx = (1-2)/sqrt(2);
   ly = (1-2)/sqrt(2);

   if ( (testlx != lx) && (testly != ly) ) {
      fprintf(ofp,"Failed bolt 1 dir cosine test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 1 dir cosine test.\n");
   }



   bolt_get_dir_cosine_a (bolt1, &testlx, &testly);
   lx = (10-15)/5.0;
   lx = (0-0)/5.0;


   if ( (testlx != lx) && (testly != ly) ) {
      fprintf(ofp,"Failed bolt 2 dir cosine test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 2 dir cosine test.\n");
   }


   return passed;
}



int
test_bolt_get_length_a(void) {

   int passed = 1;

   double length;
   double testlength;

   FILE * ofp = stdout;

   bolts[0] = bolt1;
   bolts[1] = bolt2;

   print_header_string(ofp,"length test");

   bolt_set_length_a(bolts[0]);
   testlength = bolt_get_length_a(bolts[0]);
//   printf("bolt 1 test length: %f\n",testlength);
   length = sqrt(2.0);

   if (testlength != length) {
      fprintf(ofp,"Failed bolt 1 length test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 1 length test.\n");
   }

   bolt_set_length_a(bolts[1]);
   testlength = bolt_get_length_a(bolts[1]);
//   printf("bolt 2 test length: %f\n",testlength);
   length = 5;

   if (testlength != length) {
      fprintf(ofp,"Failed bolt 2 length test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 2 length test.\n");
   }



   return passed;
}



int
test_bolt_get_dl_a(void) {

   int passed = 1;

   double dl;
   double testdl;

   FILE * ofp = stdout;

   bolts[0] = bolt1;
   bolts[1] = bolt2;

   print_header_string(ofp,"differential length test");

   bolt_set_length_a(bolts[0]);
   testdl = bolt_set_ref_length_a(bolts[0]);
   dl = 0.0;   /* No pretension on this bolt. */

   if (testdl != dl) {
      fprintf(ofp,"Failed bolt 1 dl test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 1 dl test.\n");
   }

   bolt_set_length_a(bolts[1]);
   testdl = bolt_set_ref_length_a(bolts[1]);
   dl = 0.1;

   if (testdl != dl) {
      fprintf(ofp,"Failed bolt 2 dl test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 2 dl test.\n");
   }


   return passed;


}



/**
 * This is actually a pretty good test, given the API
 * (API might be crappy, but thats a different issue).
 * What happens here is a "round trip", where the
 * reference length is computed from the given pretension,
 * then the pretension returned is computed from the
 * reference length.
 */
int
test_bolt_get_pretension_a(void) {


   int passed = 1;

   double tension;
   double testtension;

   FILE * ofp = stdout;


   print_header_string(ofp,"pretension test");

   bolt_set_length_a(bolt1);
   bolt_set_ref_length_a(bolt1);
   bolt_set_pretension_a(bolt1);
   testtension = bolt_get_pretension_a(bolt1);
   tension = 0.0;

   if (testtension != tension) {
      fprintf(ofp,"Failed bolt 1 pretension test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 1 pretension test.\n");
   }


   bolt_set_length_a(bolt2);
   bolt_set_ref_length_a(bolt2);
   bolt_set_pretension_a(bolt2);
   testtension = bolt_get_pretension_a(bolt2);
   tension = 10000.0;

//   fprintf(ofp,"testtension: %f, tension: %f\n",testtension,tension);

  /** Ask for 7 decimal digit precision. */
   if (fabs(testtension-tension) > 0.0000001) {
      fprintf(ofp,"Failed bolt 2 pretension test.\n");
      passed = 0;
   } else {
      fprintf(ofp,"Passed bolt 2 pretension test.\n");
   }


   return passed;
}


int
bolt_test_arrays() {

   test_bolt_dir_cos_a();
   test_bolt_get_length_a();
   test_bolt_get_ref_length_a();
   test_bolt_get_dl_a();
   test_bolt_get_pretension_a();

   return 0;
}


/**
 * The purpose of this function is to test that allocation and deallocation
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


   bolt_test_arrays();


//*
   if (bolt_test()) {
      return 0;
   } else {
      return -1;
   }
//*/

   return 0;

}
#endif
