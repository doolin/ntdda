/* 
 * combineddf.c
 * 
 * Contact and matrix solver for DDA.
 *
 * $Author: doolin $
 * $Date: 2001/07/15 00:04:30 $
 * $Source: /cvsroot/dda/ntdda/src/combineddf.c,v $
 * $Revision: 1.11 $
 *
 */
/*################################################*/
/* ------     df.c WINDOWS version      ------ */
/*       discontinuous deformation analysis       */
/*################################################*/


/*
 * $Log: combineddf.c,v $
 * Revision 1.11  2001/07/15 00:04:30  doolin
 * Minor changes.
 *
 * Revision 1.10  2001/07/14 23:13:44  doolin
 * Added a contact unit test, more comments in open close fns.
 *
 * Revision 1.9  2001/07/14 15:39:37  doolin
 * Worked in df18 and df22 on contact and locking.
 *
 * Revision 1.8  2001/07/01 22:43:54  doolin
 * Rewrote the logic in the getLockStates function.
 * The previous logic is saved in the getLockStatesOld function.
 * All this needs to be verified against the original code.  If there
 * is a big screw up, just use the original function, and-or back out
 * the changes from cvs using revision numbers.
 *
 * Revision 1.7  2001/07/01 20:37:56  doolin
 * Changed dimension on the locks matrix QQ in df18.
 * Note the revision number of this carefully, because these
 * changes may need to be backed out.
 *
 */

#include "analysis.h"
#include <math.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>
#include "errorhandler.h"
#include "ddamemory.h"
#include "interface.h"
#include "ddaerror.h"
#include "printdebug.h"
#include "gravity.h"
#include "contacts.h"
#include "postprocess.h"




/* BUG BUG BUG BUG
 * DISPDEP compiles in a call (df18 and df22) 
 * to a frictional decay function that provides 
 * a friction angle based on the value of the 
 * displacement of a measured point.  If there is 
 * no measured point, the call will produce spurious
 * results due to garbage in the over-allocated arrays.
 */   
//#define DISPDEP 0

extern FILEPOINTERS fp;
extern DATALOG * DLog;
extern InterFace * iface;
extern DDAError ddaerror;

/**************************************************/
/* df01: input geometric data                     */
/**************************************************/
/* df01 needs to be renamed to reflect its function
 * for scaling the problem domain.
 */
double  df01(Geometrydata * bd) 
{

   int i, j, i1, i2;
   int nBlocks = bd->nBlocks;
   double ** vertices = bd->vertices;
   int **vindex = bd->vindex;
   double maxx, minx, maxy, miny;
  /* FIXME: Why are these half values instead of full values? */
   //double halfheight, halfwidth;
   double height, width;
 
   minx = vertices[1][1];
   maxx = vertices[1][1];
   miny = vertices[1][2];
   maxy = vertices[1][2];

   for (i=1; i<= nBlocks; i++)
   {
      i1=vindex[i][1];
      i2=vindex[i][2];
      for (j=i1; j<= i2; j++)
      {
         if (minx > vertices[j][1])  
            minx = vertices[j][1];

         if (maxx < vertices[j][1])  
            maxx = vertices[j][1];

         if (miny > vertices[j][2])  
            miny = vertices[j][2];

         if (maxy < vertices[j][2])  
            maxy = vertices[j][2];
      }
   }  

   height = maxy-miny;
   width = maxx-minx;

  /* FIXME: get rid of the 2.0 divisor.  When this is done, 
   * the maxdisplacement parameter for every analysis file 
   * will have to be changed.
   */
   return max(width,height)/2.0;

}  /*  End df01  */


/* The purpose of this subroutine is to separate apparently 
 * dissimilar areas of code and help handle variable scoping.
 */
void initNewAnalysis(Geometrydata * gd, Analysisdata *ad, double **e0,
      /* double **blockArea,*/ FILEPATHS * filepath, GRAPHICS * g)
{
   int i, j, i1, i2;
   double ** vertices = gd->vertices;
   int nJointMats = ad->nJointMats;
   int **vindex = gd->vindex;
   int nBlocks = gd->nBlocks;
   extern DATALOG * DLog;
   //double ** blockArea = gd->moments;
    
  /* This will be reset if adaptive time-stepping
   * is enabled.
   */
   ad->delta_t = ad->maxtimestep;

   //g->initstresses(ad, g, nBlocks);

  /* Scale the problem domain so that the GUI will 
   * show all the action.  ac->w0 is scaled from the 
   * absolute dimensions of the physical problem. (????)
   */
   ad->constants->w0 = df01(gd);

  /* ad->pointcount is needed for saving restoring force terms 
   * for fixed points.
   */
   ad->pointCount = gd->pointCount;
  /* Moved to inputAnalysis() */
   //ad->this = ad;
   ad->isRunning = TRUE;

  /* FIXME: The xml input file has this as a user controlled 
   * parameter.  8 is default value from GHS code.
   */
   if (ad->OCLimit == 0)
      ad->OCLimit = 6+2;



  
   //if (ad->c = NULL)
   //{
  /* c : movement u v of fixed measured load points */
  /* c[pointCount+1][3]                             */
  /* It appears that c[][] is used as a flag in 
   * analysisreader(2) to indicate when to read in values
   * for time dependent properties, then later used to 
   * record movement of measured, fixed, and load
   * points.  c[i][0] = 0 indicates true for time dependence,
   * c[i][1] = 1 indicate point is not time depenedent.
   */
   ad->csize1 = ad->pointCount + 1;
   ad->csize2 = 3;
   ad->c = DoubMat2DGetMem (ad->csize1, ad->csize2);
   //}


  /**************************************************/
   /* q0: 0 accumulated real time utill this step    */
   /* q0: 1 real max. dspl./ defined max. dspl.      */
   /* q0: 2 real time interval of this step          */
   /* q0[nTimeSteps+1][3]                            */
   /* TODO: This code should probably not be in this procedure.
    * it is not initialized here, just allocated.  The 
    * name "q0" has been changed to globalTime.  See about 
    * moving this to initializeStuff().
    */
//   if (adn->nTimeSteps > adn->gravTurnonLimit)
//   {
   ad->globaltimesize1 = ad->nTimeSteps + 1;

//   }
//   else
//   {  
//        adn->globaltimesize1 = adn->gravTurnonLimit+1;
//   }
   ad->globaltimesize2 = 3;
   ad->globalTime = DoubMat2DGetMem (ad->globaltimesize1, ad->globaltimesize2);
   ad->globalTime[0][0] = 0;

   /* Miscellaneous initializations.  springstiffness is a saved
    * variable for output
    */
   ad->springstiffsize = ad->nTimeSteps + 1;
  /* FIXME: This will also need to be changed to calloc.
   */
   ad->springstiffness =
      (double *) malloc (sizeof (double) * (ad->springstiffsize));

  /* Initialize avgArea, which will be needed to compute the
   * value of the first time step.  FIXME: check to see whether
   * the next line is redundant.  FIXME: change from malloc to
   * calloc.
   */
   ad->avgArea = (double *)malloc((ad->nTimeSteps+1)*sizeof(double));
   ad->avgArea[1] = computeMoments(gd);//, blockArea);

  /* Need to save the original area for conserving mass. */
   for (i = 1; i<=gd->nBlocks; i++)
   {
      e0[i][9] = gd->moments[i][1];
   }

  /* FIXME: Move initBlockMasses() to a better place. */
   initBlockMasses(gd);


  /* Only as many knowns as blocks DOF. */
   ad->Fsize1=gd->nBlocks+1;  // + index
   ad->Fsize2=7; // blockisze + index
   ad->F = DoubMat2DGetMem(ad->Fsize1, ad->Fsize2);
   ad->Fcopy = DoubMat2DGetMem(ad->Fsize1, ad->Fsize2);

  /* Rearrange the block vertex order slightly so that the
   * area function can perform simplex integration for 
   * centroids and moments by traversing the the block vertex
   * list in a CCW direction.
   * FIXME: Move this call into a "object method" for the geometry
   * data.  Method invocation has to stay near here, in case we get 
   * block info from a file instead of dc.  Might be able to move it
   * into the tail of dc driver code, if dc could be changed to 
   * handle block input as well as joint input.
   */
   vertexInit(gd);

  /* Block number for each vertex is recorded in the nn0
   * array.  FIXME: why are we doing this?  Add more comments
   * after nn0 is used later in df. 
   * TODO: Make this a geometry object method.
   */
   blockNumberOfVertex(gd);


   initBlockProperties(gd, ad, e0);

   /* FIXME:  Why isn't initVelocity working? */
   //initVelocity(gd, ad, v0);

  /* Conservation of energy requires constant mass. Compute
   * the masses up front, use these instead of density*area
   * for inertial and energy terms.
   */
   computeMass(gd, ad, e0);


   /*------------------------------------------------*/
   /* confirm numbers of joint material              */
   /* TODO: Make this into a function. */
   /* This is "fixit" code to catch a joint material
    * number less than 1 or greater than the number of 
    * joint materials specified.  It probably does not
    * belong in here, should be moved to a range checking
    * function, and throw a warning.
    */
   for (i=1;    i<= gd->nBlocks;   i++)
   {
      i1=vindex[i][1];
      i2=vindex[i][2];
      for (j=i1-1; j<= i2+1; j++)
      {
         if ( vertices[j][0] < 1  )  
            vertices[j][0]= 1;
         if ( vertices[j][0] > nJointMats )  
            vertices[j][0]=nJointMats;
      }  /*  j  */
   }  /*  i  */
   
   
  /* Movements at fixed measured load points.  That is,
   * at the start of an analysis, these points have zero
   * displacement.  Rock bolt endpoints are initialized
   * during the geometry computation.
   */
  /* TODO: Make this into a function. */
   for (i=1; i<= gd->pointCount; i++)
   {
      for (j=1; j<=  2; j++)
      {
         ad->c[i][j]=0;
      }  /*  j  */
   }  /*  i  */

  /* Problem:  Have xml parsing code, with bolt materials
   * in the analysis side.  Have to get the bolt materials
   * into the geometry bolt array, without clashing with 
   * previous file formats.
   * FIXME: We can only use bolt material right now!  The 
   * "right" way to fix this would be to have the bolt 
   * material type given in the geometry file as it is 
   * given for joints.  Then match the type with material 
   * later in the analysis.
   */
   if (ad->boltmats != NULL)
   {
      for (i=0;i<gd->nBolts;i++)
      {
         gd->rockbolts[i][7] = ad->boltmats[0][0];
         gd->rockbolts[i][8] = ad->boltmats[0][1];
         gd->rockbolts[i][9] = ad->boltmats[0][2];
      }
   }

	 	
   ad->saveTimeStep = ad->tsSaveInterval;

   initContactSpring(ad);
   initConstants(ad);

   DLog = initDatalog(gd, ad);
  /* Clock stopped in postProces() */
   DLog->analysis_start = clock();
   DLog->assemble_runtime = 0;

   initStorageTempArrays(gd->nBlocks);
   initContactTempArrays(gd->nBlocks, gd->vertexCount);
   initSolverTempArrays(gd);
   initIntTempArrays(gd->vertexCount+1);
   initIntegrationArrays(gd,ad);

   //initCGTemp(gd);



   if(ad->gravityflag == 1)
   {
     /*------------------------------------------------*/
     /* Get memory for contact force saving to use for 
      * gravity force convergence.  This should not be 
      * needed if there is no gravity phase.
      */
      //ad->gravity = newGravity();
      initGravity(ad->gravity, gd->nBlocks, ad->delta_t);
   }  else {
      ad->gravity = NULL;  // kludge
   }

  /* Turn on the instrumentation.  This struct is really
   * a combined instrumentation and data output struct,
   * capabilities that will need to be separated later.
   */
   //DLog = initDatalog(GData, AData);

  /* File handling is global for now.  Since it is global
   * we can open everything at the same time, then later 
   * close everything at the same time.  Later this will 
   * return a value that can be checked for error status.
   */

  /* FIXME: This needs to be changed to openOutputFiles 
   * so that it can be merged with console version.
   */
   openAnalysisFiles(filepath);

   //writeMasses(ad,gd);

   writeBlockareas(ad, gd);

   initReplayFile(gd,ad);

   validateAnalysisdata(ad->this);

}  /*  Close initAnalysis()  */


