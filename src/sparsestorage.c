/*
 * sparsestorage.c
 * 
 * Handle the gruesome details of sparse block 
 * storage.
 */


#include "analysis.h"
#include <math.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>
//#if WINDOWS
//#include "wingraph.h"
//#endif
#include "errorhandler.h"
#include "ddamemory.h"
#include "contacts.h"


/* FIXME: Move the sparse storage function prototypes 
 * into their own header file or declare them static
 * in ths file.  This gets them out of analysis.h and
 * speeds up recompiles among other things.
 */
//#include "sparsestorage.h"

extern DATALOG * DLog;
extern FILEPOINTERS fp;

/* These used to be globals, then they were passed around as
 * as parameters, now they are static local to this file,
 * next they will moved to the sparse storage file when 
 * df08 gets stripped out of here.
 */
/*------------------------------------------------*/
/* k2: i new block number k2[i] old number 6000   */
/* k2[nBlocks+1]                                  */
static int __k2size;
static int * k2;
/*------------------------------------------------*/
/* k4: working array in 6000 for k and index of k */
/* k4[nBlocks+1]                                  */
static int __k4size;
static int * k4;

static int __k3size;
static int * k3;


void initStorageTempArrays(int numblocks)
{
  /* FIXME: Get comments for k2 */
   __k2size=FORTY*numblocks+1;
   k2=(int *)calloc(1,sizeof(int)*__k2size);

  /*------------------------------------------------*/
  /* k4: working array in 6000 for k and index of k */
  /* k4[nBlocks+1]                                  */
   __k4size=numblocks+1;
   k4=(int *)calloc(1,sizeof(int)*__k4size);

  /*------------------------------------------------*/
  /* k3: index of a    for lower triangle k 6000    */
  /* k3: reserved positions for non-zero storage    */
  /* k3[nBlocks*40+1]                               */
   __k3size=FORTY*numblocks+1;
   k3=(int *)calloc(1,sizeof(int)*__k3size);

}  /* close initStorageTempArrays() */

void
freeStorageTempArrays(void)
{

	  if (k2)
   {
      free(k2);  
      k2 = NULL;
   }
   
   if (k4)
   {
      free(k4);  
      k4 = NULL;
   }

   if (k3)
   {
      free(k3);
      k3 = NULL;
   }

}  /* close freeStorageTempArrays() */

/****  New df08 under here  ************/
/* Positions of nonzero storage. 
 * FIXME:  copy GHS comments from original function.
 */
/* It appears that producing k1 is the reason for this 
 * function.  It ought to be returned instead of passed.
 * After this code is fully commented, the subroutines
 * can be collapsed in a a larger function to speed things
 * up, as it was previously.  
 */
/* Builds $Q_0$ using $kk$, then transforms
 * $Q_0$ into $Q_n$(?), which is a matrix that records which 
 * entries of a lower triangular matrix are non-zero.  
 * $Q_n$ is output in the variable $kk$.
 * (was df08()).
 */
