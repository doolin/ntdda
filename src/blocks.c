/*
 * blocks.c
 *
 * Renamed from GHS original dc.c block forming code.  Lots
 * needs to be done in here, not the least of which is
 * eliminating global variables, determining which block
 * forming algorithm is used, or writing up the algorithm
 * if GHS has derived a new one.  There is also some very
 * funky file handling business that needs to be cleaned up.
 *
 * $Author: doolin $
 * $Date: 2003/12/17 23:36:36 $
 * $Source: /cvsroot/dda/ntdda/src/blocks.c,v $
 * $Revision: 1.8 $
 */


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>

#include "ddamemory.h"
#include "geometry.h"
#include "dda.h" // for dda_display_warning



/* Quick hack.  Geometry will need a full suite
 * of logging files, etc.
 */
extern FILE * ddacutlog;
extern FILE * pnpfile;

/*------------------------------------------------*/
/* a : 0-1 relation matrix point to line number   */
/* a : 0-0 relation matrix first vertex           */
/* k : 0-0 relation matrix                        */
/* k : 0-0 rotation matrix next vertices          */
/* r : 0-0 relation matrix line number i i+1 edge */
/* m : point number in a line i                   */
/* m : segments in a node in a block              */
/* m : nodes in a block                           */
/* h : vertices of blocks                         */
/* k0: block index of d[]                         */
/*------------------------------------------------*/


/*------------------------------------------------*/
/* q : 0-1 length relation matrix                 */
/* q : 0-0 angle matrix                           */
/* q : value on a block                           */
/* c : coordinates of nodes                       */
/* d : distance of vertices in a line             */
/* d : angles of segments in a node               */
/* d : vertices of blocks                         */
/* b : inputed lines x1  y1  x2  y2  n            */
/* gg0:   x1  y1  x2  y2  n  of materials   nBlMat   */
/* gg1:   x1  y1  x2  y2  n1  n2  mm1  bolt  nBolt   */
/* g :   x1  y1  x2  y2  of fixed lines   +nFPoints   */
/* g :   x   y   of loading  points       +nLPoints   */
/* g :   x   y   of measured points       +nFPoints   */
/* g :   x   y   of hole     points       +nHPoints   */
/*-------------------------------------------------*/




/* FIXME: This needs a testing framework for handling debugging
 * and loggin files.
 */
static void printPoints(Geometrydata * gd, char * location, FILE *);



/**************************************************/
/* dc03: define arrays with variable dimension    */
/**************************************************/
/* dc03 calls lns() to determine line intersections.
 * Originally, since all the array were global, dc03
 * probably allocated all the memory as well.
 */
//void dc03(Geometrydata *gd)
void
countIntersections(Geometrydata * gd)
{
   int i;
   int j;

   int n2;  /* Number of intersection points. */
   int local_kk3;
   /* FIXME: t1 and t2 are passed into lns by reference and
    * set as the intersection coordinates.  Initializing them
    * may cause some side effects.
    */
   double t1;// = 0;
   double t2;// = 0;
   double ** joints = gd->joints;
   double x21, y21, x34, y34, x31, y31, x41, y41;


  /* n2 compute the number of intersection points   */
	 	n2=0;
   for (i=1; i< gd->nJoints; i++)
   {
      for (j=i+1; j<= gd->nJoints; j++)
      {
		       x21 = joints[i][3] - joints[i][1]; /* coefficient a11 */
		       y21 = joints[i][4] - joints[i][2]; /* coefficient a21 */
		       x34 = joints[j][1] - joints[j][3]; /* coefficient a12 */
		       y34 = joints[j][2] - joints[j][4]; /* coefficient a22 */
		       x31 = joints[j][1] - joints[i][1]; /* free terms  f1  */
		       y31 = joints[j][2] - joints[i][2]; /* free terms  f2  */
		       x41 = joints[j][3] - joints[i][1]; /* co-line case    */
		       y41 = joints[j][4] - joints[i][2];

        /* compute two line intersection */
        /* kk3 looks like it might be a candidate for a return
         * variable from lns().
         */
	     local_kk3 = lns(gd->w0, x21, x31, x34, x41, y21, y31, y34, y41, &t1, &t2);


        /* This is screwy, so pay attention.  kk3 seems to be
         * boolean variable that is _set_ in lns().  It's value
         * reflects whether two lines are parallel or not.
         */
	       	if (local_kk3 == 0)     //    if (kk3 != 0)
		         	goto a301;     //       n2++;
	       	n2 += 1;          // } /*  j  */

a301:
         ;  /* Do nothing, i.e., continue.  */
      }
   }

   gd->nIntersectionPoints = n2;

}


/**************************************************/
/* dc04: intersection of lines                    */
/**************************************************/
/* This is where things start to get hairy, due to a
 * function call to lns(), which sets some global
 * variables used by the calling (dc04) routine.
 * So far, I know that dc04() dpends on the value of
 * kk3 and t1 set in lns().
 */
/* FIXME: The rest of the gotos can be driven out of here
 * pretty easily.
 */
/* FIXME: c is nodes?  c (nodesmaybe) comes in already loaded. */
void
dc04(Geometrydata *gd, int **aa, double **q, double ** nodesmaybe)
{
   int i,j,ell;
   int i0, i1;
   int kk1, kk2;
  /* Probably the coordinates of the intersection point. */
   double x0, y0;
  /* a1 is probably a reference line length */
   double a1;
   int num_int_section = gd->nIntersectionPoints;
   double w0 = gd->w0;
   double ** joints = gd->joints;
  /* Parameters of intersection. */
   double t1, t2;
  /* TRUE if lines intersect, FALSE otherwise */
   int INTERSECTION;
  /* Tolerance */
   double edgenodedist = gd->e00;

   double x21, y21, x34, y34, x31, y31, x41, y41;


  /* maximum 10 lines intersect at a point  */
  /* This is not a particularly clever piece of code,
   * and the n2 variable is determined in dc03().
   */
   for (i=1; i<= num_int_section; i++)
   {
      for (j=1; j<=10; j++)
      {
         aa[i][j]=0;
      }  /*  j  */
   }  /*  i  */

   num_int_section = 0;

   for (i=2; i<= gd->nJoints; i++)
   {
      i0 = i-1;

      if ( (i-1) > gd->nBoundaryLines)
         i0 = gd->nBoundaryLines;

      for (j=1; j<= i0; j++)
      {
        /* FIXME: Find the page number to reference this */
         x21 = joints[i][3] - joints[i][1]; /* coefficient a11 */
         y21 = joints[i][4] - joints[i][2]; /* coefficient a21 */
         x34 = joints[j][1] - joints[j][3]; /* coefficient a12 */
         y34 = joints[j][2] - joints[j][4]; /* coefficient a22 */
         x31 = joints[j][1] - joints[i][1]; /* free terms  f1  */
         y31 = joints[j][2] - joints[i][2]; /* free terms  f2  */
         x41 = joints[j][3] - joints[i][1]; /* co-line case    */
         y41 = joints[j][4] - joints[i][2];

        /* compute two line intersection                  */
	       /* lns() computes globals for t2 and kk3.  This is a
	        * real PITA.
	        */
         INTERSECTION = lns(gd->w0, x21, x31, x34, x41, y21, y31, y34, y41, &t1, &t2);

         if (INTERSECTION == FALSE)
			         continue;  // goto a401;

         x0 = joints[i][1] + x21*t1;
         y0 = joints[i][2] + y21*t1;


        /* if points coincide  n2 updating point number   */
         if (i <= gd->nBoundaryLines)
            goto a402;

         for (ell=1; ell<= num_int_section; ell++)
         {
           /* a1 is probably a reference line length */
            a1 = sqrt((nodesmaybe[ell][1]-x0)*(nodesmaybe[ell][1]-x0)
                    +(nodesmaybe[ell][2]-y0)*(nodesmaybe[ell][2]-y0));
            if ( a1 > (1.25*edgenodedist*w0) )
               continue;  // goto a403;  // continue;
            i1=ell;
            goto a404;
//a403:;
         }

a402:;
         num_int_section += 1;
         i1  = num_int_section;
         nodesmaybe[i1][1]=x0;
         nodesmaybe[i1][2]=y0;

a404:;

        /* (GHS: registrate segment i)   */
         for (ell=1; ell<= 10; ell++)
         {
            kk1=ell;
            if ( (aa[i1][ell]==i) || (aa[i1][ell]==0) )
			            break;  // goto a405;
         } /* ell */

//a405:;
         aa[i1][kk1]=i;
         q[i1][kk1]=t1;

        /* (GHS: register segment j)  */
         for (ell=1; ell<= 10; ell++)
         {
            kk2=ell;
            if ( (aa[i1][ell]==j) || (aa[i1][ell]==0) )
               break;  // goto a406;
         } /* ell */

//a406:;

         aa[i1][kk2]=j;
         q[i1][kk2]=t2;

//a401:;
      } /* j */
   } /* i */


   for (i=2;    i<= gd->nJoints;  i++)
	  {
      for (j=gd->nBoundaryLines+1; j<= i-1; j++)
      {
         x21=joints[i][3]-joints[i][1];  /* coefficient a11 */
         y21=joints[i][4]-joints[i][2];  /* coefficient a21 */
         x34=joints[j][1]-joints[j][3];  /* coefficient a12 */
         y34=joints[j][2]-joints[j][4];  /* coefficient a22 */
         x31=joints[j][1]-joints[i][1];  /* free terms  f1  */
         y31=joints[j][2]-joints[i][2];  /* free terms  f2  */
         x41=joints[j][3]-joints[i][1];  /* co-line case    */
         y41=joints[j][4]-joints[i][2];

        /* compute two line intersection  */

         INTERSECTION = lns(gd->w0, x21, x31, x34, x41, y21, y31, y34, y41, &t1, &t2);


         if (INTERSECTION == FALSE)
            continue;  // goto a411;

         x0 = joints[i][1] + x21*t1;
         y0 = joints[i][2] + y21*t1;

        /* if points coinside  n2 updating point number   */
         if (i <= gd->nBoundaryLines)
            goto a412;

         for (ell=1; ell<= num_int_section; ell++)
         {
            a1=sqrt((nodesmaybe[ell][1]-x0)*(nodesmaybe[ell][1]-x0)
                    +(nodesmaybe[ell][2]-y0)*(nodesmaybe[ell][2]-y0));
            if ( a1 > (1.25*edgenodedist*w0) )
               continue;  // goto a413;  // continue;
            i1 = ell;
            goto a414;
//a413:;
         }

a412:;
         num_int_section += 1;
         i1  = num_int_section;
        /* Looks we are storing the intersection point. */
         nodesmaybe[i1][1] = x0;
         nodesmaybe[i1][2] = y0;

a414:;
        /* (GHS: registrate segment i) */
         for (ell=1; ell<= 10; ell++)
         {
            kk1=ell;
            if ( (aa[i1][ell]==i) || (aa[i1][ell]==0) )
               break;  // goto a415;
         }

//a415:;
         aa[i1][kk1]=i;
         q[i1][kk1]=t1;

        /* registrate segment j  */
         for (ell=1; ell<= 10; ell++)
         {
            kk2=ell;
            if ( (aa[i1][ell]==j) || (aa[i1][ell]==0) )
               break;  // goto a416;
         } /* ell */

//a416:;
         aa[i1][kk2]=j;
         q[i1][kk2]=t2;

//a411:;
      } /* j */
   } /* i */

}  /* Close dc04().  */




