
/*
 *  Add some comments here!
 */

#include <math.h>
#include <assert.h>

#include "analysis.h"
#include "ddamemory.h"
#include "ddadlist.h"
#include "datalog.h"
#include "bolt.h"


extern Datalog * DLog;

extern FILEPOINTERS fp;

/* Helper var for examining variables */
static char mess[80];


static void seismicload(Geometrydata *gd, Analysisdata *ad, int *k1, double **F, double **blockArea, double ** matprops);

static void df09(Geometrydata *, Analysisdata *);
static void df10(Geometrydata *, Analysisdata *, int **, double **);

static void df12(Geometrydata *, Analysisdata *, int *, double **, double **, int **);
static void df13(int numblocks, double ** K, const int *, double **, double **, int **, int planestrainflag);
static void df14(int numblocks, const int *, double **, const double **, const double **);
static void df15(Geometrydata *, Analysisdata *, int *, double **, double **);
static void df16(Geometrydata *, int *, double **, double **, double **);


/* This was moved from ddanalysis to shorten the size
 * of the analysis loop.
 */
void
assemble(Geometrydata * GData, Analysisdata * AData,
         int ** locks, double ** matprops,
         int * k1, int * kk, int ** n, double ** U)
{

   double ** f = AData->F;
   //clock_t start, stop;
   double ** moments = GData->moments;

   //DLog->assemble_start = clock();
   //start = clock();

      df09(GData, AData);
     
     /* Initialize submatrices back to zero, set
      * some other parameter related to the global
      * time step.
      * FIXME: see if locks can be moved out of here.
      */
	   df10(GData, AData, locks,f);
     /* submatrix of inertia
      */
	   //df11(GData, AData,k1,f,matprops,v0,moments,n);
     /* WARNING!!!!  This comes in from timeintegration!!! 
      * Delete after June 15 2000. */
	 	//df11(GData, AData,k1,f,matprops,moments,n,U);


      //printKForIthBlock(AData,1,k1,n);

	  /* submatrix of fixed points.  T should probably be 
      * either local or static extern scratch var.
      */
      df12(GData,AData,k1,f,moments,n);


     /* submatrix of stiffness */
	   df13(GData->nBlocks,AData->K,k1,matprops,moments,n,AData->planestrainflag);

     /* submatrix of initial stress  */
	   df14(GData->nBlocks,k1,f,matprops,moments);

     /* submatrix of point loading  */
	   df15(GData,AData, k1,f,moments);

     /* seismic loading */
      if (AData->timehistory != NULL) {

         seismicload(GData, AData, k1, f, moments, matprops);
      }

     /* submatrix of volume force */
	   df16(GData,k1,f,matprops,moments);
       
      if (GData->nBolts > 0) {

         //rockbolts(GData->rockbolts, GData->nBolts, AData->K, k1, kk, n, moments,f);
         bolt_stiffness_arrays(GData->rockbolts, GData->nBolts, AData->K, k1, kk, n, moments,f,computeDisplacement);
      }

      /* start open-close iteration  m9:step iterations */
      //AData->extraIteration = 0;

      //printForces(GData, f, k1);
   
      //stop = clock();
      //DLog->assemble_runtime +=  stop - start; //(clock() - DLog->assemble_start)/((double)CLOCKS_PER_SEC);
   
   
   //printKForIthBlock(AData, 2, 1, kk, n, "load matrix");

}  /* close assemble() */




/**************************************************/
/* df09: time interpolation                       */
/**************************************************/
/* This appears to be linear lagrange intepolation
 * for time dependent loading points.  Note that fixed 
 * points are considered time dependent, except that from 
 * time 0 to 100000, 0 load is applied.  Behavior for
 * t > 100000 appears to be unspecified...
 */
void df09(Geometrydata *gd, Analysisdata *ad)
{
   int i;
   int j;
   int n;
   double a1;
   double current_time;
   int nLPoints = gd->nLPoints;
   int nFPoints = gd->nFPoints;
   double ** points = gd->points;
   double ** globalTime = ad->globalTime;
   double ** timeDeps = ad->timeDeps;
  /* k5 is index into timeDeps array */
   //int ** k5 = ad->k5;
  /* Note that k5/tindex increments from zero in the 
   * second slot, 1 from the leading slot.
   */
   int ** tindex = ad->tindex;
   double **lpoints;
   const int TIME = 0;
   int nfp = gd->nFPoints;
   int numloadpoints = ad->nLPoints;
   
   double dt;

  /* a3 is the elapsed time at the end of the current step.
   * This allows to find the force to apply in the current
   * configuration (applied at a point in the reference 
   * configuration).
   */
  /* FIXME: Note where globaltime is updated.
   */
   current_time = globalTime[ad->cts-1][0]+(ad->delta_t);
      

   for (i = 0; i < numloadpoints; i++) {

     	n = ad->loadpoints[i].loadpointsize1;
     	lpoints = ad->loadpoints[i].vals;
     	for (j = 0; j < n-2; j++) {
         if ( (lpoints[j][TIME] <= current_time) && 
              (current_time  <= lpoints[j+1][TIME]) ) {
            break;
          }   
	    }

      dt = (lpoints[j+1][TIME] - lpoints[j][TIME]);
      /*
      if (dt <= 0) {
         ad->display_error("Loadpoint error: Adjacent time step values must be different.\n");
      }
      */
		a1 = (current_time - lpoints[j][TIME]) / dt;
      points[i+nfp+1][4] = lpoints[j][1] + a1*(lpoints[j+1][1] - lpoints[j][1]);
      points[i+nfp+1][5] = lpoints[j][2] + a1*(lpoints[j+1][2] - lpoints[j][2]);
   }

}  