void 
sparsestorage(Geometrydata *gd, Analysisdata *ad, Contacts * ctacts,
             /*int **m, int **m1,*/ 
              int *kk, int *k1, int *nn0, int **n)
{

   int nBlocks = gd->nBlocks;

   int ** contacts = get_contacts(ctacts);
   int ** contactindex = get_contact_index(ctacts);

   //printN(n, " in df08, before initdf08");

   initdf08(n,kk,nBlocks);
   //printN(n, " in df08, after initdf08");

  /* regUpper and LowerTriangle appear to construct the 
   * Q0 matrix, p. 203, Shi 1988.
   */

   regUpperTriangle(n, kk, nn0, contacts, contactindex, nBlocks, gd->nBolts, gd->rockbolts);
   //printKK(kk, "in df08, after uppertri");
   //printN(n, " in df08, after uppertri");


  /* After regLowerTriangle, kk is almost definitely
   * the Q0 matrix on p. 203, Shi 1988.
   */
   regLowerTriangle(n, kk, nBlocks);
   //printKK(kk, "in df08, after lowertri");
   //printK1(k1, "in df08, after lowertri");
   //printN(n, " in df08, after lowertri");

  /* Write a spy plot of the matrix before reordering. */
   //if(ad->currTimeStep == 1  && ad->gravityflag == 0)
      //writeSpyfile(n, kk, gd->nBlocks, fp.spyfile1);

  /* Now that we have Q0, ???
   */
   findMinimumConnections(n, kk, k1,  k2, k4, nBlocks);
   //printKK(kk, "in df08, after minconnections");
   //printK1(k1, "in df08, after minconnections");

   //printN(n, " in df08, after minconnections");

   changetoNewBlockNumber(n, kk, k1, nBlocks);
   //printKK(kk, "in df08, after change to newblock number");
   //printN(n, " in df08, after changetonewblocknumber");

  /* FIXME: Eliminate k3 from the parameter list.  It is
   * only used as temporary swap matrix.
   */
   changetoLowerTriangle(n, kk, k3, k4, nBlocks);
   //printKK(kk, "in df08, after changetolowertri");
   //printN(n, " in df08, after changetolowertri");

   reorderK(n, kk, nBlocks);
   //printKK(kk, "in df08, after reorderK");
   //printN(n, " in df08, after reorderK");

  /* Now write a spy plot of the matrix after reordering. */
   //if(ad->currTimeStep == 1  && ad->gravityflag == 0)
      //writeSpyfile(n, kk, gd->nBlocks, fp.spyfile2);

   //printK1(k1, "in df08, after reorder");

  /* n3 accounts for all of the "produced" connections as well */
   ad->n3 = n[nBlocks][1]+n[nBlocks][2]-1;

  /* total storage needed for LDL^T. */
   DLog->memallocsize[ad->currTimeStep] = ad->n3;

}  /* close df08()  */
   

void
initdf08(int ** n, int * kk, int nBlocks)
{
   int i, i1;

  /* i:m[] nn0[]   w:nBlocks[] k3[]   o:n[] kk[] k1[] k2[] */
  /* n[i][1]  begin number of k for i-th row        */
  /* n[i][2]  number       of k for i-th row        */
  /* n[i][3]  limit number of k for i-th row        */
  /* form diagnal term of kk[]                      */
  /* This next loop is purely initialization.  n[][] may 
   * be completely modified in the code following.  
   * FIXME: explain why these variables are initialized
   * to the values here.
   */
   for (i=1; i<= nBlocks; i++) 
   {
      /* Pointer to first element for block i:
       * block 1: 1, block 2: 21, block 3: 41, etc.
       */
       n[i][1] = 20*(i-1)+1;
      /* Each block is initially in contact with
       * itself. n[i][2] accumulates the number of  
       * blocks in contacts with block i.
       */
       n[i][2] = 1;
      /* Number of elements for ith block 
       * FIXME: is 20 an arbitrary number?
       */
       n[i][3] = 20;
      /* kk[1] = block 1, kk[21] = block 2, etc. */
       i1  = n[i][1];
       kk[i1]  = i;
   }  /*  i  */

}  /* close initdf08() */


/* Appears to construct the Q0 matrix.
 */
