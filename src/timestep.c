/*
 *  Add some comments here!
 */



#include <math.h>
#include <assert.h>

#include "analysis.h"
#include "ddamemory.h"


/* Time integration temp arrays.  The velocity array
 * v0 needs to be dealt with locally as well.  It can be 
 * initialized during allocation.
 */
static int __A0size1;
static int __A0size2;
double ** __A0;

static int __V0size1;
static int __V0size2;
double ** __V0;

static int __Dsize1;
static int __Dsize2;
double ** __D;


typedef struct _integrator Integrator;

struct _integrator {

   FILE * logfile;

   double ** a;


};

/** This will get moved around a bit. */
static Integrator * integrator;


void
massmatrix_write(double SX0[7][7], double a0, int blocknum) {
  
   int i,j;
   FILE * ofp = integrator->logfile;
   /* Fix these up later. */
   int dof = 6;
   int start, stop;

   start = ((blocknum-1))*dof + 1;
   stop  = blocknum*dof;

   //fprintf(ofp,"From massmatrix...\n");

   fprintf(ofp,"M(%d:%d,%d:%d) = [\n",start,stop,start,stop);
   for (i=1; i<=6; i++) {
      for (j=1; j<=6; j++) {
         fprintf(ofp,"%30.16f ", a0*SX0[i][j]);
      }
      fprintf(ofp,"\n");
   }
   fprintf(ofp,"];\n\n");
}



Integrator * 
integrator_new(void) {

   Integrator * i = (Integrator*)malloc(sizeof(Integrator));

   memset(i,0xda,sizeof(Integrator));
   i->logfile = fopen("massmat.m","w");
   return i;
}


void 
integrator_delete(Integrator * i) {

   fclose(i->logfile);

   memset(i,0xdd,sizeof(Integrator));
   free(i);
}

   


void
initIntegrationArrays(Geometrydata * gd, Analysisdata * ad) {

   int i, i1;
   double ** materialProps = ad->materialProps;
   int nBlocks = gd->nBlocks;
   int ** vindex = gd->vindex;

   integrator = integrator_new();

   __Dsize1=nBlocks+1;
   __Dsize2=13;
   __D = DoubMat2DGetMem(__Dsize1, __Dsize2);

   __V0size1=nBlocks+1;
   __V0size2=13;
   __V0 = DoubMat2DGetMem(__V0size1, __V0size2);

   for (i=1; i<= nBlocks; i++)
   {
      i1 = vindex[i][0];
      __V0[i][1] = materialProps[i1][7]; /* v11 */
      __V0[i][2] = materialProps[i1][8]; /* v22 */
      __V0[i][3] = materialProps[i1][9]; /* vr */
     /* Most likely 4,5,6 store previous iteration values.
      * FIXME: find out what 4,5,6 are and where they are
      * initialized.  v0[][4-6] probably store strain
      * rates, as updated in df25() 
      */
      __V0[i][4] = 0;
      __V0[i][5] = 0;
      __V0[i][6] = 0;
   }  /*  i  */

   __A0size1=nBlocks+1;
   __A0size2=13;
   __A0 = DoubMat2DGetMem(__A0size1, __A0size2);

}


void
freeIntegrationArrays() {

   integrator_delete(integrator);

   if (__A0)
      free2DMat((void **)__A0, __A0size1);

   if (__V0)
      free2DMat((void **)__V0, __V0size1);

   if (__D)
      free2DMat((void **)__D, __Dsize1);

}






/* TCK 1996 p. 324 velocity rotation correction. */
/* FIXME: This does not appear to work at all. */
void
velocity_rotate(double * v, double r) {

   double c,s;
   double du0  = v[1];
   double dv0  = v[2];
   double dex  = v[4];  // \dot{e}_x
   double dey  = v[5];
   double dgxy = v[6];

   c = cos(r);
   s = sin(r);

  /* Displacements */       
   v[1] = c*du0 - s*dv0;
   v[2] = s*du0 + c*dv0;

  /* Strains */
   v[4] = c*c*dex - 2*c*s*dgxy + s*s*dey;
   v[5] = c*s*(dex+dey) + (c*c-s*s)*dgxy;
   v[6] = s*s*dex + 2*c*s*dgxy + c*c*dey;
}


