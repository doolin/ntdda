/*
 * gravity.c 
 *
 * Function to handle the gravity turn on 
 * for DDA code.  The way this works is to 
 * run the code and reset the displacements 
 * after every step, while tracking the forces
 * at block contacts.  At the start of an 
 * analysis, there won't be any forces.  At the
 * end of one delta_t, forces at contacts have
 * increased.  Then the displacements and velo-
 * cities are reset to zero, and another iteration
 * is performed.  If at the end of this iteration
 * the forces are the same as the last, then
 * a full gravity load has been applied, which is
 * an initial condition and the real analysis can
 * then proceed.  What is necessary for this procedure
 * is to track the delta_t, so that it can be reset
 * if not converged, the displacement and velocity
 * matrices which have to reset to zero if not
 * converged, and the stress/force matrices, which 
 * have to be checked to see how has changed between
 * iterations.
 *
 * $Author: doolin $
 * $Date: 2002/05/26 23:47:25 $
 * $Source: /cvsroot/dda/ntdda/src/gravity.c,v $
 * $Revision: 1.4 $
 */
//#include "compilecontrol.h"
#include<math.h>
#include<assert.h>
#include"analysis.h"
#include"ddamemory.h"
#include "gravity.h"



/* Necessary evil. FIXME: Pass in an open file pointer or something
 * instead.  Find a way to kill extern globals.
 */
extern FILEPOINTERS fp;


static void
copyparams(Gravity * grav, double ** pp, double ** F, int numblocks)
{
   int j;

   for (j=1; j<=numblocks; j++)
   {
      pp[j][1] = F[j][1];
      pp[j][2] = F[j][2];
      pp[j][3] = F[j][3];
      pp[j][4] = F[j][4];
      pp[j][5] = F[j][5];
      pp[j][6] = F[j][6];
   }  /* end copyParams */

  /* Add a field to the gravity struct to handle how often 
   * time steps get cut.
   */
   if (!(grav->gravTimeStep%10))
   {
      //char mess[180];
      //sprintf(mess,"\ngravTurnon = %d, cutting delta_t size\n",grav->gravTimeStep);
      //fprintf(fp.gravfile,mess);
      //iface->displaymessage(mess);
      grav->delta_t *= 0.3;
   }

  /* Did not converge, try again. Increment gravTimeStep
   * to check against gravity steps.
   */
   grav->gravTimeStep++;

  /* Might need to cut time step size... */

}  /* close copyparams() */




