/*
 * contact.c
 *
 * Handle all the machinery for determining 
 * block contacts.
 *
 */
#include <time.h>
#include <math.h>
#include <assert.h>
#include <malloc.h>

#include "analysis.h"
#include "contacts.h"
#include "ddamemory.h"
#include "datalog.h"
#include "constants.h"


extern Datalog * DLog;

static char mess[80];

static void df04(Geometrydata *, Analysisdata *, int *, int *, double **, 
                 double **);
static void df05(Geometrydata *, Analysisdata *, int **, int *, 
            int *, int *, double **);


static void df07(Geometrydata *, Analysisdata *, int **, int **, 
            double **, int *);

/* These used to be globals, then they were passed around as
 * as parameters, now they are static local to this file,
 * next they will moved to the sparse storage file when 
 * df08 gets stripped out of here.
 */
/*------------------------------------------------*/
/* k3: working array in 6000 for k and index of k */
/* k3[nBlocks+1]                                  */
static int __k3size;
static int * __k3;

/*  inside/outside angle at each block vertex     */
/* u[vertexCount+1][3]                            */
static int __anglesize1;
static int __anglesize2;
static double ** __angles;




void 
initContactTempArrays(int numblocks, int vertexCount)
{
  /*------------------------------------------------*/
  /* k3: index of a    for lower triangle k 6000    */
  /* k3: reserved positions for non-zero storage    */
  /* k3[nBlocks*40+1]                               */
   __k3size=FORTY*numblocks+1;
   __k3=(int *)calloc(1,sizeof(int)*__k3size);

  /*  inside/outside angle at each block vertex     */
  /* u[vertexCount+1][3]                            */
   __anglesize1=vertexCount+1;
   __anglesize2=3;
   __angles = DoubMat2DGetMem(__anglesize1, __anglesize2);
}  /* close initStorageTempArrays() */

void
freeContactTempArrays(void)
{

   if (__k3)
   {
      free(__k3);  
      __k3 = NULL;
   }

   if (__angles)
   {
      free2Darray((void **)__angles, __anglesize1);
      __angles = NULL;
   }

}  /* close freeStorageTempArrays() */

/* 
   m (contacts) is a an array that tracks vertices 
   that are in contact with each other:
             i2
             \    /
              \  /
               \/ i1
          j1 ---------- j2
             \
              \
               \
*/
/* Contains subfunctions df04, df05, df06, df07.
 * Contact finding takes a list of blocks, finds the 
 * number of vertex-vertex (v-v) or vertex-edge (v-e) contacts 
 * possible in the forthcoming time step, then determines
 * which of those contacts are possible using the corner
 * reference diagram, establishes locks (?), determines
 * length of contact for friction and cohesion computation,
 * then determines the number of block-to-block contacts
 * using the actual v-e/v-v  count (from df05).  Contact 
 * finding is described in Shi 1988, Chapter 4, pp. 139-188
 * (for df04, df05, df06, df07).  
 */
/* FIXME: Try and make c0 go local to this file, and rename 
 * the other instance of c0 for friction forces.
 */
void 
findContacts(Geometrydata * GData, Analysisdata * AData, Contacts * ctacts,
                    int * kk, int * k1, int ** n, double ** c0)
{

   clock_t start, stop;
   int * nn0 = GData->nn0;

   int ** contacts = get_contacts(ctacts);
   int ** locks = get_locks(ctacts);
   int ** contactindex = get_contact_index(ctacts);
   double ** contactlength = get_contact_lengths(ctacts);
   int ** prevcontacts = get_previous_contacts(ctacts);


   start = clock();


  /* Compute 2 angles: one angle is the angle wrt the
   * x axis in standard position, the other the interior
   * angle between 2 sides of a polygon (block).
   */
   computeVertexAngles(GData,__angles);
   //printAngles(GData, angles, " after angl()");
   //printVertices(GData, GData->vertices, GData->vindex, "Main loop");
  /* FIXME: Previous contacts are saved for what reason?
   */
  /* The "locks" array gets zeroed out in savePreviousContacts() */
   savePreviousContacts(GData,AData,contacts,locks,prevcontacts,contactlength);
   //printM2(m2, " after savePrevContacts");
  /* Find contacts using distance criteria. This function 
   * appears to act as a filter for df05, which finds 
   * using angle criteria.  df04 does set some of the 
   * array indexing. df04 sets the contact indexing matrix
   * that df05 uses to traverse over each possible contact.
   */
   df04(GData,AData,kk,__k3,c0,__angles);
   //printKK(kk, " df04");
   //printK3(k3, " df04");
  /* Find contacts by angle criteria.  df05 seems to be where
   * most of the nitty gritty of contact finding actually 
   * happens.  This function could actually *return* contacts
   * array.
   */
   df05(GData,AData,contacts,kk,__k3,nn0,__angles);
  /* Establish contacts between rockbolted blocks. */
   //if (GData->nBolts > 0)
   //{
   //   rockboltContacts(Geometrydata *bd, Analysisdata *ad, int *kk, int *k3, 
   //   double **c0, double **u);
   //}
  /* FIXME: Contact transfer.  What does this mean?
   */
  /* See section 3.3 of the manifold report (page 39). 
   * df06() is now in a file called "transfercontacts.c"
   */
   df06(GData,AData,contacts, locks, prevcontacts, 
        contactindex,contactlength,kk, __k3,nn0);


  /* FIXME (add more comments): Contact initialization.  
   * df07 appears to mostly compute the length of the 
   * contacts found in df05, and set the open/close
   * flags also.
   */
   df07(GData,AData, contacts, locks,contactlength,kk);
   //printContactLengths(GData, contactlength);

   setInitialLocks(GData, AData, contacts, locks);

   stop = clock();
 
   //assert(stop != start);

   DLog->contact_runtime += (stop - start);
   //if (DLog->contact_runtime == 0)  exit(0);

}  /* close findContacts() */

/* What the code appears to do is to store the vertex angle in
 * matrix `u'.  The vertex angle is the interior angle formed
 * by adjacent sides intersecting to form a vertex.
 *
 * Appears that u[][1] stores the angle in standard
 * position, and u[][2] stores the interior angle between the 
 * two sides.
 */
/* FIXME: This function should go into geometrydata.c and 
 * be a member of a block class.
 */
