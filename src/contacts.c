
#include <math.h>
#include "ddamemory.h"
#include "contacts.h"
#include "utils.h"  // Remove if updateLockState gets removed.


struct _contacts {

  /**************************************************/
  /* m : 1-6 i1 j1 j2  j1 i1 i2 m[][0]=0 v-e =1 v-v */
  /* m[nBlocks*11+1][7] ( + 1 for type -dmd)        */
   int contactsize1, contactsize2;
   int ** contacts;
  /*------------------------------------------------*/
  /* m0: open-close flag of contact points          */
  /* m0: 1 previous-flag  2 current-flag            */
  /* m0: 3 save flag  4 contact transfer 0 tension  */
  /* m0[nBlocks*11+1][5]                            */
  /* Tension and shear open-close flags for locking
   * contacts during open/close iteration.
   */
   int locksize1, locksize2;
   int ** locks;   /* was m0 */
  /*------------------------------------------------*/
  /* m2: 1-6 i1 j1 j2 j1 i1 i2     0 joint material */
  /* m2[nBlocks*11+1][7]                            */
  /* m2 is a copy of the index to previous contacts 
   * (see savePrevContacts) 
   */
   int prevcontactsize1, prevcontactsize2;
   int ** prevcontacts;
  /*------------------------------------------------*/
  /* m1: block index of m m0 1 start 2 end block i */
	 /* m1[nBlocks+1][3]                               */
   int contactindexsize1, contactindexsize2;
   int **contactindex;  /* was m1 */
  /*------------------------------------------------*/
  /* o : 0 normal penetration  1 shear  movement    */
  /* o : 2 contact edge ratio  3 cohesion length    */
  /* o : 4 save o[][0] 5 save o[][1]  6 save o[][2] */
  /* o[nBlocks*11+1][7]                             */
   int contactlengthsize1, contactlengthsize2;
   double ** contactlength;

};



/* These are currently toted around the place for no 
 * good reason.  They are global, no sense to be local
 * else they would be moved into the _C struct.  They 
 * can be set from an initial function, then used at will
 * in methods for _C.
 */
static double domainscale;
static double openclose;
static double opencriteria;
static double shear_norm_ratio;


/* This will be how the new contacts work. */
struct _C {

  /* This may need to be typedefed in the header so that 
   * the value can be passed in with a function.
   */
   enum {vertex_edge, vertex_vertex} contact_type;

   /* These will point into the vertex arrays. 
    * The contents of the vertex arrays should not be 
    * modified directly, just use these to assign values
    * to temporary variables in df22() 
    */
   double *x1, *y1, *x2, *y2, *x3, *y3;

   /* Shear locking point, may not need to save this, use omega
    * instead.
    */
   double x0,y0;

   double contact_length;

  /* these dist variables are probably the 
   * normal and shear "displacements".
   */
   double pen_dist;
   double shear_dist;
   double cohesion_length;

  /* These "strength" parameters have to be set 
   * somewhere, possibly in the contact code.
   */
   double phi;
   double cohesion;
   double tstrength;

  /* If the contact is a vertex_vertex contact, we have to 
   * check which vertex penetrates the most.  If the second
   * vertex penetrates more than the first vertex, then the
   * "reorder" member needs to be non-zero.  Else it is zero
   * and the first vertex penetrates more than the second
   * vertex.
   */
   int reorder; //boolean
   int tension; //boolean? See locks[][0] or m0[][0]
   int save;    //boolean? Used in transfer contacts.

   /* Shear locking ratio, should be between 0 and 1 */
   double omega;

   enum { open, sliding, locked } previous_state, current_state;

  /* In the future, might be able to adjust the penalty springs
   * on a case by case basis instead of using a global penalty
   * spring.  Also, these could all be floats, which doesn't buy
   * anything in the cpu but does save a few bytes per contact,
   * which might be handy later.
   */
  /* Contact normal stiffness */
   double kn;
  /* Contact shear stiffness */
   double ks;
  /* Contact normal damping */
   double cn;
  /* Contact shear damping */
   double cs;
  /* Lagrange multiplier, may never be needed... */
   double lagmult;
  /* Normal reaction generated at contact */
   double normalforce;
  /* Shear reaction generated at contact */
   double shearforce;

