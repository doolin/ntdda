
/*
 *  Add some comments here!
 */

#include<math.h>
#include"analysis.h"


#define NEWPOREPRESSURECODE 1

double getHead(void);
int findIndex(double **, int, double);
double interpolate();

extern FILEPOINTERS fp;

/**************************************************/
/* porepressure: submatrix of force induced by porepressure  */
/**************************************************/
/*  k1 is ???
 *  f is the "force" vector.
 *  e0 is material props: ma we e0 u0 c11 c22 c12 t-weight
 */
/* Next step for this routine is to save previous (x,y)
 * coordinates and compare whether previous and current
 * points separated by the water table.  If so, need to compute
 * the intersection of the water table and the block.
 * There several ways the integration could be done.
 * One way is to filter out all of the blocks that are
 * submerged and totally `dry', then loop over the remaining
 * blocks, integrating the pore pressure over each side.
 */
int porepressure(GEOMETRYDATA *bd, int *k1, double **f, double **e0,
          double **blockArea)
{
   //int j;
   int i2;
   int nBlocks = bd->nBlocks;
   double ** v = bd->vertices;
   int ** vIndex = bd->vIndex;
   double ** pppoly = bd->porepres;
   //double avgArea;
   //extern int currTimeStep;
   int inside;
   int block;
   int vertex;
   int numinside = 0;
#if NEWPOREPRESSURECODE
   double v1_x, v1_y,oldx, oldy;
   int index;
   //double currentx, currenty;
   int laststate;
   int v1_state;  /* State of first vertex on block.  */
   double a1;  /*  Length of block edge.  */
   double x1, x2, y1, y2;  /* Temp vars for computing normals. */
   double xp;
   //double yp;
   double p1, p2;
   double A, A1, A2;
   double gammaw = 62.4;
  /* Centroid x locations of rectangle and 
   * triangle projected on edge. 
   */  
   double cx1, cx2;
  /* Parameter for determining line segment intersection.
   */
   //double t;
   double h1, h2;
   double Fx, Fy;
   double F;  /* Resultant force.  */
   static  call = 0;
#endif

   //fprintf(fp.porefile,"%d\n", ++call);

 		for (block=1; block <= nBlocks; block++)
	 	{
      i2 = k1[block];
     /* x1=0  y1=0 here  */
        /* New code winged in for getting pore pressure 
         * integration.
         */
#if NEWPOREPRESSURECODE
        /* We need the state of the first vertex to compare 
         * with the last vertex.
         */         
         //v1_state = inpoly(pppoly, bd->nPorePres, v[vertex][1], v[vertex][2]);
         //v1_x = v[vertex][1];
         //v1_y = v[vertex][2];
        /* Copy these into variables that can be used while 
         * looping over all vertices on the block.
         */
         laststate = v1_state;
         oldx = v1_x;
         oldy = v1_y;
#endif NEWPOREPRESSURECODE 
      for (vertex=vIndex[block][1]; vertex<=vIndex[block][2]; vertex++)
      {
        /* First pass will check to see whether all vertices are
         * submerged. If so, then add gamma_w*blockarea to the
         * force vector in the y direction.  If not, continue out
         * out of this loop to the next block.
         */
         inside = inpoly(pppoly, bd->nPorePres, v[vertex][1], v[vertex][2]);
         v1_x = v[vertex][1];
         v1_y = v[vertex][2];
        /* This is a big kludge.  If _any_ vertex is not in the
         * pore pressure polygon, break and look at the next block.
         * This needs to be changed to track whether all, part or none
         * of the vertices are under the water table.
         */
         if (!inside) 
            break;
#if NEWPOREPRESSURECODE
        /* If laststate and inside below water table,
         * compute resulting force, add to F.
         */
      if (laststate && inside)
      { 
         /* "Underwater", integrate.  */
          ;
      }
        /* If laststate and inside are above and below,
         * or vice-versa, compute intersection of block 
         * edge and water table.  Integrate to find force
         * and add to F.
         */
      else if( (laststate == 1 && inside == 0) ||
               (laststate == 0 && inside == 1) )
      {
         /* Find intersection of water table with block, 
          * integrate.
          */
          ;
      } 
        /* Else, do nothing, continue. */
#endif NEWPOREPRESSURECODE
        
      }  /*  vertex  */
      if (inside)
      {          
         numinside++;  
        /* Should use gamma_w instead of 62.4 
         * to keep the units straight.  Put this in
         * analysis file.
         */
         f[i2][2] += 62.4*blockArea[block][1];
      } else
          continue;
#if NEWPOREPRESSURECODE
     /* Check the state of the last vertex with the 
      * state of the very first vertex.  This will result
      * in a set of duplicated code.  The apparent alternative 
      * is a function call, which will add overhead, or a 
      * macro, which loses type safety.  Two `identical' sets
      * of code induces a maintenance risk however.  It might 
      * be better to use the function call at first to make 
      * sure that the code works correctly.
      */
      if (laststate && inside)
      { 
         /* "Underwater", integrate.  First find the depth of 
          * each vertex below the water table.  Pressure 
          * distribution will be a trapezoid.
          */
          index  = findIndex(bd->watertable, bd->nWtable, oldx);
          h1 = interpolate(bd->watertable, index, v1_x);
          index  = findIndex(bd->watertable, bd->nWtable, oldx);
          h2 = interpolate(bd->watertable, index, oldx);
         /* Find volume of trapezoid, that is area times unit 
          * thickness.  Will need to use a1 as given below.
          * Basically, it's a1*(pres1 + pres2)/2, for unit
          * thickness.  The volume times the unit weight of
          * water will give the resulting force F.
          * Also, find centroid of trapezoid.  This can be done 
          * explicitly or by simplex.  Explicitly would be 
          * better.  
          */          
         /* First we need edge length.  */
          a1=sqrt((v1_x-oldx)*(v1_x-oldx)+(v1_y-oldy)*(v1_y-oldy));
          if (h1 > h2)
          {
             A1 = h2*a1;
             A2 = a1*(h2-h1)/2.0;
             cx2 = x1 + ((x2-x1)/3.0);
          }  else  {   /*  h1 < h2  */
             A1 = h1*a1;
             A2 = a1*(h2-h1)/2.0;
             cx2 = x1 + (2*(x2-x1)/3.0);
          }
          cx1 = (x2+x1)/2.0;
          A = A1 + A2;
          F = A*gammaw;
          fprintf(fp.porefile,"Block: %d, h1: %lf, h2: %lf, A: %lf, F: %lf\n", block,h1,h2, A, F);
         /* This is the x coordinate along the edge of the 
          * block subject to pore pressure.  Both vertices
          * are subject to pressure, distribution is a 
          * trapezoid.
          */
          xp = (A1*cx1 + A2*cx2)/A;
          
         /* Find direction of inner normal of block edge.
          * a1 := distance between adjacent vertices on
          * the ith block.
          */
          x1 = oldx;
          x2 = oldy;
          y1 = v[vertex][1];
          y2 = v[vertex][2];
          p1=(y1-y2)/a1; // x coord of unit inner normal 
          p2=(x2-x1)/a1; // y coord of unit inner normal
         /* Project water force from direction of normal 
          * to block edge along x and y directions.  This 
          * essentially a dot product because we already have
          * a unit vector.
          */
          Fx = p1*F;
          Fy = p2*F;
         /* Apply x and y forces at edge.  
          */
          ;
      }  
     /* There is probably a much slicker way to do this.
      */
      else if( (laststate == 1 && inside == 0) ||
               (laststate == 0 && inside == 1) )
      {
         /* Find intersection of water table with block, 
          * integrate.
          */
          ;
      } /* else both above water table, do nothing */
#endif NEWPOREPRESSURECODE
   }  /*  block  */

   return numinside;

}  /* Close porepressure() */



/* Stub function to get the porepressure code 
 * running.
 */
double
getHead()
{
  return 0;
} /*  Close getHead()  */



/* Finds the index where the value of the pore pressure is
 * to be determined.
 */
int
findIndex(double ** knots, int numknots, double val)
{
   int index = 0;

  /* Check range of value to ensure that the 
   * value is within the domain of the knots.
   * This should be unecessary if some preprocessing
   * of water table and block values is done before
   * analysis.  Then again, after the analysis starts,
   * block movement may require this.
   */
   if ( (val < knots[1][1]) || val > knots[numknots][1] )
      return -1;  /* out of range  */
   
  /* Find the index location.  */
   while ((val >= knots[index][0]) && (index < numknots))
      index++;

   return index;

}  /* Close findIndex()  */


/* Linear interpolation.  i is determined from the index.
 */
double
interpolate(double **knots, int i, double xval)
{
   double temp;

   temp = (knots[i-1][1]*((xval-knots[i][0])/(knots[i-1][0]-knots[i][0])) 
          + knots[i][1]*((xval -knots[i-1][0])/(knots[i][0]-knots[i-1][0])));

   return temp;

}    /* Close interpolate()  */