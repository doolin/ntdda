
#include <assert.h>
#include "datalog.h"
#include "ddamemory.h"
#include "analysisdata.h"
#include "geometrydata.h"



/*
void 
datalog_set_number_of_timesteps(Datalog * dl, int numtsteps) {
}
*/

/* This struct records a large amount of data
 * collected during an analysis.  
 */
void
datalog_delete(Datalog * dl) {

   assert(dl != NULL);

   if(dl->phi)
      free2DMat(dl->phi,dl->phisize1);

  /* FIXME: Shred all this memory */
   if(dl->openclosecount)
      free(dl->openclosecount);

   if(dl->timestepcut)
      free(dl->timestepcut);

   if(dl->numdistcontact)
      free(dl->numdistcontact);

   if(dl->numanglecontact)
      free(dl->numanglecontact); 

   if(dl->numsidesperblock)
      free(dl->numsidesperblock);

   if(dl->num_v_v_contacts)
      free(dl->num_v_v_contacts); 

   if(dl->num_v_s_contacts)
      free(dl->num_v_s_contacts);

   if(dl->num_cvex_cvex_contacts)
      free(dl->num_cvex_cvex_contacts);

   if(dl->num_cvex_ccave_contacts)
      free(dl->num_cvex_ccave_contacts);

   if(dl->memallocsize)
      free(dl->memallocsize);

  /* Store the starting mass in slot 0 */
   if (dl->blockmass)
      free2DMat(dl->blockmass, dl->blockmass_size1);

   if (dl->energy)
      free(dl->energy);

   free(dl);

} 



Datalog * 
initDatalog(Geometrydata * gd, Analysisdata * ad)
{
   int numtimesteplus1 = ad->nTimeSteps+1;

  /* Log struct for collecting data during a run.
   */
   Datalog * dlo;
   dlo = (Datalog *)calloc(1,sizeof(Datalog));
   
  /* At some point this stuff will need to be set in 
   * this function.   This will likely have to be rewritten
   * when disp dep params get implemented for real.
   */
   dlo->phisize1 = ad->nTimeSteps + 1;
   dlo->phisize2 = ad->nJointMats + 1;
   dlo->phi = DoubMat2DGetMem(dlo->phisize1, dlo->phisize2);

   dlo->openclosesize1 = numtimesteplus1;
   dlo->openclosecount = (int*)calloc(numtimesteplus1,sizeof(int));

   dlo->timestepcutsize1 = numtimesteplus1;
   dlo->timestepcut = (int*)calloc(numtimesteplus1,sizeof(int));

   dlo->numdistcontactsize1 = numtimesteplus1;
   dlo->numdistcontact = (int*)calloc(numtimesteplus1, sizeof(int));

   dlo->numanglecontactsize1 = numtimesteplus1;
   dlo->numanglecontact = (int*)calloc(numtimesteplus1, sizeof(int));

  /* This is the mean of all the blocks */
   dlo->numsidessize1 = gd->nBlocks;
   dlo->numsidesperblock = (int*)calloc(numtimesteplus1, sizeof(int));

   dlo->num_v_v_contactsize1 = numtimesteplus1;
   dlo->num_v_v_contacts = (int*)calloc(numtimesteplus1, sizeof(int));

   dlo->num_v_s_contactsize1 = numtimesteplus1;
   dlo->num_v_s_contacts = (int*)calloc(numtimesteplus1, sizeof(int));

   dlo->num_cvex_cvex_contactsize1 = numtimesteplus1;
   dlo->num_cvex_cvex_contacts = (int*)calloc(numtimesteplus1, sizeof(int));

   dlo->num_cvex_ccave_contactsize1 = numtimesteplus1;
   dlo->num_cvex_ccave_contacts = (int*)calloc(numtimesteplus1, sizeof(int));

   dlo->memallocsize1 = numtimesteplus1;
   dlo->memallocsize = (int*)calloc(numtimesteplus1, sizeof(int));

  /* Store the starting mass in slot 0 */
   dlo->blockmass_size1 = numtimesteplus1;
   dlo->blockmass_size2 = gd->nBlocks+1;
   dlo->blockmass = DoubMat2DGetMem(dlo->blockmass_size1,dlo->blockmass_size2);

  /* Contact forces handled also.  This will be trickier. */

  /* Monitor energy consumption. */
   dlo->energysize1 = numtimesteplus1;
   dlo->energy = (Energy *) calloc(numtimesteplus1,sizeof(Energy));

   return dlo;

}  



Datalog * 
cloneDatalog(Datalog * dln)
{
  /* Log struct for collecting data during a run.
   */
   Datalog * dlo;

   /* FIXME: call initDataLog() instead */
   dlo = (Datalog *)malloc(sizeof(Datalog));

   return dlo;

}  /* Close DatalogStruct() */
