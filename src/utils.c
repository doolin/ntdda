/*
 * utils.c
 *
 * General utility routines for the DDA analysis
 * procedure.  Will include area, matrix multiplication
 * matrix inverse, etc.
 *
 * $Author: doolin $
 * $Date: 2002/10/28 13:46:58 $
 * $Source: /cvsroot/dda/ntdda/src/utils.c,v $
 * $Revision: 1.24 $
 */


#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "analysis.h"
#include "contacts.h"
#include "ddamemory.h"
#include "utils.h"
#include "material.h"




/**************************************************/
/* sign: return sign                         0004 */
/**************************************************/
double sign(double f2)
{
   if (f2 > 0)
      return ( 1.0);

   if (f2 < 0)
      return (-1.0);

   if (f2 == 0)
      return ( 0.0);

   return (0.0);

}  /* close sign() */





#define DIST_PROCEDURE 0
#if DIST_PROCEDURE
/**************************************************/
/* dist: distance of a segment to a node     0014 */
/**************************************************/
/* x1  y1  x2  y2  x3  y3  d5                     */
/* Rewrite this as a MACRO and use it in df04
 * etc etc etc.
 */
void dist(void) {

   double x4,  y4;
   double t1;
   double d5;

  /* nearest point of p0 in between p2 p3           */
  /* compute distance of p0 to nearst point in p2p3 */
   t1  =  (x1-x2)*(x3-x2) + (yi-y2)*(y3-y2);
   t1 /= ((x3-x2)*(x3-x2) + (y3-y2)*(y3-y2));
   if (t1 <= 0.0)
      t1=0;
   if (t1 >= 1.0)
      t1=1;
   x4  = x2 + t1*(x3-x2);
   y4  = y2 + t1*(y3-y2);
   d5  = sqrt((x4-x1)*(x4-x1) + (y4-yi)*(y4-yi));
}
#endif


/* FIXME: This function should moved to between the analysis
 * part and the geometry part.  That way it can be cloned
 * when the geometry data is cloned at the start of an analysis.
 */
void
blockNumberOfVertex(Geometrydata * gd)
{
  /* initiate nn0 as block number index of vertices and u   */
  /* vertices is the vertex matrix, vindex is the indices of vertex
   * location in vertices, u is vertex angles (?).  nBlocks
   * is the total number of blocks in the problem (?).
   */
   int i1,i2,i,j;

   gd->nn0size = gd->vertexCount+1;
   gd->nn0 = (int *)calloc(1, sizeof(int)*gd->nn0size);

   for (i=1; i<= gd->nBlocks; i++)
   {
      i1=gd->vindex[i][1];
      i2=gd->vindex[i][2];
      for (j=i1-1; j<= i2+3; j++)
      {
         gd->nn0[j]=i;
      }  /*  j  */
   }  /*  i  */

}  // Close blockNumberOfVertex()






void
print2DMat(double mat[7][7], int m, int n, char * location, FILE * fp) {

   int i, j;

   assert ( (m!=0) && (n!=0) && (mat != NULL) );

   for ( i = 1; i < m; i++) {
      for (j=1; j< n; j++) {
         fprintf(fp,"%.4f  ", mat[i][j]);
      }
      fprintf(fp,"\n");
   }

}


/* Assume that the file pointer is open */
void
print2DArray(double ** mat, int m, int n, FILE * fp, char * location)
{
   int i, j;

   assert ( (m!=0) && (n!=0) && (mat != NULL) );

   fprintf(fp,"%s\n",location);

   for ( i = 0; i<m; i++)
   {
      for (j=0; j<n; j++)
         fprintf(fp,"%.4f  ", mat[i][j]);
      fprintf(fp,"\n");
   }

   fprintf(fp,"\n");

}  /* close print2DMat() */


void
print1DIntArray(int * mat, int n, char * location, FILE * fp) {
   int i;

   assert ( (n!=0) && (mat != NULL) );

   for (i=0; i< n; i++) {
      fprintf(fp,"%d  ", mat[i]);
   }
   fprintf(fp,"\n");
}


void
print1DDoubleArray(double * mat, int n, char * location, FILE * fp) {
   int i;

   assert ( (n!=0) && (mat != NULL) );

   fprintf(fp,"1 D Array, from %s\n",location);

   for (i=0; i< n; i++) {
      fprintf(fp,"%f  ", mat[i]);
   }

   fprintf(fp,"\nEnd 1 D Array print\n\n");
}






/* FIXME: This function may be a duplicate of the clone matrix
 * function.  If so, get rid of this in favor of clone
 * matrix.
 */