/**************************************************/
/* df10: initiate coefficient and load matrix     */
/**************************************************/
/*  I think this means initialize rather than initiate.
 *  Either way this routine needs to be abstracted into 
 *  "set_matrix_to_zero" function, and the time step 
 *  stuff put somewhere else.
 */
void df10(Geometrydata *bd, Analysisdata *ad, int **locks, 
          double **F)
{
   int i;
   int nBlocks = bd->nBlocks;
   int nContacts = bd->nContacts;  /* Not changed in df10(). */
   /* int n3 = bd->n3; */ /* Not changed in df10(). */
   /* int n3 = ad->n3; */ /* Not changed in df10(). */
   //int n3 = ad->nCurrentContacts;  /* Not changed in df10(). */

   double ** globalTime = ad->globalTime;
   double ** K = ad->K;

  /* initial a[][]=0 f[][]=0  */
  /* These two calls get moved out of here eventually
   * and the matrices initialized directly before being
   * used in the analysis.  This function needs to be 
   * turned into a macro.  FIXME: These need to get turned in 
   * 1D arrays, then a simple call to memset.
   */
   setMatrixToZero(K, ad->ksize1, ad->ksize2);
   setMatrixToZero(F, nBlocks+1, 7);

  /* (GHS: for reduce time interval     m0[][1]=0 in df07) */
  /* This will have to stay here as the whole matrix is
   * not initialized, only one index. 
   * FIXME: GHS indicates this may be needed only if the 
   * time interval is reduced.
   */
   for (i=1; i<= nContacts; i++)
   {
      locks[i][1]=0;  // was m0
   }  /*  i */
 
   globalTime[ad->cts][0] = globalTime[ad->cts-1][0] + ad->delta_t;
   globalTime[ad->cts][2] = ad->delta_t;

}  /*  Close df10  */




