#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "datalog.h"
#include "ddafile.h"
#include "ddamemory.h"
#include "analysis.h"



typedef enum _boolean { false = 0, true = 1 } boolean;

/* 
 * Some tests that need to be performed:
 * mat mult needs to check the multilying
 * by 0 returns a zero matrix, and that
 * multiplying by I returns the argument.
 */


/*  These things are really ugly.  Should try and
 * write some sort of logging code like glog.
 */
Datalog * DLog = NULL;
FILEPOINTERS * fp = NULL;


double  I[7][7] = { 
            {1,0,0,0,0,0,0},
            {0,1,0,0,0,0,0},
            {0,0,1,0,0,0,0},
            {0,0,0,1,0,0,0},
            {0,0,0,0,1,0,0},
            {0,0,0,0,0,1,0},
            {0,0,0,0,0,0,1} };



double zeromat[7][7] = {0};
double t[7][7] = {0};

void
printmat(double m[7][7], FILE * stream)
{
   int i,j;

   for (i=0;i<=6;i++)
   {
     for (j=0;j<=6;j++)
     {
        fprintf(stream, "%f  ", m[i][j]);
     }
     fprintf(stream, "\n");
   }
   fprintf(stream, "\n");
}


/* GHS matrix multiplication 
 * functions index from 1 to 6 instead of from 0 to 5.
 * This means that we can't rely on having anything at
 * all in the (0,:) row or (:,0) column slots.  
 */
boolean 
equals(double computed[7][7], double true[7][7], double tol)
{

  int i,j;

  for (i=1;i<=6;i++)
  {
     for (j=1;j<=6;j++)
     {
        if ( fabs(true[i][j]-computed[i][j]) > tol)
           return false;
     }

  }
 
/* There is some sort of problem with this when 
 * returning "true".  gcc sqawks.
 */ 
  return 1;
}

/* Create random entries for a matrix,
 * overwriting the argument.
 */
void
randomize(double m[7][7])
{
   int i,j;

   srandom(3789);

   for (i=0;i<=6;i++)
   {
      for (j=0;j<=i;j++)
      {
         m[i][j] = m[j][i] = (random()%1000) / 100.0 ;
      }
   }

}


/* Create random entries for a symmetric matrix 
 * overwriting the argument.
 */
void
symrandomize(double m[7][7])
{
   int i,j;

   srandom(3789);

   for (i=0;i<=6;i++)
   {
    /* Make it symmetric */
      for (j=6;j >= i;j--)
      {
         m[i][j] = m[j][i] = (random()%1000) / 100.0 ;
      }
   }

}



boolean
test_multnewnew(FILE * out)
{
/* Easy test matrices.  Testing should be
 * expanded to include much harder test cases,
 * which will require matlab or octave or
 * something similar.
 * These need to scope limited because the functions
 * we are testing overwrite various bits and pieces
 * of the arguments.
 */
double qq[7][7] = { 
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6},
            {0,1,2,3,4,5,6} };

/*  qq*qq */
double qqqq[7][7] = {
            {0,0,0,0,0,0,0},
            {0,21,42,63,84,105,126},
            {0,21,42,63,84,105,126},
            {0,21,42,63,84,105,126},
            {0,21,42,63,84,105,126},
            {0,21,42,63,84,105,126},
            {0,21,42,63,84,105,126} };


  multnewnew(qq,qq);
  //printmat(qq, out);
   
  if (equals(qq,qqqq,0.0000001))
     return true;
  else 
     return false;

}


boolean
test_invr(FILE * out)
{

  double a[7][7] = {0};
  double acopy[7][7] = {0};
  double ainv[7][7] = {0};

  symrandomize(a);
 /* Make a copy of a because invr overwrites 
  * both arguments.
  */
  memcpy(acopy,a,sizeof(a)); 
  invr(ainv,acopy);
 
/* a should be overwritten, ainv not overwritten */
  multnewnew(ainv,a);
  //printmat(a,out);

  if(equals(a,I,0.0000001))
     return true;
  else
     return false;

}


boolean 
test(FILE * out) 
{
   boolean passed = true;

   if(test_multnewnew(out))
      printf("Passed multnewnew unit test\n");
   else
   {
      printf("Failed multnewnew unit test\n");
      passed = false;
   }


   if(test_invr(out))
      printf("Passed invr unit test\n");
   else
   {
      printf("Failed invr unit test\n");
      passed = false;
   }


   return passed;

}





/* FIXME: Remove all the functionality out of this,
 * create a struct that has a member that can run the 
 * test, create a test() method that returns true 
 * or false, then make a source file to run all of
 * the tests at once, and log to somewhere easy to 
 * find.  Maybe log to an XML file that can be loaded 
 * into IE and the tree examined.
 */

int 
main() 
{

  FILE * out;

 out = stdout;

   if (test(out))
      fprintf(out,"Passed all unit tests\n");
   else 
      fprintf(out,"Failed unit testing\n");
   

  return 0;

}