/* Allocate or reallocate the "stiffness" matrix K.
 * If first time step, get memory for the number of 
 * contacts.  For all time steps after, check to see 
 * if the current number of contacts is less than or
 * equal to the number of previous contacts. If not,
 * reallocate space for the current number of contacts.
 * This function should eventually be rewritten to 
 * return a value indicating success, i.e., enough
 * memory allocated.
 */
void 
allocateK(Analysisdata * ad)
{
  /* For the first time step, malloc arrays for the 
   * number of contacts in the block system.  
   */ 
   //if ( (ad->gravity->gravTimeStep != 1) || (ad->currTimeStep != 1) )
   if ( (ad->currTimeStep != 1) )
   {
     /* (GHS: fprintf(logFile,"@@@@@ o e m  %d %d %d \n", j9, n3, ad->nn00); */
     /* If there are more contacts now than there were 
      * when a and b were malloced, then free the current 
      * a and b arrays and malloc larger arrays.  
      * Then reset contact number.
      */
      if (ad->nn00 >= ad->n3)
         return;
     /* free using prev size of K 
      * Is this redundant?
      */
      ad->ksize1 = ad->nn00 + 1;

      free2DMat((void **)ad->K, ad->ksize1);
      free2DMat((void **)ad->Kcopy, ad->ksize1);
      
   }

  /* Now we have current size of K, ad-> n3 is the number of 
   * contacts with respect to blocks in contact, instead of
   * number of vertices in contact (ad->nCurrContacts?).
   */
   ad->ksize1 = ad->n3+1;
   ad->ksize2 = 37;  // n x n + index

  /*------------------------------------------------*/
  /* a : general equation coefficent matrix          */
  /* a : non-zero 6*6 submatrices according k       */
  /* a[n3+1][37]                                    */
   ad->K = DoubMat2DGetMem(ad->ksize1, ad->ksize2);
  /* b : copy of a                        df20 df24 */
   ad->Kcopy = DoubMat2DGetMem(ad->ksize1, ad->ksize2);

  /* Store the number of contacts used 
   * in mallocing for K and b (old K) arrays.
   * Thus if the number of contacts increases, then the 
   * size of the K array is increased as well.
   */
   ad->nn00 = ad->n3;
      
}  /* close allocateK() */


static void 
getLockStatesOld(int ** locks, int lockstate[3][5], int contact)
{
   int j;
  /* Helper variables to increase readability of the code. */
   const int OPEN = 0;
   const int SLIDING = 1;
   const int LOCKED = 2;
   const int PREVIOUS = 1;
   const int CURRENT = 2;
     /* For each previous step and current step. Closes with label b804
      * TODO: This might be changed into a macro.  j takes on the value
      * 1 for PREVIOUS contact and 2 for CURRENT contact.
      */
      for (j=1; j<= 2;  j++)
      {
        /* FIXME: say what the following matrix represents. */
        /* This initializes qq as a 2 x 4 matrix, e.g.,:
         * qq =  [ 0 0 1 0 ]
         *       [ 1 1 0 0 ].
         */
         lockstate[j][1]=lockstate[j][2]=lockstate[j][3]=lockstate[j][4]=0;
        /* When locks[i][j] = 0, (OPEN) no springs are turned on.
         * FIXME: Find out where locks are set.
         */
         if (locks[contact][j]==OPEN) 
            continue; //goto b801;
        /* else SLIDING or LOCKED */
         lockstate[j][PREVIOUS] = SLIDING;
         lockstate[j][CURRENT] = OPEN;
        /* Else when locks[i][j] = 1, we sliding with the 
         * normal spring turned on.
         */
         if (locks[contact][j]==SLIDING) 
            continue; //goto b801;
        /* else NOTSLIDING */
         lockstate[j][PREVIOUS] = SLIDING;
         lockstate[j][CURRENT] = SLIDING;
        /*  Else when locks[i][j] = 2, the sliding and shear springs
         * are locked (or active?).
         */
         if (locks[contact][j]==LOCKED) 
            continue; //goto b801;
        /* else NOTLOCKED */
         lockstate[j][PREVIOUS] = OPEN;
         lockstate[j][CURRENT] = OPEN;
         lockstate[j][3] = SLIDING;
        /* Else locks[i][j] = 3, and the sliding 
         * "ref line -> 2 normal spring on"
         * (notes are incomprehensible here).
         */ 
//b801:;
      }  /*  j  */
      
     /* (GHS: set on-off modify coefficient) */
     /* Difference in current and previous contact springs.
      * lockstate[1][j] = -1: Spring was on in prev iteration, not in current;
      * lockstate[1][j] =  0: Spring in current and previous are same (goal?); 
      * lockstate[1][j] =  1: Spring on in current not in previous.
      */ 
      for (j=1; j<= 4; j++)
      {
         lockstate[1][j]=(lockstate[2][j]-lockstate[1][j]);
      }  /*  j  */


}  /* close getLockStatesOld() */


static void 
getLockStates(int ** locks, int lockstate[3][5], int contact)
{
   int j;
  /* Helper variables to increase readability of the code. */
   const int OPEN = 0;
   const int CLOSED = 1;
   const int SLIDING = 1;
   const int LOCKED = 2;
   const int PREVIOUS = 1;
   const int CURRENT = 2;
     /* For each previous step and current step. Closes with label b804
      * TODO: This might be changed into a macro.  j takes on the value
      * 1 for PREVIOUS contact and 2 for CURRENT contact.
      */
      for (j=1; j<= 2;  j++)
      {
        /* FIXME: say what the following matrix represents. */
        /* This initializes qq as a 2 x 4 matrix, e.g.,:
         * lockstate =  [ 0 0 1 0 ]
         *              [ 1 1 0 0 ].
         * Evidently the 1 & 2 slots track the previous and current states
         * for a VE vertex and the first vertex in a VV contact.
         * The 3 & 4 slots for the second vertex in a VV contact.
         */
         lockstate[j][1]=lockstate[j][2]=lockstate[j][3]=lockstate[j][4]=0;
        /* When locks[i][j] = 0, (OPEN) no springs are turned on.
         * FIXME: Find out where locks are set.
         */
         if (locks[contact][j]==OPEN) 
         {
            continue; //goto b801;
         }
        /* else SLIDING or LOCKED */
         //lockstate[j][PREVIOUS] = CLOSED;
         //lockstate[j][CURRENT] = OPEN;
        /* Else when locks[i][j] = 1, we sliding with the 
         * normal spring turned on.
         */
         else if (locks[contact][j]==SLIDING) 
         {  //continue; //goto b801;
            lockstate[j][PREVIOUS] = CLOSED;
            lockstate[j][CURRENT] = OPEN;
         }

        /* else NOTSLIDING */
         //lockstate[j][PREVIOUS] = CLOSED;
         //lockstate[j][CURRENT] = CLOSED;
         
        /*  Else when locks[i][j] = 2, the sliding and shear springs
         * are locked (or active?).
         */
         else if (locks[contact][j]==LOCKED) 
         {
            //continue; //goto b801;
            lockstate[j][PREVIOUS] = CLOSED;
            lockstate[j][CURRENT] = CLOSED;
         }

         /* FIXME: I think this piece of is never reached.  Check against 
          * original code in df18().
          */
         else         /* else NOTLOCKED */
         {
            lockstate[j][PREVIOUS] = OPEN;
            lockstate[j][CURRENT] = OPEN;
           /* FIXME: Explain why 3 is set here and not anywhere
            * else when it looks like this code is never reached.
            * But it is reached in vaiont, in the first time step,
            * at open-close count 4.
            */
            lockstate[j][3] = CLOSED;
            //iface->displaymessage("Reached the last else block in getLockStates");
         }
        /* Else locks[i][j] = 3, and the sliding 
         * "ref line -> 2 normal spring on"
         * (notes are incomprehensible here).
         */ 
//b801:;
      }  /*  j  */
      
     /* (GHS: set on-off modify coefficient) */
     /* Difference in current and previous contact springs.
      * lockstate[1][j] = -1: Spring was on in prev iteration, not in current;
      * lockstate[1][j] =  0: Spring in current and previous are same (goal?); 
      * lockstate[1][j] =  1: Spring on in current not in previous.
      */ 
      for (j=1; j<= 4; j++)
      {
         lockstate[1][j]=(lockstate[CURRENT][j]-lockstate[PREVIOUS][j]);
      }  /*  j  */


}  /* close getLockStates() */



static void 
setGravForces(Analysisdata * ad, Contacts * c, int contact)
{
   Gravity * grav = ad->gravity;
   double s2n_ratio = ad->constants->shear_norm_ratio;

   double ** c_length = get_contact_lengths(c);

        /* move current shear force to previous */
        /* Split the previous and current gravity shear and 
         * normal forces to separate arrays.  This will make 
         * the code much easier to read.  Probably make it 
         * slower though.  
         */
         grav->contactFS[contact][1] = grav->contactFS[contact][2];
        /* Default: assume stable condition, max shear force 
         * has not been mobilized.
         */
         //ad->contactFS[contact][2] = c_length[contact][1]*(ad->g0)/h2;
         grav->contactFS[contact][2] = c_length[contact][1]*(ad->contactpenalty)/s2n_ratio;
         grav->contactFS[contact][3] = grav->contactFS[contact][2];
        /* If max shear force Ntan(\phi) has been mobilized,
         * this value (which value exactly?) will be 
         * modified later.
         */
}  /* close setGravForces() */