void 
regUpperTriangle(int ** n, int * kk, int * nn0, int ** m, 
                 int ** m1, int nBlocks, int numbolts, double ** rockbolts)
{ 
   int i, j;
   int i0, i1, i2, i3;
   int j0, j1, j2;
  /* Change from `l' to something else. */
   int l;

  /* (GHS: registration upper triangle)  */
   for (i=1; i<= nBlocks; i++) 
   {
     /* For each contact associated with block i...
      * m1 is initialized in df06?() 
      */
      for (j=m1[i][1]; j<= m1[i][2]; j++) 
      {
        /* i1, j1 are vertex numbers */
        /* contacting vertex index, points to vIndex? */
         i1 = m[j][1];
        /* ref line vertex index, points to vIndex? */
         j1 = m[j][2];
        /* i2, j2 are block numbers; associate the vertices 
         * i1, j1 with the blocks that they occur on.
         */
         i2 = nn0[i1];
         j2 = nn0[j1];
        /* j0 is block that block i is in contact with.
         * i2 = i means that the vertex i1 is on the same block 
         * as currently considered in the outer loop.   
         */
         if (i2 != i)      
            j0=i2;
         else 
            j0 = j2;
        /* last block that block i is in contact with so far */
         i0 = n[i][1]+n[i][2]-1;
        /* block i is already in contact with block j0 */
         if (kk[i0] == j0)      
            continue;   //goto a801;
        /* else, increment the # of blocks in contact with block i */
         n[i][2] += 1;
        /* if there is enough room on the list add it, */
         if (n[i][2] <= n[i][3]) 
         {
            i3=n[i][1]+n[i][2]-1;
            kk[i3]=j0;
               continue;   //goto a802;
         }
        /* else make more room on the list. */
         n[i][3] += 20;

        /* last block, don't need to ??? the ??? */
         if (i == nBlocks)  
         {
            i3=n[i][1]+n[i][2]-1;
            kk[i3]=j0;
               continue;   //goto a802;
         }

        /* All of this is index shifting induced by 
         * adding 20 more elements to the list above.
         */
        /* move k 20 elements forward  */
         for (l=n[nBlocks][1]+n[nBlocks][2]-1; l>= n[i+1][1]; l--) 
         {
            kk[l+20]=kk[l];
         }  /*  l  */
            
        /* move k 20 element forward change index         */
         for (l=i+1; l<= nBlocks; l++) 
         {
            n[l][1]+=20;
         }  /*  l  */
            
//a802:
         i3=n[i][1]+n[i][2]-1;
         kk[i3]=j0;
            
//a801:
//            ;
      }  /*  j  */
   }  /*  i, End upper triangle registration */

  
  /* Register bolts */
		/* gg1: x1  y1  x2  y2  n1  n2  e00  t0  f0    bolt */
		/* n1 n2 carry block number        f0 pre-tension */
  /* gg1: x1  y1  x2  y2  n1  n2  e00  t0  f0 u1 v1 u2 v2
   * (for each bolt).  n1 and n2 are the "carry" block 
   * number, that is, the block number of the blocks containing
   * the respective endpoints.  f0 is the pre-tension.
   * u1 v1 u2 v2 are computed displacements of the bolt 
   * endpoints.  These are set to zero initially, and should
   * be computed pretty much the same as the way u1,...,v2
   * for the fixed, load etc. points are computed.  Assume 
   * t0 is stiffness for now.  e00 is unknown.
   */
#if ROCKBOLTSTORAGE
   for (i=1; i<=numbolts; i++)
   {
      i2 = (int)rockbolts[i][5];
      j2 = (int)rockbolts[i][6];


      j0 = i2;

      i0 = n[i2][1] + n[i2][2] - 1;

      n[i2][2]++;

      if (kk[j0] == j0)
         continue;   // already in contact with this block

     /* else add the connection */

      i3 = n[i2][1] + n[i2][2] - 1;

      kk[i3] = j0;

   }  /* end register blocks in contact by bolts */
#endif


}  /* close regUpperTriangle() */

/* Appears to construct Q0 matrix, p. 203 Shi 1988
 */
void 
regLowerTriangle(int ** n, int * kk, int nBlocks)
{
   int i;
   int j, j1, j2;
   int l;
  /* FIXME: Change variable name from `l' to 
   * something that does not look like `1'.
   */
  /* (GHS: registration of lower triangle)  */
   for (i=1; i<= nBlocks; i++) 
   {
      for (j=n[i][1]; j<= n[i][1]+n[i][2]-1; j++) 
      {
         j1=kk[j];
         if (j1 <= i) 
            continue;  //goto a803;
         n[j1][2] += 1;
         if (n[j1][2]<=n[j1][3]) 
         {
            j2    = n[j1][1]+n[j1][2]-1;
            kk[j2] = i;
               continue;  //goto a804;
         }
         n[j1][3] += 20;
         if (j1 == nBlocks) 
         {
            j2    = n[j1][1]+n[j1][2]-1;
            kk[j2] = i;
               continue;  //goto a804;
         }
           
        /* move k 20 elements forward                     */
         for (l=n[nBlocks][1]+n[nBlocks][2]-1; l>= n[j1+1][1]; l--) 
         {
            kk[l+20]=kk[l];
         }  /*  l  */
            
        /* move k 20 element forward change index         */
         for (l=j1+1; l<= nBlocks; l++) 
         {
            n[l][1]+=20;
         }  /*  l  */
            
//a804:
         j2    = n[j1][1]+n[j1][2]-1;
         kk[j2] = i;
            
//a803:
//            ;
      }  /*  j  */
   }  /*  i  end lower triangle registration */
}  /* close regLowerTriangle() */




