/*
 * ddamemory.c
 *
 * General memory functions to malloc and free specific
 * data structures in dda.  
 *
 * $Author: doolin $
 * $Date: 2002/05/19 16:43:03 $
 * $Log: ddamemory.c,v $
 * Revision 1.2  2002/05/19 16:43:03  doolin
 * More general source cleanup.
 *
 * Revision 1.1.1.1  2001/05/20 21:00:48  doolin
 * This import corresponds to development version 1.5.124
 * *
 */

#include <stdlib.h>
#include <memory.h>
#include<assert.h>
#include "ddamemory.h"



void copy2DArray(void ** out, void ** in, int n, int m)
{
   int doublesize;
   int i;

   doublesize = sizeof(double);

   for (i=0;i<n;i++)
   {
      memcpy((void*)out[i],(void*)in[i],(size_t)(m*doublesize));
   }
}  /* close copy2DArray */




int **
IntMat2DGetMem(int n, int m)
{
   int i;
   int **x;

assert ( (m!=0) && (n!=0) );

   //x = (int **)malloc(sizeof(int *)*n);
   x = (int **)calloc(n,sizeof(int *));
  /* This should be changed to call a "Out of memory,
   * close some applications" error.  For now, pass a 
   * NULL pointer back and trap the failure elsewhere.
   */
   if (x == NULL)
      return NULL;

   for ( i = 0; i < n; i++)
   {
      //x[i] = (int *)malloc(sizeof(int)*m);
      x[i] = (int *)calloc(m,sizeof(int));
      memset(x[i],0,m*sizeof(int));
      /* Error checking code here...
       */
      if(x[i] == NULL)
         return NULL;
      //else 
         //memset(x[i], 0xDDA, m);
   }

   return x;

}  /* Close IntMat2DGetMem().  */



double **
DoubMat2DGetMem(int n, int m)
{
   int i;
   double **x;

   assert ( (m!=0) && (n!=0) );

   //x = (double **)malloc(sizeof(double *)*n);
   x = (double **)calloc(n,sizeof(double *));
   if (x == NULL)
      return NULL;

   for ( i = 0; i < n; ++i)
   {
      //x[i] = (double *)malloc(sizeof(double)*m);
      x[i] = (double *)calloc(m,sizeof(double));
      memset(x[i],0,m*sizeof(double));
      if(x[i] == NULL)
         return NULL;
      //else 
         //memset(x[i], 0xDDA, m);
   }

   return x;

}  /* Close DoubMat2DGetMem().  */


double **
clone2DMatDoub(double ** mat,int n, int m)
{
   int i;
   int j;
   double **x;

   assert ( (m!=0) && (n!=0) );

   x = DoubMat2DGetMem(n, m);


   for (i=0; i<n;i++)
      for (j=0;j<m;j++)
         /* FIXME: Change to a memset */
         x[i][j] = mat[i][j];

   return x;

}  /* Close clone2DMatDoub().  */

int **
clone2DMatInt(int ** mat,int n, int m)
{
   int i;
   int j;
   int **x;

assert ( (m!=0) && (n!=0) );

x = IntMat2DGetMem(n, m);

   for (i=0; i<n;i++)
      for (j=0;j<m;j++)
         x[i][j] = mat[i][j];

   return x;

}  /* Close clone2DmatInt().  */

int *
clone1DMatInt(int * mat,int m)
{
   int i;
   int *x;

assert (m!=0);

x = (int *)calloc(m, sizeof(int));

   for (i=0; i<m;i++)
         x[i] = mat[i];

   return x;

}  /* Close clone2DmatInt().  */



#if  CENTROIDS
void
initCentroids(GRAPHICS *g, int nBlocks)
{
  /* Centroid data is useful in part for checking analytic
   * solutions, and for displacement dependent variables.
   * These can be plotted with a user option "Trajectories".
   * FIXME: Add an initialization function pointer to the
   * dl struct to take care of mallocing.
   */
#if DLCENTROIDS   
   g->centroidsize1 = AData->nTimeSteps+1;
   g->centroidsize2 = nBlocks+1;
   g->centroids = (DPoint **)malloc(sizeof(DPoint *)*(g->centroidsize1));
   for (i=0;i<=AData->nTimeSteps;i++)
      dlo->centroids[i] = (DPoint *)malloc(sizeof(DPoint)*(g->centroidsize2));
#else
   g->centroidsize1 = 0;
   g->centroidsize2 = 0;
   g->centroids = NULL;
#endif

}
#endif





void 
free2DMat(void ** mat, int m)
{
   int i;

   if (mat == NULL)
   { 
      /* Should probably be warnings instead of errors,
       * and in any case should throw a NULL pointer
       * warning function instead of a generic error 
       * function.
       */
      //error ("Null mat passed into freemat().");
      return;
   }

   for (i=0; i < m; i++)
   {
      free(mat[i]);
   }
   free(mat);
}  /* Close free2DMat() */



void
free2Darray(void ** array, int m)
{
   int i;

   if (array == NULL)
   { 
      //error ("Null array passed into free2Darray().");
      return;
   }

   for (i=0; i < m; i++)
   {
      free(array[i]);
   }
   free(array);
}  /* Close free2Darray() */


void
free2DIntArray(int ** array, int m, int n)
{
   int i;

   if (array == NULL)
   { 
      //error ("Null array passed into free2DIntArray().");
      return;
   }

  /* Check for pointer to 0xDD or 0xDA */
   if ((int)array == 0xdadadada || (int)array == 0xdddddddd)
   {
      //error("Uninitialized array pointer passed to free2DIntArray");
      return;
   }

   for (i=0; i < m; i++)
   {
      memset(array[i],0xDD,n*sizeof(int));
      free(array[i]);
   }
   free(array);

}  /* Close free2DIntarray() */


void
free2DDoubleArray(double ** array, int m, int n)
{
   int i;

   if (array == NULL)
   { 
      //error ("Null array passed into free2DDoubleArray().");
      return;
   }

  /* Check for pointer to 0xDD or 0xDA */
   if ((int)array == 0xdadadada || (int)array == 0xdddddddd)
   {
      //error("Uninitialized array pointer passed to free2DDoubleArray");
      return;
   }

   for (i=0; i < m; i++)
   {
      memset(array[i],0xDD,n*sizeof(double));
      free(array[i]);
   }
   free(array);

}  /* Close free2DDoubleArray() */



