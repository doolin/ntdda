
#include <stdio.h>
#include <math.h>


#include "stress.h"


/** Stick this in matlab or octave to get 
 * something to compare to:
 */
/*
% Plane stress

a1 = E/(1-(nu*nu));
k = a1*[
        1 nu 0 
        nu 1 0 
        0 0 (1-nu)/2
];

D = [0.001,0.001,0.0001]';


s = k*D

s =

          1.96078431372549
          1.96078431372549
        0.0335570469798658
*/

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


static void stress_init_all(double * s,
                            double rho,
                            double gamma,
                            double E,
                            double nu,
                            double s11,
                            double s22,
                            double s12,
                            double t1,
                            double t2,
                            double t12,
                            double v1,
                            double v2,
                            double v12) {


   s[0] = rho;
   s[1] = gamma;
   s[2] = E;
   s[3] = nu;
   s[4] = s11;
   s[5] = s22;
   s[6] = s12;
   s[7] = t1;
   s[8] = t2;
   s[9] = t12;
   s[10] = v1;
   s[11] = v2;
   s[12] = v12;   

}


static void stress_init_props(double * s,
                              double rho,
                              double gamma,
                              double E,
                              double nu) {

   s[0] = rho;
   s[1] = gamma;
   s[2] = E;
   s[3] = nu;
}



static void stress_init_sigma(double * s,
                              double s11,
                              double s22,
                              double s12) {

   s[4] = s11;
   s[5] = s22;
   s[6] = s12;
}


static void set_strains(double * D, double e1, double e2, double e12) {

   D[4] = e1;
   D[5] = e2;
   D[6] = e12;

}


int 
test_stress_planes(void) {

   int passed = 1;
   int testval; 

   double D[7]   = {0.0};
   double s1[13] = {0.0};
   double s2[13] = {0.0};
   double s3[13] = {0.0};

   print_header_string(stdout,"stress_planestress test");
 

   stress_init_props(s1,2.71, 25.4,1000.0,0.49);
   set_strains(D,0.001,0.001,0.0001);

   stress_init_sigma(s3,
                     1.96078431372549,
                     1.96078431372549,
                     0.0335570469798658);

   stress_planestress(s1,D);
 
   testval = array_equals_double(s1,s3,4,3);

   if (testval == 0) { 
      fprintf(stdout,"stress_planestress failed.\n");
      passed = 0;
   } else {
      fprintf(stdout,"stress_planestress passed.\n");
   }

    
   stress_init_props(s2,2.71, 25.4,1000.0,0.25);
   stress_init_sigma(s3, 1.6, 1.6, 0.04);

   stress_planestrain(s2,D);
 
   testval = array_equals_double(s2,s3,4,3);

   if (testval == 0) { 
      fprintf(stdout,"stress_planestrain failed.\n");
      passed = 0;
   } else {
      fprintf(stdout,"stress_planestrain passed.\n");
   }

    

   //stress_print(s2,(PrintFunc)fprintf,stdout);

   return 0;
}



/** Zero strain had better return zero stress. 
 */
int 
test_zero_strain() {

   int passed = 1;
   int testval;
 
   double s1[13] = {0.0};
   double s2[13] = {0.0};

   double D[7] = {0.0};
 
   print_header_string(stdout,"Zero strain test");

   stress_init_props(s1,2.71, 25.4,1000.0,0.49);
   stress_init_props(s2,2.71, 25.4,1000.0,0.49);

   stress_planestress(s1,D);
  
   testval = array_equals_double(s1,s2,0,13);

   if (testval == 0) { 
      fprintf(stdout,"Zero strain failed (plane stress).\n");
      passed = 0;
   } else {
      fprintf(stdout,"Zero strain passed (plane stress).\n");
   }

   stress_planestrain(s1,D);

   testval = array_equals_double(s1,s2,0,13);

   if (testval == 0) { 
      fprintf(stdout,"Zero strain failed (plane strain).\n");
      passed = 0;
   } else {
      fprintf(stdout,"Zero strain passed (plane strain).\n");
   }


   return passed;

}

int 
test_stress_equals() {

   int passed = 1;
   int testval;

   double s1[13] = {0.0};
   double * s2;

   print_header_string(stdout,"stress_equals test");

   stress_init_all(s1,2.71, 25.4,1000.0,0.49,0,0,0,0,0,0,0,0,0);

   s2 = stress_clone(s1);
   
   testval = stress_equals(s1,s2,0.0000001);

   if (testval == 0) { 
      fprintf(stdout,"Stress equals failed.\n");
      passed = 0;
   } else {
      fprintf(stdout,"Stress equals passed.\n");
   }
   

   return passed;
}


int 
test_stress_update(void) {

   int passed = 1;
   //int testval;
   int numblocks = 3; 

  /* Permutation vector. */
   int      k1[4] = {0,3,1,2};
   double * D[4]  = {0,0,0,0};
   double * e0[4] = {0,0,0,0};
   
  /* deformations */
   double d1[7], d2[7], d3[7];
  /* blocks */
   double s1[13],s2[13],s3[13];

   print_header_string(stdout,"stress_update test");


   set_strains(d1,0.001,0.001,0.0001);
   set_strains(d2,0.001,0.001,0.0001);
   set_strains(d3,0.001,0.001,0.0001);


  /* These need to match the permutation vector. */ 
   D[1] = d2;
   D[2] = d3;
   D[3] = d1;

   stress_init_all(s1,2.71, 25.4,1000.0,0.49,0,0,0,0,0,0,0,0,0);
   stress_init_all(s2,2.71, 25.4,1000.0,0.49,0,0,0,0,0,0,0,0,0);
   stress_init_all(s3,2.71, 25.4,1000.0,0.49,0,0,0,0,0,0,0,0,0);

   e0[1] = s1;
   e0[2] = s2;
   e0[3] = s3;

   stress_update(e0,D,k1,numblocks,stress_planestrain, strain_linear_elastic);

   return passed;
}


int
test_strain_green_lagrange() {

   int passed = 1;

   double strain[4] = {0.0};
   double F[7]      = {0.0, 0.0, 0.0, 0.5, 0.3, 0.2, 0.3};
//   double E[4]      = {

}


int
stress_test_arrays(void) {

   test_stress_equals();
   test_stress_planes();
   test_zero_strain();
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