/* resets n[][] */
void 
findMinimumConnections(int ** n, int * column_numbers, int * k1, int * k2, 
                       int * k4, int nBlocks)
{
   int i, i0, i1, i2, i3, i4;
   int j, j1, j2, j3;
  /* FIXME: Change to non-`l' name.  Change to k after 
   * k names get changed.
   */
   int l, l1; // l4;

  /* What is this?
   */
  /* k1:old-new     k2:new-old  */
   for (i=1; i<= nBlocks; i++) 
   {
      k1[i]=i;
      k2[i]=i;
   }  /*  i  */

  /* find element with minimum connection   */
   for (i=1; i<= nBlocks; i++) 
   {
      i0 = n[i][2];
      i1 = i;
     /* The following loop produces the block number i1 with 
      * the least number of blocks contacted with it,
      * for the ith block.
      */
      for (j=i; j<= nBlocks; j++) 
      {
         //if (i0<=n[j][2]) goto a805;
         if (i0 > n[j][2])
         {
            i0 = n[j][2];
            i1 = j;
         }
// a805:;
      }  /*  j  */

     /* Now we have i1, the block number with the least number 
      * of contacting blocks.
      */
     /* exchange i i1 two rows           k1 inverse k2 */
      // if (i1 == i) goto a806;
     /* If the current block i is not the same as the block
      * with the least number of contacts, swap a bunch of stuff...
      */
      if(i1 != i) 
      {
         i2     = k2[i];
         k2[i]  = k2[i1];
         k2[i1] = i2;
         k1[i2] = i1;
         i3     = k2[i];
         k1[i3] = i;
        /*------------------------------------------------*/
        /*  save row i in k4                              */
         for (j=n[i][1];   j<= n[i][1]+n[i][2]-1;   j++) 
         {
            j1     = j-n[i][1]+1;
            k4[j1] = column_numbers[j];
         }  /*  j  */
        /*------------------------------------------------*/
        /* move row i1 to row i                           */
         for (j=n[i1][1];  j<= n[i1][1]+n[i1][2]-1; j++) 
         {
            j1    = n[i][1]+j-n[i1][1];
            column_numbers[j1] = column_numbers[j];
         }  /*  j  */
        /*------------------------------------------------*/
        /* move up between row i and i1           if j2>0 */
         j2=n[i][3]-n[i1][3];
         // if (j2<=0) goto a819;
         if(j2 > 0) 
         {
            for (j=n[i+1][1]; j<= n[i1][1]+n[i1][2]-1; j++) 
            {
               column_numbers[j-j2] = column_numbers[j];
            }  /*  j  */
         }
// a819:;
         j3=0;
         if (j2>0)  j3=j2;
        /*------------------------------------------------*/
        /* put row i; in row i1 position            i<=i1 */
         for (j=n[i1][1]-j3; j<= n[i1][1]-j3+n[i][2]-1; j++) 
         {
            j1   = j-n[i1][1]+j3+1;
            column_numbers[j] = k4[j1];
         }  /*  j  */
        /*------------------------------------------------*/
        /* exchange n for row i i1    can't move downward */

/* The following is test code to investigate how this stuff
 * compiles and works.  So the index swapping works fine, 
 * but the code doesn't work! (which is to be expected)
 */
//#if 1
         {
//int * pswap;
//pswap = n[i];
//n[i] = n[i1];
//n[i1] = pswap;
         }
//#endif
         j1       = n[i][2];
         n[i][2]  = n[i1][2];
         n[i1][2] = j1;
         // if (j2<=0) goto a806;
         if(j2 > 0) 
         {
            j1       = n[i][3];
            n[i][3]  = n[i1][3];
            n[i1][3] = j1;
           /*------------------------------------------------*/
           /* move up index of row i+1 to i1                 */
            for (j=i+1; j<= i1; j++) 
            {
               n[j][1]  = n[j][1]-j2;
            }  /*  j  */
         }  // end if j2>0


      } // end if i1 != i  i.e., end swap i for i1





     /*================================================*/
     /* delete elements of lower triangle for ith row  */

     /* FIXME:  Explain why we are doing this. */
//a806:; 
      if (n[i][2]==1) 
         continue;  //goto a807;  // continue;


      for (j=n[i][1]+1; j<= n[i][1]+n[i][2]-1;   j++) 
      {
         j1 =  column_numbers[j ];
         i2 = k1[j1];
        /* This loop is trying to find out when when 
         * kk[l] == k2[i].  Why is i3 set?  What is i3?
         */
         for (l=n[i2][1];  l<= n[i2][1]+n[i2][2]-1; l++) 
         {
            if (column_numbers[l] != k2[i]) 
               continue; //goto a808;  /* continue  */
            i3 = l;
               break;  //goto a809;  /* break */
//a808:;
         }  /*  l  */
            
        /*------------------------------------------------*/
        /* delete elements of lower triangle in  i2  row  */
//a809:;
        // if (i3==n[i2][1]+n[i2][2]-1) goto a810;
        if (i3 != n[i2][1]+n[i2][2]-1) 
        {
           for (l=i3+1; l<= n[i2][1]+n[i2][2]-1; l++) 
           {
              column_numbers[l-1]=column_numbers[l];
           }  /*  l  */
        } // end if i3
/*a810:;  */
        n[i2][2]=n[i2][2]-1;
      }  /*  j  */




     /*================================================*/  
     /* add 2 produced connections        i1-i2  i2-i1 */
      if (n[i][2]<=2) 
         continue;  //goto a807;  // continue;

      for (j=n[i][1]+1;  j<= n[i][1]+n[i][2]-2;    j++)
      {
         for (l=j+1;        l<= n[i][1]+n[i][2]-1;    l++)
         {

            i1 = column_numbers[j];
            i2 = column_numbers[l];
            j1 = k1[i1];
            
            for (l1=n[j1][1]; l1<= n[j1][1]+n[j1][2]-1; l1++)
            {
               if (column_numbers[l1] == i2) 
                  goto a811;
            }  /*  l1 */


           /*------------------------------------------------*/
           /* move k 20 elements forward                     */
            n[j1][2] += 1;
            if (n[j1][2] <= n[j1][3]) 
               goto a812;
            n[j1][3] += 20;
            if (j1       ==       nBlocks) 
               goto a812;
            for (l1=n[nBlocks][1]+n[nBlocks][2]-1; l1>= n[j1+1][1]; l1--)
            {
               column_numbers[l1+20]  = column_numbers[l1];
            }  /*  l1 */
           /*------------------------------------------------*/
           /* move k 20 element forward change index         */
            for (l1=j1+1; l1<= nBlocks; l1++)
            {
               n[l1][1] += 20;
            }  /*  l1 */
           /*------------------------------------------------*/
           /* add connection in row j1   kk[j]--kk[l]   i1--i2 */
a812:;
            i3    = n[j1][1]+n[j1][2]-1;
            column_numbers[i3] = i2;
           /*================================================*/
           /* add connection in row j2   kk[l]--kk[j]   i2--i1 */
a811:;
            j2=k1[i2];
            for (l1=n[j2][1]; l1<= n[j2][1]+n[j2][2]-1; l1++)
            {
               if (column_numbers[l1] == i1)  
                  goto a813; 
            }  /*  l1 */


           /*------------------------------------------------*/
            n[j2][2] += 1;
            if (n[j2][2] <= n[j2][3]) 
               goto a814;
            n[j2][3] += 20;


            //if (j2   ==   nBlocks) 
            //   goto a814;
            if (j2   !=   nBlocks) 
            {
              /*------------------------------------------------*/
              /* move k 20 elements forward                     */
               for (l1=n[nBlocks][1]+n[nBlocks][2]-1; l1>= n[j2+1][1] ; l1--)
               {
                  column_numbers[l1+20] = column_numbers[l1];
               }  /*  l1 */
              /*------------------------------------------------*/
              /* move k 20 element forward change index         */
               for (l1=j2+1; l1<= nBlocks; l1++)
               {
                  n[l1][1] += 20;
                 /*  l1 */
               }
              /*------------------------------------------------*/
a814:;
            }




            i4    = n[j2][1]+n[j2][2]-1;
            column_numbers[i4] = i1;
a813:;
         }  /*  l  */
      }  /*  j  */
     /*------------------------------------------------*/


//a807:;
   }  /*  i  end find element with minimum connection */


} /* close findMinimumConnections() */
      

 
void
changetoNewBlockNumber(int ** n, int * kk, int * k1, int nBlocks)
{     
   int i, j, i1;

  /**************************************************/
  /*  change to new block number                    */
   for (i=1; i<= nBlocks; i++)
   {
      for (j=n[i][1]; j<= n[i][1]+n[i][2]-1; j++)
      {
        /* Is kk held over from registering upper and lower triangles? 
         * In other words, has it changed since then?
         */
         i1   = kk[j];
         kk[j] = k1[i1];
      }  /*  j  */
   }  /*  i end change to new block number */

}  /* close changetoNewBlockNumber() */