static void
setFrictionForces(Analysisdata * ad, Contacts * c, 
                  double ** c0, int contact, double s[31], int j1, int j2)
{
   int j;

   double normalforce, shearforce;
   double e11;
   double pen_dist2;  // was s4?
   double phi, cohesion;
   const double dd = 3.1415926535/180.0; /* radians to degrees */

   double ** phiCohesion = ad->phiCohesion;

   double ** c_length = get_contact_lengths(c);
   int ** prevcontacts = get_previous_contacts(c);
   int ** locks = get_locks(c);

   int joint_type;

  /***********  Probably the rest of the loop can go into a 
   * function of its own.
   ********************************/
  /* Normal distance between vertex and reference line. */
  /* The code that has been replaced here is terribly confusing.
   * c_length[][0] stores the length of normal penetration (?).
   */
   //s4=c_length[contact][0];
  /* FIXME: Where is the penetration depth initialized?
   */
   pen_dist2 = c_length[contact][0];
   //if (s4>0) 
   if (pen_dist2 > 0)
      pen_dist2 = 0;  //s4=0; /* i.e., open, no normal force */
  /* j4 is joint material type. m2 is prevcontacts 
   */
   joint_type = prevcontacts[contact][0];

  /*******  Block for setting joint materials  *********/

  /* Clean up comments after 15 Aug., 2000 */

  /* c x contact length l (1/2 L [?] see proj MMM) */
  /* t2 should be cohesion term, c_length[contact][3] is computed in 
   * proj() and represents that actual contact length
   */
   cohesion=phiCohesion[joint_type][1]*c_length[contact][3];
         
   if (ad->gravityflag == 1)
      cohesion = 10000.0;


  /* FIXME: What is this all about? */
   if (locks[contact][0]==1)  
      e11=cohesion;
   else 
      e11 = 0;

  /* Moved from after friction block */
   normalforce = fabs(pen_dist2)*(ad->JointNormalSpring);
  /* Tensile strength not used. */
   //tstrength=phiCohesion[joint_type][2]*c_length[contact][3];
   if (ad->frictionlaw == voight) //disp dep
   {
     /* BUG BUG BUG BUG
      * DISPDEP compiles in a call (df18 and df22) 
      * to a frictional decay function that provides 
      * a friction angle based on the value of the 
      * displacement of a measured point.  If there is 
      * no measured point, the call will produce spurious
      * results due to garbage in the over-allocated arrays.
      */
     /* WARNING!!!  This is broken because gd is not passed in. */
      if (joint_type == 1 || joint_type == 2)
         ;//phi = computeVFriction(gd, ad, phi);
      else 
         phi=phiCohesion[joint_type][0];
  
      shearforce=(normalforce*tan(dd*phi)+e11)*sign(c_length[contact][1]);
   }
   else if (ad->frictionlaw == negexp)
   {
     /* BUG BUG BUG BUG
      * DISPDEP compiles in a call (df18 and df22) 
      * to a frictional decay function that provides 
      * a friction angle based on the value of the 
      * displacement of a measured point.  If there is 
      * no measured point, the call will produce spurious
      * results due to garbage in the over-allocated arrays.
      */
     /* WARNING!!!  This is broken because gd is not passed in */
      if (joint_type == 1)
         ; //phi = computeFriction(gd, ad,joint_type);
      else 
         phi=phiCohesion[joint_type][0];

      shearforce=(normalforce*tan(dd*phi)+e11)*sign(c_length[contact][1]);
   }
   else if (ad->frictionlaw == tpmc)
   {
      phi=phiCohesion[joint_type][0];
      shearforce=(normalforce*tan(dd*phi)+e11)*sign(c_length[contact][1]);
   }
   else 
   {
      ;/* Houston, we have a problem up here... */
   }
 
  /* But if we are in gravity turn on mode, 
   * reset these to high values...
   */
   if (ad->gravityflag == 1)
   {
      phi = 89.0;
      shearforce=(normalforce*tan(dd*phi)+e11)*sign(c_length[contact][1]);
   } /* end if gravity flag */


/************  End block for setting joint materials  *********/

  /* Sliding force terms */
  /* ad->g7 is spring stiffness, which is not set on the 
   * first iteration.  ad->g7 is set in df22 as equal to 
   * ad->g0.  s4 is the penetration distance.  This number
   * should be s4 <= 0, and is set by multiplying the unit
   * cohesion by the length of contact.  The sign stored
   * o should indicate the direction of shear.
   */
  /* Shear strength is computed using Coulomb's relationship,
   * which may not be valid for low normal stresses.
   */
  /* Move this next line into a "Coulomb" friction
   * function or macro.  Rewrite disp dep formulation 
   * to return shear stress instead of friction angles.
   */
  /* In the original code, $s4$ is used initially to store the 
   * penetration distance, then used afterward to shear force.  
   * If there is no penetration, the value is set to zero and 
   * {\em used} to compute shear strength!  The idea being that 
   * if zero, then the evaluates the cohesion. 
   */

  /*
   if (ad->gravityflag == 1)
   {
      grav->contactFN[contact][2] = normalforce;
      grav->contactFN[contact][3] = normalforce;
      grav->contactFS[contact][2] = shearforce;  // was s4
      grav->contactFS[contact][3] = shearforce;  // was s4
   } 
   */

  /* c0 is initialized here and used in df20. It is 
   * recomputed every time df18() is entered.  FIXME:
   * The code in df20 that copies c0 into F should be
   * moved into here.  c0 should be renamed "friction"
   * or something equally suggestive.
   */
   for (j=1; j<= 6; j++)
   {
      c0[j1][j] += -shearforce*s[j+12];  // was s4
      c0[j2][j] +=  shearforce*s[j+24];  // was s4
   }  

   if (1)  //writecontacts == TRUE
   {
      char mess[80];
      sprintf(mess,"%d %f %f\n",ad->currTimeStep,normalforce,shearforce);
      fprintf(fp.cforce,mess);
      //iface->displaymessage(mess);
   }

}  /* close setFrictionForces() */


/**************************************************/
/* df18: add and subtract submatrix of contact    */
/**************************************************/
/* FIXME: write a nice little story about how this 
 * function works and how it is implemented.
 * FIXME: See if kk can be eliminated.  It is only used in 
 * two places.  k1 is only used in one place...
 */
/* locks[][0] := tension
 * locks[][1] := previous flag
 * locks[][2] := current flag
 * locks[][3] := save flag
 * locks[][4] := contact transfer
 */