  /* Not yet implemented... we need these for setting the 
   * global stiffness and force matrices.
   */
   //Block * block_i, block_j;
};

/***********  PUBLIC Methods **************/

int **
get_contacts(Contacts * c)
{
   return c->contacts;

}  /* close get_contacts() */


int ** 
get_locks(Contacts * c)
{
   return c->locks;

}  /* close get_locks() */


int **
get_contact_index(Contacts * c)
{
   return c->contactindex;

}  /*  close get_contact_index() */


double ** 
get_contact_lengths(Contacts * c)
{
   return c->contactlength;

}  /* close get_contact_lengths() */


int **
get_previous_contacts(Contacts * c)
{
   return c->prevcontacts;

}  /* close get_previous_contacts() */



/** d0 is contact distance parameter derived from a 
 * user controlled value read from the analysis input 
 * file (g2), and initialized in the "initializeStuff"
 * function.  c0 appears to be a square that completely 
 * encloses each block.  The following 4 lines appear to 
 * implement determination of contact by distance
 * given by Equation 4.3 in Shi, 1993, p. 82.  
 * Basically, if the bounding boxes of blocks i and j
 * do not overlap, no contact, check next block.
 *
 * @param d0 the maximum displacement possible for a given 
 *  time step and velocity.
 *
 * @return 1 if there is an overlap, 0 if no overlap.
 *
 */
int 
contacts_bbox_overlap(double ** bbox_c0, int ii, int jj, double d0) {

  /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
  /* (GHS: c0[][] xl xu yl yu)   */

  /* xu_i < xl_j */
   if (bbox_c0[ii][2]+d0 < bbox_c0[jj][1]) { 
      return 0; 
   }

  /* xu_j < xl_i */   
   if (bbox_c0[jj][2]+d0 < bbox_c0[ii][1]) { 
      return 0; 
   }

  /* yu_i < yl_j */
   if (bbox_c0[ii][4]+d0 < bbox_c0[jj][3]) {
      return 0; 
   }

  /* yu_j < yl_i */
   if (bbox_c0[jj][4]+d0 < bbox_c0[ii][3]) {
      return 0;   
   }

   return 1;  /** overlaps */

}

/* (GHS: c0[][] xl xu yl yu)   */
/* What c0 does is find a box that 
 * completely encloses each block.  This is used
 * in conjunction with the next block of code to 
 * determine whether 2 blocks are close enough to 
 * investigate a contact relationship.
 */
/** FIXME: Write a unit test for this. */
void
contacts_compute_bboxen(double ** bbox_c0,double ** vertices, 
                        int numblocks, int ** vindex) {

   int i,j;
   int startIndex, stopIndex;

   for (i=1; i<=numblocks; i++) {

      startIndex = vindex[i][1];
      stopIndex  = vindex[i][2];
 
      bbox_c0[i][1] = vertices[startIndex][1];
      bbox_c0[i][2] = vertices[startIndex][1];
      bbox_c0[i][3] = vertices[startIndex][2];
      bbox_c0[i][4] = vertices[startIndex][2];

     /* j is probably a vertex counter. */
      for (j=startIndex; j<= stopIndex; j++) { 
         
        /* (GHS: c0[][] xl xu yl yu)   */
        /* find lower x */
         if (bbox_c0[i][1] > vertices[j][1]) {
            bbox_c0[i][1] = vertices[j][1];
         }

        /* find upper x */
         if (bbox_c0[i][2] < vertices[j][1]) {
            bbox_c0[i][2] = vertices[j][1];
         }

        /* find lower y */
         if (bbox_c0[i][3] > vertices[j][2]) {
            bbox_c0[i][3] = vertices[j][2];
         }

        /* find upper y */
         if (bbox_c0[i][4] < vertices[j][2]) {
            bbox_c0[i][4] = vertices[j][2];
         }
      } 
   }  
}




/** These two functions probably need to go somewhere else. 
 * Move 'em into utils after that gets cleaned up.
 */
