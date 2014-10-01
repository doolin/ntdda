



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dda.h"
#include "wingraphgl.h"
#include <assert.h>
#include <gl/glut.h>
#define GEOMSCALE 1
/* Win32 calling conventions. */
#ifndef CALLBACK
#define CALLBACK
#endif

extern FILEPOINTERS fp;

char mess[80];

#if OPENGL
int 
drawCentroidsGL(GRAPHICS * g, DATALOG * dl, GEOMETRYDATA * gd)
{   
   int i, j;
   double scale = g->scale;
   POINT offset = g->offset;
   DPoint ** c = dl->centroids;
   double p1x,p1y,p2x,p2y;
   double offsetx = g->offsetx;
   double offsety = g->offsety;
   float red,green,blue;

    for (i = 1; i< g->numtimesteps-1; i++) 

   {		glPushMatrix();
			glBegin(GL_LINES);
			red=g->R[2];green=g->G[2];blue=g->B[2];
			glColor3f( red, green, blue );

 		  	for (j=1;j<=gd->nBlocks;j++)

      {
		       p1x = ((c[i][j].x)*scale - offsetx);
		       p1y = ((c[i][j].y)*scale - offsety);
		       p2x = ((c[i+1][j].x)*scale - offsetx);
		       p2y = ((c[i+1][j].y)*scale - offsety);
			glVertex2d( p1x, p1y);
			glVertex2d( p2x, p2y);
        
		glEnd();

	glPopMatrix();
		       
      }
   }  // end for() each centroid
 
   return 1;
}  /* Close drawCentroid() */


int 
drawStressesGL( GRAPHICS * g, DATALOG * dl, GEOMETRYDATA * gd)
{   

   int j;
   POINT p1, p2;
  	double scale = g->scale;
   POINT offset = g->offset;
   PStress ** s = dl->stresses;
  	double p1x,p1y,p2x,p2y;
  	float red,green,blue;

  	red=g->R[5];green=g->G[5];blue=g->B[5];
  	glColor3f( red, green, blue ); 
   
   //for (i = 1; i< g->numtimesteps-1; i++)
   //{
			glPushMatrix();
			glBegin(GL_LINES);

 		  	for (j=1;j<=gd->nBlocks;j++)
      {
        /* "Major" stresses... */
		       p1x = (s[0][j].majorx1)*scale + offset.x;
		       p1y = (s[0][j].majory1)*scale + offset.y;
		       p2x = (s[0][j].majorx2)*scale + offset.x;
		       p2y = (s[0][j].majory2)*scale + offset.y;

			glVertex2d( p1x, p1y);
			glVertex2d( p2x, p2y);

        /* "Minor" stresses... */
			p1.x = (int)((s[0][j].minorx1)*scale + offset.x);
		    p1.y = (int)((s[0][j].minory1)*scale + offset.y);
		    p2.x = (int)((s[0][j].minorx2)*scale + offset.x);
		    p2.y = (int)((s[0][j].minory2)*scale + offset.y);
		      
			glVertex2d( p1x, p1y);
			glVertex2d( p2x, p2y);

      }  // end for each block

	glEnd();
	glPopMatrix();
   //}  // end for() each time step
 
   return 1;
}  /* Close drawStresses() */

#endif