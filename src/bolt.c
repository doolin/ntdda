
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset */ 
#include <math.h>
#include <assert.h>

#include "bolt.h"
#include "ddadlist.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif




struct _bolt {

   int number;
   int type;
   int b1,b2;
   double x1,y1,x2,y2;
   double ux1,uy1,ux2,uy2;
   double dx1,dy1,dx2,dy2;
   double stiffness;
   double strength;
   double pretension;

  /** Pointers to stiffness array subblocks, which
   *  have to be updated any time the stiffness
   *  matrix is reallocated.
   */
   double * Kii;
   double * Kjj;
   double * Kij;
};



struct _boltmat{
   double e00;  // stiffness
   double t0;   // strength
   double f0;   // pretension
};


struct _boltlist {
   DList * list;
};



Bolt * 
bolt_new() {

   Bolt * b = (Bolt*)malloc(sizeof(Bolt));
   memset((void*)b,0xda,sizeof(Bolt));
   return b;
}


Bolt * 
bolt_new_1(double x1, double y1, double x2, double y2) {

   Bolt * b = (Bolt*)malloc(sizeof(Bolt));
   memset((void*)b,0xda,sizeof(Bolt));

   b->x1 = x1;
   b->y1 = y1;
   b->x2 = x2;
   b->y2 = y2;

   return b;
}

void
bolt_delete(Bolt * b) {
   free(b);
}

void   
bolt_set_endpoints(Bolt * b,double x1,double y1, 
                   double x2, double y2) {

   b->x1 = x1;
   b->y1 = y1;
   b->x2 = x2;
   b->y2 = y2;
}


int
bolt_equals(Bolt * b1, Bolt * b2) {

  if (b1->x1 != b2->x1 ||
      b1->y1 != b2->y1 ||
      b1->x2 != b2->x2 ||
      b1->y2 != b2->y2 ) {
    return 0;
  } else {
    return 1;
  }
}


void
bolt_print(Bolt * b, PrintFunc printer, void * stream) {

  printer(stream,"Bolt printing not yet implemented.\n");
}


int
bolt_get_type(Bolt * b) {
   
   return b->type;
}


void 
bolt_set_type(Bolt * b, int type) {

   b->type = type;
}

void 
bolt_get_endpoints(Bolt * b,double * x1, double * y1,
                   double *x2,double * y2) {

   *x1 = b->x1;
   *y1 = b->y1;
   *x2 = b->x2;
   *y2 = b->y2;
}


double 
bolt_length(Bolt * b) {

   return sqrt( (b->x2-b->x1)*(b->x2-b->x1) + (b->y2-b->y1)*(b->y2-b->y1) );
}




void 
bolt_set_length_a(double * bolt) {

   double x1 = bolt[1];
   double y1 = bolt[2];
   double x2 = bolt[3];
   double y2 = bolt[4];
   bolt[15] = sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
}


/** 
 * Take no chances, can't tell when double are 
 * initialized in the general case.  Maybe later
 * work out detailed for `magic' initialization 
 * numbers.
 */
double
bolt_get_length_a(double * bolt) {

   double x1 = bolt[1];
   double y1 = bolt[2];
   double x2 = bolt[3];
   double y2 = bolt[4];
   return sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
}



/**
 * Since we can't rewrite, use the array
 * in lieu of bolt struct.
 *
 * @todo This method needs to idempotent 
 *  per each bolt.
 *
 * @todo Add code to ensure that the pretension is 
 *  less than the stiffness.
 *
 * @param double * an array to various values 
 *  needed for a bolt.
 *
 * @return double dl the length differential
 *  computed by dl = f/k;       
 */
double 
bolt_set_ref_length_a(double * bolt) {

  double dl;

  dl = bolt[9]/bolt[7];
  bolt[14] = bolt[15] - dl;

  return dl;
}


double
bolt_get_ref_length_a(double * bolt) {

   return bolt[14];
}


/** Probably won't implement this.
 */
void
bolt_set_dir_cosine_a(double * bolt) {

}


void
bolt_get_dir_cosine_a(double * bolt, double * lx, double * ly) {

   double x1 = bolt[1];
   double y1 = bolt[2];
   double x2 = bolt[3];
   double y2 = bolt[4];
   double l  = bolt_get_length_a(bolt);

  *lx = (x1-x2)/l;
  *ly = (y1-y2)/l;

 /* checking the obvious... doesn't seem to work.
  * FIXME: Write a unit test for these asserts to see
  * why they didn't fire on a 0 bolt length.
  */
  assert ( (-1 <= *lx) && (*lx <= 1));
  assert ( (-1 <= *ly) && (*ly <= 1));

}



void
bolt_set_pretension_a(double * bolt) {

   double lref = bolt[14];
   double l    = bolt[15];
   double dl;

   dl = l - lref;
   bolt[9] = dl*bolt[7];
}


double
bolt_get_pretension_a(double * bolt) {

   return bolt[9];
}





Boltmat *
boltmat_new(void) {
   Boltmat * bm;
   //fprintf(stdout,"Getting new block mat\n");
   bm = (Boltmat *)malloc(sizeof(Boltmat));
   memset(bm,0xDA,sizeof(Boltmat));
   return bm;

}

