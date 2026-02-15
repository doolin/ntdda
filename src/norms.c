
#include "analysis.h"
#include "ddamemory.h"
#include <stdlib.h>



static double checkBlockNorm(double * a);


/* Check for block diagonal dominance.  Details of this
 * are covered in Demmel and Higham or the relevant lawn.
 */
int // boolean
checkDiagDominance(double ** K, int ** colindex, int numblocks)
{
   int i,j;
  /* location of starting and diagonal columns in kk,
   * and memory locations of the same in K.
   */
   int startindex, stopindex;
   double column_norm = 0;
   double diagnorm = 0;
   int index = 1;  // GHS starts all arrays at 1 instead of 0

  /* Number of columns is the same as number of blocks */
   for (i=1;i<=numblocks;i++)
   {
      column_norm = 0;  // reset for each column
      diagnorm = 0;

      if (colindex[i][2] == 1)
         continue;  //  Only one block in column, which is on diagonal

      startindex = colindex[i][1];  // location of starting column
      stopindex = colindex[i][1] + colindex[i][2] - 1;  // location of diagonal

      for (j=startindex;j<=stopindex-1;j++)
         column_norm += checkBlockNorm(K[j]+index);

      diagnorm = checkBlockNorm(K[stopindex]+index);


      if (diagnorm <= column_norm)
         return 0;  //  failed, not DD
   }

  /* Passed checks.  We have DD. */
   return 1;

}  /* close checkDiagDominance() */




double
checkBlockNorm(double * a)
{

   double * work;
   int lda = 6;
   char uplo = 'U';
   int nnn = 6;
   //double * a = AData->K[1];
   char norm = 'F';
  /* GHS indexes everything from 1, so we need to
   * advance the pointer to the array.
   */
   int index = 1;

   double val;

   work = (double *)calloc(6,sizeof(double));

  /* WARNING!!!  MS will grab from end of array bounds, when it
   * should segfault or something...!!!  In other words, we should
   * verify that nnn*lda + index = Ksize1.
   */
   val = dlansy_(&norm, &uplo, &nnn, a, &lda, work);

   //printKForIthBlock(AData, 1, k1, n, "checkBlockNorm");

   return val;

}  /* close checkBlockNorm() */