int
copy2DDoubMat(double ** mat1, double ** mat2, int n, int m)
{
   int i, j;

   for (i=0; i<n; i++)
      for (j=0; j<m; j++)
         mat1[i][j] = mat2[i][j];

   return 1;

}  /*  close copy2DDoubMat() */


int
copy2DIntMat(int ** mat1, int ** mat2, int n, int m)
{
   int i, j;

   assert( mat1 != NULL  || mat2 != NULL);

   for (i=0; i<n; i++)
      for (j=0; j<m; j++)
         mat1[i][j] = mat2[i][j];

   return 1;

}  /*  close copy2DDoubMat() */





void
initBlockProperties(Geometrydata * gd, Analysisdata * ad, double ** e0)
{
   int i, i1;
   int nBlocks = gd->nBlocks;
   int ** vindex = gd->vindex;
   double ** materialProps = ad->materialProps;
   double ** moments = gd->moments;
   int nb = ad->nBlockMats;

   double gravaccel = adata_get_grav_accel(ad);



  /** Fix any problems silently. */
   material_validate(vindex,nBlocks,nb);


  /* materialProps: ma we e0 u0 s11 s22 s12 t1 t2 t12 vx vy vr */
  /* (GHS: e0: ma we e0 u0 c11 c22 c12 t-weight)  */
  /* (GHS: v0: velocity of u v r ex ey gxy parameters) */
   for (i=1; i<= nBlocks; i++) {


     /* WARNING: The material types for the blocks must be specified
      * in order. Example: 1,2,3,4.  Any out-of-order material type,
      * such as 1,2,3,5, will result in material type 5 being demoted
      * to material type 3.  The reason for this is that the material
      * types are handled in sequential arrays.  It may be possible
      * to over-allocate e0 to handle non-successive material types,
      * but that is a project for the future.  A better implementation
      * would hash the material type under a non-integer key.
      */
     /* FIXME: Determine if ad->materialProps is ever updated.  If not,
      * it only exists to make the transfer from the input file to
      * e0, and can possibly be radically rewritten.
      */



      i1 = vindex[i][0];
      e0[i][0] = materialProps[i1][0]; /* density; unit mass */
      //e0[i][1] = materialProps[i1][1]; /* unit weight */
      /* Unit weight is now set with respect to user specified
       * gravitational acceleration.  The tag needs to be deprecated
       * in the runtime files.
       */
      e0[i][1] = materialProps[i1][0]*gravaccel; /* unit weight */
      e0[i][2] = materialProps[i1][2]; /* Young's modulus */
      e0[i][3] = materialProps[i1][3]; /* Poisson's ratio */
     /* This is marked as a possible error in Mary's
      * annotated code.
      */
     /* \sigma_{11} */
      e0[i][4] = materialProps[i1][4] + (moments[i][3]/moments[i][1])*materialProps[i1][7];
     /* \sigma_{22} */
      e0[i][5] = materialProps[i1][5] + (moments[i][3]/moments[i][1])*materialProps[i1][8];
     /* \sigma_{12} */
      e0[i][6] = materialProps[i1][6] + (moments[i][3]/moments[i][1])*materialProps[i1][9];
     /* Current mass */
      //e0[i][7] = materialProps[i1][0]*moments[i][1];
     /* z strain, needed for tracking density in plane stress. */
      e0[i][7] = 0;
     /* Damping parameter from Shi 1988 formulation. */
      e0[i][8] = materialProps[i1][13];

   }
}



void
initVelocity(Geometrydata * gd, Analysisdata * ad, double ** v0) {

   int i, i1;
   double ** materialProps = ad->materialProps;
   int nBlocks = gd->nBlocks;
   int ** vindex = gd->vindex;

  /*------------------------------------------------*/
  /* v0: velocity of u v r ex ey gxy parameters     */
  /* initial velocity                               */
   for (i=1; i<= nBlocks; i++)
   {
      i1 = vindex[i][0];
      v0[i][1] = materialProps[i1][10]; /* v11 */
      v0[i][2] = materialProps[i1][11]; /* v22 */
      v0[i][3] = materialProps[i1][12]; /* vr */
     /* Most likely 4,5,6 store previous iteration values.
      * FIXME: find out what 4,5,6 are and where they are
      * initialized.  v0[][4-6] probably store strain
      * rates, as updated in df25()
      */
      v0[i][4] = 0;
      v0[i][5] = 0;
      v0[i][6] = 0;
   }  /*  i  */
}  /* close initVelocity() */


