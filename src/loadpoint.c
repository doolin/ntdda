
#include <stdlib.h>

#include "loadpoint.h"
#include "ddamemory.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif




Loadpoint *
loadpoint_new(void) {

   Loadpoint * lp;
   lp = (Loadpoint *)calloc(1,sizeof(Loadpoint));
   return lp;
}


Loadpoint *
loadpoint_new_array(int numloadpoints) {

   return (Loadpoint *) calloc (numloadpoints, sizeof (Loadpoint));
}


void 
loadpoint_delete(Loadpoint * lp) {

   if (lp == NULL) {
      return;
   }

   if (lp->vals) {
      free2DMat((void**)lp->vals,lp->loadpointsize1);
   }

   free(lp);
}  




Loadpoint * 
loadpoint_clone(Loadpoint * lpo, int numloadpoints) {

   int i,j,m,n;
   Loadpoint * lp;

   lp = (Loadpoint *) calloc(numloadpoints,sizeof(Loadpoint));

   for (i=0;i<numloadpoints;i++) {

      lp[i].loadpointsize1 = n = lpo[i].loadpointsize1;
      lp[i].loadpointsize2 = m = lpo[i].loadpointsize2;  
      lp[i].vals = DoubMat2DGetMem(n,m);
      for (j=0;j<n;j++) {

         lp[i].vals[j][0] = lpo[i].vals[j][0];
         lp[i].vals[j][1] = lpo[i].vals[j][1];
         lp[i].vals[j][2] = lpo[i].vals[j][2];
      }
   }

   return lp;
}  

void
loadpoint_print (Loadpoint * lp, int numloadpoints, 
                 PrintFunc printer, void * bfp) {

   int i, j, n;
   double ** lpoints;

   printer (bfp, "\nLoad point verify\n");

   for (i = 0; i < numloadpoints; i++) {

     	n = lp[i].loadpointsize1;
     	printer (bfp, "loadpointsize1: %d\n", n);
     	lpoints = lp[i].vals;

     	for (j = 0; j < n; j++) {

	        printer (bfp, "%.4f  %.4f  %.4f\n", 
                    lpoints[j][0], lpoints[j][1], lpoints[j][2]);
      }
   }

   printer (bfp, "End load point verify\n\n");
}


#if 0
void
loadpoint_print_xml(Loadpoint * lp, int numloadpoints, 
                 PrintFunc printer, void * bfp) {

   int i, j, n;
   double ** lpoints;

   for (i = 0; i < numloadpoints; i++) {

     	n = lp[i].loadpointsize1;
     	printer (bfp, "loadpointsize1: %d\n", n);
     	lpoints = lp[i].vals;

     	for (j = 0; j < n; j++) {

	        printer (bfp, "%.4f  %.4f  %.4f\n", 
                    lpoints[j][0], lpoints[j][1], lpoints[j][2]);
      }
   }

}
#endif


#ifdef __cplusplus
}
#endif