/**************************************************/
/* dc05: tree cutting and forming edges           */
/**************************************************/
void dc05(Geometrydata *gd, int **aa, int **k, int **r, int **m,
          double **q, double **c, double **d)
{
   int i,j,l;
   int i1, i2;
   int l1, l2;
   double a2;
   int n3;
   int j1, j2;
  /* Set kk3 as a local variable here so that we
   * can use it as a boolean for returning from
   * lns(), which is not called from this procedure.
   */
   int local_kk3;

   int num_int_section = gd->nIntersectionPoints;

  /* Not too swift to have a label as the first "executable"
   * in a function.
   */
/* FIXME: Turn this into a while loop or something. */
a504:
  /* Try and get rid of this kk3 variable as it is not
   * set by a call to lns().
   */
   local_kk3=0;

   for (i=1; i<= gd->nJoints; i++)
   {
      i1=0;
      for (j=1; j<= num_int_section; j++)
      {
         for (l=1; l<= 10; l++)
         {
            if ( aa[j][l] != i )
               continue;  // goto a501;
		         	j1 = j; /* keep  j  */
            l1=l; /* keep  l  */
            i1=i1+1;

//a501:;
         } /* l */
      } /* j */

   	  if ( i1 != 1 )
	     	  continue;  // goto a502;

      aa[j1][l1]=0;  /* line i 1 point */
      local_kk3=1;

     /* check point number of line cross i at point j  */
      i1=0;
      for (j=1; j<= 10; j++)
      {
         if ( aa[j1][j]==0 )
			         continue;  // goto a503;
         i1=i1+1;  /* number  in ji  */
         j2=j;     /* positin in ji  */
//a503:;
      } /* j */

	     if ( i1 != 1 )
		       continue;  // goto a502;

      aa[j1][j2]=0;                    /* pnt ji 1 line  */

//a502:;
   } /* i */

   if ( local_kk3==1 )
	     goto a504;         /* found branch   */

  /* delete tree points   check point with 0 line   */

   n3=0;   /* line number    */

   for (i=1; i<= num_int_section; i++)
   {
      i1=0;
      for (j=1; j<= 10; j++)
      {
         if ( aa[i][j]==0 )
			         continue;  // goto a505;
         i1=i1+1;
//a505:;
      } /* j */

     /* i1=0 point with no intersection lines  a q c   */
      if ( i1==0 )
		       continue;  // goto a506;

	    /* This appears to be the last place that n3 is updated.
  	   */
      n3=n3+1;

	     for (j=1; j<= 10; j++)
      {
         aa[n3][j]=aa[i][j];
         q[n3][j]=q[i][j];
      } /* j */

      c[n3][1]=c[i][1];
      c[n3][2]=c[i][2];

//a506:;
   }

  /**************************************************/
  /* 0, 0 matrix            n3 points after cutting */
  /* max 10 passing lines 20 connected points       */
   for (i=1; i<= n3; i++)
   {
      for (j=0; j<= 20; j++)
      {
         k[i][j]=0;
      } /* j */
   }

  /* find all points along line i                   */
   for (i=1; i<= gd->nJoints; i++)
   {
      i1=0;  /* points in line */
      for (j=1; j<= n3; j++)
      {
         for (l=1; l<= 10; l++)
         {
            if ( aa[j][l] != i )
				           continue;  // goto a507;
            i1=i1+1;
            m[i1][1]=j;     /* point number   */
            d[i1][1]=q[j][l];  /* distance t1 t2 */
//a507:;
         } /* l */
      } /* j */


   		/* ordering points along line i  i1=0 or i1>1     */
      if ( i1==0 )
			      continue;  // goto a508;

      for (j=1; j<= i1-1; j++)
      {
         for (l=j+1; l<= i1; l++)
         {
            if ( d[j][1]<=d[l][1] )
			            continue;  //goto a509;
            i2=m[j][1];
            a2=d[j][1];
            m[j][1]=m[l][1];
            d[j][1]=d[l][1];
            m[l][1]=i2;
            d[l][1]=a2;
//a509:;
         } /* l */
      } /* j */

   		/* registrate segments along line i               */
      for (j=1; j<= i1-1; j++)
      {
         j1 =m[j  ][1];   /* point number 1 */
         j2 =m[j+1][1];   /* point number 2 */
         for (l=1; l<=   20; l++)
         {
            l1=l;
            if ( (k[j1][l]==0) || (k[j1][l]==j2) )
				           break;  // goto a510;
         } /* l */

//a510:
         if (k[j1][l1]==0)
				        r[j1][l1]=i; /* segment number */

         k[j1][l1]=j2;    /* segment ji j2  */

			      for (l=1; l<=   20; l++)
         {
            l2=l;
            if ( (k[j2][l]==0) || (k[j2][l]==j1) )
			            break;  // goto a511;
         } /* l */

//a511:
         if (k[j2][l2]==0)
		          r[j2][l2]=i; /* segment number */

			      k[j2][l2]=j1;   /* segment j2 j1  */
      } /* j */

//a508:;
   } /* i */

	 /* k[i][0] is the number of segments from i       */
   for (i=1; i<= n3; i++)
	  {
      k[i][0]  = 0;
      for (j=1; j<= 20; j++)
      {
         if (k[i][j] == 0)
			         continue; //goto a512;
         k[i][0] += 1;
//a512:;
      } /* j */
   } /* i */

  /* Kludgy.
   */
   gd->n3 = n3;

}  /* Close dc05().  */


/**************************************************/
/* dc06: merge close node-node and node-edge      */
/**************************************************/
void dc06(Geometrydata *gd, int **k, int **r, double **c)
{
   int i;
   int j;
   int l;
   double d0, d1, d2;
   int i0;
   int l0;
   int l1;
   double x3, y3;
   int kk1;
   int i1, i2, i3;
   double yi, yo, y2;
   double x0, x1, x2;
   int n3;
   int j0,j1,j2;
   double domainscale = gd->w0;
   double edgenodedist = gd->e00;
   double local_t1, local_t2;

   double x21, y21, x34, y34, x31, y31;

  /* I think w0 is some kind of tolerance value. So it
   * should be handled either with some kind of macro
   * definition, or set by the machine, or set by the user
   * as an option.
   */
   //w0 = gd->w0;

   n3 = gd->n3;

//a611:
  /* delete intersecting edges   */
   for (i =1; i <= n3;  i++)
   {
      if (i%20 == 0)
      {
         ; /* Do nothing  */
         //fprintf(fl0,"6000 v-v i= %5d \n",i);
      }

   	 /* if (i%20 == 0)  printf(    "6000 v-v i= %5d \n",i); */
      for (j =1; j <=      n3;  j++)
      {
         if  (i==j)
            continue;  // goto a603;  goto next_j
         for (l =1; l <= k[i][0];  l++)
         {
            for (l0=1; l0<= k[j][0]; l0++)
            {
               i1 = k[i][ l];
               j1 = k[j][l0];
               i2 = r[i][ l];
               j2 = r[j][l0];

               if (0 == i1)
                  continue;  // goto a604;
               if (0 == j1)
                  continue;  // goto a604;
               if (i == j1)
                  continue;  // goto a604;
               if (j == i1)
                  continue;  // goto a604;
               if (j1 == i1)
                  continue;  // goto a604;
               if (i2<=gd->nBoundaryLines && j2<=gd->nBoundaryLines)
                  continue;  // goto a604;


              /* 1:i  2:i1  3:j  4:ji                           */
               x21=c[i1][1]-c[i ][1];         /* coefficient a11 */
               y21=c[i1][2]-c[i ][2];         /* coefficient a21 */
               x34=c[j ][1]-c[j1][1];         /* coefficient a12 */
               y34=c[j ][2]-c[j1][2];         /* coefficient a22 */
               x31=c[j ][1]-c[i ][1];         /* free terms  f1  */
               y31=c[j ][2]-c[i ][2];         /* free terms  f2  */
              /* FIXME: Get rid of these absolute value calls in here. */
               if (fabs(x31)>fabs(x21)+fabs(x34))
                  continue;  // goto a604;
               if (fabs(y31)>fabs(y21)+fabs(y34))
                  continue;  // goto a604;
              /* FIXME: Reference to dissertation */
               d0 =x21*y34-x34*y21;           /* equation | |    */
               d1 =x31*y34-x34*y31;           /* | | for sol t1  */
               d2 =x21*y31-x31*y21;           /* | | for sol t2  */

              /* p1 p2   p3 p4  are two parallel lines   */
              /* FIXME: Get rid of the absolute value call in here. */
               if ( fabs(d0) < .000000001*domainscale*domainscale)
                  continue;  // goto a604;


              /* normal intersection     */
               local_t1 = d1/d0;
               if ( (local_t1 < -.0000001) || (local_t1 > 1.0000001) )
                  continue;  // goto a604;
               local_t2=d2/d0;
               if ( (local_t2 < -.0000001) || (local_t2 > 1.0000001) )
                  continue;  // goto a604;
               i0 = j;
               j0 = j1;

               if (j2 > gd->nBoundaryLines)
                  goto a605;

               i0 = i;
               j0 = i1;

a605:;
              /* delete edge i0-j0 */
               i3=1;
              /*  Nesting is now 5 levels deep.  */
               for (l1=1; l1<=k[i0][0]; l1++)
               {
                  k[i0][i3]=k[i0][l1];
                  r[i0][i3]=r[i0][l1];
                  if (k[i0][l1] != j0)
                     i3 += 1;
               }  /*  l1 */

               if (k[i0][0]  <  i3)
                  goto a606;

               k[i0][0]  -= 1;
               k[i0][i3]  = 0;

a606:
              /* delete edge j0-i0   */
               i3=1;
               for (l1=1; l1<=k[j0][0]; l1++)
               {
                  k[j0][i3] = k[j0][l1];
                  r[j0][i3] = r[j0][l1];
                  if (k[j0][l1] != i0)
                     i3 += 1;
               }  /*  l1 */

               if (k[j0][0]  <  i3)
                  goto a607;

               k[j0][0]  -= 1;
               k[j0][i3]  = 0;

a607:;

		//fprintf(fl0,"6000 v-v i= %5d i1= %5d j= %5d j1= %5d i0= %5d j0= %5d \n",
	    //        i,  i1,  j,  ji,  i0,  jo);
//a604:;
            }  /*  l0 */
         }  /*  l  */
//a603:;
      }  /*  j  */
   }  /*  i  */

  /* delete edge having small edge-node distance    */
   for (i=1; i<= n3; i++)
   {
      if (i%20 == 0)
      {
         ;  /*  Do nothing. */
         //fprintf(fl0,"6000 e-e i= %5d \n",i);
      }

      for (j=1; j<=n3; j++)
      {
         for (l=1; l<= k[j][0]; l++)
         {
            j1  = k[j][l];
            i1  = r[j][l];
            if (0 == j1)
               continue;  // goto a608;
            if (i == j)
               continue;  // goto a608;
            if (i == j1)
               continue;  // goto a608;
		          if (i1 <= gd->nBoundaryLines)
               continue;  // goto a608;
            x1  = c[i ][1];
            yi  = c[i ][2];
            x2  = c[j ][1];
            y2  = c[j ][2];
            x3  = c[j1][1];
            y3  = c[j1][2];
            d1  = (x3-x2)*(x3-x2) + (y3-y2)*(y3-y2);
            local_t1  = ((x1-x2)*(x3-x2) + (yi-y2)*(y3-y2))/d1;
            if ( (local_t1 < -.0000001) || (local_t1 > 1.0000001) )
               continue;  // goto a608;

            x0  = x2 + local_t1*(x3-x2);
            yo  = y2 + local_t1*(y3-y2);
            d0  = sqrt((x1-x0)*(x1-x0) + (yi-yo)*(yi-yo));
            if (d0 > domainscale*edgenodedist)
               continue;  // goto a608;

            i0  = j;
            j0  = j1;

           /* delete edge i0-j0 */
            i3=1;
            for (l1=1; l1<=k[i0][0]; l1++)
            {
               k[i0][i3]=k[i0][l1];
               r[i0][i3]=r[i0][l1];
               if (k[i0][l1] != j0)
                  i3 += 1;
            }  /*  l1 */

            if (k[i0][0]  <  i3)
               goto a609;

            k[i0][0]  -= 1;  // Should be --
            k[i0][i3]  = 0;

a609:

           /* delete edge j0-i0 */
            i3=1;
            for (l1=1; l1<=k[j0][0]; l1++)
            {
               k[j0][i3]=k[j0][l1];
               r[j0][i3]=r[j0][l1];
               if (k[j0][l1] != i0)
                  i3 += 1;
            }  /*  l1 */

            if (k[j0][0]  <  i3)
               continue;  // goto a610;  // continue;

            k[j0][0]  -= 1;
            k[j0][i3]  = 0;
//a610:;


//a608:;
         }  /*  l  */
      }  /*  j  */
   }  /*  i  */


  /* cut one connection node after merging          */

a601:                             /* restart search */
   kk1 = 0;
   for (i=1; i<=      n3; i++)
   {
      if (k[i][0] != 1   )
         continue;  // goto a602;
      i1=k[i][1];
      k[i][0]=0;
      k[i][1]=0;
      //fprintf(fl0,"6000: delete branch %5d  \n",i);
      kk1=1;
      i3=1;

      for (j=1; j<=k[i1][0]; j++)
      {
         k[i1][i3]=k[i1][j];
         r[i1][i3]=r[i1][j];
         if (k[i1][j] != i  )
            i3 += 1;
      }  /*  j  */

      if (k[i1][0] <  i3 )
         continue;  // goto a602;

      k[i1][0]  -= 1;
      k[i1][i3]  = 0;
//a602:;
   }  /*  i  */

  /* FIXME: Replace this with a rational data structure. */
		 if (kk1 == 1   )
      goto a601;
   //post=1;

}  /* Close dc06().  */



