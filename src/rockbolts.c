/*
 * rockbolts.c
 * 
 * Initial implementation using the model derived 
 * by GHS, p.42, 1988.
 *
 * $Author: doolin $
 * $Date: 2002/10/09 01:46:40 $
 * $Source: /cvsroot/dda/ntdda/src/Attic/rockbolts.c,v $
 * $Revision: 1.7 $
 */

#include <math.h>
#include <assert.h>

#include "analysis.h"
#include "bolt.h"


/* Attempt to load code for rock bolts. The algorithm
 * is taken from GHS p. 45.
 */
void
rockbolts(double ** rockbolt, int numbolts, double ** K, 
          int * k1, int * kk, int ** n, double ** blockArea,  double ** F)
{  
  /* loop counters */   
   int i, j, l, bolt; 

  /* block indices */
   int i2, i3;

  /* storage indices */
   int ji, j1, j2, j3;

  /* Direction cosines, p. 38  */
   double lx, ly;

  /* block numbers containing bolt endpoints */
   int ep1, ep2;

  /* x,y coordinates of bolt endpoint.  */
   double x, y;

  /* Temporary vectors for constructing K, F. */
   double E[7], G[7], T[7][7];

  /* bolt stiffness: */
   double s;

  /* bolt pre-tension */
   double t;


  /* Main loop for constructing matrices. */
   for (bolt=0; bolt<numbolts; bolt++) {
    
     /* Deal with endpoint 1 */
      x = rockbolt[bolt][1];
      y = rockbolt[bolt][2];
     /* [5] stores the block number of the `first' endpoint 
      * of a particular rockbolt.
      */
      ep1 = (int)rockbolt[bolt][5];
      computeDisplacement(blockArea, T, x,  y, ep1);

      bolt_get_dir_cosine_a(rockbolt[bolt],&lx,&ly);

     /* Do the inner product to construct Ei: */
      for (i=1; i<=6; i++) {
         E[i] = T[1][i]*lx + T[2][i]*ly;
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
      for (j=1; j<=6; j++) {
         G[j] = T[1][j]*lx + T[2][j]*ly;
      }

      s = rockbolt[bolt][7];

     /* Try copying the rock bolt stuff into the 
      * global stiffness matrix. Try endpoint 1
      * for Kii first.
      */
      i2=k1[ep1];
      i3=n[i2][1]+n[i2][2]-1;

      for (j=1; j<= 6; j++) {
         for (l=1; l<= 6; l++) {
            j1=6*(j-1)+l;
            K[i3][j1] += s*(E[j]*E[l]);
         }  
      }  

     /* Now try the second endpoint for Kjj*/
     /* i2 
      */
      i2=k1[ep2];
      i3=n[i2][1]+n[i2][2]-1;

      for (j=1; j<= 6; j++) {
         for (l=1; l<= 6; l++) {
            j1=6*(j-1)+l;  /* j1 = 1:36 */
            K[i3][j1] += s*(G[j]*G[l]);
         }  
      }  

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
         for (j=n[ji][1]; j<= n[ji][1]+n[ji][2]-1; j++) {
            i3=j;
            if (kk[j]==j2) {
               break;
            }
         }  
         
        /* submatrix ij  s01-06 i normal s07-12 i shear   */
        /* Add penetration penalty terms to Kji */
         for (j=1; j<= 6; j++) {
            for (l=1; l<= 6; l++) {
               j3=6*(j-1)+l;  /* j3 = 1:36 */
               K[i3][j3] += -s*(E[j]*G[l]);
            }  
         }  

     /* Add to Kji or Kij but not both. */
      }  else  {             /* ji < j2 */

        /* locate j2j1 in a[][] only lower triangle saved */
         for (j=n[j2][1]; j<= n[j2][1]+n[j2][2]-1; j++) {
            i3=j;
            if (kk[j]==ji) {
               break;
            }
         }  
         
        /* add bolt terms to K_{ji} */
         for (j=1; j<= 6; j++) {
            for (l=1; l<= 6; l++) {
               j3=6*(j-1)+l;  /* j3 = 1:36 */
               K[i3][j3] += -s*(G[j]*E[l]);
            }  
         }  
      }  

      t = bolt_get_pretension_a(rockbolt[bolt]);
      for (i=1; i<=6; i++) {
         F[ep1][i] += -t*E[i];
         F[ep2][i] +=  t*G[i];
      }

   }  

}  


/*
int 
rockboltContacts(Geometrydata *bd, Analysisdata *ad, int *kk, 
                 int *k3, double **u) {
return 0;
} 
*/