/* k3 is used only as a temp swap variable.
 * FIXME: Eliminate k3 from parameter list.
 */
void 
changetoLowerTriangle(int **n, int * kk, int * k3, int * k4, int nBlocks)
{
   int i, i1; 
   int j, j1;

  /*------------------------------------------------*/
  /*  change k to lower tiangle                     */
   for (i=1; i<= nBlocks; i++)
   {
      k4[i] = 0;
   }  /*  i  */
  /*------------------------------------------------*/
  /* element number of rows in lower triangle       */
   for (i=1; i<= nBlocks; i++)
   {
      for (j=n[i][1]; j<= n[i][1]+n[i][2]-1; j++)
      {
         i1      = kk[j];
         k4[i1] += 1;
      }  /*  j  */
   }  /*  i  */
  /*------------------------------------------------*/
  /* n[i][3] change to begin of low triangular row  */
   n[1][3]=1;
   for (i=2; i<= nBlocks; i++)
   {
      n[i][3] = n[i-1][3]+k4[i-1];
      k4[i-1] = 0;
   }  /*  i  */
   k4[nBlocks]  = 0;
  /*------------------------------------------------*/
  /* change k4 to point k3                          */
   for (i=1; i<= nBlocks; i++)
   {
      for (j=n[i][1]; j<= n[i][1]+n[i][2]-1; j++)
      {
         i1      =  kk[j];
         k4[i1] += 1;
         j1      = n[i1][3]+k4[i1]-1;
         k3[j1]  = i;  /* was k3 */
      }  /*  j  */
   }  /*  i  */
  /*------------------------------------------------*/
  /* change n; i<= lower tiangular                  */
   for (i=1; i<= nBlocks; i++)
   {
      n[i][1] = n[i][3];
      n[i][2] = k4[i];
   }  /*  i  */
  /* Copy k3 into kk for all indices stored in n[][] */
   for (i=1; i<= n[nBlocks][1]+n[nBlocks][2]-1; i++)
   {
      kk[i]    = k3[i];  
   }  /*  i  */

} /* close changetoLowerTriangle() */