/**************************************************/
/* dc07: directions of segments from a node       */
/**************************************************/
void
dc07(Geometrydata *gd, int **aa, int **k, int **r, int **m,
          double **q, double **c)
{
   int i;
   int j;
   int l;
   double c1;
   int i1, i2;
   double yi;
   double g2;
   double x1;
   double a2;
   int n3;
   double  vv[7][7];
   int ji;
   int j2;

   double dd = 3.14159/180;

   n3 = gd->n3;

   for (i=1; i<= n3;      i++)
   {
      for (j=1; j<= k[i][0]; j++)
      {
         aa[i][j]=0;
      } /* j */
   } /* i */


   /* direction angle of segment i k[i][j]           */
   for (i=1; i<= n3;      i++)
   {
      i1=0;
      for (j=1; j<= k[i][0]; j++)
      {
         if ( k[i][j]==0 )
            continue;  // goto a701;
         ji=k[i][j];
         x1=c[ji][1]-c[i][1];
         yi=c[ji][2]-c[i][2];
         c1=fabs(x1)+.000000001;
         //g2=(atan(yi/c1))/(DD);
         g2=(atan(yi/c1))/(dd);
         if ( x1>=0 )
            goto a702;
         g2=180-g2;

a702:
         if ( g2 >=0 )
            goto a703;
         g2=g2+360;

a703:
         i1=i1+1;
         m[i1][1]=ji;
         m[i1][0]=r[i][j];
         vv[i1][1]=g2;
//a701:;  /* Do nothing.  */

      } /* j */


      /* ordering direction angles from point i         */
      for (j=1; j<= i1-1; j++)
      {
         for (l=j+1; l<= i1; l++)
         {
            if ( vv[j][1]<=vv[l][1] )
               continue;  // goto a704;

            i2=m[j][1];
            j2=m[j][0];
            a2=vv[j][1];
            m[j][1]=m[l][1];
            m[j][0]=m[l][0];
            vv[j][1]=vv[l][1];
            m[l][1]=i2;
            m[l][0]=j2;
            vv[l][1]=a2;
//a704:;  /*  Do nothing.  */
         } /* l */
      } /* j */

      /* compute angle intervals between segments in i  */
      for (j=1; j<= i1-1; j++)
      {
         m[j][2]=m[j+1][1];              /* next point     */
         vv[j][2]=vv[j+1][1]-vv[j][1];      /* interval       */
      } /* j */

      m[i1][2]=m[1][1];
      vv[i1][2]=360+vv[1][1]-vv[i1][1];

      /* k in rotation order from x to y round point i  */
      /* a next by rotation                             */
      /* r line number of segment i k                   */
      /* q angle interval from edge rotate x to y       */
      for (j=1; j<= i1; j++)
      {
         k[i][j]=m[j][1];
         aa[i][j]=m[j][2];
         r[i][j]=m[j][0];
         q[i][j]=vv[j][2];
      } /* j */
   } /* i */

   /* debugging print statement deleted.  */

}  /* End of dc07.  */


/**************************************************/
/* dc08: assembling blocks from edges             */
/**************************************************/
/* gd is shadowed.
 */
void
dc08(Geometrydata * gd, int **aa, int **k, int **r, int **m,
          int **vindex, double **q, double **c, double ** vertices)
{
   int i, j, l;
   double d0;
   int i0;
   int mm0;
   int l1;
   int l2;
   int op;
   int i1;
   int n0;
   int n3;
   int j0, j1, j2;
   double ** joints = gd->joints;

   n3 = gd->n3;

  /* o: general order of block vertices             */
  /* mm0: start row of k[] aa[] for searching         */
  /* n0: number of blocks                           */
   op=0;
   mm0=1;
   n0=0;

/* FIXME: Try to put this into a while loop or something */
a807:
   for (i=mm0; i<= n3; i++)
   {
      for (j= 1; j<= k[i][0]; j++)
      {
         if ( aa[i][j]<=0 )
			         continue;  // goto a801;
         i1 =i;                          /* start point i1 */
         j1=k[i][j];                     /* 2nd number     */
         j0=j;                           /* 2nd position   */
        /* This looks like a legitimate goto!  */
         goto a802;                      /* k[i1][jo]=ji   */
//a801:;  /* Do nothing.  */
      } /* j */
   } /* i */

  /* This is one of the strangest control of flow
   * situations I have ever seen. If the jump to a802
   * doesn't go, then the search is over, and this jumps
   * to the end of the function.  This has *got* to be
   * replaced.  A do-while where this is a break might get it.
   */
   goto a803;  /* end searching  */

  /* mm0 start search from first i=i1 found edge     */
  /* i  number of vertices of this block            */
  /* d0 summation of the angles of current block    */

a802:
   mm0=i1;                          /* start point i1 */
   i =0;
   d0=0;

  /* find next vertex of current block: start point */
  /* for edge i1-j1 only j1 is registrated vertex   */

//a806:
   do
   {
      i=i+1;
      aa[i1][j0]= (-1)*aa[i1][j0];    /* k[i1][jo] no   */
      m[i][1]=j1;                     /* vertex ji rec  */
      for (l=1; l<= k[j1][0]; l++)
      {
         i0=l;                           /* address ji-i1  */
         if ( k[j1][l]==i1 )
            break;  //goto a804;  /* center  ji-i1  */
      } /* l */

//a804:
     /* d0 angle start from edge ji-i1 from x to y     */
     /* ji-l1 next edge of ji-i1 rotating  x to y      */
      d0 =d0 + q[j1][i0];             /* angle top ji   */
     /* ns hack (int) */
     	l1 = (int) fabs (aa[j1][i0]);
   	 	i1 =j1;   /* start point i1 */
   	  j1 =l1;   /* 2nd number     */

		    for (l=1; l<= k[i1][0]; l++)
      {
         j0=l;                           /* 2nd position   */
         if ( k[i1][l]==j1 )
            break;  // goto a805;  /* k[i1][jo]=ji   */
      } /* l */

//a805:
      m[i][0]=r[i1][j0];
	    	//if ( aa[i1][j0]>0  )
      // goto a806;
   } while   ( aa[i1][j0]>0  );

  /* (GHS: block ending: edge i1-ji already in block)  */
  /* FIXME: This might get to be a continue if the outer
   * loop were a do-while
   */
   if ( d0>i*180 )
      goto a807;      /* outer block no */

   n0=n0+1;    // n0++;             /* block number   */
	 /* This appears to be the only place that k0 is set.  */
   vindex[n0][1]=op+1;                  /* 1st block node */
   vindex[n0][2]=op+i;                  /* end block node */

  /* block rotate from x to y after inverse m[][]   */
  /* d[op+i+1] to back to first node                */
  /* d[op+i+2] reserved for s0   sx                 */
  /* d[op+i+3] reserved for sy   sxy                */
  /* d[op+i+4] reserved for sxx  syy                */
   for (l=1; l<= i; l++)
   {
      l2=m[l][1];
      j2=m[l][0];
      vertices[op+i+1-l][1] = c[l2][1];
      vertices[op+i+1-l][2] = c[l2][2];
      vertices[op+i+1-l][0] = (double)joints[j2][5];
   } /* l */

   vertices[op+i+1][0] = vertices[op+1][0];
   vertices[op+i+1][1] = vertices[op+1][1];
   vertices[op+i+1][2] = vertices[op+1][2];
   vertices[op+i+2][0] = vertices[op+2][0];        /* merge co-line  */
   vertices[op+i+2][1] = vertices[op+2][1];        /* merge co-line  */
   vertices[op+i+2][2] = vertices[op+2][2];
   op=op+i+4;
   goto a807;

/* I don't think this goto ever gets called. */
a803:;
// fprintf(ftext, "\nThere are %d blocks.\n", n0);

  /* Update the number of blocks in the problem
   * domain.
   */
   gd->nBlocks = n0;


}  /*  end of dc08  */


/**************************************************/
/* dc09: merge co-line vertices                   */
/**************************************************/
void dc09(Geometrydata *gd, int **k0, int *h, double **d)
{
   int i, j;
   int i0, i1, i2;
   double yi, y2;
   double x1, x2;
   double a1;
   int n0;
   double w0;

   w0 = gd->w0;

  /* Separate out the effects of operating on the number of
   * blocks from data we need in the problem domain.
   */
   n0 = gd->nBlocks;

  /* k0[n0][2]+4 is "number of vertices, which
   * includes a bunch of junk values between the
   * vertex lists.  This is a variable `oo' at
   * other places in the code.
   */
   for (i=1; i<= k0[n0][2]+4; i++)
   {
      h[i]=0;
   } /* i */

  /* check j  j+1  j+2 co-line  record h[j+1] */
   for (i=1; i<= n0; i++)
   {
      for (j=k0[i][1]; j<= k0[i][2]; j++)
      {
         x1 =d[j+1][1]-d[j][1];
         yi =d[j+1][2]-d[j][2];
         x2 =d[j+2][1]-d[j][1];
         y2 =d[j+2][2]-d[j][2];
         a1 =fabs(x1*y2-yi*x2);
        /* FIXME: Replace the .0000000001 with a user controlled
         * parameter.
         */
         if ( a1>.000000001*w0*w0 )
            continue;  // goto a901;
         h[j+1]=i;
//a901:;
      } /* j */
   } /* i */


      /* reset first node if last node +1 is deleted    */
   for (i=1; i<= n0; i++)
   {
      i1=k0[i][1];
      i2=k0[i][2]+1;
      if (h[i2] == 0)
		       continue;  //goto a902;  //  This goto used as a continue.
      h[i1]=h[i2];
//a902:;
   } /* i */


  /* (GHS: reset k0[i][2] reduce block node number by 1)   */
   for (i=1; i<= n0; i++)
   {
      i1 = k0[i][1]-1;
      for (j=k0[i][1]; j<= k0[i][2]; j++)
      {
         if (h[j] > 0)
            continue;  // goto a903;
         i1 += 1;
         d[i1][0]=d[j][0];
         d[i1][1]=d[j][1];
         d[i1][2]=d[j][2];
//a903:;
      } /* j */

      k0[i][2]=i1;
      i0=k0[i][1];
      d[i1+1][0]=d[i0][0];
      d[i1+1][1]=d[i0][1];
      d[i1+1][2]=d[i0][2];
   } /* i */

  /*------------------------------------------------*/
  /* re-order d[][] delete spaces                   */
  /* i0 counter of blocks  i1 counter of nodes      */
   i0=0;
   i1=0;
   for (i=1; i<= n0; i++)
   {
      if ( k0[i][2]-k0[i][1] < 2 )
         continue;  // goto a904;
      i0 +=1;                         /* block number+1 */
      i2  =i1+1;                      /* start of block */
      for (j=k0[i][1]; j<= k0[i][2]+4; j++)
      {
         i1+=1;
         d[i1][0]=d[j][0];
         d[i1][1]=d[j][1];
         d[i1][2]=d[j][2];
      } /* j */

      k0[i0][1]=i2;
      k0[i0][2]=i1-4;
//a904:;
   } /* i */

     /* Reset the number of blocks in the problem domain.
      * n0 goes into the PARAMSTRUCT at some point.
      */
      n0=i0;
	  gd->nBlocks = n0;
}


