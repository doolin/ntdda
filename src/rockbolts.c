/*
 * rockbolts.c
 * 
 * Initial implementation using the model derived 
 * by GHS, p.42, 1988.
 *
 * $Author: doolin $
 * $Date: 2002/10/05 22:36:25 $
 * $Source: /cvsroot/dda/ntdda/src/Attic/rockbolts.c,v $
 * $Revision: 1.6 $
 */

#include <math.h>
#include <assert.h>

#include "analysis.h"


/* Attempt to load code for rock bolts. The algorithm
 * is taken from GHS p. 45.
 */
int
rockbolts(double ** rockbolt, int numbolts, double ** K, 
          int * k1, int * kk, int ** n, double ** blockArea,  double ** F)
{  
   int i, j, l, bolt;  /* loop counters */
  /* block indices */
   int i2, i3;
   int ji, j1, j2, j3;
   double boltlength;
   double x1,x2,y1,y2;
  /* Direction cosines, p. 38  */
   double lx, ly;
  /* Displacements of endpoints. */
   double ux1,ux2,uy1,uy2;
  /* differential bolt length, units of L^2 */
   double dl;
  /* block numbers containing bolt endpoints */
   int ep1, ep2;
  /* x,y coordinates of bolt endpoint.  */
   double x, y;
  /* Temporary vectors for constructing K
   * FIXME: Put the math in here for these.
   */
   double Ei[7];
   double Gj[7];
  /* bolt stiffness: */
   double s;
  /* bolt pre-tension */
   double pt;
  /* Coefficient matrix. */
   //double ** K = ad->K;
   //double ** rockbolt = gd->rockbolts;
   double T[7][7];

  /* Main loop for constructing matrices. */
   for (bolt=0; bolt<numbolts; bolt++) {
    
     /* get x, y coordinates of each endpoints of 
      * the rockbolt 
      */
      x1 = rockbolt[bolt][1];
      y1 = rockbolt[bolt][2];
      x2 = rockbolt[bolt][3];
      y2 = rockbolt[bolt][4];
      boltlength = sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
      assert(boltlength > 0);
     /* Direction cosines.  Check to make sure these are
      * the correct formulas
      */
      lx = (x1-x2)/boltlength;
      ly = (y1-y2)/boltlength;
     /* checking the obvious... doesn't seem to work.
      * FIXME: Write a unit test for these asserts to see
      * why they didn't fire on a 0 bolt length.
      */
      assert ( (-1 <= lx) && (lx <= 1));
      assert ( (-1 <= ly) && (ly <= 1));

     /* rockbolt[i][10:14] are set in df25.
      */
      ux1 = rockbolt[bolt][10];
      uy1 = rockbolt[bolt][11];
      ux2 = rockbolt[bolt][12];
      uy2 = rockbolt[bolt][13];
 
     /* Compute the differential length, from 
      * Yeung  1992, p.40.  
      */
      dl = ( (x1-x2)*(ux1-ux2) + (y1-y2)*(uy1-uy2) );

      s = rockbolt[bolt][7];///boltlength;  /* stiffness */
     /* FIXME: Non-zero pt produces very strange and 
      * unusual behavior.
      */
      pt = rockbolt[bolt][9]; /* pre-tensioning? */
     /* Deal with endpoint 1 */
      x = rockbolt[bolt][1];
      y = rockbolt[bolt][2];
     /* [5] stores the block number of the `first' endpoint 
      * of a particular rockbolt.
      */
      ep1 = (int)rockbolt[bolt][5];
      computeDisplacement(blockArea, T, x,  y, ep1);
     /* Do the inner product to construct Ei: */
      for (i=1; i<=6; i++)
      {
         Ei[i] = T[1][i]*lx + T[2][i]*ly;
      }

     /* Deal with endpoint 2, get x, y coords */
      x = rockbolt[bolt][3];
      y = rockbolt[bolt][4];
     /* [6] stores the block number of the `second'
      * endpoint of a particular rockbolt.
      */
      ep2 = (int)rockbolt[bolt][6];
      computeDisplacement(blockArea, T, x,  y, ep2);
     /* Do the inner product to construct Gj:
      * $G_j = [T_j]^T\cdot \ell$. 
      */
      for (j=1; j<=6; j++)
      {
         Gj[j] = T[1][j]*lx + T[2][j]*ly;
      }

     /* Try copying the rock bolt stuff into the 
      * global stiffness matrix. Try endpoint 1
      * for Kii first.
      */
      i2=k1[ep1];
      i3=n[i2][1]+n[i2][2]-1;

      for (j=1; j<= 6; j++)
      {
         for (l=1; l<= 6; l++)
         {
            j1=6*(j-1)+l;
            K[i3][j1] += s*(Ei[j]*Ei[l]);
         }  /*  l  */
      }  /*  j  */

     /* Now try the second endpoint for Kjj*/
     /* i2 
      */
      i2=k1[ep2];
      i3=n[i2][1]+n[i2][2]-1;

      for (j=1; j<= 6; j++)
      {
         for (l=1; l<= 6; l++)
         {
            j1=6*(j-1)+l;  /* j1 = 1:36 */
            K[i3][j1] += s*(Gj[j]*Gj[l]);
         }  /*  l  */
      }  /*  j  */

     /* For the cross terms of Kij and Kji, we will need
      * access memory allocated from the contact algorithm.
      * The we should be able to get i2 for the ith block 
      * i3 for the jth block, or vice versa, to initialize
      * Kij.
      */
     /* locate j1j2 in a[][] only lower triangle saved */
     /* if j2 < j1, add terms to Kij instead of Kji */
      ji = k1[ep1];
      j2 = k1[ep2];

     /* FIXME: Find out what is wrong with this memory 
      * finding stuff.
      */
      if (j2<ji) 
      {
        /* find Kij */
         for (j=n[ji][1]; j<= n[ji][1]+n[ji][2]-1; j++)
         {
            i3=j;
            if (kk[j]==j2) 
               break;
         }  /*  j  */
         
        /* submatrix ij  s01-06 i normal s07-12 i shear   */
        /* Add penetration penalty terms to Kji */
         for (j=1; j<= 6; j++)
         {
            for (l=1; l<= 6; l++)
            {
               j3=6*(j-1)+l;  /* j3 = 1:36 */
               K[i3][j3] += -s*(Ei[j]*Gj[l]);
            }  /*  l  */
         }  /*  j  */
     /* Add to Kji or Kij but not both. */
     /* locate j2j1 in a[][] only lower triangle saved */
      }  
      else  /* ji < j2 */
      {
         for (j=n[j2][1]; j<= n[j2][1]+n[j2][2]-1; j++)
         {
            i3=j;
            if (kk[j]==ji) 
               break;
         }  /*  j  */
         
        /* add bolt terms to K_{ji} (???) */
         for (j=1; j<= 6; j++)
         {
            for (l=1; l<= 6; l++)
            {
              /* index arithmetic */
               j3=6*(j-1)+l;  /* j3 = 1:36 */
               K[i3][j3] += -s*(Gj[j]*Ei[l]);
            }  /*  l  */
         }  /*  j  */
         
     }  /* close loops for Kij/Kji */

#if 0
     /* FIXME: non-zero pt produces strange and
      * unusual behavior.  This needs to be examined
      * very closely.
      */
     /* Now load the force vector Fi:
      */
      for (i=1; i<=6; i++)
      {
         /* force vector */
         F[ep1][i] = -pt*Ei[i];
      }
     /* And the force vector Fj:
      */
      for (j=1; j<=6; j++)
      {
         /* force vector */
         F[ep2][j] = pt*Gj[j];
      }
#endif

   }  /* Close loop over rock bolts. */

   return 1;
}  /* Close rockbolts() */


/* stub function 
 */
int 
rockboltContacts(Geometrydata *bd, Analysisdata *ad, int *kk, 
                 int *k3, double **u) {
return 0;
} /* close rockboltContacts() */