void df18(Geometrydata * gd, Analysisdata *ad, Contacts * ctacts, 
          int *kk, int *k1, double **c0, int **n)
{
   double ** F = ad->F;
   double ** K = ad->K;
   double s2n_ratio = ad->constants->shear_norm_ratio;  // was h2
  /* p is penalty parameter, so named to maintain compatibility 
   * GHS 1988 notation.
   */
  /* FIXME: Change to TCK notation kn */
   double p = ad->contactpenalty;
  /* FIXME: The shear spring uses a different penalty value.
   * Since the springs are global, the shear spring value
   * can be set here, instead of in all of the inner loops.
   */
   //ks = kn/s2n_ratio;


   int ** contacts = get_contacts(ctacts);
   int ** locks = get_locks(ctacts);
   double ** c_length = get_contact_lengths(ctacts);
   int ** prevcontacts = get_previous_contacts(ctacts);

   double ** vertices = gd->vertices;
   double ** blockArea = gd->moments;
   int * nn0 = gd->nn0;
   int nBlocks = gd->nBlocks;
   int nContacts = gd->nContacts;

  /* Helper variables to increase readability of the code. */
   const int TYPE = 0;  // contact type
   const int VE = 0;     // vertex edge
   const int VV = 1;
   const int OPEN = 0;
   const int SLIDING = 1;
   const int LOCKED = 2;
   /* Has to be 1 and 2 because of the locks */
   const int PREVIOUS = 1;
   const int CURRENT = 2;

   int contact = 0;
   int blocknumber;  // was i0
   int i1,i2;
   int i3;
   int j,jj;
   int j1, j2,j3; //,j4;
   int l;
  /* Vertex numbers, used to index blocks */
   int v1,v2,v3;  // was l1, l2, l3
   double reflinelength; /* Eq. 4.12, p. 157, Shi 1988 (was b1). */
   double	dd = 3.1415926535/180; /* radians to degrees */
  /* pen_dist was s1, which was overloaded: area, then distance */
   double pen_dist;  /* Eq. 4.12, p. 158, Shi 1988. */  
  /* s2 is normalized shear displacement for the current cycle 
   * of open-close iteration.
   */
   double sheardisp;  // was s2
  /* omega is "locked index" from TCK 1995, p. 1239, Eq. 56.
   * omega = L_{20}/L_{23} where L_{20} is the length from 
   * the contact point project from P2, and  L_{23} is the 
   * length of the reference line P2P3.
   */  
  /* FIXME: Consider initializing omega to 0 when df18() is entered.
   */
   double omega; // = 0;  // was s3;
  /* vertex coordinates */
   double x1, x2, x3;
   double y1, y2, y3;

  /* qq *should* be a projection-type operator, except that it is 
   * overloaded to store actual penalty values.  qq is the 
   * "punishment projector".
   */   
   //double qq[3][5];  
   int lockstate[3][5];
  /* QQ is some funny kind of flag.  It used to hide in 
   * qq[0][1,2], but moved out to make the code more readable.
   * It is used to construct the penalty matrix/vectors.  It can 
   * most likely be turned into a pair of flags in the future.
   * QQ indicates whether the locks changes states between 
   * successive open-close iterations in a particular time step.
   * QQ = -1 :  Lock state was {open,closed}, now {closed,open}
   * QQ = 0  :  The contact lock state has not changed.
   * QQ = 1  :  Lock state was {open,closed}, now {closed,open}
   */
  /* FIXME: QQ should probably be initted to {0} when 
   * df18() is entered.
   */
   //int QQ[1][3];// = {0};
  /* This should further be broken down into two variables taking
   * an enumeration value.
   */
  /* has to index 1 and 2 for const var to match lock values. */
  /* Get rid of Q completely for state tracking, so that P and Q can
   * be used in the same way as TCK.
   */
   int QQ[3];

  /* FIXME: Make this proveable 3x7, then change to 2x6. */
   double T[7][7];
   //double T[3][7];
  /* TODO: s[] should probably be split into several arrays of
   * length 6 (or length whatever the DOF for a single block.
   * s[1-6] := er, s[7-12] := gr; p. 161, Eq 2.18, GHS 1988.
   * Other positions hold some kind of shear modifier that 
   * is not documented in GHS 1988.
   */
   double s[31];
  /* Replace s with these, from TCK 1995: */
  /*
   double h1[7];
   double h2[7];
   double h3[7];
   double h4[7];
   double h5[7];
   double h6[7];
   */

  /* To continue using TCK notation, use P and Q for the 
   * matrix elements if possible.
   */
   //double P[], Q[];

  /* These are variables needed for computing friction force. */
   //double ** phiCohesion = ad->phiCohesion;
   //double pen_dist2;  // Replaces s4 when s4 used as distance instead of force.
   //double shearforce;  //s4;
   //double normalforce;
  /* FIXME: What is e11? */
   //double e11;
   //double phi, cohesion; // was t1, t2
   //int joint_type;  // was j4 

  /* (GHS: c0[] free term of equations for friction force.) */
  /*
   * This resets the friction forces to zero at the start of 
   * every open/close iteration, then  c0 carries 
   * over into df20.
   */
  /* FIXME: Move this out of df18. */
   setMatrixToZero(c0,__c0size1,__c0size2);

  /* (GHS: set off-on of normal and shear spring)
   * This next set of loops is written in "FORTRAN".
   * Basically its an if-elseif-else block, where 
   * the default behavior is set first, then each
   * true conditional following jumps to the start of
   * the next loop.
   */
  /* For each contact */
   for (contact=1; contact<= nContacts; contact++)
   {
      if (ad->gravityflag == 1)
         setGravForces(ad,ctacts,contact);

      getLockStates(locks,lockstate,contact);

     /* (GHS: compute 4 6*6  2 6*1 contact submatrices)       */
     /* (GHS: if sliding force +- changing shear g0/h2 h2= 4) */
     /* (GHS: k00=0 set locks[][1]=0 still have friction force)  */
     /* For each "ref line" of contact...
      */   
      for (jj=0; jj<= contacts[contact][TYPE]; jj++)
      {
        /* jj != 0 implies that this is a VV contact. */
         if (jj != 0) 
         {
           /* Else 2d ref line. */
           /* Consider using the 0 leading slot for storing state
            * changes between PREVIOUS and CURRENT.
            */
            lockstate[1][1]=lockstate[1][3];
           /* FIXME:  Where is lockstate[1][4] used? Initialized to 0 */
            lockstate[1][2]=lockstate[1][4];
         }  
        
        /* FIXME: Turn these into flags, probably normal and shear */
        /* GUESS: QQ[0][1] = 0 means previous springs did not change;
         * QQ[0][2] = 0 means current did not change.
         */
         QQ[PREVIOUS]=lockstate[1][1];
         QQ[CURRENT]=lockstate[1][2];
 
        /* If the locks changed between the previous iteration
         * and the current iteration, then have to construct 
         * penalty matrices.  If the spring locks did not change,
         * we still may have to add some penalty matrices,  
         * which get tested for in the following block.
         */
        /* FIXME: Use the tested macro in macrotest.c */
        /* FIXME: Explain: If the contact is open in both states, but the contact 
         * is a vertex-vertex contact, it still has tobe checked
         * and possibly a penalty constructed because ????
         */
         if ( (QQ[PREVIOUS] == OPEN) && (QQ[CURRENT] == OPEN)) 
         {
            if (contacts[contact][TYPE]==VE)
            {
               if (locks[contact][CURRENT]==SLIDING)
               {
                  if ( ad->k00 == 0 )
                     continue;
                  if (locks[contact][PREVIOUS]==OPEN)
                     continue;
               }
               else 
                  continue;
            }
            else // contact is VV
               continue;
         }


        /* (GHS: submatrices of normal & shear spring frictions) */
        /* Now we have to add penalty terms.
         */

         v1=contacts[contact][3*jj+1];  /* contacting vertex */
        /* FIXME: Reference line endpoints are saved in CW
         * order.  Cross-reference to function where saved.
         */
         v2=contacts[contact][3*jj+2];  /* endpoint coord of ref line */
         v3=contacts[contact][3*jj+3];  /* endpoint coord of ref line */
        /* nn0 appears to be an array of block numbers, corresponding
         * to vertex numbers in array m...
         */
         i1=nn0[v1]; /* ith block containing vertex */ 
         i2=nn0[v2]; /* jth block containing ref line */
         j1=k1[i1];  /* (memory?) location of load vector for block i1 */
         j2=k1[i2];  /* (memory?) location of load vector for block i2 */
         
        /* parameter:   i i1 i2 l1 l2 l3 s1 s2 m[] o[][2] */ 
        /* Here, we gather some vertices to determine (in the 
         * next block of code) there is penetration between
         * the vertex and the reference formed from adjacent
         * vertices.  Note that the block vertices are normally 
         * stored in CCW order, but we will be using them in 
         * CW order in the next two blocks of code.  x1,y1 is
         * most likely the penetrating vertex, see p. 157, Shi 1988.
         */
         x1=vertices[v1][1]; /* x coord of vertex */
         y1=vertices[v1][2]; /* y coord of vertex */
         x2=vertices[v2][1]; /* x coord of first end of ref line */
         y2=vertices[v2][2]; /* y coord of first end of ref line */
         x3=vertices[v3][1]; /* x coord of other end of ref line */
         y3=vertices[v3][2]; /* y coord of other end of ref line */
         
        /* (GHS: compute coefficients s1 s2  s3 ratio p2-p3) */
        /* Length of ref line.  This should be the denominator of 
         * Eq. 4.12, p. 157, Shi 1988.
         */
         reflinelength = sqrt((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2));
        /* s1 is twice the area of a triangle formed by the 
         * directed area of the endpoints of the vertex and the 
         * reference line;  Eq. 4.11, p. 158, Shi 1988.
         */  
        /* s1 is now the "height" of the vertex from 
         * the reference lines. If s1 < 0, there is penetration,
         * else s1 > 0 and no penetration.  Note that we are using 
         * the blocks in CW order here. Eq. 4.12, p. 158, Shi 1988.
         * pen_dist is not used until we set the load terms past the 
         * penalty matrix initialization blocks.  Try and move this 
         * code to where it is used.
         */
         pen_dist = ((x2-x1)*(y3-y1)-(y2-y1)*(x3-x1))/reflinelength;
         //s11 = s11/reflinelength;
        /* if m[i][0] != 0 then there is more than 1 reference 
         * line and we have to go somewhere else first.  MMM 
         * notes something about "skipping shear component."

         * Basically, I think this means this is a VV contact.
         */
         //if (m[i][0] != 0) 
         //   goto b805;
         if (contacts[contact][TYPE] == VE)
         {
           /* shear component? Some kind of ratio.  o is listed 
            * as a matrix holding penetration values.  o[i][2]
            * is the "contact edge ratio".  This value is set in
            * function either df06(), df07() or proj().
            */
           /* The analysis for this is in Te-Chih Ke 1995, "Modeling
            * of particulate Media Using Discontinuous Deformation
            * Analysis. Print these out to look at 
            * them.  omega is the "lock index", Eq. 56, TCK 1995, 
            * p. 1239.
            */
           /* FIXME: Add an assert here for omega < 1 */
            omega = c_length[contact][2];
            assert (omega < 1);
           /* Eq. 58, TCK 1995, p. 1239.  sheardisp is denoted by 
            * s0 in this paper.
            */
            sheardisp = (x1-(1-omega)*x2 - omega*x3)*(x3-x2) 
                       + (y1-(1-omega)*y2 - omega*y3)*(y3-y2);
           /* s2 (sheardisp) is the normalized shear component. */
            sheardisp = sheardisp/reflinelength;
         }  /* end if VE shear component */


        /*****  Block for computing terms e_r, g_r for penalty matrices  ****/
        /*
         *  s[1-6]:    er normal, Eq.  4.18, Shi 1988
         *  s[7-12]:   gr normal, Eq.  4.18, Shi 1988
         *  s[13-18]:  er shear,  Eq.  59a, TCK 1995
         *  s[19-24]:  gr shear,  Eq.  59b, 59c, TCK 1995
         *  s[25-30]:  friction,  Eq.  FIXME
         */

        /* i0 old block number         s13-s18 i friction */
        /* p1 terms      s01-s06 i normal s13-s18 i shear */
        /* i0 is the block number */
         blocknumber=i1;
        /* getDisplacement computes, essentially returns t for
         * the ith vertex.
         */
         computeDisplacement(blockArea,T,x1,y1,blocknumber);
         
         for (j=1; j<= 6; j++)
         {
           /* "er" normal See Chapter 4, p. 161 Eq 4.18 Shi 1988 */
            s[j]  = (y2-y3)*T[1][j] + (x3-x2)*T[2][j];
            s[j] /= reflinelength;
           /* if 2 ref lines, skip following shear. */
            if (contacts[contact][TYPE] != VE) 
               continue; 
           /* Compute er, shear. TCK 1995, Eq. 59a, p. 1239 */
            s[j+12]  = (x3-x2)*T[1][j] + (y3-y2)*T[2][j];
            s[j+12] /= reflinelength;
         }  
         
        /* p2 terms      s07-s12 j normal s19-s24 j shear */
        /* p2 terms      s25-s32 j friction               */
        /* Now we work with the other block. */
         blocknumber=i2;
        /* Get t for the ith vertex. */
         computeDisplacement(blockArea,T,x2,y2,blocknumber);
         for (j=1; j<= 6; j++)
         {
           /* Compute gr normal. See Chapter 4, p. 161 Eq 4.18 Shi 1988 
            * Also, TCK 1995, Eq. 45, p. 1238.
            */
            s[j+6 ]  = (y3-y1)*T[1][j] + (x1-x3)*T[2][j];
           /* If two ref lines, skip shear. */
            if (contacts[contact][TYPE] != VE) 
               continue; 
           /* Compute gr, shear. TCK 1995, Eq. 59b, p. 1239 */
            s[j+18]  = (-x1+2*(1-omega)*x2 - (1-2*omega)*x3)*T[1][j];
            s[j+18] += (-y1+2*(1-omega)*y2-(1-2*omega)*y3)*T[2][j];
           /* Shear displacement of point p due to disp. 
            * of p2.
            */
           /* FIXME: Get eq. no.: Frictional terms, TCK 1995, Eq. ???? */
            s[j+24]  = (1-omega)*((x3-x2)*T[1][j]+(y3-y2)*T[2][j]);
         } 
         
        /* (GHS: p3 terms: s07-s12 j normal s19-s24 j shear) */
        /* (GHS: p3 terms: s25-s32 j friction) */
        /* Deal with point 3 now, just the way points 1 and 2
         * were handled.
         */
         blocknumber=i2; /* i0 is block number */
        /* Get t for ith vertex. */

         computeDisplacement(blockArea,T,x3,y3,blocknumber);

        /* This might be the second half of Eq 4.18.
         */
         for (j=1; j<=6; j++)
         {
           /* Compute gr normal. See Chapter 4, p. 161 Eq 4.18 Shi 1988 
            * Also, TCK 1995, Eq. 45, p. 1238.
            */
            s[j+6] += (y1-y2)*T[1][j]+(x2-x1)*T[2][j];
            s[j+6] /= reflinelength;
           /* 2 ref lines, skip shear. */
            if (contacts[contact][TYPE] != VE) 
               continue; //goto b808;
           /* Compute shear. */
           /* Compute gr, shear. TCK 1995, Eq. 59c, p. 1239 */
            s[j+18] += (x1-(1-2*omega)*x2 - 2*omega*x3)*T[1][j];
            s[j+18] += (y1-(1-2*omega)*y2 - 2*omega*y3)*T[2][j];
            s[j+18] /= reflinelength;
           /* FIXME: Get Eq. no. for the following friction term */
           /* Disp. of point p due to disp. of point 3 (frict). */
            s[j+24] += omega*((x3-x2)*T[1][j]+(y3-y2)*T[2][j]);
            s[j+24] /= reflinelength;
         }  /*  j  */  //b808:;

        /* j1, j2, sheardisp, and omega are carried out of the previous block */

        /* end block for computing terms of penalty matrices */
        /*****************************************************/
        /*  Start block for constructing penalty matrices */
        /* (GHS: submatrix ii  s01-06 i normal s13-18 i shear)   */
        /* 1st if for friction force                      */
        /* if true, skip penetration penalty terms and 
         * do friction force only.  Else do some more 
         * penalty stuff.  Basically checking whether state 
         * has changed.
         */
        /* FIXME: Change this to use the tested macro in macrotest.c 
         */         
        /* PREVIOUS is 1, CURRENT is 2  */
         //if ( (QQ[0][PREVIOUS] != 0)  || (QQ[0][CURRENT] != 0) ) 
         if ( (QQ[PREVIOUS] != OPEN)  || (QQ[CURRENT] != OPEN) ) 
         {   
           /* location (?) of Kii (?) MMM */
            i3=n[j1][1]+n[j1][2]-1;
           /* Add punishment terms to Kii.  
            * Eq 4.36, p. 174, chapter 4, Shi 1988.
            */
            for (j=1; j<= 6; j++)
            {
               for (l=1; l<= 6; l++)
               {
                 /* 6 is block size, also, j should range from 
                  * 0 to block size - 1, and add a 0 or 1 index
                  * to handle 0 or 1 indexing.
                  */
                  j3=6*(j-1)+l;
                  K[i3][j3] += p*lockstate[1][1]*s[j]*s[l];
                 /* These terms probably come from TCK */
                  if (QQ[CURRENT] == OPEN)
                     continue;
                  K[i3][j3] += (p/s2n_ratio)*lockstate[1][2]*s[j+12]*s[l+12];
               }  
            }  

           /* (GHS: submatrix jj  s07-12 j normal s19-24 j shear)   */
           /* location of Kji */
            i3=n[j2][1]+n[j2][2]-1;
           /* Add penetration punishment terms to Kji */  // Kjj ?
            for (j=1; j<= 6; j++)
            {
               for (l=1; l<= 6; l++)
               {
                  j3=6*(j-1)+l;
                 /* Eq. 4.40, p. 175, gr \otimes gr  */
                  K[i3][j3] += p*lockstate[1][1]*s[j+6 ]*s[l+6 ];
                  if (QQ[CURRENT] == OPEN)
                     continue;
                  K[i3][j3] += (p/s2n_ratio)*lockstate[1][2]*s[j+18]*s[l+18];
               }  
            }  


           /* locate j1j2 in a[][] only lower triangle saved */
           /* if j1 < j2, add terms to Kij instead of Kji */
            if (j1>=j2)
            {   
              /* find Kij */
               for (j=n[j1][1]; j<= n[j1][1]+n[j1][2]-1; j++)
               {
                  i3=j;
                  if (kk[j]==j2) 
                     break;
               }

              /* submatrix ij  s01-06 i normal s07-12 i shear   */
              /* Add penetration penalty terms to Kji */
               for (j=1; j<= 6; j++)
               {
                  for (l=1; l<= 6; l++)
                  {
                     j3=6*(j-1)+l;
                    /* Eq. 4.38, p. 174, er \otimes gr */
                     K[i3][j3] += p*lockstate[1][1]*s[j]*s[l+6 ];
                     if (QQ[CURRENT] == OPEN) 
                        continue;
                     K[i3][j3] += (p/s2n_ratio)*lockstate[1][2]*s[j+12]*s[l+18];
                  }  
               }  
             }
            /* Add to Kji or Kij but not both. */
             else /* j1 < j2 */
             {  
               /* locate j2j1 in a[][] only lower triangle saved */
                for (j=n[j2][1]; j<= n[j2][1]+n[j2][2]-1; j++)
                {
                   i3=j;
                   if (kk[j]==j1) 
                      break;
                } 
         
               /* submatrix j1  s07-12 j normal s19-24 j shear   */
               /* add penetration penalty terms to Kij */
                for (j=1; j<= 6; j++)
                {
                   for (l=1; l<= 6; l++)
                   {
                      j3=6*(j-1)+l;
                      K[i3][j3] += p*lockstate[1][1]*s[j+6 ]*s[l];
                      if (QQ[CURRENT] == OPEN)
                         continue;
                      K[i3][j3] += (p/s2n_ratio)*lockstate[1][2]*s[j+18]*s[l+12];
                   }  
                }  
            }  /* end else j1 < j2 */

           /***  Contact constraint methods ***/
            if (ad->contactmethod == penalty)
            {
              /* load term of normal & shear  s1 s2 coefficient */
              /* Add penetration penalty load to Fi and Fj:
               * Chapter 4, p. 175, Eq. 4.43-4.46, but check to 
               * see if these are fixed point forcing terms.
               */
               for (j=1; j<= 6; j++)
               {
                 /* normal component */
                  F[j1][j] += -lockstate[1][1]*(p*pen_dist)*s[j];
                  F[j2][j] += -lockstate[1][1]*(p*pen_dist)*s[j+6];
                 /* skip shear if no contact (?) */
                  if (QQ[CURRENT] == OPEN)
                     continue;
                 /* sheardisp (s2) is normalized shear component for the 
                  * current open close cycle. TCK 1995, Eq. 64c, p. 1239. 
                  */
                  F[j1][j] += -lockstate[1][2]*(p*sheardisp/s2n_ratio)*s[j+12];
                  F[j2][j] += -lockstate[1][2]*(p*sheardisp/s2n_ratio)*s[j+18];
               }
            }
            else if (ad->contactmethod == auglagrange)
            {
                iface->displaymessage("Lagrange multipliers not currently implemented");
            }
            else  // Big trouble
            {
             ; /* Need to quit right here, dump everything, 
                * and go home.  Try again later.
                */
            }  /* End penalty chooser */
         } /* end if penalty terms (b809): */

        /****  End block for constructing penalty matrices  ****/

        /* (GHS: sliding friction force     s4*g0 normal force) */
        /* if 2 ref lines, or current step is not
         * "sliding", we don't need any friction force, so continue on 
         */

        /* FIXME: Find out whether we can have a SLIDING lock when we have a 
         *  VV contact.  If not, these conditionals can be cleaned up a lot.
         */
         if (contacts[contact][TYPE] != VE || locks[contact][CURRENT] != SLIDING) 
            continue; 
        /* if prev step open and current iteration not first
         * iteration (of open/close), then done. (k00 = 0
         * for first iteration, that is, we haven't yet done 
         * a linear solve.)
         */
         if (locks[contact][PREVIOUS] == OPEN && (ad->k00==1))
            continue;

         setFrictionForces(ad,ctacts,c0, contact, s, j1, j2);

     /* Check whether on the second loop through the VV
      * stuff that it does or doesn't not bail depending how the 
      * normal penetration is handled.
      */
      }  /*  jj, if VE contact, loop once, if VV contact, loop twice */

   }  /*  end loop over each contact (was i)  */

}   /*  Close df18()  */