static ddaboolean
computeGravityForce(Gravity * grav, Analysisdata * adn, int n2, int numblocks)
{
   double ** F = adn->F;

   double ** cfs = grav->contactFS;
   double ** cfn = grav->contactFN;
   double ** pforces = grav->prevforces;
   double ntol = grav->nforcetolerance; 
   double stol = grav->sforcetolerance;  
   double diff, tol;

  /* residual_tol should probably be force tolerance, which is 
   * initialized elsewhere.
   */
   double residual_tol = grav->residual_tol;
   int i, j;

   static char *gconv[6] = {"u0", "v0", "r0", "ex", "ey", "exy"};


  /* Temporary write to log file to see what the contacts are doing.
   */
   fprintf(fp.gravfile,"\nContact forces for grav step %d:\n", 
                      grav->gravTimeStep);
   fprintf(fp.gravfile,"             "
                      "current    previous iter   previous step\n");

   for (j=1;j<=n2;j++) 
   {
         fprintf(fp.gravfile,"normal     %f       %f       %f  (contact %d)\n",
                         cfn[j][2], cfn[j][1], cfn[j][0], j); 
         fprintf(fp.gravfile,"shear      %f       %f       %f  (contact %d)\n",
                         cfs[j][2], cfs[j][1], cfs[j][0], j);
   }


   fprintf(fp.gravfile,"\nContact forces for grav step %d:\n", 
                      grav->gravTimeStep);
   fprintf(fp.gravfile,"             "
                      "current    previous iter   previous step\n");


   for (j=1;j<=n2;j++) 
   {
     /* Use named constants for the array indices */
      diff = fabs(cfn[j][2]-cfn[j][0]);
      tol = fabs(ntol*cfn[j][0]);

      if (diff > tol)
      {   
         fprintf(fp.gravfile,"normal     %f       %f       %f\n",
                         cfn[j][2], cfn[j][1], cfn[j][0]); 
         fprintf(fp.gravfile,"\nfailed normal contact force convergence\n"
                            "(tolerance = %f) at contact %d of %d, continuing gravity turn on\n",
                 ntol, j, n2);
         fprintf(fp.gravfile,"Difference: %f; Tolerance: %f\n", diff, tol);
         //j = n2 + 2;
         copyparams(grav,pforces,F,numblocks);
         return FALSE;
      }

      diff = fabs(cfs[j][2]-cfs[j][0]);
      tol = fabs(stol*cfs[j][0]);
      if (diff > tol)
      {
         fprintf(fp.gravfile,"shear      %f       %f       %f\n",
                         cfs[j][2], cfs[j][1], cfs[j][0]);
         fprintf(fp.gravfile,"\nfailed shear contact force convergence \n"
                            "(tolerance = %f) at contact %d of %d, continuing gravity turn on\n",
                 stol, j, n2);
         fprintf(fp.gravfile,"Difference: %f; Tolerance: %f\n", diff, tol);
         copyparams(grav,pforces,F,numblocks);
         return FALSE;
      }
   }  /* end for each contact j */

   fprintf(fp.gravfile,"Contact normal and shear forces converged.\n");


  /* Check for convergence of unknowns */
   for (j=1; j<=numblocks; j++)
   {
      for (i=1;i<=6;i++)
      {
         if ( (fabs(pforces[j][i]) > grav->residual_tol)  &&  
                (  
                  (F[j][i] - pforces[j][i]) > fabs(residual_tol*pforces[j][i])
                )
            )
         {
            fprintf(fp.gravfile,"\nFailed at %s: current = %.12f previous = %.12f\n",
                                gconv[i], F[j][i], pforces[j][i]);
        
            copyparams(grav,pforces,F,numblocks); 
            return FALSE; 
         }
      }  /* end for each stress, strain i */
   }  /* end for each block j */

   fprintf(fp.gravfile,"Deformation parameter set converged.\n");

   fprintf(fp.gravfile,"\nGravity phase complete (%d steps) with shear tolerance = %f\n", grav->gravTimeStep, stol);
  /* This can be controlled as an option in the future.
   */
   //computeMaxDisplacement(gd);
   return TRUE;

} /* Close computeGravityforce() */



/* When gravity turn on is specified, this function is
 * called at the end of every loop, until the forces,
 * strains and displacements converge, or the maximum
 * number of gravity iterations is reached.  After 
 * force, strain and displacement convergence, the
 * gravityflag member is set to 0, the current time
 * step is set to 0, velocities, displacements and
 * strains (?) are reset to zero.  This enables the 
 * analysis to "start over", with realistic forces 
 * induced by gravity self-loading imposed at the 
 * outset, instead of "ramping" up over the first 
 * few time steps.
 */
/* Most of this may eventually get moved into the 
 * computeGravity function.  For now leave it in the 
 * main loop for debugging convenience.
 */
/* TODO: Implement Yeung's (1991) open-close 2 steps */
/* TODO: Implement time step cutting for gravity phase. */
/* TODO: Move all of the following stuff into the gravity 
 * file, and call a wrapper function if gravity phase is 
 * requested.
 */
