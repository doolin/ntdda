/**
 * ghssolver.c
 *
 * LD^{-1}L^T matrix solver for DDA.
 *
 * $Author: doolin $
 * $Date: 2006/07/06 02:28:27 $
 * $Source: /cvsroot/dda/ntdda/src/ghssolver.c,v $
 * $Revision: 1.15 $
 *
 */
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>

#include "analysis.h"
#include "ddamemory.h"


void sp_mult(double * Arow, double * x, double * y, int blocksize, int index);



/* FIXME: eliminate as many of the extra work arrays as
 * possible.
 */
double * worka;
double * workb;
double * workc;
int * ipiv;


/* HUGE KLUDGE!!! This needs to be fixed to concurrently
 * keep the analysis loop clean, but also to allow arbitrary
 * data to write to arbitrary files.
 */
extern FILEPOINTERS fp;

void df20(double ** K, double ** F,int *, int **, int numblocks);
void df20blas(double ** K, double ** F,int *, int **, int numblocks);

void df21(double ** K, double ** F,int *, int **, int numblocks);



void
solve(Analysisdata* ad,double ** K, double ** F, int * kk, int * k1, int ** n, int nBlocks) {

   if (ad->solvetype == lu) {// (ghsolver)

     /* Block $L D^{-1} L^T$ decomposition solver from Shi 1988,
      * Chapter 5, Section 5.2, pp. 193-199.  Shi proves in each
      * relevant section that K is SPD.  The decomposition is
      * assumed (p. 194) to exist (!), then an ad hoc algorithm
      * is presented.  After the decomposition, the system LY = F
      * is solved by forward substitution (p. 198), then the
      * unknowns are determined by back substitution (p. 199) ?????
      * (verify all of this externally to the dissertation).
      */
      df20(K,F,kk,n,nBlocks);
      df21(K,F,kk,n,nBlocks);
   }
   else if (ad->solvetype == cg) // (congrad)
   {
     /* This is defined in analysis.h */
      //congrad(K, F, kk, k1, n, nBlocks);
      assert(1);
   }
   else
   {
      ; // do something else
   }

}


static double T[7][7];
/* v : 6*6 diagnal submatrices of a b */
/* v[nBlocks+1][37]                   */
static int __Dinvsize1;//=GData->nBlocks+1;
static int __Dinvsize2;//  blocksize^2 + index = 37;
static double ** Dinv = NULL;
static int doublesize;


void
initSolverTempArrays(Geometrydata * gd)
{

  /* FIXME: Change to set on entry */
   int blocksize = 6;
   int index = 1;

  /* No need to call sizeof inside of tight loops. */
   doublesize = sizeof(double);


  /*------------------------------------------------*/
  /* v : 6*6 diagnal submatrices of a b             */
  /* v[nBlocks+1][37]                               */
   __Dinvsize1=gd->nBlocks+1;
   __Dinvsize2=37;
   Dinv = DoubMat2DGetMem(__Dinvsize1, __Dinvsize2);
  //*diagonals = x;




   worka = (double *)calloc((blocksize*blocksize) + index, doublesize);
   workb = (double *)calloc((blocksize*blocksize) + index, doublesize);
   workc = (double *)calloc((blocksize*blocksize) + index, doublesize);
   ipiv = (int *)calloc(blocksize + index,sizeof(int));

} /* close initSolverTempArrays() */

void
freeSolverTempArrays()
{
   if (Dinv)
      free2DMat((void **)Dinv, __Dinvsize1);

   if (worka)
      free(worka);

   if (workb)
      free(workb);

   if (workc)
      free(workc);

   if (ipiv)
      free(ipiv);

} /* close freeSolverTempArrays() */