void
newmark_update(double * u, double * v, double * a,
               double a0, double a2, double a7) {

   int j;

   for (j=1; j<=  6; j++) {

      a[j+6] = a[j];
      v[j+6] = v[j];

     /*  Test newmark style updates... */
     //__A0[i][j] = (2*U[i][j])/(pts*pts) - (2/pts)*__V0[i][j+6];
      v[j] = a0*u[j] - a2*v[j+6];
      v[j] = a[j+6] + a7*a[j];
     /* FIXME: Save the previous values to use for 
      * computing kinetic energy.
      */
     /* FIXME: Where are the values of U set? */
      //__V0[i][j] = (2*U[i][j]/(pts))-__V0[i][j];
   }  
}

/**************************************************/
/* df11: submatrix of inertia                     */
/**************************************************/
/* df11() appears to implement a forward difference 
 * scheme outlined in the 1996 offprint from the 1st
 * Int. DDA Forum held in Berkeley, 1996.  The relevant
 * material is listed on pages 23-27.
 */
void df11(Analysisdata *ad, int nBlocks, int *k1, double **F,
          double ** e0, double **moments, int **n, 
          double ** U, MassMatrix massmatrix) {

   int i, j, i1, i2, ji, l;
  /* g6 g5 coefficients of mass and velocity matrix.
   * See Eqs. 2.49 & 2.52, p. 81, Shi 1988.
   */
   //double g5;
  /* No longer necessary */
   //double g6;
  /* unit mass density, was o0 */
   double rho;

  /* Mass matrix variables:  Eq. 2.57, p. 85,
   * Chapter 2, Shi 1988.
   */
   double S0,S1,S2,S3;

  /* x and y coordinates of centroid for each block */
   double x0, y0;

  /* SX0 is integrated moments matrix, Eq. 2.57, p. 85. (was TT). */
   double SX0[7][7] = {{0}};

  /* nBlocks := number of blocks in analysis */
   //int nBlocks = gd->nBlocks;

  /* STATIC or DYNAMIC  defined in dda.h */
   int analysisType = ad->analysistype;
  /* Stiffness matrix */
   double ** K = ad->K;
  /* Delta t time step */
   double ts = ad->delta_t;
   int cts = ad->cts;

  /* previous time step needed for updating. */
   //static double pts;

  /* (Delta t)^2, time step squared */
   double tss;

  /* Viscosity coefficient, GHS 1988 pp.86-88. */
   double mu;
  /* Convenience value for collecting density, viscosity etc. */
   double coef;

   double alpha = .5;
   double delta = 1.0;
   double a0;
   double a2;
   double a7;

  /* Precompute the squared time step to save a few cycles */
   tss = ts*ts;

/* Try to get the newmark stuff running... */
   a0 = 1.0/(alpha*tss);
   a2 = 1.0/(alpha*ts);
   a7 = delta*ts;

  /* Update velocity for load vector.  Eventually want to move 
   * this into a inner loop.  FIXME: The behavior of the 
   * flag `k00' needs to be completely documented.
   * FIXME: Why is this handled at time steps larger than 1?
   * The initial velocity should be used at time step 1, even 
   * if the initial velocity is zero.
   */
  /* k00 = 0 means we haven't yet done a linear solve for the 
   * system.
   */
   //if (currTimeStep > 1 && ad->k00 == 0) 
   if (cts > 1 && ad->m9 == 0) {

      for (i=1; i<= nBlocks; i++) {
         
         //newmark_update(U[i],__V0[i],__A0[i],a0,a2,a7);
#if 1
         for (j=1; j<=  6; j++) {

            __A0[i][j+6] = __A0[i][j];
            __V0[i][j+6] = __V0[i][j];

            /*  Test newmark style updates... */
            //__A0[i][j] = (2*U[i][j])/(pts*pts) - (2/pts)*__V0[i][j+6];
            __A0[i][j] = a0*U[i][j]  - a2*__V0[i][j+6];
            __V0[i][j] = __V0[i][j+6] + a7*__A0[i][j];
           /* FIXME: Save the previous values to use for 
            * computing kinetic energy.
            */
           /* FIXME: Where are the values of U set?
            */
            //__V0[i][j] = (2*U[i][j]/(pts))-__V0[i][j];
         }  
#endif

         if (0) {
            velocity_rotate(__V0[i], U[i][3]);
         }            

      }  
   }


#if 0
  /* g6 g5 coefficients of mass and velocity matrix */
   if ( (analysisType == STATIC) || (ad->gravityflag == TRUE) ) { 
      g5 = 0;  // redundant
   } else { 
      g5=2.0/(ts); 
   }
   //g6=2.0/(tss);
#endif

  /*  inertia terms                                 */
  /* g5: only difference of statics and dynamics    */
  /* g6: inertia coefficient                        */
   for (i=1; i<= nBlocks; i++) {

      rho = e0[i][0];  // density
      mu  = e0[i][8];   // damping

     /* (GHS: zero terms of mass matrix)  */
      memset(SX0,0x0,sizeof(SX0));

     /* (GHS: non-zero terms of mass matrix)  */

      gdata_get_centroid(moments[i],&x0,&y0);

     /* S0, S1, S2, S3 result from integrals derived on 
      * pp. 83-84, Chapter 2, Shi 1988.
      */
      S0 = moments[i][1];  // block area/volume
      S1 = moments[i][4]-x0*moments[i][2];
      S2 = moments[i][5]-y0*moments[i][3];
      S3 = moments[i][6]-x0*moments[i][3];

      massmatrix(SX0,rho,S0,S1,S2,S3);

     /* Log the mass matrix here, make sure to pass in the block number and 
      * time step.
      */
      if (cts == 1) {
      //if (cts == ad->nTimeSteps) {
        massmatrix_write(SX0,a0,i);  
      }

     /* Compute the kinetic energy. */
      //ke = energy_kinetic(__V0[i],S0,S1,S2,S3);

     /* (GHS: add mass matrix to a[][]) */
     /* k1 stores "permutation index",
      * n stores location of memory in 
      * stiffness matrix and force vector.
      */
      i1=k1[i];
      i2=n[i1][1]+n[i1][2]-1;
     
     /* The default value of mu is zero (mu=0). If it is 
      * nonzero, it will be set in the ddaml input file.
      */
      coef = 1; //2*( (mu/ts) + (rho/tss) );
      
      for (j=1; j<= 6; j++) {
         for (l=1; l<= 6; l++) {

            ji=6*(j-1)+l;
            K[i2][ji] += a0*coef*SX0[j][l];
         } 
      }

     /* (GHS: add velocity matrix to f[]) */
     /* FIXME: If we are running a static analysis,
      * is there any need to update accumulate here?
      */
      if (analysisType == STATIC || ad->gravityflag == TRUE) {
         continue;
      }


      for (j=1; j<= 6; j++) {
         for (l=1; l<= 6; l++) {
               
           /* Velocity component of load vector is a
            * matrix-vector product.  BLAS call is DGEMV.
            * FIXME: Move g5 out of this loop.
            */
            /* FIXME: Need to add viscous damping here? */
            //F[i1][j] += g5*rho*TT[j][l]*__V0[i][l];
            F[i1][j] += SX0[j][l]*(a2*__V0[i][l]);
         } 
      }  

   }  /* i end loop over each block */


  /* Here is where we want to set the previous time 
   * step variable, but only for each time step in 
   * time, not each change in time step value.  This is 
   * because Delta t may change during a time step.
   */
   //pts = ad->delta_t;

}


