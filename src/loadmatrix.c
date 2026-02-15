
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
#include "timehistory.h"

#include "stress.h"



/* Helper var for examining variables */
static char mess[80];

/** Move this into its own file or the timehistory file. */
static void seismicload(DList * seispoints,TimeHistory * timehistory, int timestep,
                        int *k1, double **F, double ** moments,
                        double ** matprops, TransMap transmap);









/**************************************************/
/* df10: initiate coefficient and load matrix     */
/**************************************************/
/*  I think this means initialize rather than initiate.
 *  Either way this routine needs to be abstracted into
 *  "set_matrix_to_zero" function, and the time step
 *  stuff put somewhere else.
 */
void df10(Geometrydata *bd, Analysisdata *ad, int **locks,
          double **F) {
   int i;
   int nBlocks = bd->nBlocks;
   int nContacts = bd->nContacts;  /* Not changed in df10(). */

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

}




/**************************************************/
/* df12: submatrix of fixed points                */
/**************************************************/
void df12(Geometrydata *gd, Analysisdata *ad, int *k1,
          double **F, double **blockArea, int **n,
          TransMap transmap)
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
   for (i=1; i<= nFPoints; i++) {


     /* i0 is the block number that the point is
      * associated with.  Notice that the fixed points
      * are read in as a line so that the 3,4 slots
      * are an endpoint.  This is overwritten in the
      * geometry pre-processor.
      */
      blocknumber = (int)points[i][3];
      x=points[i][1];
      y=points[i][2];

      transmap(blockArea[blocknumber],T,x,y);

      /* (GHS: 6*6 submatrix of fixed point for a[][]) */
      for (j=1; j<= 6; j++) {
         for (ell=1; ell<= 6; ell++) {

            e[j][ell] = p*(T[1][j]*T[1][ell] + T[2][j]*T[2][ell]);
         }
      }

	    /* K[][] here is only for fixed points: Eq. 2.78,
      * p. 94, Chapter 2, Shi 1988.
      */
      //i1=(int)points[i][3];
		//i2=k1[i1];
      i2 = k1[blocknumber];
		i3 = n[i2][1]+n[i2][2]-1;

      for (j=1; j<= 6; j++) {
         for (ell=1; ell<= 6; ell++) {

            j1=6*(j-1) + ell;
            K[i3][j1] += e[j][ell];
         }
      }


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
      for (j=1; j<= 6; j++) {

         s[j] = (T[1][j]*c[i][1]) + (T[2][j]*c[i][2]);
      }


     /* (GHS: for constrained or fixed point)  */
     /* This next block of code is not derived for
      * fixed points (and constrained derivation
      * was noted as unclear).  The idea here is that
      * motion of fixed points is compensated by pushing
      * in a direction opposite of that which the fixed
      * point displaced.  The fixed point displacement
      * is computed and stored in df25().
      */
      for (j=1; j<= 6; j++) {

         F[i2][j] += p*s[j];
      }

   }

}  /* Close  df12().  */





/**************************************************/
/* df15: submatrix of point loading               */
/**************************************************/
void df15(Geometrydata *gd, Analysisdata *ad, int *k1, double **F,
          double **blockArea, TransMap transmap) {


   int i, i0, i1, i2;
   int j;
   int nLPoints = gd->nLPoints;
   int nFPoints = gd->nFPoints;
   double ** points = gd->points;
   double s[7];
   double ** a = ad->K;
   double x, y;
   double T[7][7];
   double xload;
   double yload;

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

      xload = points[i][4];
	  yload = points[i][5];
      transmap(blockArea[0],T,x,y);

      y=points[i][2];

      xload = points[i][4];
      yload = points[i][5];

      transmap(blockArea[i0],T,x,y);


     /* points[i][4] points[i][5]
      * is time dependent loads initialized
      * df09()
      */
      for (j=1; j<= 6; j++) {
         s[j] = T[1][j]*points[i][4] + T[2][j]*points[i][5];
      }

      i1=(int)points[i][3];
      i2=k1[i1];

     /* BUG: There is a bug in here that results from
      * having a point load located outside of the
      * rock mass.
      */
      for (j=1; j<= 6; j++) {
         F[i2][j] += s[j];
      }
   }

}


void
seismicload(DList * seispoints, TimeHistory * timehistory, int timestep,
            int *k1, double ** F, double ** moments,
            double ** e0, TransMap transmap)
{

   DList * ptr;
   DDAPoint * ptmp;
   int j;
   int i2; // memory location for block in forcing vector
   int i0;
   double x,y;
   double force, mass, accel;
   double T[7][7] = {0.0};
   double s[7] = {0.0};

   if (timestep >= th_get_number_of_datapoints(timehistory)) {
      return;
   }

   dlist_traverse(ptr, seispoints) {

      ptmp = (DDAPoint *)ptr->val;
      i0 = ptmp->blocknum;
      x = ptmp->x;
      y = ptmp->y;
      transmap(moments[i0],T,x,y);

      accel = th_get_data_value(timehistory, timestep);

      mass = moments[i0][1]*e0[i0][1];
      force = accel*mass;

      for (j=1; j<= 6; j++) {
         s[j] = T[1][j]*force + T[2][j]*0.0;
      }

      i2 = k1[i0];

     /* BUG: There is a possible bug in here that results from
      * having a point load located outside of the
      * rock mass.
      */
      for (j=1; j<= 6; j++) {
         F[i2][j] += s[j];
      }
   }

}



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

   for (block=1; block<= nBlocks; block++) {

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

/* This was moved from ddanalysis to shorten the size
 * of the analysis loop.
 */
void
assemble(Geometrydata * gd, Analysisdata * ad,
         int ** locks, double ** e0,
         int * k1, int * kk, int ** n, double ** U,
         TransMap transmap) {

   //double ** f = AData->F;
   double ** moments = gd->moments;

/** @todo Pull the globaltime array out of here
 * and just pass the current time into the
 * interpolation function.
 */

   df09(ad->loadpoints,
          gd->points,
          ad->globalTime,
          ad->timeDeps,
          ad->tindex,
          gd->nFPoints,
          ad->nLPoints,
          ad->cts,
          ad->delta_t);

     /* Initialize submatrices back to zero, set
      * some other parameter related to the global
      * time step.
      * FIXME: see if locks can be moved out of here.
      */
	   df10(gd,ad,locks,ad->F);

	  /* submatrix of fixed points.  */
      df12(gd,ad,k1,ad->F,moments,n,transmap);

     /* df13 submatrix of stiffness */
	   stress_stiffness(gd->nBlocks,ad->K,k1,e0,moments,n,ad->planestrainflag);

     /* df14 submatrix of initial stress  */
	   stress_initial(gd->nBlocks,k1,ad->F,e0,moments);

     /* submatrix of point loading  */
	   df15(gd,ad, k1,ad->F,moments, transmap);

     /* seismic loading */
      if (ad->timehistory != NULL) {

         seismicload(gd->seispoints, ad->timehistory, ad->cts,
                     k1, ad->F, moments, e0, transmap);
      }

     /* submatrix of volume force */
	   df16(gd,k1,ad->F,e0,moments);


      if (gd->nBolts > 0) {

         bolt_stiffness_a(gd->rockbolts, gd->nBolts, ad->K,
            k1, kk, n, moments,ad->F,transmap);
      }
}