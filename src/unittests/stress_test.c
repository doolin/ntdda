
#include <stdio.h>
#include <math.h>


#include "stress.h"




void 
print_header_string(FILE * fp, const char * tag) {

   char header[] = {"\n\n================  %s =================\n"};

   fprintf(fp,header,tag);
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

/**  Start with an identity strain.
 */
static double D1[] = {
	              0.0,
	      	      0.0,
		      0.0,
		      1.0,
		      1.0,
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