// mmm: segments segbolts (type 2) into multiple GHS bolts
// most of this algorithm was taken from dc10 (produce fixed points from line)
// returns the total number of bolts
int
produce_boltsegments(Geometrydata *gd, int ** vindex, double ** vertices,
                     int num_bolts, double ** points) {

   int i, j, l, counter;

   /* Can these be renamed to something more descriptive? */
   // i1 is total_segments?
   int i1, i2=0, i3;

   double x0[26], x1[26];
   double y0[26], y1[26];

   /* Coordinates where segments are cut? */
   double xhalf, yhalf;

   /* Should be renamed to length */
   double a1;

   double temp;

   int n0 = gd->nBlocks;
   double domainscale = gd->w0;
   int maxsegments = gd->maxSegmentsPerBolt;

   /* Line intersection parameters x,y */
   double t1, t2;
   int INTERSECTION;

   /** What is t for? */
   double t[26];

   double x21, y21, x34, y34, x31, y31, x41, y41;

   /** what is i2? */
   if (num_bolts < 1) {
      return i2;
   }

   if (maxsegments > 24) {
      dda_display_warning("warning: max bolts per segment > hardwired array size in produce_boltsegments");
   }

   // move data forward to create space for possible intermediate points
   // max is currently arbitrary and hardwired
   if (maxsegments < 1) {
	   maxsegments = 1;
	   dda_display_warning("max bolt segments not set, using default 1");
   }

   i1 = (maxsegments-1) * num_bolts;

   /* What is the intent of this loop? */
   for ( i= num_bolts; i > 0; i-- ) {

      points[i+i1][0] = points[i-1][0];
	   points[i+i1][1] = points[i-1][1];
      points[i+i1][2] = points[i-1][2];
      points[i+i1][3] = points[i-1][3];
      points[i+i1][4] = points[i-1][4];
   }

  /**************************************************/
  /* produce and register bolts            */
   i2 = 0;   /* bolt counter: counts total number of bolt segments  */
   for (i=i1+1;i<= i1+num_bolts;  i++) {

		  points[i2][0] = points[i][0];
		  points[i2][1] = points[i][1];
		  points[i2][2] = points[i][2];
		  points[i2][3] = points[i][3];
		  points[i2][4] = points[i][4];


	 /*------------------------------------------------*/
     /* case bolt does not need to be segmented                          */
		a1 = fabs(points[i][0] - 2.0); //mmm type (integer) is saved in array of floating pts!! (ugh)
	  if ( a1 > 0.000000001*domainscale ) {
			i2++;
			continue;
	  } else {

	   i3=0;  /* counter for number of block edges crossed by this bolt */

		for (j=1; j<= n0; j++)  // for each block
		{
			for (l=vindex[j][1]; l<= vindex[j][2]; l++)  // for each edge
			{
				x21 = points[i][3] - points[i][1]; /* coefficient a11 */
				y21 = points[i][4] - points[i][2]; /* coefficient a21 */

				x34 = vertices[l][1] - vertices[l+1][1]; /* coefficient a12 */
				y34 = vertices[l][2] - vertices[l+1][2]; /* coefficient a22 */

				x31 = vertices[l][1] - points[i][1]; /* free terms  f1  */
				y31 = vertices[l][2] - points[i][2]; /* free terms  f2  */
				x41 = vertices[l+1][1] - points[i][1]; /* co-line case    */
				y41 = vertices[l+1][2] - points[i][2];

				INTERSECTION = lns(domainscale, x21, x31, x34, x41, y21, y31, y34, y41, &t1, &t2);

				if (INTERSECTION == FALSE)
					continue;  // next edge

				i3++; // increment number of block edges crossed by this bolt
				t[i3] = t1;
				x0[i3] = points[i][1] + x21*t1; /* intersection    */
				y0[i3] = points[i][2] + y21*t1;
				x1[i3] = fabs(x34);
				y1[i3] = fabs(y34);

				// will cross many interfaces twice
				// need to ignore duplicate crossings
            // a1 is also used for length... maybe this
            // value could renamed?
				a1 = 1000000000;
				for (counter=1; counter <i3; counter++) {

               temp = fabs(t[i3]-t[counter]);
               if (temp < a1) {
                  a1 = temp;
               }
				} // end for each previous contact

            if ( a1 < 0.000000001*domainscale ) {
               i3--; // already crossed this interface
            }

            if (i3>maxsegments) {
               dda_display_warning("warning: bolt exceeds max allowable segments");
            }

			} // end for each edge
		} // end for each block

		// reorder intersections based on t values -> cut bolt in sequential order
		for (j=1; j<i3; j++) { // for each intersection in the list
			temp = t[j];
			counter = 0;
			for (l = j+1; l<=i3; l++) {
				if (t[l] < temp) {
					counter = l;
					temp = t[l];
				}
			} // end for all other intersections in the list
			if (counter) {  // need to switch
				t[25] = t[j];
				x0[25] = x0[j];
				y0[25] = y0[j];
				x1[25] = x1[j];
				y1[25] = y1[j];
				t[j] = t[counter];
				x0[j] = x0[counter];
				y0[j] = y0[counter];
				x1[j] = x1[counter];
				y1[j] = y1[counter];
				t[counter] = t[25];
				x0[counter] = x0[25];
				y0[counter] = y0[25];
				x1[counter] = x1[25];
				y1[counter] = y1[25];
			}
		} // end reordering


		// split into segments
		for (j=1; j<=i3; j++) {

			/* Reference this length against dissertation */
         // a1 is used above for a completely different purpose.
         a1 = sqrt(x1[j]*x1[j] + y1[j]*y1[j]);  // maybe use bolt length instead of this?
			// if j is > 1, split bolt into two pieces before adding new segment
			if (j>1) { //split first
				i2++;
				// find point halfway between old endpoint and block intersection
				// this halfway point is a little off center, but close enough
				xhalf = (points[i2-1][1] + x0[j])/2;
				yhalf = (points[i2-1][2] + y0[j])/2;
				points[i2][1] = xhalf; // newly created bolt
				points[i2][2] = yhalf; // gets new first end @ halfway point
				points[i2][3] = points[i2-1][3]; // and old second end
				points[i2][4] = points[i2-1][4];
				points[i2][0] = points[i2-1][0];
				points[i2-1][3] = xhalf; // old bolt gets new
				points[i2-1][4] = yhalf; // second end @ halfway point
			} // end if i1 is odd
			i2++;  // increment number of bolts

			// flip sign if necessary to make sure segment extends into next block
         if (points[i2-1][1] < x0[j]) {
            y1[j] = -y1[j];
         }

         if (points[i2-1][2] < y0[j]) {
            x1[j] = -x1[j];
         }

			points[i2][1] = x0[j] + y1[j]/a1*0.000005*domainscale; // newly created bolt
			points[i2][2] = y0[j] + x1[j]/a1*0.000005*domainscale; // gets new first end
			points[i2][3] = points[i2-1][3]; // and old second end
			points[i2][4] = points[i2-1][4];
			points[i2][0] = points[i2-1][0];
			points[i2-1][3] = x0[j] - y1[j]/a1*0.000005*domainscale; // old bolt gets new
			points[i2-1][4] = y0[j] - x1[j]/a1*0.000005*domainscale; // second end
		} // end split into segments

	  } // end else bolt is segmented or not

	  i2++;  // increment after saving data since bolts are indexed from zero

	  /* (GHS: case whole bolt inside a block)  */
     if ( i3 < 1) {
        dda_display_warning("Segbolt inside a single block, no segments formed.");
     }

   } // end for each bolt

  return i2;

} // end produce_boltsegments


/**************************************************/
/* dc10: forming fixed points from lines          */
/**************************************************/
/* n0, the number of blocks in the problem, is carried in here
 * from being reset in dc09().
 */
void dc10(Geometrydata *gd, int ** vindex, double ** vertices)
{

   int i;
   int j;
   int l;
   int i1, i2, i3;
   double x0, x1;
   double y0, y1;
   double a1;
   int n0 = gd->nBlocks;
   int max = gd->maxFixedPointsPerFixedLine;
   double domainscale = gd->w0;
   double ** points = gd->points;
  /*  Change name of kk3 asap.
   */

   //int local_kk3;
   double t1, t2;
   int INTERSECTION;

   double x21, y21, x34, y34, x31, y31, x41, y41;


  /* FIXME: This should throw an error at some point in the future.
   */
   if (gd->nFPoints == 0)
      return;  //goto b003;  // ie return;


   /* move g[][] forward 150*nFPoints                    */
		 // i1 = 150*nFPoints;

   i1 = max*(gd->nFPoints);

   for ( i=(gd->nFPoints+gd->nLPoints+gd->nMPoints+gd->nHPoints); i >= 1; i-- )
   {
      points[i+i1][0] = points[i][0];  /* added 8/9/95 */
		    points[i+i1][1] = points[i][1];
      points[i+i1][2] = points[i][2];
      points[i+i1][3] = points[i][3];
      points[i+i1][4] = points[i][4];
   }

  /**************************************************/
  /* produce and registrate fixed points            */
		// i1 = 150*nFPoints;
	 	i1 = max*gd->nFPoints;
   i2 = 0;   /* point counter   */
   for (i=i1+1;i<= i1+gd->nFPoints;  i++)
   {
     /*------------------------------------------------*/
     /* case line is a point                           */
      a1 = fabs(points[i][1]-points[i][3]) + fabs(points[i][2]-points[i][4]);

      if ( a1 > 0.000000001*domainscale )
         goto b004;
		    i2 += 1;
	    	points[i2][0] = points[i][0];  /* added 8/9/95 */
      points[i2][1] = points[i][1];              /* point in block  */
    		points[i2][2] = points[i][2];
      continue;  // goto b001;

b004:;
      i3 = 0;                        /* counter for i   */
      /*------------------------------------------------*/
      /* case of a fixing line                          */
      for (j=1; j<= n0; j++)
      {
         for (l=vindex[j][1]; l<= vindex[j][2]; l++)
         {
            x21 = points[i][3] - points[i][1]; /* coefficient a11 */
            y21 = points[i][4] - points[i][2]; /* coefficient a21 */

            x34 = vertices[l][1] - vertices[l+1][1]; /* coefficient a12 */
            y34 = vertices[l][2] - vertices[l+1][2]; /* coefficient a22 */

            x31 = vertices[l][1] - points[i][1]; /* free terms  f1  */
            y31 = vertices[l][2] - points[i][2]; /* free terms  f2  */
            x41 = vertices[l+1][1] - points[i][1]; /* co-line case    */
            y41 = vertices[l+1][2] - points[i][2];

            INTERSECTION = lns(domainscale, x21, x31, x34, x41, y21, y31, y34, y41, &t1, &t2);

            if (INTERSECTION == FALSE)
               continue;  // goto b002;

            x0 = points[i][1] + x21*t1; /* intersection    */
            y0 = points[i][2] + y21*t1;
            x1 = -x34;         /* edge l to l+1   */
            y1 = -y34;
           /* Reference this length against dissertation */
            a1 =sqrt(x1*x1 + y1*y1);
            x0  += -y1/a1*0.005*domainscale;
            y0  +=  x1/a1*0.005*domainscale;
            i2  += 1;
            i3  += 1;
            points[i2][1] = x0;    /* a fixed point   */
            points[i2][2] = y0;
//b002:;
         }
      }

     /* (GHS: case whole line inside a block)  */
      if (i3 > 0)
         continue;  //goto b001;

      i2 += 1;
	    	points[i2][0] = points[i][0];
      points[i2][1] = points[i][1];  /* line in block   */
	    	points[i2][2] = points[i][2];

//b001:;
   }

  /**************************************************/
  /* move g[][] back and set nFPoints as fixed points   */
		// i1 = 150*nFPoints;
		i1 = max*gd->nFPoints;
		i3 = i1+gd->nFPoints;

		for (i= 1; i<= gd->nLPoints+gd->nMPoints+gd->nHPoints; i++)
  {
		   i3 += 1;
		   points[i2+i][0] = points[i3][0];
		   points[i2+i][1] = points[i3][1];
		   points[i2+i][2] = points[i3][2];
  } /* i */

  gd->nFPoints = i2;

//b003:;
} // dc10






