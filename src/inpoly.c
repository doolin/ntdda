
#include"geometry.h"

/*   1=inside, 0=outside                */
/* is target point inside a 2D polygon? */
//unsigned int poly[][2],            /*   polygon points, [0]=x, [1]=y       */
//int npoints,                       /*   number of points in polygon        */
//unsigned int xt,                   /*   x (horizontal) of target point     */
//unsigned int yt)                   /*   y (vertical) of target point       */
int                                    
pointinpoly(int blocknumber, double x11, double y11,
            int ** vindex, double ** vertices)
{
   double xnew, ynew;
   double xold, yold;
   double x1, y1;
   double x2, y2;
    
   int i;
   int start, stop;
   int inside = 0;

  /* BUG:  For the lp25.ana file, after running an analysis,
   * it crashes here on vindex null.
   */
   start = vindex[blocknumber][1]-1;
   stop = vindex[blocknumber][2]+1;
     
   xold = vertices[stop][1];
   yold = vertices[stop][2];

   for (i = start; i <= stop; i++) 
   {
      xnew = vertices[i][1];
      ynew = vertices[i][2];
      
      if (xnew > xold) 
      {
         x1 = xold;
         x2 = xnew;
         y1 = yold;
         y2 = ynew;
      }
      else 
      {
         x1 = xnew;
         x2 = xold;
         y1 = ynew;
         y2 = yold;
      }
     
      if ( (xnew < x11) == (x11 <= xold)  /* edge "open" at left end */
           && (y11-y1)*(x2-x1)
            < (y2-y1)*(x11-x1) ) 
      {
         inside = !inside;
      }

      xold = xnew;
      yold = ynew;
   }

   return(inside);

}  /* close pointinpoly() */



/***************************************************************************
 *                                                                         *
 *   INPOLY.C                                                              *
 *                                                                         *
 *   Copyright (c) 1995-1996 Galacticomm, Inc.  Freeware source code.      *
 *                                                                         *
 *   Please feel free to use this source code for any purpose, commercial  *
 *   or otherwise, as long as you don't restrict anyone else's use of      *
 *   this source code.  Please give credit where credit is due.            *
 *                                                                         *
 *   Point-in-polygon algorithm, created especially for World-Wide Web     *
 *   servers to process image maps with mouse-clickable regions.           *
 *                                                                         *
 *   Home for this file:  http://www.gcomm.com/develop/inpoly.c            *
 *                                                                         *
 *                                       6/19/95 - Bob Stein & Craig Yap   *
 *                                       stein@gcomm.com                   *
 *                                       craig@cse.fau.edu                 *
 *                                                                         *
 ***************************************************************************/
#include<stdlib.h>
#include<stdio.h>

int                                /*   1=inside, 0=outside                */
inpoly(                            /* is target point inside a 2D polygon? */
double ** poly,
int npoints,
double xt,
double yt)
//unsigned int poly[][2],            /*   polygon points, [0]=x, [1]=y       */
//int npoints,                       /*   number of points in polygon        */
//unsigned int xt,                   /*   x (horizontal) of target point     */
//unsigned int yt)                   /*   y (vertical) of target point       */
{
     //unsigned int xnew,ynew;
     //unsigned int xold,yold;
     //unsigned int x1,y1;
     //unsigned int x2,y2;
     double xnew, ynew;
     double xold, yold;
     double x1, y1;
     double x2, y2;
     static int testfile = 1;
     //FILE * tfp;
    
     int i;
     int inside=0;

/*
   if (testfile)
   {
      tfp = fopen("polytest","w");
      fprintf(tfp,"Pore pressure geometry data.\n");
      for (i=1; i <= npoints; i++)
      {
         fprintf(tfp,"%lf  %lf\n", poly[i][1], poly[i][2]);
      }
      fclose(tfp);
      testfile = 0;
   }     
*/
     if (npoints < 3) {
          return(0);
     }
     //xold=poly[npoints-1][0];
     //yold=poly[npoints-1][1];
     xold=poly[npoints-1][1];
     yold=poly[npoints-1][2];
     for (i=1 ; i <= npoints ; i++) {
          //xnew=poly[i][0];
          //ynew=poly[i][1];
          xnew=poly[i][1];
          ynew=poly[i][2];
          if (xnew > xold) {
               x1=xold;
               x2=xnew;
               y1=yold;
               y2=ynew;
          }
          else {
               x1=xnew;
               x2=xold;
               y1=ynew;
               y2=yold;
          }
          if ((xnew < xt) == (xt <= xold)         /* edge "open" at left end */
           && (yt-y1)*(x2-x1)
            < (y2-y1)*(xt-x1) ) 
          
          {
               inside=!inside;
          }

          xold=xnew;
          yold=ynew;
     }

     return(inside);

}  /* close inpoly() */