/**************************************************/
/* df12: submatrix of fixed points                */
/**************************************************/
void df12(Geometrydata *gd, Analysisdata *ad, int *k1, 
          double **F, double **blockArea, int **n)
{
   int i;
  /* i0 is the block number that the point is
   * associated with.
   */
   int blocknumber;  // was i0
   //int i1;  // redundant block number
  /* Memory locations */
   int i2, i3;
   int j;
   int j1;
   int ell;
   int nFPoints = gd->nFPoints;
   double ** points = gd->points;
   double e[7][7];
   double s[31];
   double ** K = ad->K;
  /* After the first time step, c seems to carry point displacement
   * from the previous time step.  This is used to compute a 
   * ``restoring force'' opposite to the direction that the 
   * fixed point moved during the last time step.
   */
   double ** c = ad->c;
   double x;
   double y;
   double T[7][7];

  /* g4 is a spring value set at the beginning from the 
   * initial contact spring stiffness.  The initial contact
   * spring stiffness may be changed adaptively during run
   * time, so we can't use that.  g4 has been changed to 
   * FPointSpring.  See GHS 1988, Chapter 2, p. 93.
   * FIXME:  Initialize FPointSpring in a more appropriate
   * place.
   */
   double p = (100.0)*ad->FPointSpring;




  /* i0: block number  */
   for (i=1; i<= nFPoints; i++)
   {
     /* i0 is the block number that the point is 
      * associated with.  Notice that the fixed points
      * are read in as a line so that the 3,4 slots
      * are an endpoint.  This is overwritten in the 
      * geometry pre-processor.
      */
      blocknumber = (int)points[i][3];
      x=points[i][1];
      y=points[i][2];

      computeDisplacement(blockArea,T,x,y,blocknumber);
      
      /* (GHS: 6*6 submatrix of fixed point for a[][]) */
      for (j=1; j<= 6; j++)
      {
         for (ell=1; ell<= 6; ell++)
         {
            e[j][ell] = T[1][j]*T[1][ell] + T[2][j]*T[2][ell];
         }  /*  l  */
      }  /*  j  */

	    /* K[][] here is only for fixed points: Eq. 2.78,
      * p. 94, Chapter 2, Shi 1988.
      */  
      //i1=(int)points[i][3];
		    //i2=k1[i1];
      i2 = k1[blocknumber];
		i3 = n[i2][1]+n[i2][2]-1;

      for (j=1; j<= 6; j++)
      {
         for (ell=1; ell<= 6; ell++)
         {
            j1=6*(j-1) + ell;
            K[i3][j1] += p*e[j][ell];
         }  /*  l  */
      }  /*  j  */
      
	  
     /* (GHS: 6*1 submatrix of fixed displacements for f[]) */
     /* points[][4] and points[][5] are most likely (x,y)
      * load values as set in df09() from the timeDeps array.
      * The matrix c stores displacement values of the fixed
      * points.  The penalty method does not eliminate displacement
      * of fixed points, so a ``restoring force'' oriented 
      * opposite to the displacement is added to the force vector
      * for the next (current) time step.  NOTE: For this to work 
      * correctly, the fixed points must at the beginning of the 
      * points array.
      */
      for (j=1; j<= 6; j++)
      {
		       //s[j]=T[1][j]*(points[i][4]+c[i][1])+T[2][j]*(points[i][5]+c[i][2]);
         //fprintf(fp.logfile,"s[%d] = %.30f, Fpoint: %d:\n",j,s[j],i);
         s[j] = (T[1][j]*c[i][1]) + (T[2][j]*c[i][2]);
         //fprintf(fp.logfile,"Fixed point %d,  x: %f, y: %f\n",i,points[i][4],points[i][5]);

      }  /*  j  */
      
      
     /* (GHS: for constrained or fixed point)  */
     /* This next block of code is not derived for 
      * fixed points (and constrained derivation 
      * was noted as unclear).  The idea here is that 
      * motion of fixed points is compensated by pushing
      * in a direction opposite of that which the fixed
      * point displaced.  The fixed point displacement 
      * is computed and stored in df25().
      */
      for (j=1; j<= 6; j++)
      {
         F[i2][j] += p*s[j];
         //fprintf(fp.logfile,"F[i2][%d] = %20f, Fpoint: %d:\n",j,F[i2][j],i);
      }  /*  j  */

   }  /*  i  */

}  /* Close  df12().  */



/**************************************************/
/* df13: submatrix of stiffness                    */
/**************************************************/
void 
df13(int numblocks, double ** K, const int *k1, double **e0, double **blockArea,
          int **n, int planestrainflag)
{
   int i, i2, i3;
   int j, j1;
   int l;
   int block;

   double S0;  // block volume, was a1
   double a2;
   double E,nu;

   double e[7][7]; 

 
   for (i=1; i<= 6; i++) {
      for (j=1; j<= 6; j++) {
         e[i][j]=0;
      }
   } 
   
   for (block=1; block<=numblocks; block++) {

      E  = e0[block][2];
      nu = e0[block][3];
      S0 = blockArea[block][1];
       
      if (planestrainflag == 1) {

         a2 = S0*E/(1-nu);
         e[4][4] = a2*(1-nu)/(1-2*nu);
         e[5][5] = a2*(1-nu)/(1-2*nu);
         e[4][5] = a2*(nu)/(1-2*nu);
         e[5][4] = a2*(nu)/(1-2*nu);
         e[6][6] = a2/2;

      } else {  /* PLANESTRESS */

         a2 = S0*E/(1-(nu*nu));
         e[4][4] = a2;
         e[4][5] = a2*nu;
         e[5][4] = a2*nu;
         e[5][5] = a2;
         e[6][6] = a2*(1-nu)/2;
      }

     /* i2, i3 is location of Kii in global matrix. */
      i2=k1[block];
      i3=n[i2][1]+n[i2][2]-1;

     /* This should only have to loop over 4,5,6 */
      for (j=1; j<= 6; j++) {

         for (l=1; l<= 6; l++) {

            j1 = 6*(j-1)+l;  /* set index to global matrix */
            K[i3][j1] += e[j][l];  /* add elastic coefficients */
         }  
      }  
   }  
}  

      
/**************************************************/
/* df14: submatrix of initial stress              */
/**************************************************/
void df14(int nBlocks, const int *k1, double **F, const double **e0,
          const double **moments)
{
   int i, i1;
   double S0;  // block area (per unit thickness, was a1)
   //int nBlocks = bd->nBlocks;

   for (i=1; i<=nBlocks; i++)
   {
      S0 = moments[i][1];
      i1 = k1[i];

     /* Compute virtual work from last iteration. */
     /* Add the stresses. e0 is updated in df25() */
      F[i1][4] += -S0*e0[i][4];
      F[i1][5] += -S0*e0[i][5];
      F[i1][6] += -S0*e0[i][6];
   }  
}  


