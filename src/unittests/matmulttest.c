#include <stdio.h>
#include <math.h>
#include "datalog.h"
#include "ddafile.h"
#include "ddamemory.h"


/*  These things are really ugly.  Should try and
 * write some sort of logging code like glog.
 */

DATALOG * DLog = NULL;
FILEPOINTERS * fp = NULL;


/* Easy test matrices.  Testing should be
 * expanded to include much harder test cases,
 * which will require matlab or octave or
 * something similar.
 */
double qq[7][7] = { 
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6} };


double ee[7][7] = { 
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6} };




void
printmat(double m[7][7])
{
   int i,j;

   for (i=1;i<=6;i++)
   {
     for (j=1;j<=6;j++)
     {
        printf("%f  ", m[i][j]);
     }
     printf("\n");
   }
   printf("\n");
}


int 
equals(double computed[7][7], double true[7][7], double tol)
{

  int i,j;

  for (i=0;i<=6;i++)
  {
     for (j=0;j<=6;j++)
     {
        if ( fabs(true[i][j]-computed[i][j]) > tol)
           return 0;
     }

  }
  
  return 1;
}

int 
main() 
{

  printmat(qq);
  printmat(ee);


  multnewnew(qq,ee);

 
  if(equals(ee,ee,0.00001))
     printf("Passed\n");
  else
     printf("Failed\n");
  

  printmat(ee);

  return 0;

}


