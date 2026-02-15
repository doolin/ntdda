
#include "geometrydata.h"
#include "ddafile.h"

FILEPOINTERS fp;



void
dda_display_error(const char * error) {

}

void
dda_display_warning(const char * warning) {

}

void
ddaml_read_geometry_file(void * userdata, char * filename) {


}


/** See if we are releasing all the memory. */
int
test_gdata_new(void) {

  Geometrydata * gd = gdata_new();

  printf("foo\n");

  gdata_delete(gd);

  return 0;
}

int
geometrydata_test(void) {

  test_gdata_new();

  return 0;
}


int
main() {

  geometrydata_test();

  return 0;
}