/**************************************************/
/* df15: submatrix of point loading               */
/**************************************************/
void df15(Geometrydata *gd, Analysisdata *ad, int *k1, double **F, 
          double **blockArea)
{
   int i, i0, i1, i2;
   int j;
   int nLPoints = gd->nLPoints;
   int nFPoints = gd->nFPoints;
   double ** points = gd->points;
   double s[7];
   double ** a = ad->K;
   double x, y;
   double T[7][7];


   for (i=nFPoints+1; i<= nFPoints+nLPoints; i++) {

     /* i0 is probably the number of the block 
      * associated with a particular load point.
      */
      i0=(int)points[i][3];
     /* x and y are the coordinates where the point 
      * load is applied.
      */
      x=points[i][1];
      y=points[i][2];

      computeDisplacement(blockArea,T,x,y,i0);
   
     /* points[i][4] points[i][5] 
      * is time dependent loads initialized 
      * df09()  
      */
      for (j=1; j<= 6; j++) {
         s[j] = T[1][j]*points[i][4] + T[2][j]*points[i][5];
      }  

      if (i%2) {
         fprintf(fp.logfile,"load point %d,  x: %f, y: %f\n",i,points[i][4],points[i][5]);
      }

      i1=(int)points[i][3];
      i2=k1[i1];
      
     /* BUG: There is a bug in here that results from 
      * having a point load located outside of the
      * rock mass.
      */
      for (j=1; j<= 6; j++)
      {
         F[i2][j] += s[j];
      }  /*  j  */

     /* This correction is presented on p. 37 of Maclaughlin 1997.
      * For point loads, just add a single term to the stiffness
      * matrix.
      */
     /* The following is part of MMM rigid body 2d order 
      * rotation code.
      */
     /*
      if (ad->rotationflag == 1)
      {
         a[i2][15] += (points[i][4]*T[2][3]) - (points[i][5]*T[1][3]);
      }
      */
   }  /*  i  */

}  /*  Close df15() .*/


void
seismicload(Geometrydata *gd, Analysisdata *ad, int *k1, double **F, 
          double ** moments, double ** matprops)
{

   DList * ptr;
   DDAPoint * ptmp;
   int j;
   int i2; // memory location for block in forcing vector
   int blocknumber;
   double x,y;
   double force, mass, accel;
   double T[7][7] = {0.0};
   double s[7] = {0.0};

   if (ad->cts >= th_get_number_of_datapoints(ad->timehistory))
      return;

   dlist_traverse(ptr, gd->seispoints) {

      ptmp = (DDAPoint *)ptr->val;
      blocknumber = ptmp->blocknum;
      x = ptmp->x;
      y = ptmp->y;
      computeDisplacement(moments,T,x,y,blocknumber);
      accel = th_get_data_value(ad->timehistory, ad->cts);
      mass = moments[blocknumber][1]*matprops[blocknumber][1];
      force = accel*mass;
      for (j=1; j<= 6; j++)
      {
         s[j] = T[1][j]*force + T[2][j]*0.0;
      }  

      i2 = k1[blocknumber];

     /* BUG: There is a possible bug in here that results from 
      * having a point load located outside of the
      * rock mass.
      */
      for (j=1; j<= 6; j++)
      {
         F[i2][j] += s[j];
      }  

   }

}  /* close seismicload() */

/**************************************************/
/* df16: submatrix of volume force                */
/**************************************************/
/*  k1 is ???
 *  F is the "force" vector.
 *  e0 is material props: ma we e0 u0 c11 c22 c12 t-weight
 */
void df16(Geometrydata *gd, int *k1, double **F, double **e0,
          double **moments)
{
   int block;
   int i2;
   double o1;
   double o2;
   int nBlocks = gd->nBlocks;

   for (block=1; block<= nBlocks; block++)
   {
      i2 = k1[block];
      o1 = 0;
     /* e0[1] needs to be unit weight. 
      * FIXME: unit weight is used here and 
      * density is used in inertia matrix.
      * Problem is that gravity may not be consistent
      * between the two.  The thing to do here is to
      * use the density and the gravity acceleration 
      * value given in the input file to turn gravity
      * loading on and off.
      * FIXME: Need to account for changing density here as 
      * well, either by thickness or by density.
      */
      o2 = -e0[block][1];
      F[i2][1] += o1*moments[block][1];
      F[i2][2] += o2*moments[block][1];
   }  

} 



/* Attempt to load code for viscosity. The algorithm
 * is taken from GHS, page ??? 1988.
 */
int
viscosity(Geometrydata * bd, Analysisdata * ad, double ** hb, 
          double ** blockArea, double ** F)
{
   return 0;
}  