/**
 * @brief An implementation of the Newmark algorithm using Bathe's
 * notation.
 *
 * @bugs This does not work, probably because Bathe does not do an 
 *       incremental update like DDA does.
 * @todo Derive a Newmark variant that assumes that the displacements 
 *       are incrementally updated.
 * @todo Incorporate viscous damping similarly to the ghs integration.
 */
void
newmarkIntegration(Geometrydata *gd, Analysisdata *ad, int *k1, double **F,
          double **matprops, double **moments, int **n, double ** U,

          MassMatrix massmatrix)
{
  /* Loop counters, etc. FIXME: explain i1. */
   int i, j, i1, i2, ji, l;
  /* Newmark parameters.  See Bathe and Wilson, p. 323. */
  /* displacement weight */
   double alpha = .25;
  /* velocity weight */
   double delta = .5;
  /* Hilber-Hughes-Taylor alpha */
   //double hht = 0.0;
  /* Newmark coefficients.  See Bathe and Wilson 1976, 
   * pp. 322-326 for details.  c is current parameter,
   * that is, calculated going forward, p is for previous,
   * which we need for updating.
   */
   double a0c, a1c, a2c,a3c,a4c,a5c,a6c,a7c;
   //static double a0p, a1p, a2p,a3p,a4p,a5p,a6p,a7p;
  /* unit mass density, was o0 */
   double rho;
  /* Mass matrix variables:  Eq. 2.57, p. 85,
   * Chapter 2, Shi 1988.
   */
   double S0,S1,S2,S3;
  /* x and y coordinates of centroid for each block */
   double x0, y0;
  /* TT is integrated moments matrix, Eq. 2.57, p. 85 */
   double TT[7][7] = {{0}};
  /* nBlocks := number of blocks in analysis */
   int nBlocks = gd->nBlocks;
  /* STATIC or DYNAMIC  defined in dda.h */
   int analysistype = ad->analysistype;
  /* Stiffness matrix */
   double ** K = ad->K;
  /* Delta t time step */
   double ts = ad->delta_t;
   int cts = ad->cts;
  /* (Delta t)^2, time step squared */
   double tss;

  /* Precompute the squared time step to save a few cycles */
   tss = ts*ts;

  /* Newmark coefficients.  See Bathe and Wilson 1976, 
   * p. 323 for details. 
   */
   a0c = 1.0/(alpha*tss);
   a1c = delta/(alpha*ts);
   a2c = 1.0/(alpha*ts);
   a3c = (1.0/(2*alpha)) - 1.0;
   a4c = (delta/alpha) - 1.0;
   a5c = (ts/2.0)*( (delta/alpha) - 2.0 );
   a6c = ts*(1.0-delta);
   a7c = delta*ts;

  /* For now, update all of the load vectors outside 
   * of the accumulation loops.  Later, these need to 
   * be put into the loops.  We have to use values computed
   * from the previous time step to update properly.
   */
  /* Update velocity for load vector.  Eventually want to move 
   * this into a inner loop.  FIXME: The behavior of the 
   * flag `k00' needs to be completely documented.
   */
   //print2DArray(__A0, __A0size1, __A0size2, fp.logfile, "__A0 from newmark");
   //print2DArray(__V0, __V0size1, __V0size2, fp.logfile, "__V0 from newmark");

   //if ( (currTimeStep > 1) )//  && (ad->k00 == 0) ) 
   if ( (cts > 1)  && (ad->m9 == 0) ) 
   {
      for (i=1; i<= nBlocks; i++)
      {      
         for (j=1; j<=  6; j++)
         {
            __A0[i][j+6] = __A0[i][j];
            __V0[i][j+6] = __V0[i][j];
            /** U[i][j+6] is "previous" displacement.  In DDA, the displacements 
             * are updated incrementally, with initial displacements handled as
             * constraints using penalty functions when necessary.  Part of the 
             * reason this function may not have ever worked is that the newmark
             * algorithm given in Bathe is _not_ an incremental update.  It builds
             * the displacement vector as a function of time.  To make this work,
             * it might be as little as eliminating the U[i][j+6] from the 
             * following update for new acceleration.
             */
            //__A0[i][j] = a0p*(U[i][j] - U[i][j+6]) - (a2p*__V0[i][j]) - (a3p*__A0[i][j+6]);
            __A0[i][j] = a0c*(U[i][j]) - (a2c*__V0[i][j+6]) - (a3c*__A0[i][j+6]);
            __V0[i][j] = __V0[i][j+6] + ( (a6c*__A0[i][j+6]) + (a7c*__A0[i][j]));
         }  
      }  
   }
  
  /*  inertia terms                                 */
  /* g5: only difference of statics and dynamics    */
  /* g6: inertia coefficient                        */
   for (i=1; i<= nBlocks; i++)
   {
      rho=matprops[i][0];

     /* (GHS: zero terms of mass matrix)  */
      for (j=1; j<= 6; j++)
      {
         for (l=1; l<= 6; l++)
         {
            TT[j][l]=0;
         }  /*  l  */
      }  /*  j  */
      
     /* (GHS: non-zero terms of mass matrix)  */
     /* Compute centroids.   These are also computed in 
      * the time step function. 
      */
      x0=moments[i][2]/moments[i][1];  // x0 := x centroid
      y0=moments[i][3]/moments[i][1];  // y0 := y centroid

     /* S1, S2, S3 result from integrals derived on 
      * pp. 83-84, Chapter 2, Shi 1988.
      */
      S0 = moments[i][1];  

      S1 = moments[i][4]-x0*moments[i][2];
      S2 = moments[i][5]-y0*moments[i][3];
      S3 = moments[i][6]-x0*moments[i][3];


      massmatrix(TT,1,S0,S1,S2,S3);



     /* (GHS: add mass matrix to a[][]) */
     /* k1 stores "permutation index",
      * n stores location of memory in 
      * stiffness matrix and force vector.
      */
      i1=k1[i];
      i2=n[i1][1]+n[i1][2]-1;
     
      for (j=1; j<= 6; j++)
      {
         for (l=1; l<= 6; l++)
         {
            ji=6*(j-1)+l;
            K[i2][ji] += a0c*rho*TT[j][l];//  + hht*(K[i2][ji]*__D[i][l]);
         }  /*  l  */
      }  /*  j  */

     /* (GHS: add velocity matrix to f[]) */
     /* No need to accumulate velocity terms for 
      * a static analysis.
      */
      if (analysistype == STATIC || ad->gravityflag == TRUE)
         continue;


      for (j=1; j<= 6; j++)
      {
         for (l=1; l<= 6; l++)
         {
           /* Velocity component of load vector is a
            * matrix-vector product.  BLAS call is DGEMV.
            */
            ji=6*(j-1)+l;
//            F[i1][j] += rho*TT[j][l]*(a2c*__V0[i][l+6] + a3c*__A0[i][l+6]) - hht*(K[i2][ji]*__D[i][l]);
            F[i1][j] += rho*TT[j][l]*(a2c*__V0[i][l] + a3c*__A0[i][l]);// - hht*(K[i2][ji]*__D[i][l]);
         }  /*  l  */
      }  /*  j  */

   }  /*  i end loop over each block */

  /* Save the current parameters to use for updating
   * next time through this loop.
   */
   //a0p=a0c;a1p=a1c;a2p=a2c;a3p=a3c;a4p=a4c;
   //a5p=a5c;a6p=a6c;a7p=a7c;

}  /* close newmark() */


