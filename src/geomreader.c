

/* <pre>
 * georeader.c
 * 
 * Procedure reads rock mass geometry from a file compatible
 * with Gen-Hua Shi's DDA program written in C.
 *
 * Copyright The University of California.  Distributed under 
 * the GNU Library General Public License (LGPL).  The terms of the
 * LGPL may be found at http://www.gnu.org./copyleft/lgpl.html
 *
 * DDA for Windows will be going into CVS (concurrent version
 * control in early 1999.  CVS is built on RCS (revision
 * control system), which uses the following keywords.
 * CVS is under copyright GPL, and may be found at 
 * http://www.cyclic.com.
 *
 * $Author: doolin $
 * $Date: 2002/05/26 23:47:25 $
 * $Source: /cvsroot/dda/ntdda/src/geomreader.c,v $
 * $Revision: 1.3 $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "assert.h"
#include "geometry.h"
#include "ddamemory.h"

///extern InterFace * iface;


/**************************************************/
/* geometryReader:  geometric data input          */
/**************************************************/
/* This procedure should return a pointer to a single 
 * struct that contains everything that could possibly 
 * be in a geometry input file.  If there is an error
 * in the input file, then the geometryReader should return a 
 * NULL pointer.   geometryReader replaces the dc01 
 * procedure.
 */
