/*
 * stuff that is set during compile time. 
 * these should all be made user controllable
 * at some point in the future.
 */

#include "analysisdata.h"


void
compilecontrol(Analysisdata * AData)
{
/*****  Options ***********/
  /* These need to be options put into the DDAML DTD */
   //AData->seismicflag = 0; //FALSE;
   AData->solvetype = lu;
  /* Not really implemented yet...  Next step is to set the
   * initialize the lagrange variables before the open-close
   * iteration starts for any particular time step.
   */
   //AData->contactmethod = auglagrange; 
   AData->contactmethod = penalty; 
   AData->integrator = constant;
  /* TODO: Set this from the xml file at least. */
   AData->tsSaveInterval = 1;
   AData->frictionlaw = tpmc;

  /* Desparate kludge */
   AData->writemfile = TRUE;


/**************************/

}  /* close compilecontrol() */