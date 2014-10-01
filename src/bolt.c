#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset */ 
#include <math.h>
#include <assert.h>


#include "functions.h"
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


struct _boltlist {
   DList * list;
};



struct _boltmat{
   double e00;  // stiffness
   double t0;   // strength
   double f0;   // pretension
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
 * @todo This method needs to be idempotent 
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


/** writes bolt data to output file filename_bolt.log
 * current implementation is minimal:
 * writes elapsed time followed by a colon and a list of bolt endpoints
 * each bolt has 2 pairs of x,y coordinates
 * semicolons separate data for each bolt.
 */
void 
bolt_log_a(double ** hb, int numbolts, int cts, double elapsedTime, 
           PrintFunc printer, void * stream) {

	int i;
   
   if(cts == 0) {
      printer(stream, "This analysis contains %d bolt(s)\n", numbolts);
   }

	printer(stream, "%lf:", elapsedTime);

	// mmm: added bolt force ("tension") to printout
   for (i=0; i < numbolts; i++) {
		printer(stream, " %.12f,%.12f %.12f,%.12f %.12f;", 
                               hb[i][1], hb[i][2], hb[i][3], hb[i][4], hb[i][9]);
	}

	printer(stream,"\n");
}


void 
bolt_init_materials(double ** rockbolts, int numbolts, double ** boltmats) {

   int i;

   for (i=0;i<numbolts;i++) {

      rockbolts[i][7] = boltmats[0][0];
      rockbolts[i][8] = boltmats[0][1];
      rockbolts[i][9] = boltmats[0][2];
      bolt_set_length_a(rockbolts[i]);         
      bolt_set_ref_length_a(rockbolts[i]);
   }
}


static void
bolt_stiffness_accumulate(double * K, double s, double * Hj, double * Hl) {

   int j, j1, l;

   for (j=1; j<= 6; j++) {
      for (l=1; l<= 6; l++) {
         j1=6*(j-1)+l;
         K[j1] += s*(Hj[j]*Hl[l]);
      }  
   }
}


/**
 * @param int * n stores pointers for sparse block 
 *  representation of K.
 * 
 * @param int blockloc Memory location for permuted 
 *  block number.
 *
 * @param int * kk 
 *
 * @return int i3 is location in K array for off-diagonal
 *  block.
 */
static int
extract_block_storage(int * n, int blockloc, int * kk) {

   int j, i3;

   for (j=n[1]; j<= n[1]+n[2]-1; j++) {
      i3=j;
      if (kk[j] == blockloc) {
         break;
      }
   } 
   
   return i3;
}


void
bolt_stiffness_a(double ** rockbolt, int numbolts, double ** K, 
                 int * k1, int * kk, int ** n, double ** blockArea,  
                      double ** F, TransMap transmap) {  

  /* loop counters */   
   int i, j, bolt; 

  /* block indices */
   int i2, i3;

  /* storage indices */
   int ji, j2;

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
      transmap(blockArea[ep1], T, x,  y);

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
      transmap(blockArea[ep2], T, x,  y);

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
      bolt_stiffness_accumulate(K[i3],s,E,E);

     /* Now try the second endpoint for Kjj*/
      i2=k1[ep2];
      i3=n[i2][1]+n[i2][2]-1;
      bolt_stiffness_accumulate(K[i3],s,G,G);

     /* For the cross terms of Kij and Kji, we will need
      * access memory allocated from the contact algorithm.
      * The we should be able to get i2 for the ith block 
      * i3 for the jth block, or vice versa, to initialize
      * Kij.
      */
     /* locate j1j2 in K[][] only lower triangle saved */
     /* if j2 < j1, add terms to Kij instead of Kji */
     /* k1 stores permuted order of blocks. */
      ji = k1[ep1];
      j2 = k1[ep2];

     /* Add to Kji or Kij but not both. */
      if (j2<ji) {

         i3 = extract_block_storage(n[ji],j2,kk);
         bolt_stiffness_accumulate(K[i3],-s,E,G);
      } else {

         i3 = extract_block_storage(n[j2],ji,kk);
         bolt_stiffness_accumulate(K[i3],-s,G,E);
      }  

      t = bolt_get_pretension_a(rockbolt[bolt]);
      for (i=1; i<=6; i++) {
         F[ep1][i] += -t*E[i];
         F[ep2][i] +=  t*G[i];
      }

   }  

}  


void
bolt_update_endpoints_a(double * b,double u1, double v1, double u2, double v2) {

   
   b[10] = u1;
   b[11] = v1;
   b[1] +=  u1;
   b[2] +=  v1;

   b[12] = u2;
   b[13] = v2;
   b[3] +=  u2;
   b[4] +=  v2;
      
   bolt_set_length_a(b);
   bolt_set_pretension_a(b);
}



/** Compute rock bolt end displacements.  Note that no
 * rotation correction is supplied here.
 */
/** @todo Supply a callback or something for the 
 * transplacement updating function.
 */
void
bolt_update_a(double ** bolts, int numbolts, double ** F, 
              double ** moments, TransMap transmap,
              TransApply transapply) {

   int i;
   int ep1,ep2;
   double x,y;
   double u1,v1,u2,v2;
   double T[7][7] = {{0.0}};

   for (i=0; i<numbolts; i++) {

     /* Deal with one endpoint at a time,
      * starting with the arbitrarily chosen
      * `endpoint 1'.  Since bolts is a double **,
      * we have to cast the block numbers of the 
      * endpoints.
      */
      ep1 = (int)bolts[i][5];
      x = bolts[i][1];
      y = bolts[i][2];
	   transmap(moments[ep1],T,x,y);
      transapply(T,F[ep1],&u1,&v1);

      ep2 = (int)bolts[i][6];
      x = bolts[i][3];
      y = bolts[i][4];
	   transmap(moments[ep2],T,x,y);
      transapply(T,F[ep2],&u2,&v2);

      bolt_update_endpoints_a(bolts[i],u1,v1,u2,v2);
   }  

} 


Boltmat *
boltmat_new(void) {

   Boltmat * bm;
   bm = (Boltmat *)malloc(sizeof(Boltmat));
   memset(bm,0xDA,sizeof(Boltmat));
   return bm;
}

void
boltmat_set_props(Boltmat * bm, double stiffness, double strength, 
                  double pretension) {

   bm->e00 = stiffness;
   bm->t0  = strength;
   bm->f0  = pretension;
}


void
boltmat_get_props(Boltmat * bm, double * stiffness, double * strength, 
                  double * pretension) {

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
      // mmm: added index 0 for bolt type
      // Take a look at stress.[c,h] for a slightly 
      // more maintainable way to do this.
	  array[i][0] = bolt_get_type(btmp);
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


#ifdef __cplusplus
}
#endif