/**************************************************/
/* dc11: block of fixed measured load hole points */
/**************************************************/
/* FIXME: What is q?
 */
void dc11(Geometrydata *gd, int **vindex, double **q, double **vertices)
{
   int i,j;
   int i0, i1, i2;
  /* coordinates of some specific point */
   double x, y;
  /* Index values into points array */
   int start, stop;
   double ** points = gd->points;
  /* Blocks containing hole points will need to be
   * deleted.
   */
   int holepoint;
   int const HOLEBLOCK = 3;
  /* Bad news on kk3:  it is used for three different
   * purposes.  Here, it is NOT used as a boolean.
   */
   //int local_kk3;
   DList * ptr;
   DDAPoint * ptmp;

   start = gd->nFPoints+gd->nLPoints+gd->nMPoints+1;
   stop = gd->nFPoints+gd->nLPoints+gd->nMPoints+gd->nHPoints;

  /* hole point: delete blocks with hole point      */
   for (i= start; i<= stop; i++) {

     /* (GHS: judge if points are coinside with block nodes)  */
      x = points[i][1];
      y = points[i][2];
      points[i][3] = crr(gd, x, y, vindex, q, vertices);
      assert(points[i][3] != 0);
   }


  /* case hole point without carry block  */
   //start = gd->nFPoints+gd->nLPoints+gd->nMPoints+1;
   //stop = gd->nFPoints+gd->nLPoints+gd->nMPoints+gd->nHPoints;
	 	for (i= start; i<= stop; i++)
   {
      if ( points[i][3] != 0 )
         continue; // goto b101;
      fprintf(ddacutlog,"hole point outside of all blocks %d \n",i);
      fprintf(ddacutlog,"%f , %f \n",points[i][1],points[i][2]);
//b101:;
   } /* i */


  /* (GHS: delete blocks with hole point)  */
   i0=0;  /* block counter  */
   i1=0;  /* node  counter  */
   for (i=1; i<= gd->nBlocks; i++) {

     /* Basically, what this says is that if the
      * block number matches the block number associated
      * with a certain hole point, then don't count
      * that block, ie delete it.
      */
      for (holepoint=start; holepoint<= stop; holepoint++) {

         if ( i == (int)points[holepoint][HOLEBLOCK] )
            goto b102;
      }

      i0 += 1;     /* block number+1 */
      i2  = i1+1;  /* start of block */
      for (j=vindex[i][1]; j<= vindex[i][2]+4; j++)
      {
         i1+=1;
         vertices[i1][0]=vertices[j][0];
         vertices[i1][1]=vertices[j][1];
         vertices[i1][2]=vertices[j][2];
      } /* j */

      vindex[i0][1]=i2;
      vindex[i0][2]=i1-4;

b102:;

   } /* i */


  /* This is a reset after counting the hole points.
   * A better way to do this would be to see if there
   * are any hole points at all.  If not, then completely
   * skip the previous loop.
   */
   gd->nBlocks = i0;


  /* find block for fixed loading measured points   */
   for (i= 1; i<= gd->nFPoints+gd->nLPoints+gd->nMPoints; i++)
   {
     /* finding carry blocks                           */
   	x = points[i][1];
	   y = points[i][2];
	   points[i][3] = crr(gd, x, y, vindex, q, vertices);
	   //points[i][3]=local_kk3;
   } /* i */


   if (gd->nSPoints > 0) {

      dlist_traverse(ptr, gd->seispoints) {

         int blocknumber;
         ptmp = ptr->val;
         x = ptmp->x;
         y = ptmp->y;
         blocknumber = crr(gd, x, y, vindex, q, vertices);
         if (blocknumber != 0) {
            ptmp->blocknum = blocknumber;
         }
         // else
         // throw an error
      }
   }


  /*------------------------------------------------*/
  /* case of having points without carry block      */
  /* FIXME: Remove such a point from the point list */
  /* FIXME: This code is duplicated in deleteBlock.
   * Turn it into a function.
   */
   for (i= 1; i<= gd->nFPoints+gd->nLPoints+gd->nMPoints; i++)
   {
      if ( points[i][3] != 0 )
         continue;  // goto b103;
      gd->display_warning("Point outside of block");
      fprintf(ddacutlog,"fixed load measured point outside all blocks %d \n",i);
      fprintf(ddacutlog,"%f , %f \n",points[i][1],points[i][2]);
//b103:;
   } /* i */

}   /*  Close dc11().  */



/************************************************/
/* dc12: compute area s sx sy of each block       */
/**************************************************/
/* n0 is carried globally from being reset in dc10.
 */
void dc12(Geometrydata *gd, int ** vindex, double ** vertices)
{
   int i;
   int j;
   double x3, y3;
   int i1, i2;
   double y2;
   double f1;
   double x2;
   int n0;

   n0 = gd->nBlocks;

  /* n0 is the number of blocks, last set in dc10().
   */
   for (i=1; i<= n0; i++)
   {
      i1 = vindex[i][1];
      i2 = vindex[i][2];
      vertices[i2+2][1] = 0;
      vertices[i2+2][2] = 0;
      vertices[i2+3][1] = 0;
      vertices[i2+3][2] = 0;

      for (j=i1; j<= i2; j++)
      {
        /* first node of triangle is (0,0) */
         x2 = vertices[j][1];
         y2 = vertices[j][2];
         x3 = vertices[j+1][1];
         y3 = vertices[j+1][2];
         f1 = (x2*y3-x3*y2)/2;

        /* Block area. */
        /* FIXME: What goes into d[i2+2][1]? */
         vertices[i2+2][2] += f1;
		       //fprintf(ftext, "Area of block %d = %f\n", i, d[i2+2][2]);
        /* First moments, see p. 292-293, GHS 1988, Eqs. 6.49, 6.50 */
         vertices[i2+3][1] += f1*(0+x2+x3)/3;
		       vertices[i2+3][2] += f1*(0+y2+y3)/3;
      } /* j */
	     //fprintf(ftext, "Area of block %d = %f\n", i, d[i2+2][2]);
   } /* i */
}   /*  Close dc12().  */



/**************************************************/
/* dc13: assign material number to blocks         */
/**************************************************/
void dc13(Geometrydata *gd, int ** vindex, double **q, double **d)
{

   //void printVindex(Geometrydata *, int ** vindex);

   int i, j;
   int ell;
   int i3;
  /* kk3 is used as a boolean variable returned from lns(),
   * then set by crl, and is probably NOT a boolean
   * variable.
   */
   int INTERSECTION;  // was kk3;
   double **gg0 = gd->matlines;

   double x21, y21, x34, y34, x31, y31, x41, y41;
   double x11, y11;
   int numblocks = gd->nBlocks;
   double t1, t2;
   double domainscale = gd->w0;

  /* n0 is the number of blocks.
   */
  /* set material number for each block  */
   for (i=1; i<= numblocks; i++)
   {
      vindex[i][0] = 0;
   }

   for (i=1; i<= gd->nMatLines; i++)
   {
      i3 = 0;                        /* counter for i   */

      for (j=1; j<= numblocks; j++)
      {
         for (ell = vindex[j][1]; ell <= vindex[j][2]; ell++)
         {

            x21 = gg0[i][3] - gg0[i][1];     /* coefficient a11 */
            y21 = gg0[i][4] - gg0[i][2];     /* coefficient a21 */

            x34 = d[ell][1] - d[ell+1][1];     /* coefficient a12 */
            y34 = d[ell][2] - d[ell+1][2];     /* coefficient a22 */

            x31 = d[ell][1] - gg0[i][1];     /* free terms  f1  */
            y31 = d[ell][2] - gg0[i][2];     /* free terms  f2  */
            x41 = d[ell+1][1] - gg0[i][1];     /* co-line case    */
            y41 = d[ell+1][2] - gg0[i][2];

            INTERSECTION = lns(domainscale, x21, x31, x34, x41, y21, y31, y34, y41, &t1, &t2);


            if (INTERSECTION == 0)
               continue;  // goto b302;

            //exit(0);

            i3  += 1;
            vindex[j][0]=(int)gg0[i][5];       /* material number */

//b302:;  // Do nothing.
         } /* l */
      } /* j */

      if (i3 > 0)
         continue;  //goto b301;

      x11 = gg0[i][1];                  /* line in block   */
      y11 = gg0[i][2];
      INTERSECTION = crr(gd, x11, y11, vindex, q, d);
      vindex[INTERSECTION][0] = (int)gg0[i][5];

//b301:;
   } /* i */


  /* Default: if no material number set material = 1
   * FIXME: Change this to a user-specified material
   * number.
   */
   for (i=1; i<= numblocks; i++)
   {
      if (vindex[i][0] == 0)
         vindex[i][0]=1;
   } /* i */

   //printVindex(gd, vindex);

}   /* Close dc13()   */


/**************************************************/
/* dc14: assign block number to 2 bolt ends       */
/**************************************************/

void dc14(Geometrydata *gd, int **k0, double **q, double **d)
{
   int i;
   double x11;
   double y11;
  /* Bolt endpoints */
   int  ep1,ep2;
   int nBolt = gd->nBolts;


  /* Warning!!!  This was indexed from 1 instead of 0,
   * and Very Bad Random Crap was happening to the rockbolt
   * data.
   */
   for (i=0; i< nBolt; i++)
   {
     /* Front end block of bolt.
      */
      //x11 = gg1[i][1];
      //y11 = gg1[i][2];
      x11 = gd->rockbolts[i][1];
      y11 = gd->rockbolts[i][2];

      ep1 = crr(gd, x11, y11, k0, q, d);
      //gg1[i][5]=(double)kk3;
      gd->rockbolts[i][5]=/*(double)*/ep1;


     /* Back end block of bolt?
      */
      //x11 = gg1[i][3];
      //y11 = gg1[i][4];
      x11 = gd->rockbolts[i][3];
      y11 = gd->rockbolts[i][4];
      ep2 = crr(gd, x11, y11, k0, q, d);
      //gg1[i][6]=(double)kk3;
      gd->rockbolts[i][6]=/*(double)*/ep2;

    } /* i */


}  /*  Close dc14().  */