/**************************************************/
/* angl:                                     0013 */
/**************************************************/
/* i  x1  y1  c1  d1  d2  d3  i1  i2  u[][]       */
void 
computeVertexAngles(Geometrydata *bd, double **angles)
{
   int block;
   int j;
   double c1;
   double d1, d2, d3;
   double	dd = 3.1415926535/180.0;
   int nBlocks = bd->nBlocks; 
   double ** vertices = bd->vertices;
   int ** vindex = bd->vindex;
  /* i1, i2 local in this function, used to track index of 
   * block vertices (vertices matrix) stored in vindex.
   * i1 and i2 should probably be named "start" and "stop"
   * respectively.
   */
   int startIndex;
   int stopIndex;
   double delta_x;
   double delta_y;
      
  /* Time for some real comments, like what exactly this piece
   * of code does, why it does it, and where is it referenced 
   * in the literature.
   */
   for (block=1; block<=nBlocks; block++)
   {
      startIndex=vindex[block][1];
      stopIndex=vindex[block][2];
         
     /* u[j][1]: direction angle of edge (j, j+1), where j and j+1
      * are vertices taken CCW around each block.  The way this 
      * works is that for n vertices, start with the angle from
      * nth vertex to the first vertex.  The initial angle
      * (following loop) computes this ray considering the 
      * origin at the nth vertex and the angle in standard 
      * position.  TODO: Draw up example in text, derive an 
      * algorithm for this.
      */
      for (j=startIndex-1; j<=stopIndex; j++)
      {
        /* Example: vertices[j+1][X] - vertices[j][X];  */
         delta_x=vertices[j+1][1]-vertices[j][1];
         delta_y=vertices[j+1][2]-vertices[j][2];
        /* Conjecture that the .0000001 mitigates
         * division by zero, which could be trapped 
         * if MS implemented IEEE arithmetic.  There is 
         * a distinct possibility that an angle very close
         * to 90 could produce an erroneous result, that is,
         * an angle not between 0 and 360 inclusive.
         */
         c1=fabs(delta_x)+.0000001;
         d1=atan(delta_y/c1)/dd;  /* d1 is in degree, dd does the conversion. */
        /* This is the heart of this algorithm.  
         */
         if (delta_x<0)  
            d1  = 180-d1;
         if (d1<0)  
            d1 += 360;
        /* We use an assert to catch any problems that might
         * pop up because of angles close to +- 90.
         */
         assert( d1 >= 0 && d1 <= 360);
         if ( d1 <= 0  && d1 >= 360)
         {
            /* FIXME: Deal with errorfiles somehow, maybe pass an open
             * file pointer through the analysis data struct.
             */
            //fprintf(fp.errorfile,"From angl(): vertex angle out of range: %f\n", d1);
         }
         angles[j][1]=d1;
      }  /*  j  */
         
     /* That is, j = i2 and we store a copy of the first angle
      * u[i1][1] in the next slot forward: i2+1.  
      */
      angles[stopIndex+1][1]=angles[startIndex][1];
         
     /* u[j][2]: angle j */
     /* This computations should be moved to inside the 
      * previous loop.
      */
      for (j=startIndex; j<=stopIndex; j++)
      {
         d1=angles[j][1];
         d2=angles[j-1][1]-180;
         if (d2 < 0)  
            d2 += 360;
         d3=d2-d1;
         if (d3 < 0)  
            d3 += 360;
         angles[j][2]=d3;
      }  /*  j  */
         
      angles[startIndex-1][2]=angles[stopIndex][2];
      angles[stopIndex+1][2]=angles[startIndex][2];
   }  /*  block  */

}  /*  Close computeVertexAngle() */
   


/* The positions of previous contacts need to be saved.
 * This code used to reside in df04() but was removed in 
 * the interest of making the code easier to understand.
 * Note that MMM gravity code shows an locks initialization
 * block invoked on the first time step only, but that code
 * did not make it into the Windows version.
 */
void
savePreviousContacts(Geometrydata *bd, Analysisdata *ad, int **m, 
                     int **locks, int **m2, double **o)
{
  /* Save contact positions of previous step  
   * The "m2" array appears to be previous contact
   * positions.
   */
   int i, j;
  
  /* If we are in the first time step, do nothing and exit 
   * this function.  Else, transfer the contacts.  MMM gravity 
   * code had an extra block in here to initialize locks array
   * to all zeros for the first time step.  Might be a good 
   * idea to put that back in.
   */
  /* Test run of having the init loops back in.  If this works
   * out, make a function call here.  FIXME: This code is
   * probably meaningless.
   */
#if JHLJHLKJHKLJHKLJHKLJHKLJ
   if  (ad->currTimeStep == 1)
   {
     /* Zero out the locks array.
      * FIXME: what is locks and where is it set?
      * FIXME: If this init loop is needed, unroll it instead of 
      * making a memset call.
      */
      for (i=1; i<= bd->nContacts; i++)
      {
         for (j=0; j<=4; j++)
         {
            locks[i][j]=0;
         }  /*  j  */
      }  /*  i  */
      
      //return; /* do nothing for now */
   }
#endif

   /* FIXME: Remove the dependence on the gravity time step, 
    * and then move the return into the previous block.
    */
   //if ( ad->gravity->gravTimeStep == 1 || (ad->currTimeStep == 1 && ad->analysistype >= 0) )
   //if (ad->currTimeStep == 1) 
   //   return;
   //else /* not first time step */
   //{
     /* Save a copy of previous vertices and reference lines. */
      for (i=1; i<= bd->nContacts; i++)
      {
         for (j=0; j<= 6; j++)
         {
            m2[i][j]=m[i][j];
         }  /*  j  */
      }  /*  i  */
 
   
     /* Save contact displacements of previous step.  
      * For each contact in previous step... 
      */
      for (i=1; i<= bd->nContacts; i++)
      {
         locks[i][3]=locks[i][2];  /* copy of contact flag */
         o[i][4] = o[i][0];  /* copy of normal penetration */
         o[i][5] = o[i][1];  /* copy of shear movement */
         o[i][6] = o[i][2];  /* copy of copy of contact edge ratio ?/? */
      }  /*  i  */
 
     /* release tension or shear locks:
      * for each contact of previous step,
      * if contact was not locked at end of previous 
      * time step, release contact lock.
      */
      for (i=1; i<= bd->nContacts; i++)
      {
         if (locks[i][2] != 2)  
            locks[i][0]=0;
      }  /*  i  */
     /* save the number of contacts in the previous step. */
      ad->nPrevContacts=bd->nContacts;
   //}
    
}  /*  Close savePreviousContacts()  */




