/* 
 * geomdriver.c 
 *
 * Drives the set of procedure that construct a block system
 * from a set of lines (fracture traces).
 * 
 * $Author: doolin $
 * $Date: 2001/08/26 02:15:50 $
 * $Source: /cvsroot/dda/ntdda/src/geomdriver.c,v $
 * $Revision: 1.2 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "geometry.h"
#include "interface.h"


extern InterFace * iface;

FILE * ddacutlog;
FILE * pnpfile;

/* 
 * ddacut is the driver routine for all of the dc code
 * functions.
 */
Geometrydata * 
ddacut(FILEPATHS *filestruct, GRAPHICS * graphics)
{
  
   //HWND hwMain = geomhwnd;
   
  /* Get rid of one of these, keep the other.
   */
   Geometrydata *geomdata;  
   Geometrydata *returnGeom;   
   char  *geomfile = filestruct->gfile;
   int **aa;
   int **k;
   int **r;
   int **m;
   int *h;
  /* FIXME: What is q? */
   double **q;
  /* FIXME: If c is local to block cutting, change the 
   * name to something different.
   */
   double **c;
  /* FIXME: Handle vindexdc and verticesdc in the geomdata 
   * struct.
   */
   int **vindexdc;
   double **verticesdc;

  /* This is a temporary hack.  Later I will need to fully
   * implement the geometry part.
   */
   ddacutlog = fopen("cut.log", "w");
   pnpfile = fopen("pnp.log","w");

   assert(geomfile != NULL);
   geomdata = geometryInput(geomfile);
   if (!geomdata) 
   {
      iface->displaymessage("Error in geometry file" /* Error */);
         return NULL;
   }

   geomdata->this = geomdata;

   assert(geomdata != NULL);

   geomdata->blocksize = 6;
   geomdata->index = 1;

  /* FIXME: Grab the seed from the input file.
   * FIXME: Initializing the generator here breaks
   * the point in polygon code.  
   */
   //srand(7243689);
 
  /* draw input lines and points          */
   computeDomainscale(geomdata);  // was dc02(geomdata);  

  /* dc03() used to define arrays with variable dimension,
   * that is, allocate the memory after determining the 
   * number of line intersections, etc.  This function 
   * will eventually disappear as linked lists get moved in.
   */
	countIntersections(geomdata);   // was dc03()  

   allocateGeometryArrays(geomdata, &aa, &k, &r, &m,
                          &h, &vindexdc, &q, &c, &verticesdc);

  /* intersection of lines */
   dc04(geomdata, aa, q, c);  
  /* tree cutting and forming edges        */
   dc05(geomdata, aa, k, r, m, q, c, verticesdc);  

  /* merge close node-node and node-edge   */
   dc06(geomdata, k, r, c);  

  /* directions of segments from a node    */
   dc07(geomdata, aa, k, r, m, q, c);  


  /* Part of what dc08 does is determine the number of 
   * blocks in the problem geometry.
   */
  /* assembling blocks from edges  */
   dc08(geomdata,aa, k, r, m, vindexdc, q, c, verticesdc);  
  /* merge co-line vertices  */
   dc09(geomdata, vindexdc, h, verticesdc);     
   
  /* forming fixed points from lines */
   dc10(geomdata, vindexdc, verticesdc);     

  /* block of fixed measured load hole pnt */
  /* Calls crr for point-in-poly */
   dc11(geomdata, vindexdc, q, verticesdc);
    
  /* compute area s sx sy of each block    */
  /* FIXME: Replace this with a call to computeMoments,
   * invoked via a function pointer.
   */
   dc12(geomdata, vindexdc, verticesdc);

  /* assign material number to blocks      */
  /* Calls crr for point-in-poly */
   dc13(geomdata, vindexdc, q, verticesdc);

  /*  Might be able to resurrect the rock bolting stuff.  */
  /* assign block number to 2 bolt ends    
   * Calls crr for point-in-poly
   */
   dc14(geomdata, vindexdc, q, verticesdc);

  /* check if a block inside another block */
  /* dc16 is the only function calling rlt for 
   * point-in-polygon algorithm.
   */
   dc16(geomdata, vindexdc, q, verticesdc);     
   
  /* check block mesh */
   dc17(geomdata, vindexdc, verticesdc);     

  /* Use dc19 to test the returnGeom function. */
   //dc19(geomdata, vindexdc, verticesdc);  /* save block data to a file */
  	
  /* FIXME:  This function needs to be rewritten to use handy
   * matrix cloning functions, as soon as the cloning functions
   * are fully vetted.  Even better, eliminate this function 
   * completely just having the geomdriver return the geometry
   * struct directly, and have the previous block file written 
   * as an option.
   */
   returnGeom = geometryToReturn(geomdata, vindexdc, verticesdc);

  /* A better idea would be to free geomdata here.  It is 
   * simple struct.  
   */
   deallocateGData(geomdata, aa, k, r, m, vindexdc, h, q, c, verticesdc);

   fclose(ddacutlog);
   fclose(pnpfile);

   return returnGeom;

}   /* Close geom()  */





