
#include <stdio.h>

#include "constants.h"
#include "unittest.h"


int 
test_constants_new(void) {

  Constants * c = constants_new();

  constants_set_defaults(c);

  constants_init(c,0.01);

  constants_print_xml(c,(PrintFunc)fprintf,stdout);

  constants_delete(c);

  return 1;
}



int
test_constants_new_defaults(void) {

  Constants * c = constants_new_defaults();

  constants_delete(c);

  return 0;
}



int 
test_constants_get_set_w0(void) {
	
  int passed = 1;

  double w0 = 0.01; 
  double test_w0;
   
  Constants * c = constants_new_defaults();

  constants_set_w0(c,w0);
  test_w0 = constants_get_w0(c);

  if (test_w0 != w0) {
     fprintf(stdout,"Failed constants_get_set_w0\n");
     passed = 0;
  } else {
     fprintf(stdout,"Passed constants_get_set_w0\n");
  }

  return passed;

}



int 
test_constants_get_set_shear_norm_ratio(void) {
	
  int passed = 1;

  double shear_norm_ratio = 2.51; 
  double test_shear_norm_ratio;
   
  Constants * c = constants_new();

  constants_set_shear_norm_ratio(c,shear_norm_ratio);
  test_shear_norm_ratio = constants_get_shear_norm_ratio(c);

  if (test_shear_norm_ratio != shear_norm_ratio) {
     fprintf(stdout,"Failed constants_get_set_shear_norm_ratio\n");
     passed = 0;
  } else {
     fprintf(stdout,"Passed constants_get_set_shear_norm_ratio\n");
  }

  return passed;

}


int 
test_constants_get_set_openclose(void) {
	
  int passed = 1;

  double openclose = 0.0002001; 
  double test_openclose;
   
  Constants * c = constants_new();

  constants_set_openclose(c,openclose);
  test_openclose = constants_get_openclose(c);

  if (test_openclose != openclose) {
     fprintf(stdout,"Failed constants_get_set_openclose\n");
     passed = 0;
  } else {
     fprintf(stdout,"Passed constants_get_set_openclose\n");
  }

  return passed;

}



int 
test_constants_get_set_opencriteria(void) {
	
  int passed = 1;

  double opencriteria = 0.0000021; 
  double test_opencriteria;
   
  Constants * c = constants_new();

  constants_set_opencriteria(c,opencriteria);
  test_opencriteria = constants_get_opencriteria(c);

  if (test_opencriteria != opencriteria) {
     fprintf(stdout,"Failed constants_get_set_opencriteria\n");
     passed = 0;
  } else {
     fprintf(stdout,"Passed constants_get_set_opencriteria\n");
  }

  return passed;

}


int 
test_constants_get_set_norm_spring_pen(void) {
	
  int passed = 1;

  double norm_spring_pen = 0.0002001; 
  double test_norm_spring_pen;
   
  Constants * c = constants_new();

  constants_set_norm_spring_pen(c,norm_spring_pen);
  test_norm_spring_pen = constants_get_norm_spring_pen(c);

  if (test_norm_spring_pen != norm_spring_pen) {
     fprintf(stdout,"Failed constants_get_set_norm_spring_pen\n");
     passed = 0;
  } else {
     fprintf(stdout,"Passed constants_get_set_norm_spring_pen\n");
  }

  return passed;

}


int 
test_constants_get_set_angle_olap(void) {
	
  int passed = 1;

  double angle_olap = 3.001; 
  double test_angle_olap;
   
  Constants * c = constants_new();

  constants_set_angle_olap(c,angle_olap);
  test_angle_olap = constants_get_angle_olap(c);

  if (test_angle_olap != angle_olap) {
     fprintf(stdout,"Failed constants_get_set_angle_olap\n");
     passed = 0;
  } else {
     fprintf(stdout,"Passed constants_get_set_angle_olap\n");
  }

  return passed;

}



int 
test_constants_get_set_min_refline_factor(void) {
	
  int passed = 1;

  double min_refline_factor = 3.001; 
  double test_min_refline_factor;

  Constants * c = constants_new();

  fprintf(stdout,"min_reflines not yet implemented...\n");
  return 0;
   

  constants_set_min_refline_factor(c,min_refline_factor);
  test_min_refline_factor = constants_get_min_refline_factor(c);

  if (test_min_refline_factor != min_refline_factor) {
     fprintf(stdout,"Failed constants_get_set_min_refline_factor\n");
     passed = 0;
  } else {
     fprintf(stdout,"Passed constants_get_set_min_refline_factor\n");
  }

  return passed;

}


int 
test_constants_get_norm_extern_dist(void) {
	
  int passed = 1;

  double norm_extern_dist = -3.001; 
  double test_norm_extern_dist;
   
  Constants * c = constants_new_defaults();

  test_norm_extern_dist = constants_get_norm_extern_dist(c);

  if (test_norm_extern_dist != norm_extern_dist) {
     fprintf(stdout,"Failed constants_get_set_norm_extern_dist\n");
     passed = 0;
  } else {
     fprintf(stdout,"Passed constants_get_set_norm_extern_dist\n");
  }

  return passed;

}



int 
test_constants_get_norm_pen_dist(void) {
	
  int passed = 1;

  double norm_pen_dist = -3.001; 
  double test_norm_pen_dist;
   
  Constants * c = constants_new_defaults();

  test_norm_pen_dist = constants_get_norm_pen_dist(c);

  if (test_norm_pen_dist != norm_pen_dist) {
     fprintf(stdout,"Failed constants_get_set_norm_pen_dist\n");
     passed = 0;
  } else {
     fprintf(stdout,"Passed constants_get_set_norm_pen_dist\n");
  }

  return passed;

}




TestFunc tests[] = {
{test_constants_new,          "constants_new"            },
{test_constants_new_defaults, "constants_new_defaults"   },
{test_constants_get_set_w0,   "constants_get_set_w0"},
{test_constants_get_set_shear_norm_ratio,"constants_get_set_shear_norm_ratio"},
{test_constants_get_set_openclose,   "constants_get_set_openclose"},
{test_constants_get_set_opencriteria,   "constants_get_set_opencriteria"},
{test_constants_get_set_norm_spring_pen, "constants_get_set_norm_spring_pen"},
{test_constants_get_set_angle_olap, "constants_get_set_angle_olap"},
{test_constants_get_set_min_refline_factor, "constants_get_set_min_refline_factor"},
{test_constants_get_norm_extern_dist, "constants_get_norm_extern_dist"},
{test_constants_get_norm_pen_dist, "constants_get_norm_pen_dist"},
{NULL,                        NULL                       }
};





int 
constants_test(void) {

  int passed = FALSE;

  if (unittest(tests)) {
     fprintf(stdout,"Passed constants_test\n");
     passed = TRUE;
  } else {
     fprintf(stdout,"Failed constants_test\n");
  }


  return passed;

}

int
main() {

  constants_test();

  return 0;

}