void saveState(double ** K, double ** Kcopy, int n3,
               double ** F, double ** Fcopy, int numblocks,
               double ** c0) {
   int i,j;

  /* (GHS: save equation coefficient matrix before change) */
  /* FIXME: Why do we need to do this?  Also, this
   * could be turned into a macro.
   */
   for (i=1; i<= n3; i++) {
      for (j=1; j<= 36; j++) {
         Kcopy[i][j] = K[i][j];
      }
   }

  /* (GHS: save f[]          add friction loading to f[]) */
  /* There is some very strange stuff going on in this
   * next block of code.  The Fcopy array has to be set
   * exactly where it is right now, or the analyses will
   * not work correctly.  This was changed in either in
   * 1.5.117 or 1.5.118.
   */
   for (i=1; i<= numblocks; i++) {
      for (j=1; j<=6; j++) {
        /* If the time step has to be cut during an analysis,
         * we will need the original values again.
         * FIXME: Take out the dependency in the time step cutting
         * on this saved F vector.  Or at least check the
         * performance against adaptive time stepping.
         */
         Fcopy[i][j]  =  F[i][j];
        /* Frictional terms from c0 get transferred into
         * the "Force" vector, and we have no further use
         * of c0 until the next round of open/close
         * iteration.
         * FIXME: Explain this in detail.  Also, this should
         * probably be done before calling the solver, so that
         * this solver can be replaced with any other. This
         * should be moved back into df18().
         */
         F[i][j] += c0[i][j];
      }
   }

}



/* For use with dgemm.  These can later be moved into a header file
 * or somewhere more appropriate.
 */
static char transa = 'N', transb = 'N';
static int rows_a = 6, cols_b = 6, rowcol = 6, rows = 6, cols = 6;
static int lda = 6, ldb = 6, ldc = 6;
static double alpha = 1, beta = 0;
static int info;
static int lwork = 37;
static int incx = 1, incy = 1;


/**************************************************/
/* df20: triangle discomposition equation solver  */
/**************************************************/
/* kk: records which blocks of a lower triangular matrix are
 * non-zero.  Final value of this array is set in df08,
 * in the new subfunction reorderK.
 * k1:
 * n[i][1]: points at the start of each in lower triangular
 * matrix of non-zero storage positions kk;
 * n[i][2]: value is the number of elements in ith row
 * of lower triangular matrix.  This can be used with
 * the first value (n[i][[1]) to loop over the entries
 * in a lower triangular matrix.
 */