Geometrydata * geometryReader1(char *geometryInputFile)
{
   int i;  /* loop counter */
   /*  This is the struct that will get passed back.
    * A new one of these is grabbed on every call to this
    * routine, so they will have to be disposed of at some
    * point, probably at the end of the driver routine.
    */
   Geometrydata * gid;
   int n5;  /* scratch variable for array malloc. */
  /* geomInFile is the geometry data input file pointer.  
   */
   FILE * geomInFile;

   geomInFile = fopen (geometryInputFile, "r");
  /* Should throw an exception here. */
   assert (geomInFile != NULL);
  /* initialize the geometry struct...
   */
   //gid = initGeometrydata();
   gid = gdata_new();


  /* Input geometric data.  All of this needs to 
   * be range and type checked.
   */
   fscanf(geomInFile,"%lf",&gid->e00);  /* minimum edge-node distance */
   fscanf(geomInFile,"%d%d", &gid->nJoints,&gid->nBoundaryLines);
   fscanf(geomInFile,"%d", &gid->nMatLines);
   fscanf(geomInFile,"%d", &gid->nBolts);
   fscanf(geomInFile,"%d", &gid->nFPoints);
   fscanf(geomInFile,"%d", &gid->nLPoints);
   fscanf(geomInFile,"%d", &gid->nMPoints);
   fscanf(geomInFile,"%d", &gid->nHPoints);

  /* Catch non-zero numbers of rock bolts.  Since bolts are 
   * not implemented in the solver, but alot of the code exist between
   * here and there, just catch it for now then exit.
   */
   if (gid->nBolts != 0)
   {
//#if WINDOWS
      //MessageBox(hwMain, "Rock bolts are not yet implemented.", "Rock bolts", MB_OK);
      dda_display_warning("Rock bolts are not yet implemented" /* "Warning" */);
//#else
//      printf("Rock bolts are not yet implemented.\n");
//#endif
      //exit(0);  // Change to return
   }

  /* Catch non-zero numbers of load points.  
   */
   if (gid->nLPoints != 0)
   {
//#if WINDOWS
      //MessageBox(hwMain, "Load points not available in this file format.", "Load points", MB_OK);
      dda_display_warning("Load points not available in this file format" /* "Warning" */);
//#else
//      printf("Load points not avaiable in this file format.\n");
//#endif
      // exit(0);  // Change to return
      
   }

   gid->nPoints = gid->nFPoints + gid->nMPoints + gid->nLPoints + gid->nHPoints;
      
  /* b is Joint lines: x1  y1  x2  y2  n  where 
   * n is the material type.
   */
   gid->jointsize1=gid->nJoints+1;
   gid->jointsize2=6;
   gid->joints = DoubMat2DGetMem(gid->jointsize1, gid->jointsize2);
   if (gid->joints == NULL) 
      return NULL;

  
  /* gg0:   x1  y1  x2  y2  n  of material lines     */
   gid->matlinesize1=gid->nMatLines+1;
   gid->matlinesize2=6;
   gid->matlines = DoubMat2DGetMem(gid->matlinesize1, gid->matlinesize2);
   if (gid->matlines == NULL)
      return NULL;
   
  /* g : +nFPoints  x1  y1  x2  y2  of fixed lines   */
		/* g : +nLPoints  x   y   of loading  points        */
		/* g : +nMPoints  x   y   of measured points        */
		/* g : +nHPoints  x   y   of hole     points        */
 		// n5=nFPoints*(150+1)+nLPoints+nMPoints+nHPoints+1;
	 	n5 =  gid->nFPoints*(15+1)+gid->nLPoints+gid->nMPoints+gid->nHPoints+1;
	 	gid->pointsize1=n5+1;
	 /*	gid->pointsize2=7; */ /* was 5, changed to handle point disp and cum disp */
   gid->pointsize2 = 9;  /* add cum disp in [][7] and [][8]  */
   gid->points = DoubMat2DGetMem(gid->pointsize1, gid->pointsize2);
   setMatrixToZero(gid->points, gid->pointsize1, gid->pointsize2);
   gid->prevpoints = clone2DMatDoub(gid->points, gid->pointsize1, gid->pointsize2);


/*

   gid->points = DoubMat2DGetMem(gid->pointsize1, gid->pointsize2);
   if (gid->points == NULL) 
      return NULL;
*/

  /* Note loops start from 1 here and 0 elswhere.  
   */
  /* This loop reads in "line data" from the input 
   * file.  The original variable was b, passed back
   * through the struct as gid->joints.  Data file 
   * has five points on each line this scans.
   */
   for (i=1; i<= gid->nJoints; i++)
   {
	 fscanf(geomInFile,"%lf %lf %lf %lf %lf",
	         &gid->joints[i][1],&gid->joints[i][2],&gid->joints[i][3],
                 &gid->joints[i][4],&gid->joints[i][5]);
   } /* i */
		
  /* Scans in 5 points per line.  
   */
  for (i=0; i< gid->nMatLines; i++)
  {
    fscanf(geomInFile,"%lf  %lf  %lf  %lf  %lf",
        &gid->matlines[i][1], &gid->matlines[i][2], 
        &gid->matlines[i][3], &gid->matlines[i][4], &gid->matlines[i][5]);
  } /* i */
		
  
  /* Scans in 4 points per line.
   */
	 	for (i= 1; i<= gid->nFPoints; i++)
		 {
      fscanf(geomInFile,"%lf  %lf  %lf  %lf",
	 	         &gid->points[i][1], &gid->points[i][2], &gid->points[i][3], &gid->points[i][4]);
		    gid->points[i][0] = 0;  /* added 8/9/95 for graphics */
	 	}   /* i */
  
  /* Scans in 2 points per line.
   */
   for (i=gid->nFPoints+1; i<= gid->nFPoints+gid->nLPoints; i++)
	 	{
      fscanf(geomInFile,"%lf %lf",&gid->points[i][1],&gid->points[i][2]);
		    gid->points[i][0] = 2;  /* added 8/9/95 for graphics */
	 	}   /* i */
  
  
   for (i=gid->nFPoints+gid->nLPoints+1; i<= gid->nFPoints+gid->nLPoints+gid->nMPoints; i++)
	 	{
	  	   fscanf(geomInFile,"%lf %lf",&gid->points[i][1],&gid->points[i][2]);
		     gid->points[i][0] = 1;  /* added 8/9/95 for graphics */
	 	}   /* i */
  
   for (i=gid->nFPoints+gid->nLPoints+gid->nMPoints+1;i<= gid->nFPoints+gid->nLPoints+gid->nMPoints+gid->nHPoints; i++)
   {
		    fscanf(geomInFile,"%lf %lf",&gid->points[i][1],&gid->points[i][2]);
	    	gid->points[i][0] = 3;  /* added 8/9/95 for graphics */
   }   /* i */
		
   fclose(geomInFile);

   gid->pointCount = (gid->nFPoints) + (gid->nLPoints) + (gid->nMPoints);

   return gid;

}   /* Close geometryReader.  */

