#include "material.h"

int
test_material_new(void) {

  int numblocks = 3;
  Material * m = material_new(numblocks);
  material_delete(m);
  return 1;
}

int
material_test(void) {

  test_material_new();
  return 0;
}

int
main () {
  material_test();
  return 0;
}