/**
 * @warning Only the GHS integrator works.
 * @todo Move the GData->moments dereference into 
 * the relevant functions.
 */
void
timeintegration(Geometrydata * GData, Analysisdata * AData,
                double ** matprops, int * k1, /*double ** moments, */ int ** n,
                double ** U,
                MassMatrix massmatrix) {

   double ** F = AData->F;
   //clock_t start, stop;
   double ** moments = GData->moments;

   //start = clock();

  /* submatrix of inertia */
  /* df11() works by forward difference as derived in 
   * offprint of 1st Int DDA Forum, Berkeley, 1996,
   * pp. 23-27.
   */
   //AData->integrator = newmark;
   switch (AData->integrator)
   {
      case constant:
         df11(AData,GData->nBlocks,k1,F,matprops,moments,n,U,massmatrix);
         break;

     /* Newmark is from Bathe and Wilson, pp. 322-326, 1976. */
      case newmark:
         newmarkIntegration(GData, AData,k1,F,matprops,moments,n,U,massmatrix);
         break;
  
      default:
        /* Do something bad here */
         break;
   }
   
   //stop = clock();
   //DLog->integration_runtime += stop - start;

} 

/**************************************************/
/* step: compute next time interval          0006 */
/**************************************************/
/* tt                                             */
/* w6 is set in spri() and is not used until 
 * after first iteration.
 * FIXME: Have this function RETURN the time step
 * value.  Also, have a switch to control whether the
 * function computes the time step adaptively, or
 * returns the preset time step.
 */