/**************************************************/
/* df04: contact finding by distance criteria     */
/* FIXME: Remove bbox array from this argument list,
 * make it local to this contact file.
 */
void df04(Geometrydata *bd, Analysisdata *ad, int *kk, int *k3, 
          double **bbox_c0, double **u)
{
   int block;
   int startIndex;
   int stopIndex;
  /* Loop counters over blocks. */
   int ii, jj;
   int i, j;
  /* x, y coords of normals and vectors of segments. */
   double p1, p2, p3, p4, p5, p6, p7, p8;
  /* x, y coords of endpoints of segment on iith block */
   double x1, y1, x2, y2;
  /* x, y coords of endpoints of segment on jjth block  */
   double x3, y3, x4, y4;
  /* x, y midpoint coords of segments on iith, jjth block. */
   double x5, y5, x6, y6;
  /* Distance of segments on iith, jjth blocks */
   double a1, a2;
  /* Inner products of inner unit normals with vertex to 
   * midpoint vector of adjacent blocks.
   */
   double d1, d2, d3, d4;

   int nBlocks = bd->nBlocks;
   double ** vertices = bd->vertices;
   int ** vindex = bd->vindex;
  /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
   //double d0 = ad->constants->norm_extern_dist;
   double d0 = constants_get_norm_extern_dist(ad->constants);
   //double d9 = ad->constants->norm_pen_dist;
   double d9 = constants_get_norm_pen_dist(ad->constants);

  /* (GHS: c0[][] xl xu yl yu)   */
  /* What c0 does is find a box that 
   * completely encloses each block.  This is used
   * in conjunction with the next block of code to 
   * determine whether 2 blocks are close enough to 
   * investigate a contact relationship.
   */
  /* FIXME: Remove the bounding box code and turn it 
   * into a function that can be tested given an array
   * of vertices.
   */
   for (block=1; block<= nBlocks; block++)  
   {
      startIndex=vindex[block][1];
      stopIndex=vindex[block][2];
 
      bbox_c0[block][1]=vertices[startIndex][1];
      bbox_c0[block][2]=vertices[startIndex][1];
      bbox_c0[block][3]=vertices[startIndex][2];
      bbox_c0[block][4]=vertices[startIndex][2];
     /* j is probably a vertex counter. */
      for (j=startIndex; j<= stopIndex; j++)
      {  
        /* (GHS: c0[][] xl xu yl yu)   */
        /* find lower x */
         if (bbox_c0[block][1]>vertices[j][1])  
            bbox_c0[block][1]=vertices[j][1];
        /* find upper x */
         if (bbox_c0[block][2]<vertices[j][1])  
            bbox_c0[block][2]=vertices[j][1];
        /* find lower y */
         if (bbox_c0[block][3]>vertices[j][2])  
            bbox_c0[block][3]=vertices[j][2];
        /* find upper y */
         if (bbox_c0[block][4]<vertices[j][2])  
            bbox_c0[block][4]=vertices[j][2];
      }  /*  j  */
   }  /*  block  */

  /**************************************************/
  /* contact finding by distance criteria           */
  /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
  /* d0 : normal external    distance for contact   */
  /* d9 : normal penetration distance for contact   */
  /* m6 : number of contact vertices                */
   ad->nCurrentContacts=0;
  
  /* The four level loop should be pretty easy to 
   * break up the goto statements.  They all seem to be 
   * performing a continue operation.
   */
  /* if two boxes of blocks are near  */
   for (ii=1;    ii<= nBlocks-1; ii++)
   {
      for (jj=ii+1; jj<= nBlocks;   jj++)
      {
        /* d0 is contact distance parameter derived from a 
         * user controlled value read from the analysis input 
         * file (g2), and initialized in the "initializeStuff"
         * function.  c0 appears to be a square that completely 
         * encloses each block.  The following 4 lines appear to 
         * implement determination of contact by distance
         * given by Equation 4.3 in Shi, 1993, p. 82.  
         * Basically, if the bounding boxes of blocks i and j
         * do not overlap, no contact, check next block.
         */
        /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
        /* FIXME: Baecher model may crash here.
         */        
        /* (GHS: c0[][] xl xu yl yu)   */
         if (bbox_c0[ii][2]+d0 < bbox_c0[jj][1]) /* xu_i < xl_j */
            continue; 
         if (bbox_c0[jj][2]+d0 < bbox_c0[ii][1]) /* xu_j < xl_i */
            continue; 
         if (bbox_c0[ii][4]+d0 < bbox_c0[jj][3]) /* yu_i < yl_j */
            continue; 
         if (bbox_c0[jj][4]+d0 < bbox_c0[ii][3]) /* yu_j < yl_i */
            continue;         
        /* Next, traverse over vertices of the iith and 
         * jjth blocks.
         */
         for (i=vindex[ii][1]; i<=vindex[ii][2]; i++)
         {
            for (j=vindex[jj][1]; j<=vindex[jj][2]; j++)
            {
              /* Construct endpoints of line segments between the 
               * ith-(i+1)th vertex of the iith block,
               * and the jth and (j+1)th vertex of the 
               * jjth block.
               */
               x1=vertices[i  ][1];
               y1=vertices[i  ][2];
               x2=vertices[i+1][1];
               y2=vertices[i+1][2];
               x3=vertices[j  ][1];
               y3=vertices[j  ][2];
               x4=vertices[j+1][1];
               y4=vertices[j+1][2];

              /* v1 - v3 contact    v1-v3 v1 concave v3 concave */
              /* if true, pt 1 to pt 3 contact not possible, so check
               * some other kind of contact.
               */
              /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
               if ((x1-x3)*(x1-x3)+(y1-y3)*(y1-y3)> d0*d0) 
               {
                   x5=(x1+x2)*.5;  /* x midpoint of 1-2 */
                   y5=(y1+y2)*.5;
                   x6=(x3+x4)*.5;  /* x midpoint of 3-4 */
                   y6=(y3+y4)*.5;

                  /* p12: inner  normal of line 12     */
                  /* p34: vector along     line 12     */
                  /* p56: inner  normal of line 34     */
                  /* p78: vector along     line 34     */
/*                       
                   /|\ P56
          \ (x6,y6) |         /
  (x3,y3)  `o-------o------o'  (x4,y4)
          
 (x2,y2)    o-------o-------o  (x1,y1)
           /        |         \
         / (x5,y5) \|/          \
                       P12 
*/                      
                  /* a1 := distance between adjacent vertices on
                   * the iith block.
                   */
                   a1=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
                   p1=(y1-y2)/a1; // x coord of unit inner normal 
                   p2=(x2-x1)/a1; // y coord of unit inner normal
                   p3=(x2-x1)/a1; // x coord of unit vector
                   p4=(y2-y1)/a1; // y coord of unit vector
                  /* a2 := distance between adjacent vertices on
                   * the jjth block.
                   */
                   a2=sqrt((x4-x3)*(x4-x3)+(y4-y3)*(y4-y3));
                   p5=(y3-y4)/a2;  // x coord of inner normal
                   p6=(x4-x3)/a2;  // y coord of inner normal
                   p7=(x4-x3)/a2;  // x coord of position vector
                   p8=(y4-y3)/a2;  // y coord of position vector

                  /* Inner products of inner unit normals with 
                   * vertex to midpoint vectors of adjacent blocks.
                   */
                   d1=p5*(x1-x6)+p6*(y1-y6);
                   d2=p7*(x1-x6)+p8*(y1-y6);
                   d3=p1*(x3-x5)+p2*(y3-y5);
                   d4=p3*(x3-x5)+p4*(y3-y5);

                  /* v1 - e43 contact  */
                  /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
                  /* d0  d9  in cons   */
                  /* These gotos can eventually be eliminated by moving
                   * the a405 labeled code up along with a few judicious
                   * continue statements.
                   */

                   if ((x1-x4)*(x1-x4)+(y1-y4)*(y1-y4)<= (d0*d0) || 
                          (fabs(d2) > a2/2)  ||  (d1 < -d0 || d1 > d9) )
                
                   {
/* This is at least a dozen lines of duplicated code here.
 */
                     /* j=v3 concave  v_i-e_j+1 e_j  v_i-e_je_i-j      */
    
                     /* v3 - e21 contact  */
                     /* vertices 2 and 3 are close together, pt to pt 
                      * contact possible.
                      */
                     /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
                      if ((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2)<=d0*d0) 
                         continue; 
                     /* vertex 3 too far from edge 12 for contact,
                      * where distance measured parallel.
                      */ 
                      if (fabs(d4) > a1/2) 
                         continue; 
                     /* vertex 3 too far from edge 12 for contact,
                      * where distance is measured normal, or
                      * too much penetration.
                      */
                     /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
                      if (d3 < -d0 || d3>d9) 
                         continue;

                     /* Most likely the block and contact info
                      * is here.
                      */
                     /* This appears to be code for recording vertex-vertex
                      * contacts between blocks ii and jj.
                      */
                      ad->nCurrentContacts+=1; /* increment number of contacts */
                     /* According to some notes from MMM, kk[i] < 0 denotes
                      * pt/edge contact, and kk[i] > 0 means pt/pt contact.
                      */
                      kk[ad->nCurrentContacts] = (-1)*j;  /* pt-pt contact */
                     /* MMM: vertex j is in contact with vertex i of block ii */
                      k3[ad->nCurrentContacts] = i;
                      continue;

                   }  else  {
                     
                      ad->nCurrentContacts+=1;
                      kk[ad->nCurrentContacts]  = (-1)*i;
                      k3[ad->nCurrentContacts] =      j;

                     /* j=v3 concave  v_i-e_j+1 e_j  v_i-e_je_i-j      */
                     /* v3 - e21 contact  */
                     /* vertices 2 and 3 are close together, pt to pt 
                      * contact possible.
                      */
                     /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
                      if ((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2)<=d0*d0) 
                         continue;
                     /* vertex 3 too far from edge 12 for contact,
                      * where distance measured parallel.
                      */ 
                      if (fabs(d4) > a1/2) 
                         continue; 
                     /* vertex 3 too far from edge 12 for contact,
                      * where distance is measured normal, or
                      * too much penetration.
                      */
                     /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
                      if (d3 < -d0 || d3>d9) 
                         continue; 

                     /* Most likely the block and contact info
                      * is here.
                      */
                      ad->nCurrentContacts+=1; /* increment number of contacts */
                     /* According to some notes from MMM, kk[i] < 0 denotes
                      * pt/edge contact, and kk[i] > 0 means pt/pt contact.
                      */
                      kk[ad->nCurrentContacts] = (-1)*j;
                     /* MMM: vertex j is in contact with edge of block ii */
                      k3[ad->nCurrentContacts] = i;
                      continue;
                  }
               }

              /* i=v1 concave  v_j-e_i+1 e_i  v_j-e_ie_i-1      */
               if (u[i][2] < 180+.0001) 
               {
                  if (u[j][2] < 180+.0001) 
                  {
                     ad->nCurrentContacts+=1;
                     kk[ad->nCurrentContacts]  = i;
                    /* FIXME: A very high penalty spring will
                     * crash this code here on the vaiont file.
                     */
                     k3[ad->nCurrentContacts] = j;
                     continue; 
                  }
                  ad->nCurrentContacts+=1;
                  kk[ad->nCurrentContacts]  = (-1)*i;
                  k3[ad->nCurrentContacts] =      j;
                  ad->nCurrentContacts+=1;
                  kk[ad->nCurrentContacts]  = (-1)*i;
                  k3[ad->nCurrentContacts] =    j-1;
                  if (j==vindex[jj][1])  
                  {
                     k3[ad->nCurrentContacts] = vindex[jj][2];
                  }
                  continue;
               }

              /* What does this stuff do?
               */
               ad->nCurrentContacts+=1;  /* was m6 */
               kk[ad->nCurrentContacts]  = (-1)*j;
               k3[ad->nCurrentContacts] =      i;
               ad->nCurrentContacts+=1;
               kk[ad->nCurrentContacts]  = (-1)*j;
               k3[ad->nCurrentContacts] =    i-1;
               if (i==vindex[ii][1])  
               {
                  k3[ad->nCurrentContacts] = vindex[ii][2];
               }
               continue;  
            }  /*  j  */
         }  /*  i  */
      }  /*  jj */
   }  /*  ii */

}  /* Close df04().  */