/**************************************************/
/* df22: contact judge after iteration            */
/**************************************************/
/* This is where all the open-close action takes place.
 * Lagrange multipliers can probably be stuck into the 
 * back of the contacts array.  This code appears to 
 * implement material in Shi 1988, Chapter 4, p. 157,
 * Section 4.3.
 */
/* FIXME: This function needs to be renamed "setLockStates" because that 
 * is what it does.
 */
/* FIXME: Try to get rid of the Geometrydata struct if possible. */
void 
df22(Geometrydata *gd, Analysisdata *ad, Contacts * ctacts, int *k1)
{
   double ** F = ad->F;
   double ** vertices = gd->vertices;
   double ** moments = gd->moments;
  /* FIXME: Explain what nn0 is and try to get rid of it.  I think it 
   * is the vertex block point array.
   */
   int * nn0 = gd->nn0;
   double ** phiCohesion = ad->phiCohesion;
  /* problem domain scaling, was w0 */   
   double domainscale = ad->constants->w0; 

  /* FIXME: This should be stored in the ctacts struct instead of the 
   * geometry struct.
   */
   int nContacts = gd->nContacts;
   Gravity * grav = ad->gravity;

   int ** contactlist = get_contacts(ctacts);
   int ** locks = get_locks(ctacts);
   double ** c_length = get_contact_lengths(ctacts);
   int ** prevcontacts = get_previous_contacts(ctacts);


  /* s0 is used only one time in this function! */
   double openclose = ad->constants->openclose;  // was s0, hardwired to 0.0002
  /* f0 is used twice */
   double opencriteria = ad->constants->opencriteria;  // was f0, hardwired to 0.0000002
  /* h2 is used only once */
   double shear_norm_ratio = ad->constants->shear_norm_ratio;  //  was h2

   int contact, blocknumber;  // was i, i0 
   int CTYPE; //i1;
   const int VE = 0;
   const int VV = 1;
   const int OPEN = 0;
   const int SLIDING = 1;
   const int LOCKED = 2;
   const int PREVIOUS = 1;
   const int CURRENT = 2;
   const int TENSILE = 0;
   const int REORDER = 3;  // Was SWAP
   const int PENDIST = 0;
   int i2;

  /* Loop counters. */
   int i,j,k;
   int joint_type;
  /* These are just loop counters.  Unfortunately, l and 1
   * are almost identical glyphs.  So l -> ell, l0 -> ell0
   * l1 -> ell1. 
   */
  /* FIXME: Change these loop counters to i and k, because 
   * i and k have not been used in this function yet.
   */
   //int ell;
   //int ell1;
  /* vertexnumber was ell0, was l0 */
   int vertexnumber;
  /* Original contact determinant, Shi 1988, p. 159. 
   * S0 < 0 means penetration occurred for the previous
   * time-step.
   * FIXME: This has been previously computed and should 
   * be stored if possible instead of being recomputed.
   */
   double S0;  //  was d1
  /* Determinant of displaced contact, was b1 
   * dispdet < 0 means penetration during the current
   * open-close iteration.
   */
   double dispdet;  //  was b1
  /* Length of the original reference line.  Shi 1988,
   * p. 159, under Eq. 4.13 given as l.
   */
   double reflinelength;  // was a1
   double tanphi; // was a2
  /* FIXME: What are these variables for? */
   double b2, e1, s1;
   double penetration;  // was c1
  /* This will disappear when the friction angles
   * all get precomputed.
   */
   const double	dd = 3.1415926535/180;
  /* t0 parameterizes something, t \in [0,1] */
   double t0;
  /* FIXME: This ratio does not appear to be documented in 
   * the dissertation or in the tech report.  ratio is used
   * for contact length determination, possibly only for 
   * shear contacts, but not sure about that yet.  The 
   * value satisfies ratio \in [0,1]
   */ 
   //double ratio;
   double omega;

   double  phi, cohesion, tstrength;  // were t1, t2, t3
  /* q1, q2 are used to determine shear locks and shear
   * directions.  q1 ends up being -1 or 1, q2 a normalized (?)
   * value of shear (?) displacement along the contact; 
   * normalized by the length of the contact (?).
   */
   double prev_shear_direction;  //was q1
   double curr_shear_length;  //was q2
  /* Controls the availability of mobilized shear strength.
   * Coupled with the scale of the domain w0.  Used twice in 
   * df22()
   * FIXME: make this a user parameter.
   */
   const double SHEAR_TOL = 0.0000001;
  /* qq is temp storage for the three vertices involved 
   * in the contact triangle.
   */
   double qq[4][3];
  /* s (now pendist) is temp storage for penetration depths. 
   * pendist < 0 means penetration.
   */
   double pendist[3];// = {0};  // was s[]
  /* p is temp storage for contact vertex displacements */
   double p[4][4] = {0};
  /* T is the usual, i.e., T \equiv [T] */
   double T[7][7];
  /* FIXME: Change this to: */
   //double T[3][7] = {0};
  /* x,y are temporary variables used to carry contact vertex
   * locations for computing displacements
   */
   double x,y;
  /* x1 to y3 are pre-displacement x,y locations of
   * the vertices involved in the contact.
   * x1,y1 is the penetrating vertex (Shi 1988, p. 157, \S 4.3).
   */
   double x1,x2,x3,y1,y2,y3;
  /* x0,y0 having something to do with ratioed displacements */
   double x0,y0;
  /* x4 to y6 are "displaced" vertices, which may be controlled
   * by a ratio quantity for what reason ???
   */ 
   double x4, x5, x6, y4, y5, y6;

   int i7,i8;  // used to test convergence


  /* Can't do this yet.  Set from the contact spring. */
   //double JointNormalSpring = ad->JointNormalSpring;

  /* This refers to vertex and edge contacts, for which there
   * may be more than 1 per pair of blocks.
   */
  /* open-close record transfer  */
   for (contact=1; contact<= nContacts; contact++)
   { 
     /* Transfer the open close info.  The info from the 
      * previous solution is in locks[][2] and we want to save 
      * it for comparing to the results of the current solution.
      * The contact info resulting from the current solution 
      * will be stored in locks[][2].  The possible values are 
      * 0 for open, 1 for sliding, 2 for locked, 3 means to 
      * reorder vertices.
      */
      locks[contact][PREVIOUS] = locks[contact][CURRENT];

      if (ad->gravityflag == 1)
      {
        /* Save current value [i][2] into previous value [i][1]
         * for this open-close (?)
         */
         grav->contactFN[contact][1] = grav->contactFN[contact][2];
      }  /* end if grav flag */

   }  /*  i  */
   
   /* l0 vertex   i0 block number   p[] displacement 
    */
   for (contact=1; contact<= nContacts; contact++)
   {
     /* i1 is the type of contact, takes the value 0 for 
      * vertex-edge or 1 for vertex-vertex.
      */
      CTYPE = contactlist[contact][0];

     /* COMPUTE PENETRATION DISTANCE.  If the contact is 
      * vertex-vertex, then j varies from 0 to 1.  That 
      * is, this loop runs twice.  Else for 
      * vertex-edge contacts it loops only once.  Basically,
      * this loop determines the penetration distance for each 
      * contact.  If the contact is vertex-edge, it loops once,
      * otherwise the contact is vertex-vertex (VV) and loops
      * twice.
      */
      for (j=0; j<= CTYPE; j++)
      {  
        /* COMPUTE VERTEX DISPLACEMENTS
         * The code in this inner loop gets called three times 
         * for vertex-edge contact, and 6 times for vertex-vertex
         * contact.  (vertex-vertex needs two triangles)
         */
         for (i=1; i <= 3; i++)
         {
            vertexnumber = contactlist[contact][j*3+i];
           /* nn0 stores the number (label) of the block associated 
            * with vertex ell0.
            */
    		   blocknumber = nn0[vertexnumber];
           /* Store the vertex locations temporarily.  qq gets 
            * dereferenced later into x1,x2,x3, y1,x2,y3.
            */
            qq[i][1]  = x  = vertices[vertexnumber][1];
            qq[i][2]  = y  = vertices[vertexnumber][2];

            computeDisplacement(moments,T,x,y,blocknumber);

           /* FIXME: Use a memset on this outside of the loop. */
            p[i][1]  = 0;
            p[i][2]  = 0;
           /* Find memory location associated with the 
            * i0th block stored in sparse form.
            * k1 is used only here in this function 
            */
            i2 = k1[blocknumber];
           /* p appears to be displacement.  F is deformation
            * unknowns overwritten into the load vector.
            */
            for (k=1; k<= 6; k++)
            {
               p[i][1] += T[1][k]*F[i2][k];
               p[i][2] += T[2][k]*F[i2][k];
            }  /*  k */
         }  /*  i  */

        /* After transferring values, qq is not used until 
         * the next contact.  The x,y pairs are carried through.
         * x1,y1 is the penetrating vertex (Shi 1988, p. 157, \S 4.3).
         * NOTE: This assignment is purely convenience to help
         * match the mathematical exposition.
         */
         x1 = qq[1][1];
         y1 = qq[1][2];
         x2 = qq[2][1];
         y2 = qq[2][2];
         x3 = qq[3][1];
         y3 = qq[3][2];
         
        /* a1 is original length of ref line. FIXME: There should be a 
         * way to store this instead of having to recompute
         * it here every time we are in openclose iteration.
         */
         reflinelength  = sqrt((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2));  // was a1
        /* S0: Original det of contact area. S0 < 0 means penetration during 
         * previous open-close iteration.
         */
         S0  = (x2-x1)*(y3-y1) - (y2-y1)*(x3-x1);
        /* b1 (now dispdet) is det due to displacement, 
         * terms added to the right of S0 in Eq. 4.14,
         * Shi 1988, p. 159.  If dispdet < 0, then we should
         * have penetration during this open-close iteration.
         */
         dispdet  = p[1][1]*(y2-y3) + p[1][2]*(x3-x2);
         dispdet += p[2][1]*(y3-y1) + p[2][2]*(x1-x3);
         dispdet += p[3][1]*(y1-y2) + p[3][2]*(x2-x1);
        /* New distance from reference line.  The sign 
         * convention is given in Shi 1988, p. 157, Section
         * 4.3, Eq. 4.11, where penetration past ref line 
         * produces s[] < 0.  This particular equation is 
         * Eq. 4.17 on page 160,161.  Seems to take values
         * between -1 and 1.
         */
         pendist[j+1] = (S0+dispdet)/reflinelength;  // was s[] = (d1+b1)/a1;
      }  /*  j, end loop for computing penetration distance  */
      
      //fprintf(fp.logfile,"pendist (df22): %f\n",pendist[1]);

     /* SET PEN DIST, s[1] is normal penetration */
     /* If it is just a VE contact, then c_length is already
      * set from the previous line. Otherwise have to check
      * which vertex penetrated the farthest and swap.
      * FIXME: reimplement this as a max or min macro, 
      * whichever is appropriate.
      */
      if ( (CTYPE==VE) || (pendist[1] >= pendist[2]) ) 
      {
        /* s[1] is normal penetration */
         c_length[contact][PENDIST]   =  pendist[1];
        /* FIXME: Why is this set to sliding?  Probably check it later 
         * and lock it then if necessary
         */
         locks[contact][CURRENT]  =  SLIDING;
      }
      else  /* VV contact */
      {
        /* Basically this is a swap from the c_length set just 
         * before the if().  What this means is that for the 
         * vertex-vertex contact, we want to use the vertex that
         * has penetrated (furthest,least?) past the reference line.
         */
         c_length[contact][PENDIST]   =  pendist[2];
        /* Setting the value to 3 flags for another swap further down in 
         * in the code.  The swap means the "other" triplet of vertices
         * must be used (for what purpose?).  It could also mean reorder 
         * the vertices.
         */
         locks[contact][CURRENT]  =  REORDER;
      }  /* end if block for setting penetration distance. */
      

/******  Block for setting Joint properties   ******/

     /* FIXME: What is m2? */
      joint_type = prevcontacts[contact][0];

     /* Friction is displacement dependent, not cohesion or tension. */
      cohesion=phiCohesion[joint_type][1]*c_length[contact][3];
      tstrength=phiCohesion[joint_type][2]*c_length[contact][3];

      if (ad->frictionlaw == voight) //disp dep
      {
        /* BUG BUG BUG BUG
         * DISPDEP compiles in a call (df18 and df22) 
         * to a frictional decay function that provides 
         * a friction angle based on the value of the 
         * displacement of a measured point.  If there is 
         * no measured point, the call will produce spurious
         * results due to garbage in the over-allocated arrays.
         */
         if (joint_type == 1 || joint_type == 2)
            phi = computeVFriction(gd, ad, phi);
         else 
            phi=phiCohesion[joint_type][0];
      }
      else if (ad->frictionlaw == negexp)
      {
        /* BUG BUG BUG BUG
         * DISPDEP compiles in a call (df18 and df22) 
         * to a frictional decay function that provides 
         * a friction angle based on the value of the 
         * displacement of a measured point.  If there is 
         * no measured point, the call will produce spurious
         * results due to garbage in the over-allocated arrays.
         */
         if (joint_type == 1)
            phi = computeFriction(gd, ad,joint_type);
         else 
            phi=phiCohesion[joint_type][0];
      }
      else if (ad->frictionlaw == tpmc)
      {
         phi=phiCohesion[joint_type][0];
      }
      else 
      {
        /* Houston, we have a problem up here... */
         ;
      }
 

     /* TODO: rewrite the logic for gravity friction etc.
      * These values should be set by the user.
      */
      if (ad->gravityflag == 1)
      {
         phi = 89.0;
         cohesion = 10000.0;
         tstrength = 10000.0;
         tanphi = tan(dd*phi);
      } /* end if gravity flag */


/******  End block for setting Joint properties  *******/
      


     /* (GHS: open-close set  a-a shortest g7:g0 forming oi0) */
     /* FIXME: Find somewhere else to update this value. */
      ad->JointNormalSpring = ad->contactpenalty;



/* The following block seems to check the open close state based 
 * on normal penetration only.  Shear states follow further on.
 */
/******   Start block normal (?) locks *******/

/* locks[contact][CURRENT] is set to either 1 (SLIDING) or 3 (SWAP).
 */
// fprintf(fp.logfile,"locks[contact][CURRENT]: %d\n",locks[contact][CURRENT]);

/* FIXME: Where are locks for normal contact set?
 */

     /* (GHS: cases: pre-close pre-open pre-close alt-side)   */
     /* Check to see if the previous spring flag is set.
      */
      if (locks[contact][PREVIOUS] == OPEN) 
      {
        /* If the previous had no spring, check to see if
         * the normal penetration is greater than the product of 
         * the open-close criteria (formerly s0) and domain scaling. 
         * FIXME: Explain the difference between the variables
         * openclose and opencriteria.
         */
         if (c_length[contact][PENDIST] >  -openclose*domainscale)  
            locks[contact][CURRENT]  = OPEN;
      }
      else if ((CTYPE == VE) || (locks[contact][PREVIOUS] == locks[contact][CURRENT]) )
      { 
         e1 = 0;
         if (locks[contact][TENSILE]==1 && CTYPE == VE)  
           /* FIXME: Figure out when joint normal spring is set, 
            * the move the dereference up.
            * FIXME: Check units on these values to ensure consistency.  tstrength 
            * must be given in units of stress.
            */
            e1 = tstrength/(ad->JointNormalSpring);

        /* opencriteria was f0 */
         if (c_length[contact][PENDIST] >   e1 + opencriteria*domainscale)  
           /* FIXME: Where was this initialized? */
            locks[contact][CURRENT]  = OPEN;
      }
      else if (locks[contact][PREVIOUS] != locks[contact][CURRENT]) 
      {
        /* c1 is penetration depth (?). Set, then check to see if the 
         * distance is more than the other vertex penetration distance.
         * If not, then swap to the biggest.  This should be rewritten 
         * as a macro call: FIXME: c1 = max(s[1],s[2]).  FIXME: min???
         */
         if (locks[contact][PREVIOUS] == REORDER)  
            penetration=pendist[2];
         else 
            penetration = pendist[1];

        /* opencriteria was f0 */
        /* If the penetration depth is greater then the 
         * opencriteria, set the current lock to OPEN. Note 
         * that this should be in a block where the contact 
         * was previously open as well.
         */
         if (penetration > opencriteria*domainscale)  
            locks[contact][CURRENT]  = OPEN;
      }
      else 
      {
        /* If we hit this assert, we got real problems */
         assert(1);
      }

/***  End block ***/

      //fprintf(fp.logfile,"S0, dispdet, S0/dispdet: %f %f %f\n",S0, dispdet,S0/dispdet);

     /* previously close: alternate near side for v-v  */
     /* s0=.0002  cons()                               */
     /* contact position  v-e close  keep old position */
      if ( (CTYPE == VV) || (locks[contact][CURRENT] == OPEN) ) 
         continue; // probably goto nextcontact; //goto c207;
     /* Separate these out for readability: */
     /* if (locks[contact][CURRENT] == OPEN) 
           continue;
      */

     /* So we have a vertex-edge contact and the CURRENT 
      * contact is CLOSED.
      */

      if (locks[contact][PREVIOUS] == OPEN)
      { 
        /* S0 is (twice) the original area of the contact before
         * displacement, set in the loop that determines the 
         * penetration distance.  dispdet is the "displacement
         * determinant".  This ratio appears to be all over the map
         * printing out and looking at it.  
         */
         t0 = -S0/dispdet;
         if (t0<0)  // S0, dispdet same sign
            t0=0;
         if (t0>1)  // S0, dispdet opposite sign
            t0=1;

        /* This `if' was previous and triggered a jump the preceding two blocks.
         * FIXME: Replace function fabs with macro FABS.
         * FIXME: Replace the small constant with a tolerance value
         * selectable by the operator.
         */
         if ( (S0<0) || (fabs(dispdet) < 0.000000001*domainscale*domainscale)) //1e-10
            t0=0;

        /* (GHS: from open to close compute new lock position)  */
        /* So what the parameter t0 does is controls how much of the 
         * displaced contact to use for updating the open-close 
         * scheme.  If t0 = 0, use the original; if t1 = 1, use the 
         * original plus the displaced locations.  So why the ratio?
         */
         x4 = x1+t0*p[1][1]; y4 = y1+t0*p[1][2];
         x5 = x2+t0*p[2][1]; y5 = y2+t0*p[2][2];
         x6 = x3+t0*p[3][1]; y6 = y3+t0*p[3][2];
        /* If t0 = 0, b2 = S0? */
         b2 = (x6-x5)*(x6-x5) + (y6-y5)*(y6-y5);
        /* If t0 = 0, the following equals unity? 
         * c_length[contact][2] is the omega shear locking parameter.
         */
         c_length[contact][2] = ((x4-x5)*(x6-x5) + (y4-y5)*(y6-y5))/b2;
      }  /* close if (previously OPEN contacts) */
       
     /* FIXME: What is this? */
     /* If the previous lock was closed, c_length[contact][2] will 
      * already be set. (omega)
      */
      //ratio = c_length[contact][2];
      omega = c_length[contact][2];
     /* ratio \in [0,1] , but not always... it is probably a mistake if 
      * ratio is not in the unit interval.  
      * FIXME: Find out why ratio is sometimes < 0 or > 1.
      */
      //assert(0.0 <= ratio  && ratio <= 1.0);
      assert(0.0 <= omega  && omega <= 1.0);
      //fprintf(fp.logfile,"ratio: %f\n",ratio);

     /* (GHS: case e-v sliding distance   a1: previous) */
     /* From Mary's notes, these are coordinates of 
      * old contact points.
      */
     /* FIXME: Add more comments */
      //x0 = (1 - ratio)*x2 + ratio*x3;
      //y0 = (1 - ratio)*y2 + ratio*y3;
     /* Delta (???) contact point coords due to displacement increment.
      */
      //x4 = (1 - ratio)*p[2][1] + ratio*p[3][1];
      //y4 = (1 - ratio)*p[2][2] + ratio*p[3][2];

     /* FIXME: Add more comments */
      x0 = (1 - omega)*x2 + omega*x3;
      y0 = (1 - omega)*y2 + omega*y3;
     /* Delta (???) contact point coords due to displacement increment.
      */
      x4 = (1 - omega)*p[2][1] + omega*p[3][1];
      y4 = (1 - omega)*p[2][2] + omega*p[3][2];

     /* FIXME: Comment profusely */
     /* Perpendicular offset of (x1,y1) from the line through (x0,y0)
      * perpendicular to the old ref line.
      */
      s1  = (x1-x0)*(x3-x2) + (y1-y0)*(y3-y2);
     /* contribution to perp offset from displacement of pt 1 */
      s1 += (p[1][1]-x4)*(x3-x2) + (p[1][2]-y4)*(y3-y2);
     /* contribution to perp offset from displacement of ref line endpoints */
      s1 += (x1-x0)*(p[3][1]-p[2][1]) + (y1-y0)*(p[3][2]-p[2][2]);

     /* direction of shear displacement from previous iteration */
      prev_shear_direction = sign(c_length[contact][1]);
     /* (shear disp)*(orig ref line length)/(orig ref line length) = shear disp
      * FIXME: The previous comment makes no sense.
      */
     /* curr_shear_length seems to be between -1 and 1.
      */
      curr_shear_length  = s1/reflinelength;
      //fprintf(fp.logfile,"curr_shear_length: %f\n", curr_shear_length);

     /* Shear displacement or movement, value in [-1,1] (???) */
      c_length[contact][1] = s1/reflinelength;

      //fprintf(fp.logfile,"c_length[contact][1] (df22): %f\n", c_length[contact][1]);
 
      /*================================================*/

/************  VE,  Shear locks (?)  *************/

/* The following block probably implements the material on 
 * contact locking from pages 171 and 172 of Shi, 1988.
 * Also, the results of this are probably used in df18.
 */

/* All the contacts in the following block are VE because VV
 * contacts continued to next a while back.  Also, the
 * CURRENT contacts are closed, which was initialized in 
 * probably df06() or df07().
 */

 /* As the following print statement verifies, somewhere 
  * previously, all the CURRENT locks are set to 1, which 
  * means SLIDING (?).  So when we get here, all we have to do 
  * is check whether or not to lock it (?).  Presumably, the 
  * shear resistance will be handled later, in df18() maybe.
  */
  //fprintf(fp.logfile,"locks[contact][CURRENT]: %d\n",locks[contact][CURRENT]);

      /* cases: open lock inherit-slide reverse-slide   */
      /* locks[][2]=1 is previously set for all cases   */
      if (locks[contact][PREVIOUS] == OPEN) 
      {
        /* (GHS: previous open (no shear spring was set))  */
        /* FIXME: precompute friction tangents */
         tanphi = tan(dd*phi);
        /* FIXME: Replace fabs with a macro */
        /* LOCK contact if shear movement, force less than 
         * shear strength.
         */
        /* If the shear movement is less than the product of the
         * normal penetration distance with tanphi, LOCK the 
         * CURRENT contact.
         */
        /* FIXME: There is no spring stiffness here.  Explain why this
         * works, possibly gets divided out.
         */
        /* FIXME: Why is there no cohesion or tension forces in the 
         * this case?
         */
         if (fabs(c_length[contact][1]) < fabs(tanphi*c_length[contact][0]))  
            locks[contact][CURRENT] = LOCKED;
         continue; //goto c207;
      }
      else if (locks[contact][PREVIOUS] == LOCKED) 
      {
         e1 = 0;

        /* cohesion */
        /* FIXME: Check the value of locks[][0], which is 
         * supposedly tension, but here is associated with 
         * cohesion value???
         */
         if (locks[contact][TENSILE] == 1)  
            e1 = cohesion/(ad->JointNormalSpring);

        /* if no penetration, no normal force */
         if (c_length[contact][PENDIST] > 0)  
            tanphi=0;
         else 
            tanphi = tan(dd*phi);

        /* if shear strength of ith contact > shear load, lock */
        /* Specifically, c_length[][0] should be the normal force, not just the 
         * penetration depth.  Which means the c_length[][1] needs to be the 
         * shear strength of some sort.  So the normal force has to be greater
         * than 2.5 times the shear force to LOCK the contact.
         */
         if (fabs(c_length[contact][PENDIST])*tanphi+e1 > fabs(c_length[contact][1])/shear_norm_ratio)  
            locks[contact][CURRENT] = LOCKED;
         continue; //goto c207;
      }
     /* Else we have contact SLIDING! */
     /* If the sliding is the same direction this iteration as it 
      * was the previous iteration...
      */
      else if (prev_shear_direction*curr_shear_length <  -SHEAR_TOL*domainscale)   
      {
        /* FIXME: Then sliding direction changes */
         if (phi > .9)  
           locks[contact][CURRENT] = LOCKED;
        /* (GHS?: normal force near zero) */
         if (ad->gravityflag == 1)
         {                                                                                                                                                                                                                                                                                                                                                                                                                                                    
            grav->contactFN[contact][2] = c_length[contact][0]*(ad->contactpenalty);
            grav->contactFN[contact][3] = c_length[contact][0]*(ad->contactpenalty);
            //assert(fabs(grav->contactFN[contact][3]) > 0);
         }
         continue;
      }
     /* Else if sliding direction same as previous step.  
      * FIXME: This is redundant code, find a way to eliminate
      * this.
      */
     /* FIXME: Where are locks[][] initialized for a time step?
      */
      else if (prev_shear_direction*curr_shear_length >=  -SHEAR_TOL*domainscale)  
      {     
        /* sliding direction is same as previous step  */
         continue;
      }
      else 
      {
        /* If this assertion fires, we are in big trouble. */
         assert(1);
      }

   }  /*  (contact, was i)  end loop for each contact */
   







/**************  End of VE, Shear locking block  *******************/


  /* Check convergence */
   //if (1) //ad->contactmethod == penalty)
   {  
     /* i7,i8: Indicators for open-close convergence. 
      */
      int contact;
     /* (GHS: open-to-close i7   close-to-open i8) */
     /* Check the state of the locks.  If locks have changed
      * flag the change with i7 or i8.
      * FIXME: Add these state change variables to the data that is written
      * out and graphed.  This is going to be difficult unless plain old fprintf
      * is used.
      */ 
      i7=0;
      i8=0;
      for (contact=1; contact<= nContacts; contact++)
      {
         if (locks[contact][PREVIOUS] == OPEN && locks[contact][CURRENT] > OPEN)  
            i7+=1;
         if (locks[contact][CURRENT] == OPEN && locks[contact][PREVIOUS] > OPEN)  
            i8+=1;
      }  /*  contact, was i  */
      //fprintf(fp.logfile,"From df22, i7 = %d, i8 = %d\n", i7,i8);
     /* If we had a change of state, then do another round 
      * of open-close iteration.  ad->n9 appears to be a control
      * variable ensuring that the open does something.
      */  
   }


/* WARNING:  Testing for augmented lagrangian convergence here. */
   if (ad->contactmethod == penalty)
   {
      if (i7+i8>0 || ad->n9==1)
         return;
   }
   else if (ad->contactmethod == auglagrange)
   {
      iface->displaymessage("Aug. Lagrange not currently implemented");
   }
   else  // bad news...
   {
     /* Something is very wrong and we have a problem. */
      ;
   }

  /* Else m9 = -1 means passed open-close iteration.  m9 
   * IS NOT A FLAG!!!  It is a counter. 
   * FIXME: Change this so that open-close convergence is
   * flagged, then the number of open-close counts can 
   * be set elsewhere as well as recorded.
   */
   DLog->openclosecount[ad->currTimeStep] = ad->m9;
   ad->m9= -1;
   
#if COUNTCLOSEDCONTACTS
  /* If locks[][2] > 0, then the contact is closed.
   * This gives a count of the number of closed contacts
   * vice total number of contacts.
   */  
  /* i9 is the current number of closed contacts in 
   * this subroutine.  Hang on to this for a while.
   */
   { 
      int i, i9;
      i9=0;
      for (contact=1; contact<= nContacts; contact++)
      {
         if (locks[contact][CURRENT] != OPEN)  
            i9+=1;
      }  /*  i  */
   }
#endif /* COUNTCLOSEDCONTACTS */

//c213:;
}  /* Close df22().  */





