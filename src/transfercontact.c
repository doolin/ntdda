
/*
 * transfercontact.c
 *
 * Update block contacts from previous time step 
 *
 */
#include <time.h>
#include"analysis.h"
#include"math.h"
#include"ddamemory.h"
#include<assert.h>
#include<malloc.h>



/* See section 3.3 of the manifold report (page 39) */
/**************************************************/
/* df06: contact transfer                         */
/**************************************************/
void df06(Geometrydata *bd, Analysisdata *ad, int **contacts, int **locks, 
          int **prevcontacts, int **c_index, double **c_length, int *kk, 
          int *k3, int * nn0)
{
   int contact;  // was i
   int block;  // was i
   int  prevctact;  // number of previous contacts
   int /*i0,*/ i1, i2;
   int previoustate, currentstate;
   int j, jj, /*j0,*/ j1, j2;
   int contact_type;
   int nBlocks = bd->nBlocks;
   double ** vertices = bd->vertices;
   int nContacts = bd->nContacts;
   Gravity * grav = ad->gravity;


  /* Used for the locks array */
   const int OPEN = 0;
   const int LOCKED = 2;
   const int PREVIOUS = 1;
   const int CURRENT = 2;
   const int SAVE = 3;
   const int TRANSFER = 4;
   const int SWAP = 3;

  /* These are for the contactindex array */
   const int FIRST = 1;
   const int LAST = 2;
   const int TYPE = 0;
   const int VE = 0;
   const int VV = 1;

  /* (GHS: make index of first block; currTimeStep=1; currTimeStep>1) */
  /* Initialize the number of blocks in contact with block i to 0 */
   for (block=1; block<= nBlocks; block++)
   {
      c_index[block][1] = 0;
   }  /*  i  */
 
  /* for each contact, do some initialization */
   for (contact=1; contact<= nContacts; contact++)
   {
      locks[contact][PREVIOUS] = OPEN; /* previous (?) contact flags to 0 */
      locks[contact][CURRENT] = OPEN; /* current (?) contact flag to 0 */
     /* TCK's omega shear contact normalized edge length parameter. */
      c_length[contact][2]  = 0; 
     /* Length of contact for computing cohesion. */
      c_length[contact][3]  = 0;
     /* Set as a result of one test in df06() to record the state of 
      * which reference line is used (?????),
      * then passed to df07() to pass to proj().
      */
      kk[contact] = 0; 
   }  /*  i  */
  
  /* The next two loops build m1, which seems to be a contact
   * index of some sort.
   */

  /* For each contact, determine the block numbers that are 
   * in contact with each other.  Accumulate the number of 
   * contacts into c_index[block][1] (m1)?.
   */
   for (contact=1; contact<= nContacts; contact++)
   {
      i1 = contacts[contact][1]; /* contacting vertex */
      j1 = contacts[contact][2]; /* endpoint 1 of contacted ref line */
      i2 = nn0[i1]; /* block number of vertex i1 involved in contact */
      j2 = nn0[j1]; /*           "            j1         "           */
     /* use block with lower number, closer to beginning of list. */
      if (i2>j2)  
         i2=j2;
     /* accumulate the number of blocks in contact with block i2
      * (lower block number). c_index[][1] gets overwritten with 
      * accumulation in the next block of code.
      */
      c_index[i2][FIRST] += 1;
   }  /*  i  */
  
  /* Initialize cumulative contacts to zero. */
  /* m1 is an index array.  Here we set up the start and
   * stop positions of the index.  
   */
   c_index[0][LAST] = 0;
   for (contact=1; contact<=nBlocks; contact++)
   {
     /* cumulative number of contacts including contacts
      * of block i, that is, 
      * "stop value" = current cumulative + previous cumulative
      */
      c_index[contact][LAST] = c_index[contact][FIRST] + c_index[contact-1][LAST];
     /* first contact associated with block i,
      * "start" = stop - cumulative + 1
      */
      c_index[contact][FIRST] = c_index[contact][LAST] - c_index[contact][FIRST] + 1;
   }  /*  i  */

  /* Finished building m1, the "contact index"? */

  /* Now build ???
   */
   //if ((ad->gravity->gravTimeStep==1) || (ad->currTimeStep == 1  && ad->analysistype >= 0)) 
   if (ad->cts == 1)
      //goto a601;
   {
     /* FIXME: This loop repeats at end of df06().
      * The loop could be sent to a function call, then 
      * written as a macro.
      */
     /* setContactJointType(vertices, m, m2); */
      for (contact=1; contact<= nContacts; contact++)
      {
        /* i1 =m[i ][3]; */
        /* vertex corresponding to ref line for contact */
        	i1 = contacts[contact][3]+1;		 /* changed 11/14/94 */
        /* set joint type for ref line */
         prevcontacts[contact][0]= (int) vertices[i1][0];
      }  /*  i  */
      return;
   }

  /* If not in first time step, copy all the locks from
   * the previous time step.
   */
   for (contact=1; contact<= ad->nPrevContacts; contact++)
   {
      k3[contact] = locks[contact][0];
   }  /*  i  */
  
   for (contact=1; contact<= nContacts; contact++)
   //for (i=1; i<=nCurrentContacts; i++)
   {     
     /* locks is contact flags for tension, etc. 
      * locks[i][0] = 0 supposedly sets tension value
      * to zero for each contact.
      */
      locks[contact][0] = OPEN;
   }  /*  i  */
  


  /* (GHS: transfer contacts     ad->currTimeStep>1) */
  /* for each contact in the previous time step... 
   * npc is ordinal number (#) of previous contacts 
   */
   for (prevctact=1; prevctact<= ad->nPrevContacts; prevctact++)
   {
      int i;
     /* contact transfer flag initialized to 0 */
      locks[prevctact][TRANSFER] = 0;

     /* If we are in gravity mode, make sure to update all of the 
      * contacts, unless we are in the  first step, in which case
      * there are no previous contacts to transfer forward.
      */
      //if ( (ad->gravity->gravTimeStep == 0)  && (locks[prevctact][SAVE] == 0) ) 
      if (locks[prevctact][SAVE] == 0) 
         continue;  /* goto a602; */


     /* else, if save contact locks[ii][3] has second vertex
      * penetrating its reference line, flag contact transfer
      * to true and save contact to 0 (false?)
      */
      if (locks[prevctact][SAVE] == 3) 
         locks[prevctact][TRANSFER] = 1;

      if (locks[prevctact][SAVE] == 3) 
         locks[prevctact][SAVE] = 0;
     /* i0 is contact type, 0 for pt/edge, 1 for pt/pt.
      * (otherwise locks[ii][3] = 1 or 2, locks[ii][4] = 0
      */
      //i0=m2[ii][0];
      contact_type = prevcontacts[prevctact][0];
     /* FIXME: what exactly is m2[ii][6]? */
      if (prevcontacts[prevctact][6] > 0) 
         contact_type = 1;  //   i0=1;

     /* This is strange looking, but what it means is to 
      * loop once for a pt/edge contact and twice for 
      * pt/pt contact.
      */
      //for (jj=0; jj<= i0; jj++)
      for (jj=0; jj<=contact_type; jj++)
      {
        /* i1 and j1 are the "contact" vertices,
         * m2 is listed as copy of contacts from 
         * previous time step.  Probably should mean
         * contacting vertices.
         */
         i1 = prevcontacts[prevctact][1];
         j1 = prevcontacts[prevctact][2];
        /* i2, j2 are the block numbers associated with 
         * the i1 and j1 vertices, respectively.
         */
         i2 = nn0[i1];
         j2 = nn0[j1];
        /* Reset to use the lower block number.  FIXME: Why? */
         if (i2>j2)  i2=j2;
      
        /* Now loop over contacts for each individual block.
         * For each new contact associated with block i2...
         */
         for (i=c_index[i2][1]; i<= c_index[i2][2]; i++)
         {
            //j0= m[i][0];
            contact_type = contacts[i][0];
            if (contacts[i][6] > 0) 
               contact_type = 1;  //j0=1;

           /* Loop once for pt/edge contact, twice for 
            * pt/pt contact
            */
            //for (j=0;         j<= j0;        j++)
           /* j goes from 0 to 0 for pt-edge (VE) contact, j from 
            * 0 to 1 is VV contact.
            */
            for (j=0; j<=contact_type; j++)
            {
              /* If the new vertex does not match the old vertex,
               * do not transfer.  Get next vertex ?????
               *  ?? of new contact.
               */
               if (contacts[i][3*j+1] != prevcontacts[prevctact][3*jj+1]) 
                  continue; /* goto a603; */
               if (contacts[i][3*j+2] != prevcontacts[prevctact][3*jj+2]) 
                  continue; /* goto a603; */
               if (contacts[i][3*j+3] != prevcontacts[prevctact][3*jj+3]) 
                  continue; /* goto a603; */
      
              /* Else, all three vertices match (vertex and ref line) so we
               * transfer contact data from prev step to current step.
               */
              /* (GHS: o[][0] penetration o[][1] sliding) */
              /* displacement normal to ref line, penetration distance */
               c_length[i][0]=c_length[prevctact][4];
              /* displacement parallel to ref line. */
               c_length[i][1]=c_length[prevctact][5];
              /* contact edge ratio 2p(?)/23(?) set in df?? */
               c_length[i][2]=c_length[prevctact][6];

              
               if (ad->gravityflag == 1)
               {
                  grav->contactFN[i][0] = grav->contactFN[prevctact][3];
                  grav->contactFN[i][1] = grav->contactFN[prevctact][3];
                  grav->contactFN[i][2] = grav->contactFN[prevctact][3];
                  grav->contactFS[i][0] = grav->contactFS[prevctact][3];
                  grav->contactFS[i][1] = grav->contactFS[prevctact][3];
                  grav->contactFS[i][2] = grav->contactFS[prevctact][3];
               }  /* end if gravity flag */

              /* (GHS: from angle to edge set locks[][]=2)  */
              /* locks[new contact][# of ref line (1 or 2)]
               * = locks[old contact][ref line 1 or 2];
               */
               locks[i][j+1] = locks[prevctact][jj+3];
              /* Corresponding value from old contact */
               locks[i][0]   = k3[prevctact];
              /* if current contact_type (m[i][0]) is pt/edge and
               *    loop counter is 0   and
               *    open close flag of prev contact save = 2  and
               *    first loop counter for jj is 0
               * then
               *    kk[i] = 1
               * What this means is: 
               * contacts [i][0] = 0  ->  VE contact
               * j = 0  -> in this case is first reference line of second contact
               * locks[ii][SAVE] = LOCKED  -> 
               * jj = 0 for previous contact, we have first vertex.
               */
               if (contacts[i][0]==0 && j==0 && locks[prevctact][SAVE]==LOCKED && jj==0) 
                  kk[i] = 1;
              /* if current contact_type (m[i][0] is pt/edge and
               * copy of prev contact type not pt/edge
               * then
               * open close flag previous = 2
               * FIXME: where is this used next?
               */
               if (contacts[i][0] == 0 && prevcontacts[prevctact][0] > 0)  
                  locks[i][PREVIOUS] = LOCKED;
/* a603:; */
            }  /*  j  */
         }  /*  i  */
      }  /*  jj */
/* a602:; */
   }  /*  ii */


  /* (GHS locks[][1]=0  locks[][2]=1 => locks[][2]=3   v-e to v-v) */
  /* FIXME:  Explain what this does. */
   for (contact=1; contact<= nContacts; contact++)
   {
      previoustate = locks[contact][PREVIOUS];
      currentstate = locks[contact][CURRENT];
     /* set locks[i][1]=0  before open-close iterations */
      locks[contact][PREVIOUS] = OPEN;
      locks[contact][CURRENT] = previoustate;

      if (currentstate != OPEN)  
         locks[contact][CURRENT] = currentstate;

      if (contacts[contact][TYPE] == VE)  
         continue; /* goto next contact a604; */

      if (previoustate != OPEN)  
         locks[contact][CURRENT] = 1;

      if (currentstate != OPEN)  
         locks[contact][CURRENT] = SWAP;

/* a604:; */
   }  /*  i  */

//a601:
  /* The loop could be sent to a function call, then 
   * written as a macro.
   */
  /* set m2[i][0] as joint material number */
  /* FIXME: Find a different place to stash this.  The same
   * location in "contacts" is contact type.
   */
   for (contact=1; contact<=nContacts; contact++)
   {
     /* i1 =m[i][3]; */
     /* vertex corresponding to ref line for contact */
     	i1 = contacts[contact][3] + 1;		 /* changed 11/14/94 */
     /* set joint type for ref line */
      prevcontacts[contact][0]= (int) vertices[i1][0];
   }  /*  i  */

}  /* Close df06  */