/**************************************************/
/* dc16(): check if a block inside another block  */
/**************************************************/
void dc16(Geometrydata *gd, int **vindex, double **q, double ** vertices)
{
   //int i;
   //int j;
   int blk_i, blk_j;
   int j4;
   double e1;
   double x5, x6;
   int i0;
   int i1, i2, i4;
   double y5, y6;
  /* Centroid coordinates */
   double y0, x0;
   double x11, y11;
   double a4, a5;
   double d1;
   int numblocks = gd->nBlocks;
   double edgenodedist = gd->e00;
   double w0 = gd->w0;
   int kk3;

   //fprintf(fl0,"16000 if block in block \n");

  /* Find radius of block for n0 number of blocks. */
   for (blk_i=1; blk_i<=numblocks; blk_i++)
   {
      e1  = 0;
      i1  = vindex[blk_i][2];  // Stop index?
     /* vertices[i1+2,3][2,1,2] set in dc12() */
     /* x_0 = S_x/S, Eq. 6.49, GHS 1988 */
      x0  = vertices[i1+3][1]/vertices[i1+2][2];
     /* y_0 = S_y/S, Eq. 6.50, GHS 1988 */
      y0  = vertices[i1+3][2]/vertices[i1+2][2];

      for (blk_j = vindex[blk_i][1]; blk_j <= vindex[blk_i][2];  blk_j++)
      {
         d1  = (vertices[blk_j][1]-x0)*(vertices[blk_j][1]-x0)
              +(vertices[blk_j][2]-y0)*(vertices[blk_j][2]-y0);
         if ( e1 < d1 )
            e1 = d1;
      } /* j */

      vertices[i1+2][1] = sqrt(e1);  // Block radius?
     /* x0, y0 are centroid coordinates. */
      vertices[i1+4][1] = x0;
      vertices[i1+4][2] = y0;
   } /* i */

   for (blk_i=1; blk_i<=numblocks; blk_i++)
   {
     /* radius in i4+2 and center of block i  in i4+4  */
      i4  = vindex[blk_i][2];
      i1  = vindex[blk_i][1];

      x6  = 0.5*(vertices[i1][1] + vertices[i1+1][1]);
	     y6  = 0.5*(vertices[i1][2] + vertices[i1+1][2]);
      x5  = vertices[i1+1][1] - vertices[i1][1];
      y5  = vertices[i1+1][2] - vertices[i1][2];
      a4  = sqrt(x5*x5 + y5*y5);
      a5  = 0.01*edgenodedist;

/* Replace label b601 with a do-while */
b601:
     /* printf("check block %d \n",i); */
      a5 *= 0.9;
      x11 = x6  - y5*a5/a4;
      y11 = y6  + x5*a5/a4;
      //i0  = i;

      //kk3 = rlt(gd, blk_i, x11, y11, vindex, q, vertices);
      kk3 = rlt(w0, blk_i, x11, y11, vindex, q, vertices);
      //kk3 = pointinpoly(blk_i, x11, y11, vindex, vertices);

      if ( kk3 == 0 )
         goto b601;

      for (blk_j=1; blk_j<=numblocks; blk_j++)
      {
        /* radius and center of block i  j   */
        /* d1 distance of two block centers  */
         j4  = vindex[blk_j][2];

        /* vertices[i4+2][1] is a radius, so here we are adding the
         * radii of each block.
         */
         e1  = vertices[i4+2][1] + vertices[j4+2][1];

        /* vertices[i4+4][1,2] is x0,y0, so here the distance
         * between the block centroids is being computed.
         */
         d1  = (vertices[j4+4][1]-vertices[i4+4][1])*(vertices[j4+4][1]-vertices[i4+4][1])
             + (vertices[j4+4][2]-vertices[i4+4][2])*(vertices[j4+4][2]-vertices[i4+4][2]);
         d1  = sqrt(d1);

         if ( e1 < d1 )
		        	 continue;  // goto b602;

        /* FIXME: Move this to the beginning of the loop to cut out
         * some unnecessary work and clutter.
         */
	      	 if ( blk_j == blk_i )   // i.e., same block
	        			continue;  // goto b602;

      			if ( vindex[blk_j][2] == 0 )
		        		continue;  // goto b602;

         //i0 = j;

         //kk3 = rlt(gd, blk_j, x11, y11, vindex, q, vertices);
         kk3 = rlt(w0, blk_j, x11, y11, vindex, q, vertices);
         //kk3 = pointinpoly(blk_j, x11, y11, vindex, vertices);



         if ( kk3 == 1 )
		          vindex[blk_i][2] = 0;

        /* This should be redundant */
         if ( kk3 == 0 )
            continue;  //  goto b602;
	       /*
            fprintf(cutlog,"block i= %d lies in block j= %d \n",i,j);
	        */
//b602:;
      } /* j */
   } /* i */


  /* delete blocks inside other block               */
   i0=0;                           /* block counter  */
   i1=0;                           /* node  counter  */
   for (blk_i=1; blk_i<= numblocks; blk_i++)
   {
      if ( vindex[blk_i][2] == 0 )
			      continue;  // goto b603;

      i0 +=1;                         /* block number+1 */
      i2  =i1+1;                      /* start of block */
      for (blk_j = vindex[blk_i][1]; blk_j <= vindex[blk_i][2]+4; blk_j++)
      {
         i1+=1;
         vertices[i1][0] = vertices[blk_j][0];
         vertices[i1][1] = vertices[blk_j][1];
         vertices[i1][2] = vertices[blk_j][2];
      } /* j */

      vindex[i0][1]=i2;
      vindex[i0][2]=i1-4;
//b603:;
	  } /* i */

   gd->nBlocks = i0;

  /* Update the parameter struct to reflect deleted blocks.
   */
   //gd->nBlocks = n0;

}  /* Close dc16().  */



/**************************************************/
/* dc17: check block mesh                         */
/**************************************************/
void dc17(Geometrydata *gd, int **k0, double **d)
{
   int i;
   int j;
   int l;
   double d0, d1, d2;
   int l0;
   double e1;
   double e2;
   int i1, i2;
   double yi, yo, y2;
   double x0, x1, x2;
   double x21, y21, x34, y34, x31, y31;
   int numblocks = gd->nBlocks;
   double domainscale = gd->w0;
   double local_t1, local_t2;


  /* find radius of block                           */
   for (i=1; i<=numblocks; i++)
   {
      e1  = 0;
      i1  = k0[i][2];
      x0  = d[i1+3][1]/d[i1+2][2];
      yo  = d[i1+3][2]/d[i1+2][2];
      for (j =k0[i][1]; j <=k0[i][2];  j++)
      {
         d1  = (d[j][1]-x0)*(d[j][1]-x0)+(d[j][2]-yo)*(d[j][2]-yo);
         if ( e1 < d1 )
			         e1 = d1;
      } /* j */

      d[i1+2][1]=sqrt(e1);
      d[i1+4][1]=x0;
      d[i1+4][2]=yo;
      d[i1+2][0]=0;
      d[i1+3][0]=0;
      d[i1+4][0]=0;
   } /* i */


  /* check block mesh                               */
   for (i=1; i<=numblocks; i++)
   {
      i1  = k0[i][2];
      e1  = d[i1+2][1];
      x1  = d[i1+4][1];
      yi  = d[i1+4][2];

      for (j=i; j<=numblocks; j++)
      {
         i2  = k0[j][2];
         e2  = d[i2+2][1];
         x2  = d[i2+4][1];
         y2  = d[i2+4][2];
         if ( e1+e2 < fabs(x2-x1) )
            continue;  // goto b702;

         if ( e1+e2 < fabs(y2-yi) )
            continue;  // goto b702;

         d1  = (x2-x1)*(x2-x1)+(y2-yi)*(y2-yi);
         d1  = sqrt(d1);
         if ( e1+e2 < d1 )
            continue;  // goto b702;

         for (l =k0[i][1]; l <=k0[i][2];  l++)
         {
            for (l0=k0[j][1]; l0<=k0[j][2]; l0++)
            {
               x21 = d[l +1][1] - d[l   ][1];
               y21 = d[l +1][2] - d[l   ][2];
               x34 = d[l0  ][1] - d[l0+1][1];
               y34 = d[l0  ][2] - d[l0+1][2];
               x31 = d[l0  ][1] - d[l   ][1];
               y31 = d[l0  ][2] - d[l   ][2];
               d0=x21*y34-x34*y21; /* equation | |   */
               d1=x31*y34-x34*y31; /* | | for sol t1 */
               d2=x21*y31-x31*y21; /* | | for sol t2 */

               if ( fabs(d0)<.000000001*domainscale*domainscale)
                  continue;  // goto b701;

               local_t1=d1/d0;
               if ( (local_t1<0.0000001) || (local_t1>0.9999999) )
                  continue;  // goto b701;

               local_t2=d2/d0;
               if ( (local_t2<0.0000001) || (local_t2>0.9999999) )
                  continue;  // goto b701;
//b701:;
            } /* l0 */
         } /* l  */
//b702:;
      } /* j  */
   } /* i  */

}  /*  Close dc17().  */



/**************************************************/
/* dc19: save block data to file                  */
/**************************************************/
/* k0 is probably the vindex matrix */
void dc19(Geometrydata *gd, int **k0, double **d)
{
   int i;
   int oo;
   int numblocks = gd->nBlocks;
   //FILE *blockOutFile;

  /* Uses the rootname of the current geometry.*/
  /* FIXME:  Rewrite all this stuff to put out a decent block file. */
   FILE *blkfile;
   double ** g = gd->points;
   double ** gg1 = gd->rockbolts;

  /* This entire piece of code needs to be
   * moved to somewhere else.  For now,
   * the best place would be to move it to
   * the top of the geometry file, or
   * have it called from the geometry dialog
   * box.
   */


  /*  For now, print them both out.
   */
   blkfile = fopen("block.in", "w");
  /* FIXME:  Figure out some way of using the filepath struct
   * for both analysis and geometry data.
   */
   //blkfile = fopen(filepath->blockfile, "w");


  /* n0   number of blocks   nBolt number of bolts   */
  /* nFPoints fixed points       nLPoints loading points    */
  /* nMPoints measured points                           */
   oo = k0[numblocks][2]+4;
  /* Writes to "root.blk" */
   fprintf(blkfile,"%d %d %d \n",   numblocks,gd->nBolts,  oo);
   fprintf(blkfile,"%d %d %d \n", gd->nFPoints,gd->nLPoints,gd->nMPoints);


  /* material number  block start  block end  index */
  /* The first (0th) array element should be the block
   * material number.  I don't believe this is currently
   * implemented.  Also, wherever this is determined, there
   * needs to be some argument checking.
   */
   for (i=1; i<= numblocks; i++)
   {
      fprintf(blkfile,"%d %d %d \n", k0[i][0], k0[i][1], k0[i][2]);
   } /* i */


  /* x  y of each block  0  s0  sx  sy  0  0        */
   for (i=1; i<= k0[numblocks][2]+4; i++)
   {
      fprintf(blkfile,"%f %f %f\n",d[i][0],d[i][1],d[i][2]);
   } /* i */


  /* gg1: x1  y1  x2  y2  n1  n2  e00  t0  f0    bolt */
  /* n1 n2 carry block number        f0 pre-tension */
  /* This has to be indexed from 0!!!!
   */
   for (i=0; i< gd->nBolts; i++)
   {
     /* Write to the "rootname.blk" file. */
      fprintf( blkfile, "%f %f %f\n", gg1[i][1],gg1[i][2],gg1[i][3]);
		    fprintf( blkfile, "%f %d  %d\n", gg1[i][4],(int)gg1[i][5],(int)gg1[i][6]);
      fprintf( blkfile, "%f %f %f\n", gg1[i][7],gg1[i][8],gg1[i][9]);
   }   /* i */


  /* x  y  of fixed loading measured points         */
   for (i=1; i<= gd->nFPoints+gd->nLPoints+gd->nMPoints; i++)
   {
    		fprintf(blkfile,"%f %f %f %f \n",
              g[i][0],g[i][1],g[i][2],g[i][3]);
   } /* i */

      fflush(blkfile);
      fclose(blkfile);
}  // Close dc19