/*------------------------------------------------*/
/*  u v  or x+u y+v  of vertices in the `vertices'*/
/* matrix,                                        */
/* u[vertexCount+1][3]                            */
/* WARNING!!!  U is overloaded in df04, df05 for 
 * storing the inside and outside block angles.
 */
static int __vtxcopysize1;
static int __vtxcopysize2;
static double **vtxcopy;

void
initIntTempArrays(int size1)
{

   __vtxcopysize1=size1;
   __vtxcopysize2=3;

   vtxcopy = DoubMat2DGetMem(__vtxcopysize1, __vtxcopysize2);

}  /* close  initIntTempArrays() */

void
freeIntTempArrays()
{
   if (vtxcopy)
      free2DMat((void **)vtxcopy, __vtxcopysize1);

} /* close freeIntTempArrays() */


/**************************************************/
/* df24: displacement ratio and iteration drawing */
/**************************************************/
void df24(Geometrydata *gd, Analysisdata *ad, int *k1)
{
   int i, i0, i1, i2, i3;
   int j;
   int l;
   double a1, a2;
   double x, y;

   double ** F = ad->F;
   double ** Fcopy = ad->Fcopy;

   int nBlocks = gd->nBlocks;
   int nContacts = gd->nContacts;
   int n3 = ad->n3;  /* Not changed here.  */
   double ** K = ad->K;
   double ** Kcopy = ad->Kcopy;
   double T[7][7];
   
   double ** vertices = gd->vertices;
   int ** vindex = gd->vindex;
   double ** moments = gd->moments;
   double ** globalTime = ad->globalTime;
   double maxdisplacement = ad->maxdisplacement;  // was g2
   double domainscale = ad->constants->w0;

  /* (GHS: vertex displacements    i0 old block number) */
	for (i=1; i <= nBlocks; i++)
   {
	  	i1 = vindex[i][1];
	  	i2 = vindex[i][2];
      
	  	for (j=i1-1; j<=i2+1; j++)
      {
	     	x  = vertices[j][1];
	      y  = vertices[j][2];
        /* I should get rid of i0 if possble and just use
         * i to pass into getDisplacement.  i0 might be used elsewhere 
         * though.
         */
	      i0 = i;

         computeDisplacement(moments,T,x,y,i0);
         
         i3 = k1[i];
         
	       vtxcopy[j][1]  = 0;
	       vtxcopy[j][2]  = 0;

         for (l=1; l<= 6; l++)
         {
           /* Skip the 3 entry, add the rotation correction 
            * after the loop.
            */
            if (ad->rotationflag == 1)
            {
               if(l==3) 
                  l++;
            }
		      vtxcopy[j][1] += T[1][l]*F[i3][l];
            vtxcopy[j][2] += T[2][l]*F[i3][l];
         }  /*  l  */

         if (ad->rotationflag == 1)
         {
            //u[j][1] += (-T[2][3]*F[i3][3]*F[i3][3]/2.0) + (T[1][3]*F[i3][3]);
            //u[j][2] += (T[2][3]*F[i3][3]) + (T[1][3]*F[i3][3]*F[i3][3]/2.0);
            vtxcopy[j][1] += T[2][3]* (cos(F[i3][3]) - 1) + T[1][3]*sin(F[i3][3]);
            vtxcopy[j][2] += T[2][3]*sin(F[i3][3]) - T[1][3]*(cos(F[i3][3])-1);
         }

      }  /*  j  */
		 }  /*  i  */


  /* (GHS: relative maximum displacement ratio) */
  /* This appears to compute the maximum displacement suffered
   * by any single vertex.  FIXME: Check whether replacing this 
   * with max centroid displacement and max angle is better.  This
   * will require TCK rotation results.
   */
		 a1=0;
		 for (i= 1; i<=  nBlocks; i++)
   {
      i1 = vindex[i][1];
      i2 = vindex[i][2];
      for (j=i1; j<=i2+1; j++)
      {
         a2 = sqrt(vtxcopy[j][1]*vtxcopy[j][1] + vtxcopy[j][2]*vtxcopy[j][2]);
         if (a1<=a2)  
            a1 = a2;
      }  /*  j  */
   }  /*  i  */

   globalTime[ad->currTimeStep][1] = (a1/domainscale)/maxdisplacement;
   
  /* recover  a[][]  f[][] after equation solving   */
  /* m9 = -1 means iteration finished               */
   if ((ad->m9) != -1) 
   {   
      for (i=1; i<= n3; i++)
      {
         for (j=1; j<= 36; j++)
         {
           /* FIXME: Use an array copy function or a memset */
            K[i][j] = Kcopy[i][j];
         }  /*  j  */
      }  /*  i  */
   
      for (i=1; i<= nBlocks; i++)
      {
         for (j=1; j<=  6; j++)
         {
           /* FIXME: Use an array copy function or a memset. */
            F[i][j] = Fcopy[i][j];
         }  /*  j  */
      }  /*  i  */
   }  
   
  /* draw deformed blocks, change u[][]  */
   for (i= 1;   i<=  nBlocks; i++)
   {
      i1 = vindex[i][1];
      i2 = vindex[i][2];
      for (j=i1-1; j<=i2+1; j++)
      {
         vtxcopy[j][1] += vertices[j][1];
         vtxcopy[j][2] += vertices[j][2];
      }  
   } 
   
}  /*  Close df24()  */