void df20(double ** K, double ** F, int *kk,int **n, int nBlocks)
{
   int i, i3;
   int j, j0, j3;
   int i9,j9;
   int h, h1;  // was l, l1; bad letters for indices.
   int currColIthRow; //i2
   int endColIthRow;  //i1, diagi?
   int currColJthRow; //j2
   int endColJthRow;  //j1, diagj?
   const int blocksize = 6;
   const int index = 1;

   //double worka[37] = {0};
   //double workb[37] = {0};

   double e[7][7];
   double qq[7][7];

  /* L U triangle   A=L D^(-1) L^T  i1:i-i j1:j0-j0 */
   for (i=1; i<= nBlocks; i++)
   {
     /* If the ith block doesn't contact any other
      * blocks, that is, if n[i][2] == 1, jump to
      * c001, and solve for this block.
      */
     /* Another way to see this is testing whether or
      * not this is a block on the diagonal, which means
      * we need to do the inversion.  Write some test
      * code to figure this out.
      */
     /* if(MACRO_noContacts(n[i][2])) */
      //if (n[i][2]==1)
      //   goto c001;  /* this conditional can be switched to !=,
       //  then all the following code scoped with braces. */
      if (n[i][2]!=1)   /* then we contact other blocks */
      {
        /* For the ith row, j traverses over the columns. */
         for (j=n[i][1]; j<= n[i][1]+n[i][2]-1; j++)
         {
           /* i1 records the index at which the last
            * (diagonal?) element of the ith row is
            * stored in kk.  Example, if i = 3, and
            * block 3 contacts block 2 and block 1,
            * which do not contact each other, than the
            * value of i1 should be 5.
            */
            //i1 = n[i][1]+n[i][2]-1;
            endColIthRow = n[i][1]+n[i][2]-1;
           /* Since j loops over the indices in n[i][],
            * j0 stores which particular entry of the lower
            * triangular matrix we are working with.
            * i.e., j0 strides over columns
            */
            j0 = kk[j];
           /* Now, we are in the ith row and the jth column of
            * a lower triangular matrix.  ????
            */
            if (n[j0][2]==1)
               continue; //goto c002;  //continue;

           /* j1: index location of end of jth row */
            //j1 = n[j0][1]+n[j0][2]-1;
            endColJthRow = n[j0][1]+n[j0][2]-1;
           /* i2: index location of start of ith row in kk */
            //i2 = n[i ][1];
            currColIthRow = n[i][1];
           /* j2: index location of start of j0th row in kk */
            //j2 = n[j0][1];
            currColJthRow = n[j0][1];
           /* i:i row  j:j0 row  i2:k j2:k      i-j non-zero */

/* If we have to come back to this label, we's in deep doo doo.
 * The code shows an infinite loop.
 */
c005:;
            //if (kk[i2]>=kk[j2]) goto c003;
            if (kk[currColIthRow] < kk[currColJthRow]) //goto c003;
            {
/* This label is very nasty reentrant code */
c006:;
               currColIthRow += 1;
              /* This looks like the end-of-row condition */
               if (currColIthRow > endColIthRow-1 /*i1-1*/)
                  continue; //goto c002;
            }
//c003:;
            //if (kk[i2]<=kk[j2]) goto c004;
            if (kk[currColIthRow] > kk[currColJthRow])
            {
               currColJthRow += 1;
               if (currColJthRow > endColJthRow-1 /*j1-1*/)
                  continue; //goto c002;
            }
/* If we get to this label, the next if statement should
 * ALWAYS be false, otherwise it should be an infinite
 * loop.  Check with assert().  But not true.  The bolt
 * analysis triggers the assert.
 */
//c004:;
            if (kk[currColIthRow]!=kk[currColJthRow])
            {
               //assert(0);
               goto c005;
            }
           /* else this is what we really want to do anyway: */
            j3  = kk[currColIthRow];

/*
            if (0)  // blas
            {
               alpha = 1.0;
               beta = 0.0;
               transa = 'T';
               transb = 'T';
               dgemm_(&transa, &transb, &rows_a, &cols_b, &rowcol,
                      &alpha, K[currColIthRow]+index, &lda,
	                   Dinv[j3]+index, &ldb, &beta, worka+index, &ldc);

               transa = 'N';
               transb = 'N';
               dgemm_(&transa, &transb, &rows_a, &cols_b, &rowcol,
                      &alpha, worka+index, &lda,
	                   K[currColJthRow]+index,&ldb, &beta, workb+index, &ldc);


               for (h =index; h < blocksize+index;  h++)
               {
                  for (h1=index; h1< blocksize+index; h1++)
                  {
                     i3 = blocksize*(h-index)+h1;
                     K[j][i3] += -workb[6*(h1-1)+h];
                  }
               }

               //print1DDoubleArray(workb, 37, "df20");
*/
            //}
            //else  // ghs factor
            //{
              /* Compute the lower triangular blocks using
               * formula 5.9 (or 5.12) Shi 1988, Section 5.2,
               * p. 197.  All of this could probably be speeded
               * up quite a bit.
               */
              /* L_{ik} L_{kk}^{-1}   Q*E => Q      i2:i-k  k=j3 */
              /* FIXME: Try and work out some way of doing a memcpy here.
               * It can be worked out and tested in the unit testing
               * code for matrices.
               */
               for (h=index; h < blocksize+index;  h++)
               {
                  for (h1=index; h1< blocksize+index; h1++)
                  {
                     i3 = blocksize*(h-index)+h1;
                     qq[h][h1] = K[currColIthRow][i3];
                     e[h][h1] = Dinv[j3][i3];
                  }  /*  l1 */
               }  /*  l  */

              /* "Returns" qq */
               //mult(T, e, qq);
               multnew(e, qq);

              /* Q=L_ik L_kk^-1 E=L_jk^T     E--E^T    Q*E => Q */
               for (h =index; h < blocksize+index;  h++)
               {
                  for (h1=index; h1< blocksize+index; h1++)
                  {
                     i3       = blocksize*(h-index)+h1;
                     e[h1][h] = K[currColJthRow][i3];
                  }  /*  l1 */
               }  /*  l  */

              /* "Returns" qq */
               //mult(T, e, qq);
               multnew(e, qq);

              /* L_{ij} = A_{ij} - \sum_{k=1}^j-1 L_{ik} L_{kk}^{-1} L_{jk}^T    */
               for (h =index; h < blocksize+index;  h++)
               {
                  for (h1=index; h1< blocksize+index; h1++)
                  {
                     i3 = blocksize*(h-index)+h1;
                     K[j][i3] += -qq[h][h1];
                  }  /*  l1 */
               }  /*  l  */
               //print2DMat(qq,6,6,"df20");


            //}  //   end if blas else ghs factor

           /* This label jumps essentially to the inside of an
            * if statement.
            */
            goto c006;
//c002:;
         }  /*  j  */
      }  /* end if non-diagonal block (???) */


     /* Jump to here directly if block is "self-intersecting",
      * or on diagonal I think...
      * otherwise fall-through from previous loop.
      */
      i9=n[i][1]+n[i][2]-1;

      //if (0) // lapack inverse
      //{
      //   memcpy((void*)Dinv[i],(void*)K[i9],(size_t)doublesize*(blocksize*blocksize+index));
        /*
         INFO    (output) INT
            = 0:  successful exit
            < 0:  if INFO = -i, the i-th argument had an illegal value
            > 0:  if INFO = i, U(i,i) is exactly zero. The factorization
                  has been completed, but the factor U is exactly
                  singular, and division by zero will occur if it is used
                  to solve a system of equations.
         */
         //rows = cols = lda = 6;
      //   dgetrf_(&rows, &cols, Dinv[i]+index, &lda, ipiv, &info);
      //   dgetri_(&rows, Dinv[i]+index, &lda, ipiv, workc, &lwork, &info);
      //}
      //else
      //{
        /* inverse of diagnal 6*6 submatrices  Q^-1 => E  */
//c001:;
         //endIthRow=n[i][1]+n[i][2]-1;

         for (j=1; j<= 6; j++)
         {
            for (h=1; h<= 6; h++)
            {
               j9 = 6*(j-1)+h;
               qq[j][h] = K[i9][j9];
            }  /*  l  */
         }  /*  j  */

         invr(e, qq);

	     /* printf("Inverted Kii matrix for block %d:\n", i); */
         for (j=1; j<= 6; j++)
         {
            for (h=1; h<= 6; h++)
            {
               j9 = 6*(j-1)+h;
               Dinv[i][j9] = e[j][h];
            }  /*  l  */
         }  /*  j  */

          //printDinv(Dinv, i, "df20");


      //}  /* end inverse if block */


   }  /* i  end loop over each row in lower triangular matrix. */

}  /* Close df20 */