void
checkGravityConvergence(Gravity * grav, Geometrydata * gd, Analysisdata * ad)
{

   ddaboolean converged;  
   char tempstring[100];

  /* WARNING !!! Having this here instead of further down stiffens the 
   * system and requires less steps to gravity convergence.
   * Lets get rid of this for a while, and set the spring stiffness 
   * to be a constant during the gravity phase.
   */
   initContactSpring(ad);
  /* FIXME: These two criteria need to be separated
   */
   converged = computeGravityForce(grav, ad, gd->nContacts, gd->nBlocks);
   if ( (grav->gravTimeStep > grav->gravTurnonLimit)  ||
        (converged == TRUE)  )
   {
     /* If we get in here, we are done with gravity 
      * for whatever reason.
      */

      ad->currTimeStep = 0;
      //ad->currentTime = 0.0;
      ad->elapsedTime = 0.0;
      ad->gravityflag = 0;
      ad->n9 = 0;
     /* Gravity phase handles its own time step
      * cutting algorithm.  After convergence, reset to
      * initial value stored in maxtimestep.
      */
      ad->delta_t = ad->maxtimestep;

     /* Having this here instead of further up softens the 
      * system and requires more steps to gravity convergence.
      */
      //initContactSpring(AData);

      if (converged)
         sprintf(tempstring,"Gravity convergence in (%d) steps", grav->gravTimeStep);
      else
         sprintf(tempstring,"Gravity step limit (%d) reached", grav->gravTurnonLimit);

      dda_display_info(tempstring);
           
     /* now free the gravity stuff */

   }


  /* May have to do a bunch of other stuff in here if gravity is not 
   * converged yet.
   */
   else
   {
     /* The time step might get cut on nonconvergence.
      */
      ad->delta_t = grav->delta_t;
   }

}  /* close checkGravityConvergence() */








/* FIXME: Change this to handle a Gravity * 
 * instead of an Analysisdata *.
 */
void
initGravity(Gravity * grav, int nBlocks, double delta_t)
{
  /* 21 is an arbitrary number (?) */
   //int n7 = 21*ad->nBlocks + 1;
   int n7 = 21*nBlocks + 1;

  /* gravTimeStep is a time step counter for the gravity 
   * turn on phase.  
   */
   //if (grav->gravityflag == 1)
   grav->gravTimeStep = 1;
   //else 
   //    grav->gravTimeStep = 0;  /* redundant, default value */

  /* Keep the time step values separate as much as possible. */
   grav->delta_t = delta_t;

   grav->contactfnsize1 = n7;
   grav->contactfnsize2 = 4;
   grav->contactFN = DoubMat2DGetMem(grav->contactfnsize1, grav->contactfnsize2);
   setMatrixToZero(grav->contactFN, grav->contactfnsize1, grav->contactfnsize2);

   grav->contactfssize1 = n7;
   grav->contactfssize2 = 4;
   grav->contactFS = DoubMat2DGetMem(grav->contactfssize1, grav->contactfssize2);
   setMatrixToZero(grav->contactFS, grav->contactfssize1, grav->contactfssize2);

   grav->prevforcesize1 = nBlocks+1;
   grav->prevforcesize2 = 7;
   grav->prevforces = DoubMat2DGetMem(grav->prevforcesize1, grav->prevforcesize2);
   setMatrixToZero(grav->prevforces, grav->prevforcesize1, grav->prevforcesize2);

  /*
   grav->gravTurnonLimit = 5000;
   grav->nforcetolerance = 0.01;  
   grav->sforcetolerance = 0.01;  
   grav->residual_tol = 0.001;
   */

}  /* close initGravity()  */


Gravity * 
newGravity()
{
   Gravity * g;
   g = (Gravity *)malloc(sizeof(Gravity));
   memset((void *)g,0xDA,sizeof(Gravity));
   return g;
}  /* close newGravity() */

void
destroyGravity(Gravity * grav)
{

  /* Should check to see these are used first. */
   free2DMat((void **)grav->contactFN, grav->contactfnsize1);
   free2DMat((void **)grav->contactFS, grav->contactfssize1);
   free2DMat((void **)grav->prevforces, grav->prevforcesize1);

   memset((void*)grav,0xDD,sizeof(Gravity));
   free(grav);
   return;
}  /* close destroyGravity() */



int
reinitDisplacements(Geometrydata * geomdata, Geometrydata * gd)
{

   /* check to see if size are the same.  They should be 
    * because gd was cloned from geomdata
    */
    if (gd->vertexsize1 != geomdata->vertexsize1  || 
        gd->vertexsize2 != geomdata->vertexsize2  ||
        gd->vindexsize1 != geomdata->vindexsize1  ||
        gd->vindexsize2 != geomdata->vindexsize2      )
    return 0;  /* uh oh... problems elsewhere */

   /* copy arrays  */
    copy2DDoubMat(gd->vertices, geomdata->vertices, gd->vertexsize1, gd->vertexsize2); 
    copy2DIntMat(gd->vindex, geomdata->vindex, gd->vindexsize1, gd->vindexsize2); 

    return 1;

}  /* close reinitDisplacements() */