/* n -> colindex
 * kk -> column_number
 */
/* This looks like it reorders the column number in kk (column_number)
 * to go from smallest to largest.
 */
void 
reorderK(int ** colindex, int * column_number, int nBlocks)
{
   int row, j, k;
   int start, stop;
   int column;  // was i1


  /*------------------------------------------------*/
  /* ordering numbers of k                          */
   for (row=1; row<= nBlocks; row++)
   {
      start = colindex[row][1];
      stop = colindex[row][1] + colindex[row][2] - 1;
     
     /* Block only contacts "itself", that is, block i
      * is not in contact with any other block, continue
      * to check the next block.
      */
      if (colindex[row][2]==1) 
         continue;

     /* n indexes kk */
      //for (j=colindex[i][1]; j<= colindex[i][1]+colindex[i][2]-2; j++)

      for (j=start; j<=stop-1; j++)
      {
         for (k=j+1; k<=stop; k++)
         {
            if (column_number[j] < column_number[k]) 
               continue; 

           /* FIXME: replace this with a SWAP macro */
            //i1   = column_number[j];
            column = column_number[j];
            column_number[j] = column_number[k];
            //column_number[k] = i1;
            column_number[k] = column;

         }  /*  l  */
      }  /*  j  */

   }  /*  i  end ordering numbers of k  */

} /* reorderK() */
