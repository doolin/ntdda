
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include "ddamatrix.h"

static void allocArray(DDAMatrix *, int m, int n);
static void * freeDDAMatrix(DDAMatrix *);

DDAMatrix *
getNewDDAMatrix()
{
   DDAMatrix * dm;
   dm = (DDAMatrix *)malloc(sizeof(DDAMatrix));
   memset((void *)dm, 0xda, sizeof(DDAMatrix));

   dm->alloc = allocArray;
   dm->free = freeDDAMatrix;

   return dm;
}  /* close getNewDDAMatrix() */



static void
allocArray(DDAMatrix * dm, int m, int n)
{
   int i;
   double **x;

   dm->m = m;
   dm->n = n;
   
   assert ( (m!=0) && (n!=0) );

   x = (double **)calloc(n,sizeof(double *));
   //if (x == NULL)
      //return NULL;

   for ( i = 0; i < n; ++i)
   {
      //x[i] = (double *)malloc(sizeof(double)*m);
      x[i] = (double *)calloc(m,sizeof(double));
      //if(x[i] == NULL)
      //   return NULL;
      //else 
         //memset(x[i], 0xDA, m);
   }

   dm->val = x;

}  /* close initializeArray() */


static void *
freeDDAMatrix(DDAMatrix * dm)
{
   int i;
   double ** array = dm->val;
   int m = dm->m;

  /* FIXME: Implement an error substructure. */
  /*
   if (array == NULL)
   { 
      error ("Null array passed into free2Darray().");
      return;
   }
   */

   for (i=0; i < m; i++)
   {
      free(array[i]);
   }
   free(array);

   free(dm);
  
   return NULL;

}  /* close freeDDAMatrix() */