void 
computeTimeStep(Geometrydata *bd, Analysisdata *ad)
{
   int i;
   int j;
   int point;  /* Loop over number of points.  */
               /* Scratch variables used for intermediate calculations
                * for computing the delta_t size.
                */
   double a1, a2, a3, a9;
   double max_load = 0;
   double max_stress;
   double max_velocity;  /* was a2  */
   int nLPoints = bd->nLPoints;  /* Number of load points I think. */
   int nFPoints = bd->nFPoints;  /* Number of fixed points(?).  */
   double ** points = bd->points;
  /* k5 is an array of indices into the timeDeps array. */
   int ** k5 = ad->tindex;  //ad->k5;
  /* w6 is related to the penalty parameter or spring
   * stiffness somehow.
   */
   double w6 = ad->w6;
   double ** globalTime = ad->globalTime;
  /* Block material properties matrix indexed by the 
   * number of material properties nb, starting from 1,
   * and mass density, unit weight, young's modulus,
   * poisson's ratio, 3 stress/3 strain/3 velocity 
   * components, indexed from 0.
   */
   double ** materialProps = ad->materialProps;  
   double ** timeDeps = ad->timeDeps;
   double avgArea; 
   //double w0 = ad->constants->w0;
   double domainscale = constants_get_w0(ad->constants);
   
   /* nb seems to be the number of block materials,
   * NOT the number of blocks.
   */
   int nb = ad->nBlockMats;
   //double g2 = ad->g2;
   double maxdisplacement = ad->maxdisplacement;
   int analysisType = ad->analysistype;
   
   avgArea = ad->avgArea[1];

  /* Gravity algorithm handles its own time step cutting. */
   if (ad->gravityflag == 1)
      return;

  /* if fixed time step */
   if (ad->autotimestepflag == 0)
      return;



  /* a1=points: force/mass   a2: velocity  */
   if (ad->cts == 1)
   {
     /* This kind of code really is somewhat incomprehensible.
      * avgArea is factored out and seems to work the same. 
      * materialProps[1][1] is the ``weight'' of the first block material.
      * materialProps[1][0] is the density of the first block material.
      */
      //a1 = (avgArea*materialProps[1][1])/(avgArea*materialProps[1][0]);
      a1 = materialProps[1][1] / materialProps[1][0];
      
     /* nb is the number of block materials.  Note that 
      * the matrix materialProps is initialized from 1, not zero. 
      * Find the max velocity of all the block materials.  (was a2)
      */
      max_velocity = 0;
      for (i=1; i<= nb; i++)
      {
         for (j=10;j<= 12; j++)
         {
            if (max_velocity < fabs(materialProps[i][j])) 
               max_velocity = fabs(materialProps[i][j]);
         }  /*  j  */
      }  /*  i  */
      
         /* Determine the maximum stress value.
      */
      max_stress = 0;
      for (i=1; i<= nb; i++)
      {
         for (j=4; j<=  6; j++)
         {
            if (max_stress < fabs(materialProps[i][j])) 
               max_stress = fabs(materialProps[i][j]);
         }  /*  j  */
      }  /*  i  */
      
         /* Reset the maximum stress value.
      */
      max_stress = 4*max_stress*sqrt(avgArea)/(avgArea*materialProps[1][0]);
      if (a1 < max_stress) 
         a1 = max_stress;
 
     /* FIXME: This is boogered up when using the new loadpoint structure. 
      * This algorithm should probably change to reflect the maximum load at
      * any particular time step, instead of the maximum load for all the  
      * time steps, or at least initialize using max, but modify according
      * to max load during a particular time step.
      */
     /* FIXME: This is a huge kludge.  This is induced by the file format 
      * changes in DDAML, etc.  All of the time dependent loading needs to 
      * be rewritten again.
      */
      if (k5 != NULL)
      {      
        /* point load   */
         max_load = 0;
         for (point=nFPoints+1; point <= nFPoints+nLPoints; point++)
         {
           /* FIXME:  Segfaults when loadpoints arrays are not initialized (duh).
            * k5 := ad->tindex is initialized in ???
            */
            for (j=k5[point][0]; j<=k5[point][1]; j++)
            {
               if (max_load < fabs(timeDeps[j][1]))     
                  max_load = fabs(timeDeps[j][1]);
               if (max_load < fabs(timeDeps[j][2])) 
                  max_load = fabs(timeDeps[j][2]);
            }  /*  j  */
         }  /*  i  */
      } 
     /* Reset the maximum load value.  (Why?)
      */
      max_load = max_load/(avgArea*materialProps[1][0]);
      if (a1 < max_load)
         a1= max_load;
      
      a3 = -maxdisplacement*domainscale;
      /*  Solve the quadratic formula.  Need some error 
      * checking in here to make sure we don't get a 
      * negative time step.
      */
      ad->delta_t = (-max_velocity+sqrt((max_velocity*max_velocity)-(2*a1*a3)))/a1;
      if (ad->delta_t < 0)
      {
         //fprintf(fp.errorfile, "Warning: delta_t value is less than zero.\n");
         //fprintf(fp.errorfile, "Current time step: %d\n", ad->currTimeStep);
         //fprintf(fp.errorfile,"delta_t: %f\n", ad->delta_t);
         ad->delta_t = 0.001;  /* arbitrary value > 0 */
      }
   }
   
   else  /* past the first time step.  */
   {
     /* point load  */  /* <--  ???  */
      a3 = 0;
      for (point=nFPoints+1; point<= nFPoints+nLPoints; point++)
      {
        /* find the maximum value of point load in either the 
         * x or y direction
         */
         if (a3 < fabs(points[point][4])) 
            a3=fabs(points[point][4]);
         if (a3 < fabs(points[point][5])) 
            a3=fabs(points[point][5]);
      }  /*  i  */
     /* normalize the maximum point load */ 
      a3 = a3/(avgArea*materialProps[1][0]);
      //a1 = w6*w0*(ad->g7)/(avgArea*materialProps[1][0]);
      a1 = w6*domainscale*(ad->JointNormalSpring)/(avgArea*materialProps[1][0]);
      if (a1 < a3) 
         a1=a3;
      
      a2 = globalTime[ad->cts-1][1]*maxdisplacement*domainscale/(ad->delta_t);

      if (analysisType == STATIC) /* analysisType == 0  is static type analysis, */
         a2=0;
      a3 = -maxdisplacement*domainscale;
      /* Solve a quadratic polynomial.  This is not the 
      * exact formula, it is missing a 2 in the denominator
      * and a 4 in the radicand.  Might not make much difference.
      * Reference for the procedure is Shi, 1995, Waterways
      * manifold method forum.
      */
      a9 = (-a2+sqrt(a2*a2-2*a1*a3))/a1;
      /* This next block is a total kludge.  Time doesn't
      * go backwards!  So its marked and handled, but the
      * underlying algorithm is hosed if it produces a 
      * negative time step for ANY (valid) input.
      */
      if (a9 < 0)
      {
        /* FIXME: Replace these with a print time error function */
         //fprintf(fp.errorfile, "Warning: delta_t value is less than zero.\n");
         //fprintf(fp.errorfile, "Current time step: %d\n", ad->currTimeStep);
         //fprintf(fp.errorfile,"delta_t: %f\n", a9);
         a9 = ad->delta_t;
      }
      
      /* Adjust the delta_t either up or down depending 
      * on the previous value of the time step.
      */
      if ( a9 <  1.3*(ad->delta_t)) 
         ad->delta_t=a9;
      else //if ( a9 >= 1.3*(ad->delta_t)) 
         ad->delta_t=1.3*(ad->delta_t);

   }  /* end past first time step */
   
}  /* Close computeTimeStep() */

