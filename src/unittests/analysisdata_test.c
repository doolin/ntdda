

#include "analysisdata.h"
#include "gravity.h"
#include "analysis.h"
#include "geometry.h"

/** See if we are releasing all the memory. */
int 
test_adata_new(void) {

  Analysisdata * ad = adata_new();

  adata_delete(ad);

  return 0;
}

int
analysisdata_test(void) {

  test_adata_new();

  return 0;
}


int 
main() {



  return 0;
}
