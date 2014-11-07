

#include "analysisdata.h"
#include "ddafile.h"
#include "datalog.h"

FILEPOINTERS fp;
Datalog * DLog;

void
dda_display_error(const char * error) {
}

void
ddaml_read_analysis_file(void * userdata, char * filename) {
}

/** See if we are releasing all the memory. */
int
test_adata_new(void) {

  Analysisdata * ad = adata_new();
  adata_delete(ad);
  return 0;
}

int
test_adata_clone(void) {
  Analysisdata * ad1, * ad2;
  ad1 = adata_new();
  ad2 = adata_clone(ad1);
  adata_delete(ad1);
  adata_delete(ad2);
  return 0;
}

int
analysisdata_test(void) {

  test_adata_new();
  test_adata_clone();
  return 0;
}


int
main() {
  analysisdata_test();
  return 0;
}