/* If the spring stiffness is not given as a parameter,
 * this function is called from initNewAnalysis().
 */
void
initContactSpring(Analysisdata * ad)
{
   int i;
   int nb = ad->nBlockMats;
   double a1;
   double ** materialProps = ad->materialProps;

/* This is out for the moment because the fixed point
 * penalty is set from the contact spring penalty.
 */
//   if (ad->contactmethod == penalty)
//   {
     /* initial contact spring stiffness g0 */
      a1 = 0;
      for (i=1; i<= nb; i++)
      {
        /* That is, set the initial spring stiffness
         * equal to the highest Young's modulus...
         */
         if (a1 < materialProps[i][2])
            a1=materialProps[i][2];
      }  /*  i  */
     /* then reset it to be 50 times stiffer than the
      * highest Young's modulus.
      */
      //ad->g0 = 50.0*a1;

     /* WARNING!!!  This may be reduced to run very stiff
      * problems to try and reduce elastic rebound.
      */
     /* pfactor gets read in from a file */
      ad->contactpenalty = ad->pfactor*a1;

      //ad->springstiffness[0] = ad->g0;
      ad->springstiffness[0] = ad->contactpenalty;

}  /* close initContactSpring() */



void allocateAnalysisArrays(Geometrydata * GData,
                            int ** kk,
                            int ** k1,
                            double *** c0,
                            //double *** e0,
                            double *** U,
                            int *** n)
{

  /* k : index of a    2*block contact + 20*nBlocks      */
  /* k : reserved positions for non-zero storage    */
 	/* kk[nBlocks*40+1]                                     */
   __kksize1 = FORTY*GData->nBlocks+1;
  /* Changed to calloc so that unused entries would be zero.
   * This makes it easier for printing out the entries for
   * examination.
   */
  *kk = (int *)calloc(1,sizeof(int)*__kksize1);

  /*------------------------------------------------*/
  /* k1: i old block number k1[i] new block number  */
  /* k1[nBlocks+1]                                  */
   __k1size1=GData->nBlocks+1;
  *k1 = (int *)calloc(1,sizeof(int)*__k1size1);

  /*------------------------------------------------*/
  /* c0: xl xu yl yu block box in 3000              */
  /* c0: u v r ex ey gxy of sliding friction force  */
  /* c0[nBlocks+1][7]                               */
   __c0size1=GData->nBlocks+1;
   __c0size2=7;
  *c0 = DoubMat2DGetMem(__c0size1, __c0size2);

  /*  u v  or x+u y+v  of vertices in vertices          */
  /* u[vertexCount+1][3]                                */
   __Usize1=GData->nBlocks+1; //GData->vertexCount+1;
   __Usize2=13;
  *U = DoubMat2DGetMem(__Usize1, __Usize2);

  /**************************************************/
  /* n : 1 begin number of k a b for i-th row       */
  /* n : 2 number of k a b       for i-th row       */
  /* n : 3 limit of k element number of i row       */
  /* n[nBlocks+1][4]                                */
   __nsize1=GData->nBlocks+1;
   __nsize2=4;
  *n = IntMat2DGetMem(__c0size1, __c0size2);

}


void

deallocateAnalysisArrays(int *kk,

                         int *k1,
                         double **c0,

                         //double **e0,
                         double **U,

                         int ** n)
{
   extern void freeCGTemp();


   freeSolverTempArrays();
   freeStorageTempArrays();
   freeContactTempArrays();
   freeIntTempArrays();
   //freeCGTemp();
   freeIntegrationArrays();


   if (n)
      free2DMat((void **)n, __nsize1);


	  if (c0)
      free2DMat((void **)c0, __c0size1);

   //if (v0)
   //   free2DMat((void **)v0, __v0size1);


       //if (moments)
      //free2DMat((void **)moments, __momentsize1);


   if (U)
      free2DMat((void **)U, __Usize1);

  /* 1D arrays  */
   if (kk)
   {
      free(kk);
      kk = NULL;
   }

   //if (k3)
   //{
   //   free(k3);
   //   k3 = NULL;
   //}

  	if(k1)
   {
      free(k1);
      k1 = NULL;
   }

}



/** Needs to be fixed to not depend on e0. */
void
computeMass(double * mass, double ** moments, double ** e0, int numblocks) {

   int i;
   double currentarea,thickness,density;

   for (i=1; i<=numblocks; i++) {

      currentarea = moments[i][1];
      thickness = 1/(1+e0[i][7]);
      density = e0[i][0];
      mass[i] = currentarea*thickness*density;
   }
}


