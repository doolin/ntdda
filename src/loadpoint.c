
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


void
loadpoint_print_current(Loadpoint * lp, PrintFunc printer, void * stream) {

   printer(stream, "%f %f %f\n", lp->time, lp->xload, lp->yload);
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





/**************************************************/
/* df09: time interpolation                       */
/**************************************************/
/* This appears to be linear lagrange intepolation
 * for time dependent loading points.  Note that fixed 
 * points are considered time dependent, except that from 
 * time 0 to 100000, 0 load is applied.  Behavior for
 * t > 100000 appears to be unspecified...
 */
//void df09(Geometrydata *gd, Analysisdata *ad)

/** @todo The whole loading point scheme needs to be rewritten.
 * It is still too confusing to maintain.
 */
void df09(Loadpoint * lp,
          double ** points,
          double ** globalTime,
          double ** timeDeps,
          int ** tindex,
          int nfp,
          int numloadpoints,
          int cts,
          double delta_t) {

   int i;
   int j;
   int n;
   double a1;
   double current_time;
   double **lpoints;
   const int TIME = 0;
  
   double dt;

  /* a3 is the elapsed time at the end of the current step.
   * This allows to find the force to apply in the current
   * configuration (applied at a point in the reference 
   * configuration).
   */
  /* FIXME: Note where globaltime is updated.
   */
   current_time = globalTime[cts-1][0]+(delta_t);
      

   for (i = 0; i < numloadpoints; i++) {

     	n = lp[i].loadpointsize1;
     	lpoints = lp[i].vals;
     	for (j = 0; j < n-2; j++) {
         if ( (lpoints[j][TIME] <= current_time) && 
              (current_time  <= lpoints[j+1][TIME]) ) {
            break;
          }   
	    }

      dt = (lpoints[j+1][TIME] - lpoints[j][TIME]);
      
      
      if (dt <= 0) {
         lp->display_error("Loadpoint error: Adjacent time step values must be different.\n");
      }
      
		a1 = (current_time - lpoints[j][TIME]) / dt;
      lp->time = current_time;
      lp->xload = lpoints[j][1] + a1*(lpoints[j+1][1] - lpoints[j][1]);
      lp->yload = lpoints[j][2] + a1*(lpoints[j+1][2] - lpoints[j][2]);
      points[i+nfp+1][4] = lp->xload;
      points[i+nfp+1][5] = lp->yload;
   }

}  




#ifdef __cplusplus
}
#endif