static void 
updateStresses(Geometrydata * gd, Analysisdata * ad,
               double ** e0, int * k1)
{
   int nBlocks = gd->nBlocks;
   double ** D = ad->F;
   double ** moments = gd->moments;
   int timestep = ad->currTimeStep;

   int i, i1;
   double a1;
   double gamma_0; 
   double c,s;
   double sigmaxx, sigmayy, tauxy;

  /* This will get wrapped in a user controlled option 
   * at some point.
   */
  /* (GHS: compute updating stresses)  */
   for (i=1; i<= nBlocks; i++)
   {
      double E = e0[i][2];
      double nu = e0[i][3];

      i1 = k1[i];
      gamma_0 = D[i1][3];
      c = cos(gamma_0);
      s = sin(gamma_0);

      if(ad->planestrainflag == 1)
      {
         a1 = E/(1 + nu);
         e0[i][4] += a1*( ((D[i1][4]*(1-nu))/(1-2*nu)) + ((D[i1][5]*nu)/(1-2*nu)) );
         e0[i][5] += a1*((D[i1][4]*nu)/(1-2*nu) + (D[i1][5]*(1-nu)/(1-2*nu)));
         e0[i][6] += a1*D[i1][6]/2.0;
      }
      else  /* Plane stress */
      {     
         a1        = E/(1-(nu*nu));
         e0[i][4] += a1*(D[i1][4]+D[i1][5]*nu);
         e0[i][5] += a1*(D[i1][4]*nu+D[i1][5]);
         e0[i][6] += a1*D[i1][6]*(1-nu)/2;
      }  /* end if planestrain else planestress */

     /* TCK stress rotation correction, Eq. 17, p. 324,
      * ICADD 1 proceedings.  See also Eqs. 18 and 19 for
      * formulas for updating the deformation rates.
      */
     /* TODO: verify that doing the updating here is 
      * mathematically correct.  It might need to be done 
      * before adding to the existing block stresses which
      * are (presumably) already in referential coordinates.
      */
      if (1)
      {
         sigmaxx = e0[i][4];
         sigmayy = e0[i][5];
         tauxy = e0[i][6];
         e0[i][4] = c*c*sigmaxx - 2*c*s*tauxy + s*s*sigmayy;
         e0[i][5] = s*s*sigmaxx - 2*c*s*tauxy + c*c*sigmayy;
         e0[i][6] = c*s*(sigmaxx-sigmayy) + (c*c - s*s)*tauxy;
      }
      
     /* Now compute e_zz, which should be 0 if we are in plane
      * strain, and will be used for computing mass if in 
      * plane stress.  This is for density correction.  See
      * TCK, p. 213, Fung, p. 267.
      * Note: we have to accumulate e_z to use for calculating
      * current thickness of block when calculating mass.
      */
      e0[i][7] += -(nu/(1-nu))*(D[i1][4] + D[i1][5]);
      //if (ad->planestrainflag == 1)
      //{
         //assert (fabs(e0[i][7]) <= 10.0e-3);
      //}


     /* Virtual work performed by stresses given at the start of 
      * the time step through the time step.  See Doolin, p. ???
      * 200?.
      */
     /* It would also be reall cool to track this on a per block per
      * time step basis, but that would be lots of dereferencing
      * and ram usage.  To see how that would work, change timestep
      * to `i' to see the stress for each block.
      */
      
      DLog->energy[timestep].istress += moments[i][1]*(D[i1][4]*e0[i][4] 
                                 + D[i1][5]*e0[i][5] + D[i1][6]*e0[i][6]);
   }  

}  /* close updateStresses() */