/**************************************************/
/* df05: contact finding by angle criteria        */
/**************************************************/
void df05(Geometrydata *bd, Analysisdata *ad, int **m, int *kk, int *k3, 
          int *nn0, double **u)
{
   int i, i0, i1, i2, i3;
   int j, j0, j1, j2, j3, j6;
   double a1, a2, b1, b2, d1, d2;
   double	dd = 3.1415926535/180.0;
   double e1, e2, e3, e4, e5;
   static double e6;
   //double e[7][7];
   double e[7][3];

   double ** vertices = bd->vertices;
   int ** vindex = bd->vindex;
   //double h1 = ad->constants->angle_olap;
   double h1 = constants_get_angle_olap(ad->constants);
   double y1, y2;
   double x1, x2;
   int nContacts = bd->nContacts;

  /* nContacts number of contacts m[]   */
   nContacts=0;

   for (i=1; i<= (ad->nCurrentContacts); i++)
   {
    /* vector i2i1 i2i3 j2j1 j2j3 rotate from x to y  */
    /* vertex in vindex[][1] vindex[][2] for contact transfer */
     i2=labs(kk[i]);
     j2=k3[i];
    /* nn0 records which block number a particular vertex 
     * is associated with. nn0 is initialized in its own 
     * function: blockNumberOfVertex().
     */
     i0=nn0[i2];
     j0=nn0[j2];

     i3=i2-1;

     if (i2==vindex[i0][1])  
        i3=vindex[i0][2];

     j3=j2-1;

     if (j2==vindex[j0][1])  
        j3=vindex[j0][2];

     i1=i2+1;

     if (i2==vindex[i0][2])  
        i1=vindex[i0][1];

     j1=j2+1;

     if (j2==vindex[j0][2])  
        j1=vindex[j0][1];
     
    /* angle from x to y of i2i1 i2i3 j2j1 j2j3       */
     e[1][1]=u[i2][1];
     e[2][1]=u[i3][1]-180;
     e[3][1]=u[j2][1];
     e[4][1]=u[j3][1]-180;
   
    /* v - e 180 angle j j2j1 j1j2 rotate from x to y */
     if (kk[i]<0)  
        e[4][1]=e[3][1]-180;
   
     for (j=1; j<=  4; j++)
     {
        if (e[j][1]<0)  
           e[j][1] += 360;
     }  /*  j  */
     
     
    /* e1 angle i2    e2 angle j2                     */
     e1=e[2][1]-e[1][1];
     e2=e[4][1]-e[3][1];
     if (e1<0)  e1+=360;
     if (e2<0)  e2+=360;
    
    /*------------------------------------------------*/
    /* e3=180 choose i1i2 arrange 2 entrances on i2   */
     e3=e[3][1]-e[1][1];
     e4=e[4][1]-e[2][1];
     if (e3<0)  e3+=360;
     if (e4<0)  e4+=360;
     e5=360-e3-e2;
     e6=e3-e1;
	   /* printf("\ne1 = %f\te2 = %f\te5 = %f\te6 = %f\n", e1, e2, e5, e6); */
	   
    /* angle e1e2 > e3e4 angle penetration check      */
     e[1][2]=e[1][1];
     e[2][2]=e[1][1]+e1;
     e[3][2]=e[3][1];
     e[4][2]=e[4][1];
     e[0][2]=e[3][1]+0.5*e2;
    /*
     if (e1>e2) 
        goto a502;
     */
     if (e1 <= e2)
     {
        e[1][2]=e[3][1];
        e[2][2]=e[3][1]+e2;
        e[3][2]=e[1][1];
        e[4][2]=e[2][1];
        e[0][2]=e[1][1]+0.5*e1;
     }
//a502:
    /* angle overlapping check for contacts           */
    /* e[0][2] is for two side contacts               */
    /* h1 : angle overlapping degrees   for contact 3 */
     if (e[1][2]+h1<e[0][2]     && e[0][2]    <e[2][2]-h1) //goto a501;
        continue;
     if (e[1][2]+h1<e[3][2]     && e[3][2]    <e[2][2]-h1) //goto a501;
        continue;
     if (e[1][2]+h1<e[4][2]     && e[4][2]    <e[2][2]-h1) //goto a501;
        continue;
     if (e[1][2]+h1<e[0][2]+360 && e[0][2]+360<e[2][2]-h1) //goto a501;
        continue;
     if (e[1][2]+h1<e[3][2]+360 && e[3][2]+360<e[2][2]-h1) //goto a501;
        continue;
     if (e[1][2]+h1<e[4][2]+360 && e[4][2]+360<e[2][2]-h1) //goto a501;
        continue;
     //if (kk[i] > 0) 
     //   goto a503;
     if (kk[i] <= 0)
     {
       /* (GHS: v - e  vertix i2  edge j1j2)             */
       /* m[][0]=0 v-e  m[][0]=1  v-v  m[][6]=-1 v-e     */
       /* m[][4] m[][5] 2 side of i2 for lock legth      */
       /* If the m array seg faults, check to see if 
        * reducing the max displacement will reduce the 
        * number of contacts.
        */
        nContacts+=1;
        m[nContacts][0]=0;
        m[nContacts][1]=i2;
        m[nContacts][2]=j1;
        m[nContacts][3]=j2;
        m[nContacts][4]=0;
        m[nContacts][5]=0;
        m[nContacts][6]=0;
        if (e5<h1)  
           m[nContacts][4]=i1;
        if (e6<h1)  
           m[nContacts][5]=i3;
        //goto a501;
        continue;
     }

    /* v - v vertex i2  vertex j2      i2<180  j2<180 */
    /* reduce to v-e when adjacent lines are parallel */
 
//a503:;
      if (e5<h1) //goto a504;
{
      /*------------------------------------------------*/
      /* contact edge of e5 as entrance  m[][6] >= 0    */
      /* no angle=180  generally no e6 contact          */
 
//a504:
     /* Save point/edge contacts.  According to MMM, this 
      * is equivalent to a parallel edge/edge contact.
      */
      nContacts+=1;  /* increment the number of contacts */

     /*
      if(__contactsize1 <= nContacts)
      {
         realloc(m, __contactsize1 = 2*nContacts);
      }
      */

      m[nContacts][0]=0;  /* flag: pt/edge contact */
      m[nContacts][1]=i2; /* number of contacting vertex */
      m[nContacts][2]=j2; /* endpoint 1 of contacted ref line j->j-1 */
      m[nContacts][3]=j3; /* endpoint 2 of contacted ref line j->j-1*/
      m[nContacts][4]=j2; /* contacting vertex */
      m[nContacts][5]=i1; /* endpoint 1 i->i+1 */
      m[nContacts][6]=i2; /* endpoint 2 i->i+1 */
     /* Add another pt/edge contact if other contact edges 
      * are approximately parallel.
      */
      if (e6<h1) 
      //goto a505;
      {
        /*------------------------------------------------*/
        /* contact edge e6 of as entrance  m[][6] >= 0    */
        /* no angle=180  generally no e5 contact          */
 
//a505:
         nContacts+=1;
         m[nContacts][0]=0;
         m[nContacts][1]=j2;
         m[nContacts][2]=i2;
         m[nContacts][3]=i3;
         m[nContacts][4]=i2;
         m[nContacts][5]=j1;
         m[nContacts][6]=j2;
         //goto a501;
         continue;
      }  else  {
         //goto a501;
         continue;
      }
  }
      if (e6<h1) //goto a505;
      {
//a505:
         nContacts+=1;
         m[nContacts][0]=0;
         m[nContacts][1]=j2;
         m[nContacts][2]=i2;
         m[nContacts][3]=i3;
         m[nContacts][4]=i2;
         m[nContacts][5]=j1;
         m[nContacts][6]=j2;
         //goto a501;
         continue;
      }
//      goto a506;

     /*------------------------------------------------*/
     /* v - v  i2<180  j2<180                          */
     /* entrance line j1j2 or i1i2   if e3=180 large y */
 
//a506:
      a1=0.5*e1+e[1][1];
      a2=0.5*e2+e[3][1];
      d1=fabs(sin(dd*a1));
      d2=fabs(sin(dd*a2));
      nContacts+=1;
      m[nContacts][0]=1;
      m[nContacts][1]=i2;
      m[nContacts][2]=j1;
      m[nContacts][3]=j2;
      b1=e[3][1];
      if ( !(e3<=180-.3*h1 || (e3<=180+.3*h1 && d2>=d1)) ) 
         //goto a507;
      {
         m[nContacts][1]=j2;
         m[nContacts][2]=i1;
         m[nContacts][3]=i2;
         b1=e[1][1];
      }

      /*------------------------------------------------*/
      /* v - v  i2<180  j2<180                          */
      /* entrance line i2i3 or j2j3   if e3=180 large y */
//a507:;
      m[nContacts][4]=j2;
      m[nContacts][5]=i2;
      m[nContacts][6]=i3;
      b2=e[2][1];
      if (e4<=180-.3*h1||(e4<=180+.3*h1&&d1>=d2)) 
         //goto a508;
      {
// a508:;
         d1=fabs(sin(dd*b1));
         d2=fabs(sin(dd*b2));
         if (d1>=d2) //goto a501;
            continue;
         for (j=1; j<= 3; j++)
         {
            j6         = m[nContacts][j];
            m[nContacts][j]   = m[nContacts][j+3];
            m[nContacts][j+3] = j6;
         }  /*  j  */
         continue;
      }

      m[nContacts][4]=i2;
      m[nContacts][5]=j2;
      m[nContacts][6]=j3;
      b2=e[4][1];
      /*------------------------------------------------*/
      /* in symmetry mesh make first entrance symmetry  */
      /* if 1st entrace locked initialy have symmetry   */

//a501:;
   }  /*  i  */

/* This could probably be moved to its own function.
 */
  /**************************************************/
  /* angle < 180  v - v     set i1 in between i1 i2 */
  /* switch m[][j] m[][j+3] contact position change */
   for (i=1; i<= nContacts; i++)
   {
      if (m[i][0]==1 || m[i][6]==0) 
         //goto a509;
      {
         bd->nContacts = nContacts;
         continue;
      }
      i1 = m[i][1];
      i2 = m[i][2];
      i3 = m[i][3];
      x1 = vertices[i2][1]-vertices[i1][1];
      y1 = vertices[i2][2]-vertices[i1][2];
      x2 = vertices[i3][1]-vertices[i1][1];
      y2 = vertices[i3][2]-vertices[i1][2];
      d1 = (x1*x2 + y1*y2);
      if (d1 <= 0) 
         //goto a509;  
      {
         bd->nContacts = nContacts;
         continue;   // (why?)
      }
      m[i][1] = m[i][4];
      m[i][2] = m[i][5];
      m[i][3] = m[i][6];
      m[i][4] = i1;
      m[i][5] = i2;
      m[i][6] = i3;
//a509:
//  ;
//  bd->nContacts = nContacts;
	  }  /*  i  */

}  // Close df05()





