/*
 * lns.c
 *  
 * Subroutine to determine whether two line segments
 * intersect.
 *
 * $Author: doolin $
 * $Date: 2001/05/20 21:00:08 $
 * $Source: /cvsroot/dda/ntdda/src/lns.c,v $
 * $Revision: 1.1 $
 */


#include "geometry.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
//#include "resource.h"
#include <string.h>
#include "ddamemory.h"


/**************************************************/
/* lns: intersection point of two lines           */
/**************************************************/
/* double:  x21 y21 x31 y31 x34 y34 x41 y41      */
/* int: kk3, double: t1, t2;  double:d0, d1,d2, a1, b1 */
/* lns determines whether lines defined by two point
 * sets intersect.  It is called by dc03(), dc04(),
 * dc10() and dc13().
 */
int lns(double w0, double x21, double x31, double x34, double x41,
		 double y21, double y31, double y34, double y41, double *pt1, double *pt2)

{
   double c1;
   double d0, d1, d2;  /* Determinants */
   double b1;
   double a1;
   int local_kk3;
  /* t1 and t2 are parameters in the equation of
   * two lines.  For line segments, 0 <= t1 <= 1
   * and 0 <= t2 <= 1.  These are given as t and T
   * in GHS diss. page 181. or BSM page 148.  
   * O'Rourke uses r and s as the parameters, which 
   * is probably more standard.
   */
   /* FIXME: Big problem here b/c t1 and t2 may be returned to the 
    * calling function uninitialized.
    */
   //double t1, t2;
   /* FIXME: Test whether this works correctly.
    */
    double t1 = 0;
    double t2 = 0;


  /* kk3=0 no intersection.
   * kk3=1 output t1 t2 parameters.  
   */
  /* kk3 probably is a global flag.  */
   local_kk3=0;

  /* d0, d1, d2 are determinants.
   */
   d0=x21*y34-x34*y21;   /* equation | |   */
   d1=x31*y34-x34*y31;   /* | | for sol t1 */
   d2=x21*y31-x31*y21;   /* | | for sol t2 */
     
  /* p1 p2 p3 p4  4 points co-line   */
   if ((fabs(d0)<(.000000001*w0*w0)) && (fabs(d1)<(.000000001*w0*w0)))  
      goto ln02;
     
  /* p1 p2   p3 p4  are two parallel lines */
   if ( fabs(d0) < (.000000001*w0*w0) )  
      goto ln01;  // ie, return;
   goto ln03;
      
ln02:
  /* p1 p2 p3 p4  4 points in same line. */
   a1=x21;
   b1=x31;
   if ( fabs(x21) > fabs(y21) ) 
      goto ln05;
   a1=y21;
   b1=y31;
 
ln05:
  /* (x3 y3) lies in p1 p2 segment. */
   t1=b1/a1;
   if ( (t1<-.0000001) || (t1>1.0000001) ) 
      goto ln04;
   t2=0;
   local_kk3=1;
      
ln04:
   a1=x21;
   b1=x41;
   if ( fabs(x21) > fabs(y21) ) 
	   goto ln06;
   a1=y21;
   b1=y41;
   
ln06:
  /* (x4 y4) lies in p1 p2 segment                  */
  /* both p3 p4   in p1 p2 segment no intersection  */
   c1=b1/a1;

   if ( (c1<-.0000001) || (c1>1.0000001) ) 
      goto ln01;  /* ie. return;  */

   t1=b1/a1;
   t2=1;
   local_kk3 += 1;  // kk3 was global.  ---dmd
   if ( local_kk3 == 2 )  
	     local_kk3=0;
   goto ln01;  /*  ie, return; ---dmd */
   //return local_kk3=0;

ln03:
  /* Normal intersection.  */
   t1=d1/d0;
   if ( (t1<-.0000001) || (t1>1.0000001) ) 
      goto ln01;
   t2=d2/d0;
   if ( (t2<-.0000001) || (t2>1.0000001) ) 
      goto ln01;
   local_kk3=1;

ln01:

   /* FIXME: There is a problem with *pt1,*pt2 reading uninitialized 
    * data here.
    */
   *pt1 = t1;
   *pt2 = t2;

   return local_kk3; 

}  /*  Close lns().  */