/* Instead of writing data to a file, copy into a new struct that
 * can be returned to the calling routine.  This routine mallocs,
 * the calling routine is responsible for freeing the memory.
 */
/* FIXME: Eliminate this function completely by pushing stuff into
 * the geometry data structure that should be in there.
 */
Geometrydata *
geometryToReturn(Geometrydata *gd, int **k0, double **d)
{
   int i;
   int n2 = gd->nIntersectionPoints;
   /* int n5 =  gd->nFPoints*(15+1)+gd->nLPoints+gd->nMPoints+gd->nHPoints+1; */

   gd->vertexCount = k0[gd->nBlocks][2] + 4;

  /* Set some handy sizes in the struct.  This will help
   * when proting to c++.
   */
   gd->vertexsize1 = 8*n2+1;
   gd->vertexsize2 = 3;
   gd->vertices = DoubMat2DGetMem(8*n2+1,3);
   gd->origvertices = DoubMat2DGetMem(8*n2+1,3);

   gd->vindexsize1 = 2*n2+1;
   gd->vindexsize2 = 3;
   gd->vindex = IntMat2DGetMem(2*n2+1,3);


  /* material number  block start  block end  index */
  /* The first (0th) array element should be the block
   * material number.  I don't believe this is currently
   * implemented.  Also, wherever this is determined, there
   * needs to be some argument checking.
   */
   for (i=1; i<= gd->nBlocks; i++)
   {
      gd->vindex[i][0] = k0[i][0];
      gd->vindex[i][1] = k0[i][1];
      gd->vindex[i][2] = k0[i][2];
   } /* i */


  /* x  y of each block  0  s0  sx  sy  0  0        */
   for (i=1; i<= k0[gd->nBlocks][2]+4; i++)
   {
      gd->vertices[i][0] = d[i][0];
      gd->vertices[i][1] = d[i][1];
      gd->vertices[i][2] = d[i][2];
      gd->origvertices[i][0] = d[i][0];
      gd->origvertices[i][1] = d[i][1];
      gd->origvertices[i][2] = d[i][2];
   } /* i */

   gd->momentsize1=gd->nBlocks+1;
  /* __momentsize2=7;*/
  /* Add 2 for centroid (x,y) */
   gd->momentsize2=9;
   gd->moments = DoubMat2DGetMem(gd->momentsize1, gd->momentsize2);

   return gd;

}



/**************************************************/
/* crr: find carry block for a point              */
/**************************************************/
/* k0[][] d[][] q[][]  j l jo j1 i2 l0 mm3 e1 a1   */
/* x1 y1 x21 y21 x31 y31 x34 y34 t1 t2 d0 d1 d2   */
/* x11 y11 kk3                   x2 y2 x3 y3 x4 y4 */
/*  Called by dc11, dc13 and dc14, sets the value of kk3,
 * which is a real pain the neck.
 */
/* x11, y11 are copies of the point passed in from the calling function.
 *
 * k0 (vindex) is an array of integers that point at vertices located
 * in d.
 *
 * d is an array of doubles containing (x,y) pairs of vertex locations.
 * It stores all the vertices for all of the blocks.  The starting and
 * ending blocks are pointed to from an integer array k0. A copy of
 * the ending vertex may be stored in the position before the starting
 * vertex for a block, and the starting vertex location for a block may
 * be stored after the ending vertex.  This allows "cyclic" traversal
 * over the array of vertices to eg compute determinants in a single
 * loop without storing vertices and without doing any work outside the
 * loop.
 *
 * q is the "length-relation" matrix.  I do not understand this variable
 * at all.
 *
 * gd is a defined type struct Geometrydata.  See the header file dda.h
 * for the type definition.  Note that some of the fields of the struct
 * are not initialized until after all the functions in the block
 * cutting code have been called.
 *
 * Called from dc11(), dc13(), dc14().
 */
int
crr(Geometrydata *gd, double  x11, double y11, int ** vindex, double **q,
        double **vertices)
{
  /* blocknum is an index counter (was j), but it will be returned if
   * the point under investigation gets is in the particular
   * block.
   */
   int blocknum;
   int ell;  // loop counter
   double d0, d1, d2;
  /* l0 counts the number of times a ray crosses an edge
   * of the polygon.
   */
   int l0;
   //int raycrossing;
   int mm3;  // temp var, delete
   double e1;
   double x3, x4;
   int i2;
   double y3, y4;
   double y1, y0, y2;
   double x21, y21, x34, y34, x31, y31, x41, y41;
   double x0, x1, x2;
  /* a1 is probably side length */
   double distance;  //  was a1
  /* n0 is number of blocks to loop over */
   int numblocks = gd->nBlocks;
   int j0, j1;
  /* w0 may be a domain scaling variable */
   double w0 = gd->w0;
  /* parameters */
   double t1, t2;
  /* kk3 does NOT appear to used as a boolean variable in this
   * procedure.  What is even more interesting is that if it is
   * declared local to this procedure, then none of the hole
   * points work properly.
   */
   int kk3;

  /* Note that kk3 is reinitialized on _every_ call.
   */
   kk3 = 0;

  /* This loop returns a block number when a point coincides with
   * a node of a particular block.
   */
   for (blocknum = 1; blocknum <= numblocks; blocknum++)
   {
     /* (GHS: case point coinside with a block node)  */
     /* (GHS: q[i][1] is   sin angle from y to x)     */
      j0 = vindex[blocknum][1];  /* j0 := start index */
      j1 = vindex[blocknum][2];  /* j1 := stop index  */

      for (ell=j0; ell<= j1; ell++)
      {
         x1 = vertices[ell][1] - x11;
         y1 = vertices[ell][2] - y11;
         distance = sqrt((x1*x1) + (y1*y1));
         if ( distance >= (.00001*w0) )
         {
           /* FIXME: What is this? */
            q[ell-j0+1][1] = fabs(x1)/distance;
            continue;
			         //goto cr01;
         }
         return blocknum;
	   } /* l */

     /* choose a ray not passing any block nodes       */
     /* 0< e1 <1  e1=sin angle from y to x             */
     /* srand and rand may have some peculiar features.
      * These should probably be investigated.  They have
      * been known to fail.
      */
      srand(2);

/* FIXME: Change this to a while or do-while loop. */
cr04:
     /* See note on srand previous */
      i2 = rand();
     /* The following line is commented out back to version 1.1 */
      // e1 = (double)i2/(1024.0*1024.0*2048.0);
     /* Why is this parameter different than the e1 computed
      * in rlt() ?
      */
      e1 = (double)i2/2048.0;
     /* Try a new value... from rlt() */
      //e1 = (double)i2/(1024.0*1024.0*2);

     /* This should probably get changed into a do or while loop
      * to eliminate the goto
      */
      for (ell=1; ell<= j1-j0+1; ell++)
	     {
        /* FIXME: Where is q[][] initialized? */
         if ( fabs(q[ell][1]-e1)>.001 )
			         continue;  // goto cr03;   //continue;
         goto cr04;
//cr03:;
   	  } /* l */


     /* (GHS: number of the intersection points    l0)      */
     /* (GHS: e1=sin angle from y to x  direction  x0 yo)   */
    		l0  = 0;
    		j0  = vindex[blocknum][1];
    		j1  = vindex[blocknum][2];

    		for (ell= j0; ell<= j1; ell++)
      {
    	   	x1  = vertices[ell][1];
	       	y1  = vertices[ell][2];
	       	x2  = vertices[ell+1][1];
       		y2  = vertices[ell+1][2];
       		x0  = e1;
       		y0  = sqrt(1 - (e1*e1) );
	       	x3  = x11;
         y3  = y11;
         x4  = x11 + (x0*5*w0);
         y4  = y11 + (y0*5*w0);         /* 5*w0 can go out */
        /*------------------------------------------------*/
         x21 = x2 - x1;                 /* coefficient a11 */
         y21 = y2 - y1;                 /* coefficient a21 */
         x34 = x3 - x4;                 /* coefficient a12 */
         y34 = y3 - y4;                 /* coefficient a22 */
         x31 = x3 - x1;                 /* free terms  f1  */
         y31 = y3 - y1;                 /* free terms  f2  */
         x41 = x4 - x1;                 /* co-line case    */
         y41 = y4 - y1;
        /*------------------------------------------------*/
         d0 = x21*y34 - x34*y21;        /* equation | |    */
         d1 = x31*y34 - x34*y31;        /* | | for sol t1  */
         d2 = x21*y31 - x31*y21;        /* | | for sol t2  */

        /*------------------------------------------------*/
        /* p1 p2   p3 p4  are two parallel lines          */
         if ( fabs(d0) < (.00000000001*w0*w0) )
            continue;  // goto cr05;  //continue
        /*------------------------------------------------*/
        /* normal intersection                            */
        /* x11 y11 in edge intersection point can be even */
         t1 = d1/d0;
         if ( (t1 < -.000000001) || (t1 > 1.000000001) )
            continue;  // goto cr05;  //continue;

         t2 = d2/d0;

        /*
         if (fabs(t2) > 0.000000001 )
            goto cr07;

         kk3 = blocknum;
            return kk3;  // goto cr02;
         */

         if (fabs(t2) <= 0.000000001 )
         {
            kk3 = blocknum;
            return kk3;  // goto cr02;
         }

//cr07:
         if ( (t2 < -.000000001) || (t2 > 1.000000001) )
            continue; //goto cr05;  //continue

         l0=l0+1;
//cr05:;
      } /* l */

     /* GHS implements arithmetic modulo 2 with the following code.
      * The epsilon 1e-6 is probably not needed.  Since the function
      * implements Jordan's theorem, this checks for an even or odd
      * number of intersections of a ray through the polygon.
      * FIXME:  Replace this with the c modulo operator %
      * if(l0%2)
      *    return blocknum;  // odd number of ray crossings.
      */
		    mm3= (int) (l0-floor(l0/2+.000001)*2);
      if ( mm3 == 0 )
         continue;  // goto cr06;  //continue;
      kk3 = blocknum;
         return kk3;  // goto cr02;  //break;????
//cr06:;
   } /*  j */

//cr02:;
  /* This should return 0 for out, blocknum for in.  */
	  return kk3;
}  /*  Close crr().  */



/**************************************************/
/* rlt  judge if a point in a block               */
/**************************************************/
/* k0[][] d[][] q[][]  j l jo j1 i2 l0 mm3 e1 a1   */
/* x1 y1 x21 y21 x31 y31 x34 y34 t1 t2 d0 d1 d2   */
/* i0 x11 y11 kk3                x2 y2 x3 y3 x4 y4 */
/* rlt returns in or out for a point in a block. crr returns
 * the applicable block number.  The number of the block is passed
 * in as the variable named "blocknumber", which wasnamed "i0".
 * Called by dc16() only.
 */
/* blocknumber (was i0) is the label number of the block for which
 * to check for point inclusion.
 *
 * x11, y11 are copies of the point passed in from the calling function.
 *
 * k0 (vindex) is an array of integers that point at vertices located
 * in d.
 *
 * d (vertices) is an array of doubles containing (x,y) pairs of vertex locations.
 * It stores all the vertices for all of the blocks.  The starting and
 * ending blocks are pointed to from an integer array k0. A copy of
 * the ending vertex may be stored in the position before the starting
 * vertex for a block, and the starting vertex location for a block may
 * be stored after the ending vertex.  This allows "cyclic" traversal
 * over the array of vertices to eg compute determinants in a single
 * loop without storing vertices and without doing any work outside the
 * loop.
 *
 * q is the "length-relation" matrix.  I do not understand this variable
 * at all.
 *
 * gd is a defined type struct Geometrydata.  See the header file dda.h
 * for the type definition.  Note that some of the fields of the struct
 * are not initialized until after all the functions in the block
 * cutting code have been called.
 */