/**************************************************/
/* df21: backward and forward substitution        */
/**************************************************/
void df21(double ** K, double ** F,int *kk, int **n,int nBlocks)
{
   int i, i1;
   int j, ji;
   int l, l1;
   double e[7][7];
   double qq[7][7];
   double s[31];
   //int startindex, stopindex;
   //double T[7][7];
   int index = 1;  // or 0
   int blocksize = 6;

  /* F_i - \sum_{k=1}^(i-1) L_{ik} Y_k  i:i k:i1=kk[j]   back */
   for (i=1; i<=nBlocks; i++)
   {

      for (j=n[i][1]; j<=n[i][1]+n[i][2]-2; j++)
      {
         i1=kk[j];

         for (l =1; l <= 6;  l++)
         {
            for (l1=1; l1<= 6; l1++)
            {
               F[i][l] += -K[j][6*(l-1)+l1]*F[i1][l1];
            }  /*  l1 */
         }  /*  l  */

      }  /*  j  */

     /* Y_i = L_{ii}^{-1} (F_i - \sum_{k=1}^{i-1} L_{ik} Y_k) back */
      for (j=1; j<= 6; j++)
      {
         s[j]  = 0;

         for (l=1; l<= 6; l++)
         {
            s[j] += Dinv[i][6*(j-1)+l]*F[i][l];
         }  /*  l  */

      }  /*  j  */

      for (j=1; j<= 6; j++)
      {
         F[i][j] = s[j];
      }  /*  j  */
   }  /*  i  */


  /* find L_ki^T  k>i  i:i k:j=>j1   fore */
   for (i=nBlocks-1;    i>=1;                 i--)
   {

      for (j=i+1;     j<=nBlocks;                j++)
      {

         for (l=n[j][1]; l<=n[j][1]+n[j][2]-1; l++)
         {
            ji=l;
		      if (kk[l]==i)
               goto c102;   //  break
         }  /*  l  */

         continue; //goto c101;   // continue;
 c102:;

/*
         if(0)  // blas
         {
            transa = 'N';
            alpha = 1;
            beta = 0;
            dgemv_(&transa, &rows, &cols, &alpha, K[ji]+index, &lda, F[j]+index, &incx, &beta, worka, &incy);
            transa = 'T';
            dgemv_(&transa, &rows, &cols, &alpha, Dinv[i]+index, &lda, worka, &incx, &beta, workb, &incy);

            for (l =index; l < blocksize+index;  l++)
            {
                  F[i][l] += -workb[l];
            }

         }
*/
         //else   // ghs
         //{

           /* Q=L_ii^(-1)  E=L_ki^T  E => E^T   fore  */
            for (l=1; l <= 6;  l++)
            {
               for (l1=1; l1<= 6; l1++)
               {
                  qq[l][l1] = Dinv[i ][6*(l-1)+l1];
                  e[l1][l] = K[ji][6*(l-1)+l1];
               }  /*  l1 */
            }  /*  l  */

           /* E=L_ki^T  F[j]=X_k  E*F => T=L_ki^T X_k   fore */
            for (l=1;   l<= 6;  l++)
            {
               T[l][1]=0;
               for (l1=1; l1<= 6; l1++)
               {
                  T[l][1]+=e[l][l1]*F[j][l1];
               }  /*  l1 */
            }  /*  l  */

           /* X_i = Y_i - S_(k=i+1)^n L_ii^(-1) L_ki^T X_k   */
            for (l =1;  l<= 6;  l++)
            {
               for (l1=1; l1<= 6; l1++)
               {
                  F[i][l] += -qq[l][l1]*T[l1][1];
               }  /*  l1 */
            }  /*  l  */

         //}

//c101:;
      }  /*  j  */


   }  /*  i  */


}   /*  Close df21 */