void
boltmat_set_props(Boltmat * bm, double stiffness, double strength, double pretension) {
   bm->e00 = stiffness;
   bm->t0  = strength;
   bm->f0  = pretension;
}


void
boltmat_get_props(Boltmat * bm, double * stiffness, double * strength, double * pretension) {
  *stiffness  = bm->e00;
  *strength   = bm->t0;
  *pretension = bm->f0;
}




Boltlist * 
boltlist_new() {

   Boltlist * bl = (Boltlist*)malloc(sizeof(Boltlist));
   bl->list = dlist_new();

   return bl;
}


void
boltlist_delete(Boltlist * bl) {

  dl_delete(bl->list,(FreeFunc)bolt_delete);
  free(bl);
}


void
boltlist_append(Boltlist * boltlist, Bolt * bolt) {

   dl_insert_b(boltlist->list,bolt);
}

#if 0
Bolt * 
boltlist_next_bolt(Boltlist * bl) {

  //return 

}
#endif


void
boltlist_get_array(Boltlist * boltlist, double ** array) {

   int i = 0;
   DList * ptr;
   Bolt * btmp;
   double x1,y1,x2,y2;

   dlist_traverse(ptr, boltlist->list) {

      btmp = ptr->val;

      bolt_get_endpoints(btmp,&x1,&y1,&x2,&y2);
      array[i][1] = x1;
      array[i][2] = y1; 
      array[i][3] = x2;
      array[i][4] = y2;
      array[i][15] = bolt_length(btmp);
      i++;
   }
}


int
boltlist_length(Boltlist * bl) {
   return dlist_length(bl->list);
}


void 
boltlist_print(Boltlist * bl, PrintFunc printer, void * stream) {
 
   DList * ptr;
   Bolt * b;

   dlist_traverse(ptr, bl->list) {

      b = (Bolt*)ptr->val;
      bolt_print(b,printer,stream);
   }
}



/** This doesn't work very well. */
#if 0
bolt_load_matrix(Bolt * bolt, double ** K, 
          int * k1, int * kk, int ** n, double ** blockArea,  double ** F) {
            
  /* loop counters */
   int i, j, l;  

  /* block indices */
   int i2, i3;
   int ji, j1, j2, j3;

   double boltlength;

  /* Endpoint coordinates. */
   double x1 = bolt->x1;
   double y1 = bolt->y1;
   double x2 = bolt->x2;
   double y2 = bolt->y2;

  /* Displacements of endpoints. */
   double ux1 = bolt->ux1;
   double uy1 = bolt->uy1;
   double ux2 = bolt->ux2;
   double uy2 = bolt->uy2;

  /* block numbers containing bolt endpoints */
   int ep1 = bolt->b1;
   int ep2 = bolt->b2;
   
   double s = bolt->stiffness;
   double t = bolt->pretension;
   
  /* Direction cosines, p. 38  */
   double lx, ly;

  /* differential bolt length, units of L^2 */
   double dl;

  /** Temporary vectors for constructing K
   * @todo Put the math in here for these.
   */
   double E[7];
   double G[7];
   double T[7][7];


    
     /* get x, y coordinates of each endpoints of 
      * the rockbolt 
      */
      boltlength = bolt_length(bolt);
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

 
     /* Compute the differential length, from 
      * Yeung  1992, p. 40, Eq. ???.
      */
      dl = ( (x1-x2)*(ux1-ux2) + (y1-y2)*(uy1-uy2) );

      s = bolt->stiffness;

     /* Deal with endpoint 1 */
      computeDisplacement(blockArea, T, x1,  y1, ep1);
     /* Do the inner product to construct Ei: */
      for (i=1; i<=6; i++) {
         E[i] = T[1][i]*lx + T[2][i]*ly;
      }


     /* endpoint 2 */
      computeDisplacement(blockArea, T, x2,  y2, ep2);
     /* Do the inner product to construct Gj:
      * $G_j = [T_j]^T\cdot \ell$. 
      */
      for (j=1; j<=6; j++) {
         G[j] = T[1][j]*lx + T[2][j]*ly;
      }


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
            j1=6*(j-1)+l;
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
      if (j2<ji) {
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
     /* locate j2j1 in a[][] only lower triangle saved */
      } else { /* ji < j2 */
         for (j=n[j2][1]; j<= n[j2][1]+n[j2][2]-1; j++) {
            i3=j;
            if (kk[j]==ji) 
               break;
         } 
         
        /* add bolt terms to K_{ji} (???) */
         for (j=1; j<= 6; j++) {
            for (l=1; l<= 6; l++) {
               j3=6*(j-1)+l;  /* j3 = 1:36 */
               K[i3][j3] += -s*(G[j]*E[l]);
            }
         }  
         
     }  /* close loops for Kij/Kji */

#if 0
      for (i=1; i<=6; i++) {
         F[ep1][i] += -t*E[i];
         F[ep2][j] +=  t*G[j];
      }

#endif

}
#endif


#ifdef __cplusplus
}
#endif