/**************************************************/
/* df07: contact initialization                   */
/**************************************************/
void df07(Geometrydata *gd, Analysisdata *ad, int **contacts, int **locks, 
          double **c_length, int *kk)
{
  /* i is index for the contact number. */
   int contact;  // was i
   int i1,i2,i3,i4;
   double ** vertices = gd->vertices;
   //double openclose = ad->constants->openclose;
   double openclose = constants_get_openclose(ad->constants);
  /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
   //double d0 = ad->constants->norm_extern_dist;
   double d0 = constants_get_norm_extern_dist(ad->constants);
   //double w0 = ad->constants->w0;
   double w0 = constants_get_w0(ad->constants);
   int nContacts = gd->nContacts;
   Gravity *grav = ad->gravity;

  /* Used to index locks array */
   //const int OPEN = 0;
   //const int LOCKED = 2;
   //const int SWAP = 3;
   //const int PREVIOUS = 1;
   //const int CURRENT = 2;

  /* Used for indexing contacts array */
   const int TYPE = 0;
   const int VE = 0;
   const int VV = 1;

  /* contact length and contact position  currTimeStep=1 currTimeStep>1 */
   //    if (k02 == 1)  printf("--- 7000--- \n");
   for (contact=1; contact<= nContacts; contact++)
   {
      //if (ad->gravity->gravTimeStep == 1 || (ad->currTimeStep==1 && ad->analysistype >= 0)) // || (ad->gravTimeStep==1)) 
     /* FIXME: Check to see if this is redundant */      
      if (ad->cts==1) // && ad->analysistype >= 0) // || (ad->gravTimeStep==1)) 
         locks[contact][0] = 0;   // Tension flag?

      if (contacts[contact][TYPE] ==  VV) 
         continue; /* goto a701; */

     /* Everything past here is either pt-edge or edge-edge contact.
      * No lengths are associated with v-v contacts.
      */
      if (contacts[contact][6] ==  0) 
      {  /* Code below gets duplicated later */
         if (contacts[contact][4] == 0) 
         {

           /* (GHS: original v-e  m[][5]) */
            if (contacts[contact][5] == 0) 
            {
              /* for case no contact edge   m[i][4]=0 m[i][5]=0 */
               i1 = contacts[contact][1];  // penetrating vertex
               i2 = contacts[contact][2];  // endpoint 2 of contacted ref line 
               i3 = contacts[contact][3];  // endpoint 3 of contacted ref line
               i4 = 0;  // contacted edge is single pt only.
               proj(vertices,w0,c_length,kk,contact,i1,i2,i3,i4);
               continue;
            }

            i1 = contacts[contact][1];
            i2 = contacts[contact][2];
            i3 = contacts[contact][3];
            i4 = contacts[contact][5];

            //if (ad->gravity->gravTimeStep==1 || (ad->currTimeStep==1 && ad->analysistype >= 0))      
            if (ad->cts==1) // && ad->analysistype >= 0))      
               locks[contact][0] = 1;  // Tension?

            proj(vertices,w0,c_length,kk,contact,i1,i2,i3,i4);
            continue;  /* goto a701; */
         }

         i1 = contacts[contact][1];
         i2 = contacts[contact][2];
         i3 = contacts[contact][3];
         i4 = contacts[contact][4];

         //if (ad->gravity->gravTimeStep==1 || (ad->currTimeStep==1 && ad->analysistype >= 0))      
         if (ad->cts==1)// && ad->analysistype >= 0)      
            locks[contact][0] = 1;

         proj(vertices,w0,c_length,kk,contact,i1,i2,i3,i4);
         continue;  /* goto a701; */
      }

      if (contacts[contact][6] > 0) 
      {
         i1 = contacts[contact][1];
         i2 = contacts[contact][2];
         i3 = contacts[contact][3];
        /* If we have an edge-edge contact, need the second endpt of
         * edge that is in contact.
         */ 
         if (i4==i1) 
            i4 = contacts[contact][6];
         else 
            i4 = contacts[contact][5];

        /* FIXME: Explain why initial time step does not need length
         * computed.
         */
         //if (ad->gravity->gravTimeStep==1 || (ad->currTimeStep==1 && ad->analysistype >= 0))      
         if (ad->cts==1) // && ad->analysistype >= 0)      
            locks[contact][0] = 1;

         proj(vertices,w0,c_length,kk,contact,i1,i2,i3,i4);
         continue;
      }

     /* (GHS: original v-e  m[][4]) */
     /* Fall through */
      if (contacts[contact][4] != 0)
      {
         i1 = contacts[contact][1];
         i2 = contacts[contact][2];
         i3 = contacts[contact][3];
         i4 = contacts[contact][4];

         //if (ad->gravity->gravTimeStep == 1 || (ad->currTimeStep==1)) //  && ad->analysistype == 1))
         if (ad->cts==1) //  && ad->analysistype == 1))
            locks[contact][0] = 1;

         proj(vertices,w0,c_length,kk,contact,i1,i2,i3,i4);
         continue;  /* goto a701; */
      }
      else 
      {  
        /* (GHS: original v-e  m[][5]) */
         if (contacts[contact][5] == 0) 
         {
           /* for case no contact edge   m[i][4]=0 m[i][5]=0 */
            i1 = contacts[contact][1];
            i2 = contacts[contact][2];
            i3 = contacts[contact][3];
            i4 = 0;  /*  pt-edge instead of vertex-edge contact */
            proj(vertices,w0,c_length,kk,contact,i1,i2,i3,i4);
            continue;
         }

         i1 = contacts[contact][1];
         i2 = contacts[contact][2];
         i3 = contacts[contact][3];
         i4 = contacts[contact][5];

        /* FIXME: Explain why the first time step sets this value. */
         if (ad->cts==1)      
            locks[contact][0] = 1;

         proj(vertices,w0,c_length,kk,contact,i1,i2,i3,i4);
         continue;  /* goto a701; */
      }
      
     /* (GHS: for case no contact edge   m[i][4]=0 m[i][5]=0) */
     /* (GHS: angle<180 v-v e5 e6<h1                  m[][4]) */
     /*  Fall through code.  Might be able to eliminate this */
      i1 = contacts[contact][1];
      i2 = contacts[contact][2];
      i3 = contacts[contact][3];

      if (i4 == i1) 
         i4 = contacts[contact][6];
      else 
         i4 = contacts[contact][5];

     /* FIXME: Explain why first time step sets this value.  */
      //if (ad->gravity->gravTimeStep==1 || (ad->currTimeStep==1 && ad->analysistype >= 0))      
      if (ad->gravity->gravTimeStep==1 || (ad->cts==1 && ad->analysistype >= 0))      
         locks[contact][0] = 1;

      proj(vertices,w0,c_length,kk,contact,i1,i2,i3,i4);
/* a701:; */
   }  /*  i  */
      
}  /* close df07() */