/**************************************************/
/* mult: muliplication of 6*6 matrices       0003 */
/**************************************************/
/* l1  l2  l3  t[][]  qq[][]                       */
void mult(double t[][7], double e[][7], double qq[][7])
{
  /* Array indices.  */
   int i, j, k;

   for (i=1; i<= 6; i++)
   {
      for (j=1; j<= 6; j++)
      {
        /* FIXME: This assignment should not
         * be necessary.
         */
         t[i][j]=0;
         for (k=1; k<= 6; k++)
         {
            t[i][j] += qq[i][k]*e[k][j];
         }  /*  l3 */
      }  /*  l2 */
   }  /*  l1 */


  /* This appears to be a simple cloning loop.
   * FIXME: Check to see if we can get rid of
   * the t array in the previous, and just use
   * the qq array.
   */
   for (i=1; i<= 6; i++)
   {
      for (j=1; j<= 6; j++)
      {
         qq[i][j]=t[i][j];
      }  /*  l2 */
   }  /*  l1 */

}  /* Close mult() */

/**************************************************/
/* mult: muliplication of 6*6 matrices       0003 */
/**************************************************/
/* l1  l2  l3  t[][]  qq[][]                       */
//void mult(double t[][7], double e[][7], double qq[][7])
void multnew(double e[][7], double qq[][7])
{
  /* Array indices.  Should be renamed i, j, k just to
   * conform to accepted practice.
   */
   int i, j, k;
  /* FIXME: Explain why t needs to be static?
   * FIXME: Init t[7][7] = {{0}} here instead of in
   * the loop.
   */
   static double t[7][7];

   for (i=1; i<= 6; i++)
   {
      for (j=1; j<= 6; j++)
      {
        /* FIXME: This assignment should not
         * be necessary.
         */
         t[i][j]=0;
         for (k=1; k<= 6; k++)
         {
            t[i][j] += qq[i][k]*e[k][j];
         }  /*  l3 */
      }  /*  l2 */
   }  /*  l1 */


  /* This appears to be a simple cloning loop.
   * FIXME: Check to see if we can get rid of
   * the t array in the previous, and just use
   * the qq array.
   */
  /* FIXME: If this can't be removed, change to
   * a memcpy.
   */
   for (i=1; i<= 6; i++)
   {
      for (j=1; j<= 6; j++)
      {
         qq[i][j]=t[i][j];
      }  /*  l2 */
   }  /*  l1 */

}  /* Close multnew() */



/**
 * 6-by-6 gemm taking arrays as arguments, not
 * pointers to arrays.  Second argument is
 * overwritten as a return value.
 */
