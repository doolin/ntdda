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

typedef struct _integrator Integrator;

struct _integrator {

   FILE * logfile;
   //double ** a;
};


/** This will get moved around a bit. */
static Integrator * integrator;


void
massmatrix_write(double SX0[7][7], int blocknum) {
  
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
         fprintf(ofp,"%30.16e ", SX0[i][j]);
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

   

/**
 * This needs to be changed to accomdate a general 
 * newmark scheme.  Some of them are not self-starting
 * like the DDA scheme.
 */
void
initIntegrationArrays(Geometrydata * gd, Analysisdata * ad) {

   int i, i1;
   double ** materialProps = ad->materialProps;
   int nBlocks = gd->nBlocks;
   int ** vindex = gd->vindex;

   integrator = integrator_new();

   __V0size1=nBlocks+1;
   __V0size2=13;
   __V0 = DoubMat2DGetMem(__V0size1, __V0size2);

   for (i=1; i<= nBlocks; i++) {

      i1 = vindex[i][0];
      /* If these were indexed to i+6, might be able to get 
       * rid of a conditional in df11.
       */
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


/** 
 * Update using the Bathe and Wilson procedure.
 * 
 * @todo Pass in the size of the arrays instead of hard
 *  coding for size 6. 
 */
void
newmark_update(double * u, double * v, double * a, int size,
               double a0, double a2, double a3, double a6, double a7) {

   int i;
   int s = size;

   for (i=1; i<=size; i++) {

      a[i+s] = a[i];
      v[i+s] = v[i];

      a[i] = a0*u[i] - a2*v[i+s] - a3*a[i+s];
      v[i] = v[i+s]  + a6*a[i+s] + a7*a[i];
   }  
}


/**************************************************/
/* df11: submatrix of inertia                     */
/**************************************************/
/* Implements a forward difference 
 * scheme outlined in the 1996 offprint from the 1st
 * Int. DDA Forum held in Berkeley, 1996.  The relevant
 * material is listed on pages 23-27.
 */
void df11(double ** K, int analysisType, double h, int cts, int m9, 
          int nBlocks, int *k1, double **F,
          double ** e0, double **moments, int **n, 
          double ** U, MassMatrix massmatrix,int lasttimestep) {

   int i, j, i1, i2, ji, l;

  /* unit mass density, was o0 */
   double rho;

  /* Second moments for mass matrix:  Eq. 2.57, p. 85,
   * Chapter 2, Shi 1988.
   */
   double S0,S1,S2,S3;

  /* x and y coordinates of centroid for each block */
   double x0, y0;

  /* SX0 is integrated moments matrix, Eq. 2.57, p. 85. (was TT). */
   double SX0[7][7] = {{0}};

  /* h^2, time step squared */
   double hh = h*h;

  /* Viscosity coefficient, GHS 1988 pp.86-88. */
   double mu;

  /* Convenience value for collecting density, viscosity etc. */
   double coef;

   /* Bathe and Wilson is non-standard on Newmark parameters.
    * Most others use beta <-> alpha and gamma <-> delta.
    */
   double alpha, delta;
   double a0,a1,a2,a3,a4,a5,a6,a7;

  /* The 0.5625 value comes from Wang et al. 1996. */
   alpha = .5;
   delta = 1.0;

  /* Newmark coefficients.  See Bathe and Wilson 1976, 
   * p. 323 for details. 
   */
   a0 = 1.0/(alpha*hh);
   a1 = delta/(alpha*h);
   a2 = 1.0/(alpha*h);
   a3 = (1.0/(2*alpha)) - 1.0;
   a4 = (delta/alpha) - 1.0;
   a5 = (h/2.0)*( (delta/alpha) - 2.0 );
   a6 = h*(1.0-delta);
   a7 = delta*h;


  /* Update velocity for load vector.  
   * 
   * @todo Figure out how to make the conditional 
   *  expression redundant, then move this block into 
   *  the next loop.  Probably should load up initial 
   *  velocities so that this code can be called at the 
   *  first time step.
   */
   if (cts > 1 && m9 == 0) {

      for (i=1; i<= nBlocks; i++) {
         
         newmark_update(U[i],__V0[i],__A0[i],6,a0,a2,a3,a6,a7);            
      }  
   }


   for (i=1; i<= nBlocks; i++) {

      rho = e0[i][0];  // density
      mu  = e0[i][8];  // damping

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
      //if (cts == lasttimestep) {
        massmatrix_write(SX0,i);  
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
      coef = 1; //2*( (mu/h) + (rho/hh) );
      
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
      if (analysisType == STATIC) {
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
            //F[i1][j] += SX0[j][l]*(a2*__V0[i][l]);
            F[i1][j] += SX0[j][l]*(a2*__V0[i][l] + a3*__A0[i][l]);
         } 
      }  
   }
}



void
timeintegration(Geometrydata * gd, Analysisdata * ad,
                double ** matprops, int * k1, int ** n,
                double ** U,
                MassMatrix massmatrix) {

   double ** F = ad->F;
   double ** moments = gd->moments;

  /* STATIC or DYNAMIC  defined in dda.h */
   int analysisType = ad->analysistype;
  /* Stiffness matrix */
   double ** K = ad->K;
  /* Delta t time step */
   double ts = ad->delta_t;
   int cts = ad->cts;
   int m9 = ad->m9;

   df11(K,analysisType,ts,cts,m9,gd->nBlocks,k1,F,matprops,
        moments,n,U,massmatrix,ad->nTimeSteps);
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