void
setInitialLocks(Geometrydata * gd, Analysisdata * ad, int **contacts, int **locks)
{
   
  /* i is index ofr contact loop */
   int contact;  // was i
  /* j is helper index over VE or VV contact */
   int j;
  /* penetration distance */
   double pendist[3];  // was s[3]
  /* Indexes the location of the vertex coordinates */
   int ep1, ep2, ep3;  // was j1,j2,j3;
  /* x1,y1 is the penetrating vertex in the ith contact. */
   double x1,y1;
  /* x2 to y3 are the reference line vertices for the ith contact */
   double x2,x3,y2,y3;
   double reflinelength; //  was a1;
   double ** vertices = gd->vertices;
   //double openclose = ad->constants->openclose;
   double openclose = constants_get_openclose(ad->constants);
  /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
   //double d0 = ad->constants->norm_extern_dist;
   double d0 = constants_get_norm_extern_dist(ad->constants);
   int nContacts = gd->nContacts;

  /* Used to index locks array */
   const int OPEN = 0;
   const int LOCKED = 2;
   const int SWAP = 3;
   //const int PREVIOUS = 1;
   const int CURRENT = 2;

  /* Used for indexing contacts array */
   const int TYPE = 0;
   const int VE = 0;
   //const int VV = 1;


  /**************************************************/
  /* for currTimeStep=1 set initial open-close state  currTimeStep=1 */

  /*
   if (  ad->gravity->gravTimeStep > 1   || 
        (ad->currTimeStep > 1)           || 
        (ad->currTimeStep == 1           && 
         ad->analysistype<=0) 
      )
   */
   if ( ad->cts > 1 || (ad->cts==1  && ad->analysistype == 0) )
      return;  /* goto a706;  */

   for (contact=1; contact<= nContacts; contact++)
   {
      if (locks[contact][CURRENT] != LOCKED)  
         locks[contact][CURRENT] = 1;

      for (j=0; j<= contacts[contact][TYPE]; j++)
      {
        /* j1,j2,j3 are the vertex numbers, that is, the 
         * leading index into the vertex array, for the 
         * three endpoints associated with a contact.
         */
         ep1 = contacts[contact][j*3+1];
         ep2 = contacts[contact][j*3+2];
         ep3 = contacts[contact][j*3+3];

         x1 = vertices[ep1][1];
         y1 = vertices[ep1][2];
         x2 = vertices[ep2][1];
         y2 = vertices[ep2][2];
         x3 = vertices[ep3][1];
         y3 = vertices[ep3][2];

        /*------------------------------------------------*/
        /* j1 j2 j3 rotate from ox to oy = no penetration */
        /* nearly close = close  s[] penetration distance */
         reflinelength = sqrt((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2));
         pendist[j+1] = ((x2-x1)*(y3-y1)-(y2-y1)*(x3-x1))/reflinelength;

        /* d0 \equiv 2\rho in dissertation p. 142, figure 4.2, p. 143 */
         if (pendist[j+1]> openclose*d0 && locks[contact][CURRENT] != LOCKED)  
            locks[contact][CURRENT] = OPEN;
      }  /*  j  */

      /*------------------------------------------------*/
      /* (GHS: v - e  if close  set locking)            */
      if (locks[contact][CURRENT] == OPEN) 
         continue;  /* goto a707;  */

      if (contacts[contact][TYPE] == VE) 
         locks[contact][CURRENT] = LOCKED;

      if (contacts[contact][TYPE] == VE) 
         continue;  /* goto a707;  */

      /*------------------------------------------------*/
      /* (GHS: v - v if close choose shortest distance) */
      locks[contact][CURRENT] = 1;

      if (pendist[1] < pendist[2])  
         locks[contact][CURRENT] = SWAP;
      }  /*  loop over each contact, was i  */

}  /* close setInitialLocks() */


