
#include <stdio.h>

#include "loadpoint.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif


static int 
test_loadpoint_new(void) {

   int passed = TRUE;

   Loadpoint * lp = loadpoint_new();
   loadpoint_delete(lp);

   return passed;
}



struct _funcs {
  int (*test)(void);
  const char * testname;
};

static struct _funcs testfunc[] = {
  {*test_loadpoint_new,"loadpoint_new"},
  {NULL,"null"}
};


int
loadpoint_test() {

   int i = 0;
   int passed = TRUE;
  
   while (testfunc[i].test != NULL) {

      if (testfunc[i].test()) {
	      fprintf(stdout,"Passed test_%s.\n",testfunc[i].testname);
      } else {
	      fprintf(stdout,"Failed test_%s.\n",testfunc[i].testname);
	      passed = FALSE;
	   }
      i++;
   }

   return passed;

}



#ifdef __cplusplus
}
#endif


#ifdef STANDALONE
#include <stdio.h>

int 
main (int argc, char ** argv) {

   int checkval; 

   checkval = loadpoint_test();
    

   //printf("---\nFALSE value: %d\nTRUE value:  %d\n---\n",FALSE,TRUE);

   if (checkval != 0) {
      fprintf(stdout,"loadpoint unit test passed\n");
      return 0;
   } else {
      fprintf(stdout,"loadpoint unit test failed\n");
      return -1;
   }

}
#endif /* STANDALONE */
