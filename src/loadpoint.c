
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




#ifdef __cplusplus
}
#endif