/* FIXME: There are a lot of tolerance values in this code that are
 * completely undocumented.  Document them.
 */
int
//rlt(Geometrydata *gd, int blocknumber, double x11, double y11, int ** vindex,
//         double ** q, double ** vertices)
rlt(double w0, int blocknumber, double x11, double y11, int ** vindex,
         double ** q, double ** vertices)
{
   int ell;
   double d0, d1, d2;
  /* l0 counts the number of times a ray crosses an edge
   * of the polygon.
   */
   int l0;
   //int raycrossing;
   int mm3;  // temp var, delete
   double e1;
   double x3, x4;
   int i2;
   double y0, y1, y2, y3, y4;
   double x21, y21, x34, y34, x31, y31;
   double x0, x1, x2;
  /* a1 is probably a side length */
   double distance;  //  was a1
  /* j0, j1 are start, stop indices for vertex array */
   int j0, j1;
  /* kk3 is boolean in this function */
   int kk3;
  /* t1, t2 probably parameters. */
   double t1, t2;

   kk3 = 0;

		/* (GHS: case point coinside with a block node)  */
		/* (GHS: q[i][1] is   sin angle from y to x)     */
		 j0 = vindex[blocknumber][1];
	 	j1 = vindex[blocknumber][2];

   for (ell=j0; ell<= j1; ell++)
	 	{
		    x1 = vertices[ell][1] - x11;
		    y1 = vertices[ell][2] - y11;

		    distance = sqrt(x1*x1 + y1*y1);

     /* If the points are far enough apart, compute the
      * sin(?) of the angle between them.
      */
      if ( distance >= .0000001*w0 )
      {
         q[ell-j0+1][1] = fabs(x1)/distance;
         continue;
         //goto rl01;  // continue;
		    }
      // else it is close enough to call it "in"
      return 1;  // 1 = In?
//rl01:;
   } /* l */


  /* (GHS: choose a ray not passing any block nodes)    */
		/* (GHS: 0< e1 <1  e1=sin angle from y to x)          */
  /* FIXME: Change the seed from 2 to something a little
   * more robust.  This should not be called from here.
   * FIXME: This random number really only works one time.
   * srand should be initialized one time at the beginning
   * of the program.  However, unless srand is initialized
   * here every call to crr or rlt, these functions do not
   * work properly.  There is a really bad side effect here.
   */
		 srand(2);




/* FIXME: Replace this label with a while or do-while loop. */
rl04:;

    		i2 = rand();
      //		e1 = (double)i2/(1024.0*1024.0*2048.0);
      e1 = (double)i2/(1024.0*1024.0*2);

     /* This loop spins for each block.  The main purpose is
      * probably to assign a value to e1 for use later.
      */
      for (ell=1; ell<=j1-j0+1; ell++)
      {
        /* FIXME: Replace this fabs call with a double conditional.
         */
        /* The idea seems to be to get e1 at least a certain value
         * away from any and every value of q[][1].
         */
         if( fabs(q[ell][1]-e1) > .001 )
            continue;  //goto rl03;  //continue;
        /* else e1 is too close to q[ell][1], get a new e1 */
         else
            goto rl04;  //  Change to while loop


      } /* l */

		/* (GHS: number of the intersection points    l0)       */
		/* (GHS: e1=sin angle from y to x  direction  x0 y0)    */
 		l0  = 0;
	 	j0 = vindex[blocknumber][1];
		 j1 = vindex[blocknumber][2];

   for (ell= j0; ell<= j1; ell++)
	 	{
     /* Get the endpoints for a line segment defining the
      * edge of a block.
      */
		    x1  = vertices[ell][1];
		    y1  = vertices[ell][2];
		    x2  = vertices[ell+1][1];
		    y2  = vertices[ell+1][2];
     /* e1 assigned in previous for loop. */
	 	   x0  = e1;
		    y0  = sqrt(1 - (e1*e1) );
		    x3  = x11;
		    y3  = y11;
 		   x4  = x11 + (x0*5.0*w0);
	 	   y4  = y11 + (y0*5.0*w0);      /* 5*w0 can go out */

		    x21 = x2 - x1;                /* coefficient a11 */
		    y21 = y2 - y1;                /* coefficient a21 */
		    x34 = x3 - x4;                /* coefficient a12 */
	 	   y34 = y3 - y4;                /* coefficient a22 */
		    x31 = x3 - x1;                /* free terms  f1  */
		    y31 = y3 - y1;                /* free terms  f2  */

		    d0 = x21*y34 - x34*y21;       /* equation | |    */
	 	   d1 = x31*y34 - x34*y31;       /* | | for sol t1  */
		    d2 = x21*y31 - x31*y21;       /* | | for sol t2  */

     /*------------------------------------------------*/
	 	  /* p1 p2   p3 p4  are two parallel lines          */
		    if ( fabs(d0)< (.00000000001*w0*w0)  )
         continue;

     /* normal intersection                            */
		   /* x11 y11 in edge intersection point can be even */
		    t1 = d1/d0;

	 	   if ( (t1 < -.000000001) || (t1 > 1.000000001) )
         continue;
		    t2 = d2/d0;

     /*
      fprintf(pnpfile, "\nmm3: %d\n l0: %d\n e1: %f\n x4: %f\n y4: %f\n i2: %d\n"
                       " t1: %f\n t2: %f\n kk3: %d\n",mm3,l0,e1,x4,y4,i2,t1,t2,kk3);
      */

      if ( fabs(t2) <= 0.000000001)
         return 1;  // 1 = In?

     /* printf("x11 y11 on edge l= %d t2 = %f \n",l,t2); */

		    if ( (t2 < -.000000001) || (t2 > 1.000000001) )
         continue;  // goto rl05;  //continue;

		    l0=l0+1;

   } /* l */

  /* GHS implements arithmetic modulo 2 with the following code.
   * The epsilon 1e-6 is probably not needed.  Since the function
   * implements Jordan's theorem, this checks for an even or odd
   * number of intersections of a ray through the polygon.
   * FIXME:  Replace this with the c modulo operator %
   */
   mm3 = (int)(l0-floor(l0/2+.000001)*2);
   if ( mm3 == 0 )
      return kk3; //goto rl02;  //return kk3;  ( kk3 = 0?)

  /* printf("intersection number l0= %d \n",l0);    */
   return 1; //kk3 = 1;  // 1 = In?

/* There is a empty statement ";" after the goto label "rl02".
 * This keeps the compiler happy.
 */
//rl02:;
//   return kk3;

}   /*  Close rlt().  */


/* FIXME: Take k0 and d out of this function and have their
 * memory handled directly through the geometry struct.
 */
/* FIXME: Handle memory free by function calls.
 */
/* Necessary globals for memory management.
 */
static int __vertexsize1;
static int __vertexsize2;
static int __vindexdcsize1;
static int __vindexdcsize2;
static int __csize1;
static int __csize2;
static int __csize2;
static int __aasize1;
static int __aasize2;
static int __rsize1;
static int __rsize2;
static int __ksize1;
static int __ksize2;
static int __msize1;
static int __msize2;
static int __hsize1;
static int __qsize1;
static int __qsize2;

void
deallocateGData(Geometrydata *gd, int **aa, int **k, int **r, int **m,
                     int **vindexdc, int *h, double **q, double **c,
                     double ** vertex)
{
   int i;

		/* deallocate matrices */
		for(i=0; i<__aasize1; i++)
     free(aa[i]);
		free(aa); // aa = NULL;

  for(i=0; i<__ksize1; i++)
     free(k[i]);
		free(k);  k = NULL;

		for(i=0; i<__rsize1; i++)
     free(r[i]);
		free(r);  r = NULL;

  free2DMat((void **)vertex, __vertexsize1);

  for(i=0; i<__msize1; i++)
     free(m[i]);
		free(m);  m = NULL;

		for(i=0; i<__vindexdcsize1; i++)
		   free(vindexdc[i]);
		free(vindexdc);
		vindexdc = NULL;

		free(h);
  h = NULL; /* single pointer only */

  for(i=0; i<__qsize1; i++)
     free(q[i]);
		free(q);  q = NULL;

  for(i=0; i<__csize1; i++)
     free(c[i]);
		free(c);  c = NULL;


}  /* Close deallocateGData().  */



void
allocateGeometryArrays(Geometrydata * gd, int *** aa, int *** k, int *** r,
                       int *** m,int ** h, int *** vindexdc, double *** q,
                       double *** c, double *** verticesdc)
{
   int numintersectpoints = gd->nIntersectionPoints;

  /* n2 including coinside points   edge<=2*n2      */
  /* a : 0-1 relation matrix point to line number   */
	 /* a : 0-0 relation matrix first vertex           */
	 	__aasize1=numintersectpoints+1;
	 	__aasize2=21;
  *aa = IntMat2DGetMem(__aasize1, __aasize2);

		/* k : 0-0 relation matrix                        */
		/* k : 0-0 rotation matrix next vertices          */
	 	__ksize1=numintersectpoints+1;
	 	__ksize2=21;
  *k = IntMat2DGetMem(__ksize1, __ksize2);

		/* r : 0-0 relation matrix line number i i+1 edge */
	 	__rsize1=numintersectpoints+1;
	 	__rsize2=21;
  *r = IntMat2DGetMem(__rsize1, __rsize2);

  /* m : point number in a line i                   */
  /* m : segments in a node in a block              */
  /* m : nodes in a block                           */
   __msize1=numintersectpoints+1;
   __msize2=3;
  *m = IntMat2DGetMem(__msize1, __msize2);

  /* no merge point so <= 4 edges/point edge=2 ends */
  /* 2*n2 edges 4*n2 block nodes 4*n2 >=3 nodes/blk */
  /* <= 4*n2/3 blocks (least 3 edges per block)     */
  /* total vertices 4*n2 + 4*(4*n2/3) block:4*n2/3  */
  /* h : vertices of blocks                         */
   __hsize1=8*numintersectpoints+1;
   //h =(int *)malloc(sizeof(int)*n4);
  *h =(int *)calloc(__hsize1,sizeof(int));

  /* number of blocks (given?) by Euler formula              */
  /* k0: block index of d[]                         */
   __vindexdcsize1=2*numintersectpoints+1;
   __vindexdcsize2=3;
  *vindexdc = IntMat2DGetMem(__vindexdcsize1, __vindexdcsize2);

		/* q : 0-1 length relation matrix                 */
		/* q : 0-0 angle matrix                           */
		/* q : value on a block                           */
	 	__qsize1=numintersectpoints+1;
	 	__qsize2=21;
  *q = DoubMat2DGetMem(__qsize1, __qsize2);

  /* c : coordinates of nodes  */
   __csize1=numintersectpoints+1;
   __csize2=3;
  *c = DoubMat2DGetMem(__csize1, __csize2);

  /* no merge point so <= 4 edges/point edge=2 ends */
  /* 2*n2 edges 4*n2 block nodes 4*n2 >=3 nodes/blk */
  /* <= 4*n2/3 blocks (least 3 edges per block)     */
  /* total vertices 4*n2 + 4*(4*n2/3) block:4*n2/3  */
  /* d : distance of vertices in a line             */
  /* d : angles of segments in a node               */
  /* d : vertices of blocks                         */
   /* n4=8*n2+1; */
   __vertexsize1 = 8*numintersectpoints+1;
   /* n8=3; */
   //__vertexsize2 = 3;
  /* From 0: leading (?) joint type; coords:  x, y, z;
   * centroid coords: x_0, y_0, z_0
   */
   __vertexsize2 = 7;
  *verticesdc = DoubMat2DGetMem(__vertexsize1, __vertexsize2);

}  /* close allocateGeometryArrays() */