double
compute_length(const double x1, const double y1, const double x2, const double y2) {

   return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

double
compute_length_squared(const double x1, const double y1, const double x2, const double y2) {

   return ((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}




/*********  PRIVATE Methods **************/

static void init_contacts(Contacts * c, int nblocks)
{
   c->contactsize1 = 21*nblocks+1;  /* changed 19 July 1999  */
	c->contactsize2 = 7 + 1; /* store contact type -- dmd */
   c->contacts = IntMat2DGetMem(c->contactsize1, c->contactsize2);

}  /* close init_contacts() */


static void free_contacts(Contacts * c)
{
   int m = c->contactsize1;
   int n = c->contactsize2;
   free2DIntArray(c->contacts, m, n);

}  /* close free_contacts() */


static void init_locks(Contacts * c, int nblocks)
{
  	c->locksize1 = 21*nblocks+1; /* changed feb 7 '95 */
   c->locksize2 = 5;
   c->locks = IntMat2DGetMem(c->locksize1, c->locksize2);

}  /* close init_locks() */


static void free_locks(Contacts * c)
{
   int m = c->locksize1;
   int n = c->locksize2;
   free2DIntArray(c->locks,m,n);

}  /* close free_locks() */

static void init_contact_index(Contacts * c, int nblocks)
{
   c->contactindexsize1 = nblocks+1;
   c->contactindexsize2 = 3; /* 4 = 3 + 1 for bolt flag */
   c->contactindex = IntMat2DGetMem(c->contactindexsize1, c->contactindexsize2);

}  /* close init_contact_index() */

static free_contact_index(Contacts * c)
{

   int m = c->contactindexsize1;
   int n = c->contactindexsize2;
   free2DIntArray(c->contactindex,m,n);

}  /* close free_contact_index() */


static void init_contact_length(Contacts * c, int nblocks)
{
   c->contactlengthsize1 = 21*nblocks+1; /* changed feb 7 '95 */
   c->contactlengthsize2 = 7;
   c->contactlength = DoubMat2DGetMem(c->contactlengthsize1, c->contactlengthsize2);

}  /* init_contact_length() */


static void free_contact_length(Contacts * c) 
{
   int m = c->contactlengthsize1;
   int n = c->contactlengthsize2;
   free2DDoubleArray(c->contactlength,m,n);

}  /* close free_contact_length() */


static void
init_previous_contacts(Contacts * c, int nblocks)
{
   c->prevcontactsize1 = 21*nblocks+1;
   c->prevcontactsize2 = 7;
   c->prevcontacts = IntMat2DGetMem(c->prevcontactsize1, c->prevcontactsize2);

}  /* close init_previous_contacts() */


static void 
free_previous_contacts(Contacts * c) 
{
   int m = c->prevcontactsize1;
   int n = c->prevcontactsize2;
   free2DIntArray(c->prevcontacts, m, n);

}  /* close free_previous_contacts() */



Contacts *
contacts_new(int nblocks) {

   Contacts * c;
   c = (Contacts *)malloc(sizeof(Contacts));

#if _DEBUG
   memset((void*)c,0xDA,sizeof(Contacts));
#endif

   init_contacts(c,nblocks);
   init_locks(c,nblocks);
   init_contact_index(c,nblocks);
   init_contact_length(c,nblocks);
   init_previous_contacts(c,nblocks);

   return c;
}


void 
contacts_delete(Contacts * c) {

   free_contacts(c);
   free_locks(c);
   free_contact_index(c);
   free_contact_length(c);
   free_previous_contacts(c);

#if _DEBUG
   memset((void *)c, 0xdd, sizeof(Contacts));
#endif
} 


/*  Taken mostly from manifold tech report, p. 109 */
void
updateLockState(C * c)
{
   switch (c->previous_state)
   {
      case open:
         if (c->pen_dist > 0)
            c->current_state = open;
         else if (fabs(c->shearforce) > fabs(c->normalforce)*tan(c->phi))
            c->current_state = sliding;
         else 
            c->current_state = locked;
         break;

      case sliding:
         if (c->pen_dist > 0)
            c->current_state = open;
        /* This test is bogus.  The description in manifold
         * tech note (p. 109) has typos and the wording confuses me.
         */
         else if (sign(c->shear_dist) == -sign(c->shearforce))
            c->current_state = sliding;
         else 
            c->current_state = locked;
         break;

      case locked:
         if (c->pen_dist > 0)
            c->current_state = open;
         else if (fabs(c->shearforce) > fabs(c->normalforce)*tan(c->phi))
            c->current_state = sliding;
         else 
            c->current_state = locked;      
         break;

      default:
         break;
   }
}