/**************************************************/
/* df25: compute step displacements               */
/**************************************************/
/* Compute displacements for each time step.
 */
void 
df25(Geometrydata *gd, Analysisdata *ad, int *k1,
          double **e0, double **U)
{
   int i, i0, i1, i2;
   int j; 
   //double a1; 
   int nBlocks = gd->nBlocks;
   int nLPoints = gd->nLPoints;
   int nFPoints = gd->nFPoints;
   int nMPoints = gd->nMPoints;
   double ** points = gd->points;
   double ** prevpoints = gd->prevpoints;
   double ** vertices = gd->vertices;
   int ** vindex = gd->vindex;
   double ** moments = gd->moments;
   double ** hb = gd->rockbolts;
   double ** c = ad->c;
   double x, y;
  /* Displacements */
   double x1, y1;
  /* End point block number of rock bolts. */
   int ep1,ep2;
  /* Helper vars for updating stresses etc. */
   //double nu, youngsmods;
   double T[7][7];
  /* Deformations D overwrite the forcing vector F during 
   * LU decomposition.
   * FIXME: Where is the forcing vector rezeroed?
   */
   double ** D = ad->F;
   clock_t start, stop;

   int timestep = ad->currTimeStep;

   start = clock();

  /* (GHS: change to new deformed block shape  u[][] df24) */
  /* In df24(), U gets over-written with vertex locations 
   * updated at the end of each open-close time step.  Here
   * we copy the vertex locations back to the appropriate 
   * array.  We need U afterwards for displacements.
   */
   for (i=1; i<=  nBlocks; i++)
   {
      i1=vindex[i][1];
      i2=vindex[i][2];
      
      for (j=i1-1; j<=i2+1; j++)
      {
         vertices[j][1]=vtxcopy[j][1];
         vertices[j][2]=vtxcopy[j][2];
      }  /*  j  */
   }  /*  i  */


  /* F has been overwritten with the solution vector.  Since 
   * we need updated velocities and accelerations for time
   * integration schemes, we also need to save a copy of the 
   * previous displacements.
   * FIXME: U is allocated for number of vertices, not number
   * of blocks!!!  This is very confusing because we really
   * need a copy of the displacements, as well as a copy of the
   * vertex updates.  
   */
   for (i=1; i<= nBlocks; i++)
   {
      i1=k1[i];
      for (j=1; j<=  6; j++)
      {
         U[i][j+6] = U[i][j];
         U[i][j] = D[i1][j];
      }  /*  j  */
   }  /*  i  */

  /* For each measured point, save the current location,
   * i.e., (x,y) coordinates to compare with the updated
   * coordinates.  This is used to determine the 
   * displacements of the measured points.  Beware
   * the index shifting here.
   * FIXME: Write a copy array function.
   */   
   for (i=1; i<=nFPoints+nLPoints+nMPoints; i++)
   {
      prevpoints[i][1] = points[i][1];
      prevpoints[i][2] = points[i][2];
   }  /* close cloning loop */

   for (i=1; i<=nFPoints+nLPoints+nMPoints; i++)
   {
      i0 = (int)points[i][3];
      x  = points[i][1];
      y  = points[i][2];

      computeDisplacement(moments,T,x,y,i0);
      
      i1 = k1[i0];
      x1 = 0;
      y1 = 0;
      
     /* displacements of fixed measured load points    */
      for (j=1; j<= 6; j++)
      {
        /* skip 3, add the rotation correction 
         * after the loop.
         */       
         if (ad->rotationflag == 1)
         {
            if (j==3)
               j++;
         }
         x1 += T[1][j]*D[i1][j];
         y1 += T[2][j]*D[i1][j];
      }  /*  j  */
      
      if (ad->rotationflag == 1)
      {  /* 2d order */
         //x1 += (-T[2][3]*F[i1][3]*F[i1][3]/2.0) + (T[1][3]*F[i1][3]);
         //y1 += (T[2][3]*F[i1][3]) + (T[1][3]*F[i1][3]*F[i1][3]/2.0);
         x1 += (T[2][3] * (cos(D[i1][3])-1) + T[1][3]*sin(D[i1][3]));
         y1 += (T[2][3] * sin(D[i1][3]) - T[1][3]*(cos(D[i1][3])-1)); 
      }

     /* Compute work of load points here, before the point
      * location is updated.
      * FIXME: Can fixed points get measured here also?
      */
      points[i][1] +=  x1;
      points[i][2] +=  y1;
     
     /* These appear to be used only in df12() for computing 
      * fixed point matrices.  Here, x1, y1 are displacements.
      * Since the penalty method does not prohibit fixed points
      * from any movement at all, the amount that the fixed point
      * moved is stored, then used to compute a ``restoring 
      * force'' in df12().  NOTE: The fixed points must be at the 
      * beginning of the points array for this implementation.
      */
      c[i][1] += -x1;
      c[i][2] += -y1;
   }


  /* Amazingly enough, this appears to work! */
   if (gd->nSPoints > 0)
   { 
      DList * ptr;
      DDAPoint * ptmp;
      M_dl_traverse(ptr, gd->seispoints)
      {
         //Extract the point struct from the DList.
         ptmp = ptr->val;
         i0 = ptmp->blocknum;
         x  = ptmp->x;
         y  = ptmp->y;
         computeDisplacement(moments,T,x,y,i0);
         i1 = k1[i0];
         x1 = 0;
         y1 = 0;
        /* displacements of fixed measured load points    */
         for (j=1; j<= 6; j++)
         {
           /* skip 3, add the rotation correction 
            * after the loop.
            */       
            if (ad->rotationflag == 1)
            {
               if (j==3)
                  j++;
            }
            x1 += T[1][j]*D[i1][j];
            y1 += T[2][j]*D[i1][j];
         }  /*  j  */
      
         if (ad->rotationflag == 1)
         {  /* 2d order */
            //x1 += (-T[2][3]*F[i1][3]*F[i1][3]/2.0) + (T[1][3]*F[i1][3]);
            //y1 += (T[2][3]*F[i1][3]) + (T[1][3]*F[i1][3]*F[i1][3]/2.0);
            x1 += (T[2][3] * (cos(D[i1][3])-1) + T[1][3]*sin(D[i1][3]));
            y1 += (T[2][3] * sin(D[i1][3]) - T[1][3]*(cos(D[i1][3])-1)); 
         }
         ptmp->x += x1;
         ptmp->y += y1;

      }
   }

  /* Track displacement of load points for an initial hack 
   * of disp dep variables.  Need to store a copy of the previous
   * position of the points first, then take the difference of the
   * current and previous points.  Note that indexing is from 
   * zero in the struct array.  Note also the 
   * counter (i) is tested as a `<' rather than
   * `<='
   */
  /* Huge kludge: Since GHS put all of the points into a
   * single array  bah.  bah. 
   */
   for (i=nFPoints+nLPoints+1; i<=nFPoints+nLPoints+nMPoints; i++)
   {
     /* delta x I hope */
      points[i][5] = points[i][1] - prevpoints[i][1];
     /* accumulate x displacement */
      points[i][7] += points[i][1] - prevpoints[i][1];
     /* delta y */
      points[i][6] = points[i][2] - prevpoints[i][2];
      points[i][8] += points[i][1] - prevpoints[i][1];

   }  /* close cloning loop */
   

   updateStresses(gd, ad, e0, k1);

   
{
   double avgarea;
  /*------------------------------------------------*/
  /* compute s sx sy sxx syy sxy    g0 of this step */
   avgarea = computeMoments(gd); //, moments);
   if (avgarea == 0)
   {
      ad->abort(ad->this);
      ddaerror.error = __ZERO_MOMENT_ERROR__;
      iface->displaymessage("Zero block area");
   }
   ad->avgArea[ad->currTimeStep] = avgarea;
   computeMass(gd, ad, e0);
  /* This should go into the geometrydata.c file.  In the 
   * future we will ask the blocks to write out there areas.
   */
   writeBlockareas(ad,gd);

}

   
  /* Compute rock bolt end displacements.  Note that no
   * rotation correction is supplied here.
   */
   for (i=0; i<gd->nBolts; i++) 
   {
     /* Deal with one endpoint at a time,
      * starting with the arbitrarily chosen
      * `endpoint 1'.  Since hb is a double **,
      * we have to cast the block numbers of the 
      * endpoints.
      */
      ep1 = (int)hb[i][5];
      x = hb[i][1];
      y = hb[i][2];
      computeDisplacement(moments,T,x,y,ep1);
      for (j=1; j<= 6; j++)
      {
         x1 += T[1][j]*D[ep1][j];
         y1 += T[2][j]*D[ep1][j];
      }  /*  j  */
      hb[i][1] +=  x1;
      hb[i][2] +=  y1;

     /* Now for endpoint 2, the other end of the same bolt.
      */
      ep2 = (int)hb[i][6];
      //if (ep1 == ep2) exit(0);
      x = hb[i][3];
      y = hb[i][4];
      computeDisplacement(moments,T,x,y,ep2);
      for (j=1; j<= 6; j++)
      {
         x1 += T[1][j]*D[ep2][j];
         y1 += T[2][j]*D[ep2][j];
      }  /*  j  */
      hb[i][3] +=  x1;
      hb[i][4] +=  y1;
            
   }  /* Close rock bolt displacement loop.  */

  /* FIXME: This looks stupid.  There has to be a better way...
   */
   ad->elapsedTime = ad->globalTime[ad->currTimeStep][0];

   stop = clock();

   DLog->update_runtime += stop - start;

} /*  Close df25(). */

