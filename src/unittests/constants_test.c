
#include <stdio.h>

#include "constants.h"

int 
test_constants_new(void) {

  Constants * c = constants_new();

  constants_set_defaults(c);

  constants_init(c,0.01);

  constants_print_xml(c,(PrintFunc)fprintf,stdout);

  constants_delete(c);

  return 0;
}


int 
constants_test(void) {

  test_constants_new();

  return 0;

}

int
main() {

  constants_test();

  return 0;

}