void
multnewnew(double e[][7], double qq[][7])
{
   int i, j, k;
   double t[7][7] = {0};

   for (i=1; i<= 6; i++)
   {
      for (j=1; j<= 6; j++)
      {
         for (k=1; k<= 6; k++)
         {
            t[i][j] += qq[i][k]*e[k][j];
         }
      }
   }

   memcpy(qq,t,sizeof(t));

}  /* Close multnewnew() */

/**************************************************/
/* invr: inverse of 6*6 matrix               0002 */
/**************************************************/
/* jj  j  l  l1  e[][]  qq[][]                     */
/* WARNING!!!  e is overwritten for return, but
 * q is also overwritten.
 */
void invr(double e[][7], double qq[][7])
{

   int j, jj, l, l1;

   //printf2Dmat(qq, 7, 7, " from invr");

  /* (GHS: E=I as n=6 free term columns)  */
  /* This loop zeroing out the matrix may be unnecessary
   * if the matrix is initialized to zero.  Leave the loop
   * for now.
   */
  /* FIXME: do a memset to 0, then a single loop to
   * initialize the diagonals to 1.
   */
   for (j=1; j<= 6; j++)
   {
      for (l=1; l<= 6; l++)
      {
         e[j][l]=0;
      }  /*  l  */
      e[j][j]=1;
   }  /*  j  */

  /* L_ij = A_ij - S_(k=1)^j-1 L_ik L_kk^-1 L_jk^T  */
   for (j =1; j <= 6;    j++)
   {
      for (l =1; l <= j;    l++)
      {
         for (l1=1; l1<= l-1; l1++)
         {
            qq[j][l] += -qq[j][l1]*qq[l][l1]/qq[l1][l1];
         }  /*  l1 */
      }  /*  l  */
   }  /*  j  */

  /* Y_i = L_ii^-1 (F_i - S_k=1^i-1 L_ik Y_k)  back */
   for (jj=1; jj<= 6; jj++)
   {
      for (j =1; j <= 6;  j++)
      {
         for (l =1; l <=j-1; l++)
         {
            e[j][jj] += -qq[j][l]*e[l][jj];
         }  /*  l  */
         e[j][jj] /= qq[j][j];
      }  /*  j  */
   }  /*  jj */


  /* X_i = Y_i - S_(k=i+1)^n L_ii^(-1) L_ki^T X_k   */
   for (jj=1; jj<= 6; jj++)
   {
      for (j=5;   j>= 1;  j--)
      {
         for (l=j+1; l<= 6;  l++)
         {
            e[j][jj] += -qq[l][j]*e[l][jj]/qq[j][j];
         }  /*  l  */
      }  /*  j  */
   }  /*  jj */

//printf2Dmat(e, 7, 7, " from invr");

}  /*  Close invr().  */


/* Try to match the blas or lapack calling parameters.
 * This function multiplies a square block matrix A
 * stored in row form by a vector x to produce a vector
 * y. The variable `index' handles storage from non-zero
 * initial pointer, and requires that all arguments use
 * the same indexing scheme whether starting from 0, 1,
 * or whatever.
 */
/* For blockwise Ax, this function should be callable
 * as many times as there are actual elements in the
 * row of A, which is sparse.
 */
/* This function is equivalent to dgemv. */
void
sp_mult(double * Arow, double * x, double * y, int blocksize, int index)
{
   int i,j,k;

   for (i=index;i<blocksize+index;i++)
   {
      k = blocksize*(i-index);
      for (j=index;j<blocksize+index;j++)
         y[i] +=  Arow[k+j]*x[j];
   }

}  /* close sp_mult() */