/**************************************************/
/* proj: projection of a edge to other edge  0015 */
/**************************************************/
/* In other words, compute the actual contact length
 * of one block edge on another.  This function is  
 * due for a rewrite, should take vertices, etc, and
 * return contact length.  Only called from df07()?
 */
/* i  i0   i1  i2  i3  i4  d1  o[][3]  vertices  m0[][2] */
/* j l d1 i0 s[] x1 y1 x2 y2 x3 y3                */
/* v1, v2, v3 are the vertex numbers implicated in
 * this contact.
 */
/* FIXME: c_length[][2] or c_length[][3] are what gets set for 
 * "return values."  There should be some way of rewriting this
 * function in a much cleaner way.
 */
void proj(double ** vertices, double domainscale, double **c_length, int *kk, 
          int i, int v1, int v2, int v3, int i4)
{
   int j;
   int ell;
   double reflinelength;  // was d1;
   //double ** vertices = bd->vertices;
   double s[5];  // s[0] is a temp value
   //double domainscale = ad->constants->w0;
   double y1, y2, y3;
   double x1, x2, x3;

  /* compute projection of p1 and p4 to edge p2p3 */
   x1   = vertices[v1][1];
   y1   = vertices[v1][2];
   x2   = vertices[v2][1];
   y2   = vertices[v2][2];
   x3   = vertices[v3][1];
   y3   = vertices[v3][2];
   s[2] = 0;
   s[3] = 1;

  /* contact position */
  /* FIXME: Replace the constant with a parameter that
   * can be controlled by the user.  The reason for 
   * using the parameter is to avoid dividing by zero
   * when determining the penetration distance (?).
   * Also, since there are various tolerances built into 
   * the contact finding code, this may be partly to 
   * compensate for those.
   * Add as "MinReflineFactor, min_refline_factor"
   */
   reflinelength = ((x3-x2)*(x3-x2) + (y3-y2)*(y3-y2) 
                 + .000000001*domainscale);  // Probably don't need this factor.
   
  /* Numerator is dot-product */
   s[1] = ((x1-x2)*(x3-x2) + (y1-y2)*(y3-y2))/reflinelength;

  /* FIXME: What, exactly, does this mean? */
   if (kk[i] == 0)  
   {
     /* This is the omega parameter given by TCK 1995, Eq. 58(?),
      * p. 1239.  
      */
      c_length[i][2] = s[1]; // if contact locked after previous step
   }

  /* i4 = 0 => contact is single point only, not edge-edge */
   if (i4==0) 
      return;  /* goto pr02;  */

  /* Coordinates of second endpoint of contacting edge. */
   x1   = vertices[i4][1];
   y1   = vertices[i4][2];

   s[4] = ((x1-x2)*(x3-x2) + (y1-y2)*(y3-y2))/reflinelength;

  /* ordering i1 i2 i3 i4 on edge i2 i3             */
  /* o[][3] locking length      o[][3] locking flag */
   for (j=1; j<=3; j++)
   {
      for (ell=j+1; ell<=4; ell++)
      {

         if (s[j]  <=  s[ell]) 
            continue;   // in order
        /* Else swap em */
         s[0] = s[j];
         s[j] = s[ell];
         s[ell] = s[0];
       }  /*  ell  */
   }  /*  j  */
      
  /* c_length[i][3] is given as cohesion length in some 
   * previous comments.  The .5 results from having half the
   * cohesion induced by the current contact, and the other half 
   * by the contact of the jth block into the ith block.
   * The sqrt comes from un-normalizing (???).
   */
   c_length[i][3] = .5*(s[3]-s[2])*sqrt(reflinelength);
 
  /* m0[i][2]  = 2; */  /* d */
  
/* pr02:;  */

}  /*  Close proj().  */