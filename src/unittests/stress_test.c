
#include <stdio.h>
#include <math.h>


#include "stress.h"





void 
print_header_string(FILE * fp, const char * tag) {

   char header[] = {"\n\n================  %s =================\n"};

   fprintf(fp,header,tag);
}



int
array_equals_double(double * d1, double * d2, int index, size_t size) {

  int i;

  for (i=index; i<(index+size); i++) {
    if (fabs(d1[i]-d2[i]) > 0.0000001) {
      return 0;
    }
  } 
  return 1;
}


static double stress[] = {
	                  1.0,
	                  9.81,
	                  1000.0,
	                  0.25,
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

/**  7 slots, index from 1, not 0. */
static double D1[] = {
                      0.0,
	              0.0,
	      	      0.0,
		      0.0,
		      0.001,
		      0.001,
		      0.0
                     };


int 
test_stress_update(void) {

   stress_update_a(stress,D1,0);

   stress_print(stress,(PrintFunc)fprintf,stdout);

   return 0;
}



int
stress_test_arrays() {

   test_stress_update();

   return 0;
}



#ifdef STANDALONE
int
main(int argc, char ** argv) {


   stress_test_arrays();

   return 0;
}
#endif
