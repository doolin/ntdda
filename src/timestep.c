/*
 *  Add some comments here!
 */



#include"analysis.h"
#include<math.h>
#include<assert.h>
#include "ddamemory.h"

extern FILEPOINTERS fp;
extern DATALOG * DLog;


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

static void df11(Geometrydata *, Analysisdata *, int *, double **,
            double **, double **, int **, double ** U);

/* Integration constants for Newmark */
static double a0c, a1c, a2c,a3c,a4c,a5c,a6c,a7c;

/* Handy variables for computing kinetic energy */
/* velocity terms */
static double d1, d2, d3, d4, d5, d6;
/* Diagonal terms of generalised inertia matrix. */
static double E11, E22, E33, E44, E55, E66;
/* Off-diagonal terms of generalized inertia matrix */
static double E34, E35, E36, E46, E56;

void
initIntegrationArrays(Geometrydata * gd, Analysisdata * ad)
{
   int i, i1;
   double ** materialProps = ad->materialProps;
   int nBlocks = gd->nBlocks;
   int ** vindex = gd->vindex;

   __V0size1=nBlocks+1;
   __V0size2=13;
   __V0 = DoubMat2DGetMem(__V0size1, __V0size2);

   for (i=1; i<= nBlocks; i++)
   {
      i1 = vindex[i][0];
      //__V0[i][1] = materialProps[i1][10]; /* v11 */
      //__V0[i][2] = materialProps[i1][11]; /* v22 */
      //__V0[i][3] = materialProps[i1][12]; /* vr */
      __V0[i][1] = materialProps[i1][10]; /* v11 */
      __V0[i][2] = materialProps[i1][11]; /* v22 */
      __V0[i][3] = materialProps[i1][12]; /* vr */
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

}  /* close initIntegrationArrays() */


void
freeIntegrationArrays()
{

   if (__A0)
      free2DMat((void **)__A0, __A0size1);

   if (__V0)
      free2DMat((void **)__V0, __V0size1);

}  /* close freeIntegrationArrays() */


void
timeintegration(Geometrydata * GData, Analysisdata * AData,
         double ** matprops, int * k1, /*double ** moments, */ int ** n,
         double ** U)
{
   double ** f = AData->F;
   clock_t start, stop;
   double ** moments = GData->moments;

   start = clock();

  /* submatrix of inertia */
  /* df11() works by forward difference as derived in 
   * offprint of 1st Int DDA Forum, Berkeley, 1996,
   * pp. 23-27.
   */
   switch (AData->integrator)
   {
      case constant:
         df11(GData, AData,k1,f,matprops,moments,n,U);
         break;

     /* Newmark is from Bathe and Wilson, pp. 322-326, 1976. */
      case newmark:
         newmarkIntegration(GData, AData,k1,f,matprops,moments,n, U);
         break;
  
      default:
        /* Do something bad here */
         break;
   }
   
   stop = clock();
   DLog->integration_runtime += stop - start;

}  /* close timeintegration() */



/**************************************************/
/* df11: submatrix of inertia                     */
/**************************************************/
/* df11() appears to implement a forward difference 
 * scheme outlined in the 1996 offprint from the 1st
 * Int. DDA Forum held in Berkeley, 1996.  The relevant
 * material is listed on pages 23-27.
 */
void df11(Geometrydata *gd, Analysisdata *ad, int *k1, double **F,
          double ** e0, double **moments, int **n, 
          double ** U)
{
   int i, j, i1, i2, ji, l;
  /* g6 g5 coefficients of mass and velocity matrix.
   * See Eqs. 2.49 & 2.52, p. 81, Shi 1988.
   */
   double g5;
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
  /* TT is integrated moments matrix, Eq. 2.57, p. 85 */
   double TT[7][7] = {{0}};
  /* nBlocks := number of blocks in analysis */
   int nBlocks = gd->nBlocks;
  /* STATIC or DYNAMIC  defined in dda.h */
   int analysisType = ad->analysistype;
  /* Stiffness matrix */
   double ** K = ad->K;
  /* We have to take care of Initial Conditions, too */
   int currTimeStep = ad->currTimeStep;
  /* Delta t time step */
   double ts = ad->delta_t;
  /* previous time step needed for updating, 
   * set in df25().  FIXME: Find a way to set this
   * value in this function instead of df25().
   */
   //double pts = ad->pts;  
   static double pts;
  /* (Delta t)^2, time step squared */
   double tss;
  /* need to index the kinetic energy */
   int tstep = ad->currTimeStep;
  /* Viscosity coefficient, GHS 1988 pp.86-88. */
   double mu;  //ad->mu;
  /* Convenience value for collecting density, viscosity etc. */
   double coef;

  /* Precompute the squared time step  to save a few cycles */
   tss = ts*ts;

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
   if (currTimeStep > 1 && ad->m9 == 0) 
   {
      for (i=1; i<= nBlocks; i++)
      {
         for (j=1; j<=  6; j++)
         {
           /* FIXME: Save the previous values to use for 
            * computing kinetic energy.
            */
           /* FIXME: Where are the values of U set?
            */
            __V0[i][j] = (2*U[i][j]/(pts))-__V0[i][j];
         }  
        /* TCK 1996 p. 324 velocity rotation correction. */
        /* FIXME: This does not appear to work at all. */
         if (0) 
         {
            double r,c,s;
            double du0 = __V0[i][1];
            double dv0 = __V0[i][2];
            double dex = __V0[i][4];  // dot e_x
            double dey = __V0[i][5];
            double dgxy = __V0[i][6];
            r = U[i][3];
            c = cos(r);
            s = sin(r);
           /* Displacements */       
            __V0[i][1] = c*du0 - s*dv0;
            __V0[i][2] = s*du0 + c*dv0;
           /* Strains */
            __V0[i][4] = c*c*dex - 2*c*s*dgxy + s*s*dey;
            __V0[i][5] = c*s*(dex+dey) + (c*c-s*s)*dgxy;
            __V0[i][6] = s*s*dex + 2*c*s*dgxy + c*c*dey;
         }            
      }  
   }
   //fprintf(fp.logfile,"%f\n",ts);


  /* g6 g5 coefficients of mass and velocity matrix */
   if ( (analysisType == STATIC) || (ad->gravityflag == TRUE) )  
      g5 = 0;  // redundant
   else 
      g5=2.0/(ts); 

   //g6=2.0/(tss);


  /*  inertia terms                                 */
  /* g5: only difference of statics and dynamics    */
  /* g6: inertia coefficient                        */
   for (i=1; i<= nBlocks; i++)
   {
      rho = e0[i][0];  // density
     /* Need to compute current density */
      //rho = (e0[i][9]*e0[i][0])/((1+e0[i][7])*moments[i][1]);
      mu = e0[i][8];   // damping

     /* (GHS: zero terms of mass matrix)  */
     /* When I get the guts, I will zero this out with memset: */
      // memset(TT,0x0,sizeof(TT));
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
      //x0=moments[i][2]/e0[i][9]; //moments[i][1];  // x0 := x centroid
      //y0=moments[i][3]/e0[i][9]; //moments[i][1];  // y0 := y centroid

     /* S1, S2, S3 result from integrals derived on 
      * pp. 83-84, Chapter 2, Shi 1988.
      */
      S0=moments[i][1];  // block area/volume
      //S0=e0[i][9]; //moments[i][1];  // block area/volume
      S1=moments[i][4]-x0*moments[i][2];
      S2=moments[i][5]-y0*moments[i][3];
      S3=moments[i][6]-x0*moments[i][3];

     /* Build the mass matrix using Eq. 2.57, p. 85,
      * Chapter 2, Shi 1988.
      */
      TT[1][1]=S0;
      TT[2][2]=S0;
      TT[3][3]=S1+S2;
      TT[3][4]= -S3;
      TT[4][3]=TT[3][4];
      TT[3][5]=S3;
      TT[5][3]=TT[3][5];
      TT[3][6]=(S1-S2)/2;
      TT[6][3]=TT[3][6];
      TT[4][4]=S1;
      TT[4][6]=S3/2;
      TT[6][4]=TT[4][6];
      TT[5][5]=S2;
      TT[5][6]=S3/2;
      TT[6][5]=TT[5][6];
      TT[6][6]=(S1+S2)/4;

     /* Compute the kinetic energy.
      * FIXME: Rewrite this as a function, then
      * turn it into a macro 
      */
      d1 = __V0[i][1];
      d2 = __V0[i][2];
      d3 = __V0[i][3];
      d4 = __V0[i][4];
      d5 = __V0[i][5];
      d6 = __V0[i][6];
      E33 = (S1+S2);
      E44 = S1;
      E55 = S2;
      E66 = E33/4.0;
      E34 = -S3;
      E35 = S3;
      E36 = (S1-S2)/2.0;
      E46 = S3/2.0;
      E56 = S3/2.0;
     /* FIXME: stash a local var in here to cut down on all this
      * dereferencing.
      * FIXME: Add the viscosity potential to this.
      */
      if (ad->k00 == 0) 
      {
         DLog->energy[tstep].KEcentroid += rho*S0*( d1*d1 + d2*d2);
         DLog->energy[tstep].KEdeform += rho*((E33*d3*d3) + (E44*d4*d4) + (E55*d5*d5)
                                   + (E66*d6*d6) + (2*E34*d3*d4) + (2*E35*d3*d5)
                                   + (2*E36*d3*d6) + (2*E46*d4*d6) + (2*E56*d5*d6));
      }

     /* (GHS: add mass matrix to a[][]) */
     /* k1 stores "permutation index",
      * n stores location of memory in 
      * stiffness matrix and force vector.
      */
      i1=k1[i];
      i2=n[i1][1]+n[i1][2]-1;
     
     /* KLUDGE: The default value of mu is zero (mu=0). If it is 
      * nonzero, it will be set in the ddaml input file.
      */
      coef = 2*( (mu/ts) + (rho/tss) );
      for (j=1; j<= 6; j++)
      {
         for (l=1; l<= 6; l++)
         {
            ji=6*(j-1)+l;
            /* Ok, so here we want to extend to add the viscosity,
             * which means we may as well move this out of the inner
             * loop.
             */
            //K[i2][ji] += g6*rho*TT[j][l];
            K[i2][ji] += coef*TT[j][l];

         }  /*  l  */
      }  /*  j  */

     /* (GHS: add velocity matrix to f[]) */
     /* FIXME: If we are running a static analysis,
      * is there any need to update accumulate here?
      */
      if (analysisType == STATIC || ad->gravityflag == TRUE)
         continue;


      for (j=1; j<= 6; j++)
      {
         for (l=1; l<= 6; l++)
         {      
           /* Velocity component of load vector is a
            * matrix-vector product.  BLAS call is DGEMV.
            * FIXME: Move g5 out of this loop.
            */
            F[i1][j] += g5*rho*TT[j][l]*__V0[i][l];
         }  /*  l  */
      }  /*  j  */

   }  /*  i end loop over each block */


  /* Here is where we want to set the previous time 
   * step variable, but only for each time step in 
   * time, not each change in time step value.  This is 
   * because Delta t may change during a time step.
   */
   pts = ad->delta_t;

  /* Total kinetic energy
   */   
   if (ad->m9 == 0) 
   {
      DLog->energy[tstep].ke = 0.5*(DLog->energy[tstep].KEcentroid + DLog->energy[tstep].KEdeform);
   }


}  /*  Close df11().  */


void
newmarkIntegration(Geometrydata *gd, Analysisdata *ad, int *k1, double **F,
          double **matprops, double **moments, int **n, double ** U)
{
  /* Loop counters, etc. FIXME: explain i1. */
   int i, j, i1, i2, ji, l;
  /* Newmark parameters.  See Bathe and Wilson, p. 323. */
  /* displacement weight */
   double alpha = .58;
  /* velocity weight */
   double delta = .9;
  /* Hilber-Hughes-Taylor alpha */
   double hht = 0.11111;
  /* Newmark coefficients.  See Bathe and Wilson 1976, 
   * pp. 322-326 for details.  c is current parameter,
   * that is, calculated going forward, p is for previous,
   * which we need for updating.
   */
   double a0c, a1c, a2c,a3c,a4c,a5c,a6c,a7c;
   static double a0p, a1p, a2p,a3p,a4p,a5p,a6p,a7p;
  /* unit mass density, was o0 */
   double rho;
  /* Mass matrix variables:  Eq. 2.57, p. 85,
   * Chapter 2, Shi 1988.
   */
   double S1,S2,S3;
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
   int currTimeStep = ad->currTimeStep;
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
   
   if ( (currTimeStep > 1)  && (ad->k00 == 0) ) 
   {
      for (i=1; i<= nBlocks; i++)
      {      
         for (j=1; j<=  6; j++)
         {
            __A0[i][j+6] = __A0[i][j];
            __A0[i][j] = a0p*(U[i][j] - U[i][j+6]) - (a2p*__V0[i][j]) - (a3p*__A0[i][j+6]);
            __V0[i][j] = __V0[i][j] + ((a6p*__A0[i][j+6]) + (a7p*__A0[i][j]));
         }  
      }  
      fprintf(fp.logfile,"Updated V and A (newmark)\n");
   }
   fprintf(fp.logfile," newmark %d  %f\n",currTimeStep,ts);
   
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
      S1=moments[i][4]-x0*moments[i][2];
      S2=moments[i][5]-y0*moments[i][3];
      S3=moments[i][6]-x0*moments[i][3];

     /* Build the mass matrix using Eq. 2.57, p. 85,
      * Chapter 2, Shi 1988.
      */
      TT[1][1]=moments[i][1];  // area S0
      TT[2][2]=moments[i][1];  // area S0
      TT[3][3]=S1+S2;
      TT[4][4]=S1;
      TT[5][5]=S2;
      TT[6][6]=(S1+S2)/4;

      TT[3][4]= -S3;
      TT[4][3]=TT[3][4];
      TT[3][5]=S3;
      TT[5][3]=TT[3][5];
      TT[3][6]=(S1-S2)/2;
      TT[6][3]=TT[3][6];
      TT[4][6]=S3/2;
      TT[6][4]=TT[4][6];
      TT[5][6]=S3/2;
      TT[6][5]=TT[5][6];

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
            K[i2][ji] += a0c*rho*TT[j][l]  + hht*(K[i2][ji]*U[j][l]);
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
            F[i1][j] += rho*TT[j][l]*(a0c*U[i][l+6] + a2c*__V0[i][l+6] - a3c*__A0[i][l+6])
                          - hht*(K[i2][ji]*U[j][l]);
         }  /*  l  */
      }  /*  j  */

   }  /*  i end loop over each block */

  /* Save the current parameters to use for updating
   * next time through this loop.
   */
   a0p=a0c;a1p=a1c;a2p=a2c;a3p=a3c;a4p=a4c;
   a5p=a5c;a6p=a6c;a7p=a7c;

}  /* close newmark() */


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
   double max_load, max_stress;
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
   double domainscale = ad->constants->w0;
   
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
   if (ad->currTimeStep == 1)
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
      
      a2 = globalTime[ad->currTimeStep-1][1]*maxdisplacement*domainscale/(ad->delta_t);

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

/*******************************************/
/*******************************************/
/*******************************************/
