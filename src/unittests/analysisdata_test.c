

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
analysisdata_test(void) {

  test_adata_new();

  return 0;
}


int 
main() {

  analysisdata_test();

  return 0;
}