int
reinitPointPositions(Geometrydata * geomdata, Geometrydata * gd)
{
    int i, j;

   /* check to see if size are the same.  They should be 
    * because gd was cloned from geomdata
    */
    if (gd->pointsize1 != geomdata->pointsize1  || 
        gd->pointsize2 != geomdata->pointsize2     )
    return 0;  /* uh oh... problems elsewhere */

   /* Movements at fixed measured load points.  That is,
    * at the start of an analysis, these points have zero
    * displacement.  Rock bolt endpoints are initialized
    * during the geometry computation.
    */
   for (i=1; i<= gd->pointCount; i++)
   {
      for (j=1; j<=  2; j++)
      {
         /* FIXME: ad is extern. */
         //ad->c[i][j]=0;
      }  /*  j  */
   }  /*  i  */

   /* copy arrays  */
    copy2DDoubMat(gd->points, geomdata->points, gd->pointsize1, gd->pointsize2); 

    return 1;

}  /* close reinitPointPositions() */


Gravity *
cloneGravity(Gravity * grav_in)
{
   Gravity * grav_out;

   grav_out = (Gravity *)malloc(sizeof(Gravity));
   
   memcpy((void*)grav_out,(void*)grav_in, sizeof(Gravity));
  
  /* The contact forces arrays are used in the 
   * gravity turn on phase.  The next is normal 
   * forces.
   */
   //ado->contactfnsize1 = adn->contactfnsize1;
   //ado->contactfnsize2 = adn->contactfnsize2;
   grav_out->contactFN = clone2DMatDoub(grav_in->contactFN, grav_in->contactfnsize1, grav_in->contactfnsize2);
  /* contact shear forces */
   //ado->contactfssize1 = adn->contactfssize1;
   //ado->contactfssize2 = adn->contactfssize2;
   grav_out->contactFS = clone2DMatDoub(grav_in->contactFS, grav_in->contactfssize1, grav_in->contactfssize2);
  /* Not sure what or why, but prevParams is in Mary's
   * gravity turn on formulation, so it gets put in 
   * here.
   */
   //ado->prevparamsize1 = adn->prevparamsize1;
   //ado->prevparamsize2 = adn->prevparamsize2;
   if (grav_out->prevforces != NULL)
      grav_out->prevforces = clone2DMatDoub(grav_in->prevforces, grav_in->prevforcesize1, grav_in->prevforcesize2);

   return grav_out;

}  /* close cloneGravity() */











#if EXTRAGRAVSTUFF
/* Private functions.  Declare static to keep scope at 
 * file level.
 */
static double computeMaxDisplacement(Geometrydata * gd);
static double cutTimeStep();
#endif



#if EXTRAGRAVSTUFF
/* Compute the maximum vertex displacement */
static double
computeMaxDisplacement(Geometrydata * gd)
{
   int j;
   double maxDisp, tempDX, tempDY, tempDisp;
   double ** dOrig = geomdata->vertices;
   double ** d = gd->vertices;
   int oo = gd->vertexCount;  /* total number of vertices  (?) */

   maxDisp = 0;

   for (j=1; j<=oo; j++)
   {
      tempDX = d[j][1] - dOrig[j][1];
      tempDY = d[j][2] - dOrig[j][2];
      tempDisp = sqrt(tempDX*tempDX + tempDY*tempDY);
      if (tempDisp > maxDisp)
           maxDisp = tempDisp;
   }  /* end for each vertex j */

   fprintf(fp.gravfile,"maximum vertex displacement = %f\n",maxDisp);

   return maxDisp;

}  /* close computeMaxDisplacement() */


/* Cuts the time step to help gravity convergence 
 */
static double
cutTimeStep()
{
   double newtimestep = 0;

   return newtimestep;

}  /* close cutTimeStep()  */

#endif /* EXTRAGRAVSTUFF */