/* Writes a matlab file of stiffness and load vectors. */
void
writeMFile(double ** K, double ** F, double ** D,
           int * kk, int * k1, int ** n, int numblocks) {

   int I,J;  // Loop over sub-block
   int row;  // was i
   int colindex, column;  // was j
   int i;

   int startrow, stoprow;
   int startcol, stopcol;
   int startindex, stopindex;
   //int i1;
   //int kii;
   //int currIcol, endIcol;
   //int currJcol, endJcol;
   const int blocksize = 6;

  /* Set up initial stiffness matrix K */
   fprintf(fp.mfile,"bs = %d; %% blocksize\n",blocksize);
   fprintf(fp.mfile,"nb = %d; %% number of blocks\n",numblocks);
   fprintf(fp.mfile,"K = zeros(bs*nb,bs*nb);\n");
   fprintf(fp.mfile,"F = zeros(bs*nb,1);\n");
   fprintf(fp.mfile,"D = zeros(bs*nb,1);\n");


  /* Now write out diagonal submatrices. */

  /* FOR EACH ROW */
   for (row=1;row<=numblocks;row++) {

      startindex = n[row][1];
      stopindex = n[row][1] + n[row][2] - 1;

      startrow = (blocksize*(row-1)) + 1;
      stoprow = blocksize*row;


     /* FOR EACH COLUMN IN ROW */
      for (colindex=startindex;colindex<=stopindex;colindex++) {

         column = kk[colindex];

         startcol = blocksize*(column-1) + 1;
         stopcol = blocksize*column;

        /* Write the lower triangular part... */
         fprintf(fp.mfile,"K(%d:%d,%d:%d) = [\n",startrow,stoprow,startcol,stopcol);
         for (I=1;I<=blocksize;I++) {
            for (J=1;J<=blocksize;J++) {

               fprintf(fp.mfile," %30.16e ",K[colindex][blocksize*(I-1)+J]);
            }
            fprintf(fp.mfile,"\n");
         }
         fprintf(fp.mfile,"];\n");



        /* and the upper triangular part. */
         fprintf(fp.mfile,"K(%d:%d,%d:%d) = [\n",startcol,stopcol,startrow,stoprow);
         for (I=1;I<=blocksize;I++)
         {
            for (J=1;J<=blocksize;J++)
            {
               fprintf(fp.mfile," %30.16e ",K[colindex][blocksize*(I-1)+J]);
            }
            fprintf(fp.mfile,"\n");
         }
         fprintf(fp.mfile,"]';\n");



      }
      //fprintf(fp.mfile,"];\n");
   }

  /* Write the forcing vector */

   fprintf(fp.mfile,"%% Forcing vector is already permuted.\n");
   for (i=1;i<=numblocks;i++)
   {
      row = k1[i];
      startrow = (blocksize*(row-1)) + 1;  // + index;
      stoprow = blocksize*row;

      fprintf(fp.mfile,"F(%d:%d,1) = [\n",startrow,stoprow);

            for (J=1;J<=blocksize;J++)
            {
               fprintf(fp.mfile," %30.16e\n ",F[row][J]);
            }
         fprintf(fp.mfile,"];\n");

   }

  /* Write the solution vector. */
   fprintf(fp.mfile,"%% Solution vector.\n");
   for (i=1;i<=numblocks;i++) {
      row = k1[i];
      startrow = (blocksize*(row-1)) + 1;  // + index;
      stoprow = blocksize*row;

      fprintf(fp.mfile,"D(%d:%d,1) = [\n",startrow,stoprow);

      for (J=1;J<=blocksize;J++) {
         fprintf(fp.mfile," %30.16e\n ",D[row][J]);
      }

      fprintf(fp.mfile,"];\n");
   }


}  /* close writeMFile() */






/* Writes a matlab file of stiffness and load vectors. */
void
writeSolutionVector(double ** D, int * kk, int * k1, int ** n, int numblocks) {
   int i;
   int J;  // Loop over sub-block
   int row;  // was i
   //int colindex, column;  // was j
   int startrow, stoprow;
   //int startcol, stopcol;
   //int startindex, stopindex;
   //int i1;
   //int kii;
   //int currIcol, endIcol;
   //int currJcol, endJcol;
   const int blocksize = 6;
   /* Write the solution vector. */
   //fprintf(fp.mfile,"%% Solution vector.\n");

   for (i=1;i<=numblocks;i++) {
      row = k1[i];
      startrow = (blocksize*(row-1)) + 1;  // + index;
      stoprow = blocksize*row;

      //fprintf(fp.mfile,"D(%d:%d,1) = [\n",startrow,stoprow);

      for (J=1;J<=blocksize;J++) {
         fprintf(fp.dfile," %30.16f ",D[row][J]);
      }
   }

   fprintf(fp.dfile,"\n");

}  /* close writeSolutionVector() */

