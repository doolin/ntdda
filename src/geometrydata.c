/*
 * geometrydata.c
 *
 * Implements all the public methods associated with a
 * a geometry struct.  
 */

#include <stdlib.h>
#include <assert.h>


#if _DEBUG
#include "printdebug.h"
#endif


#include "geometrydata.h"

#include "ddamemory.h"
#include "ddaml.h"
#include "inpoly.h"

#define I1 "   "
#define I2 "      "

static void dumpDDAMLGeometryFile(Geometrydata *, char *);

static void deleteBlock(Geometrydata * gd, int blocknumber);



static void
emitJoints(Geometrydata * gd, PrintFunc printer, void * stream) {


   int i;

   printer(stream,I1"<Jointlist>\n");


   for (i=1;i<=gd->nJoints;i++) {


      printer(stream,I2"<Joint type=\"%d\">",(int)gd->joints[i][5]);
      printer(stream," %14.10f %14.10f %14.10f %14.10f ",
              gd->joints[i][1],gd->joints[i][2],
              gd->joints[i][3],gd->joints[i][4]);
      printer(stream,"</Joint>\n");
   }
   printer(stream,I1"</Jointlist>\n\n");

}


/* FIXME: Completely rewrite point handling code,
 * for the entire project.
 */
static void
emitPoints(Geometrydata * gd, PrintFunc printer, void * stream) {


   int i;

/*************  Fixed points  **************/
if (gd->nFPoints > 0)
{
   printer(stream,"<Fixedpointlist>\n");


   for(i=1;i<=gd->nFPoints;i++) {


      printer(stream,"<Line>");
      printer(stream," %f %f %f %f ",
              gd->points[i][1],gd->points[i][2],
              gd->points[i][3],gd->points[i][4]);
      printer(stream,"</Line>\n");
   }
   printer(stream,"</Fixedpointlist>\n");
}

/************** Load points ****************/
if (gd->nLPoints > 0)
{
   printer(stream,"<Loadpointlist>\n");
   for(i=gd->nFPoints+1; i<= gd->nFPoints+gd->nLPoints; i++)
   {
      printer(stream,"<Point>");
      printer(stream," %f %f ",
              gd->points[i][1],gd->points[i][2]);
      printer(stream,"</Point>\n");
   }
   printer(stream,"</Loadpointlist>\n");
}

/**************** Measured points  ************/
if (gd->nMPoints > 0)
{
   printer(stream,"<Measuredpointlist>\n");      
   for (i=gd->nFPoints+gd->nLPoints+1; i<= gd->nFPoints+gd->nLPoints+gd->nMPoints; i++)
	{
      printer(stream,"<Point>");
      printer(stream," %f %f ",
              gd->points[i][1],gd->points[i][2]);
      printer(stream,"</Point>\n");
   }
   printer(stream,"</Measuredpointlist>\n");
}

/**************  Hole points ****************/
if (gd->nHPoints > 0)
{
   printer(stream,"<Holepointlist>\n");      
   for (i=gd->nFPoints+gd->nLPoints+gd->nMPoints+1;i<= gd->nFPoints+gd->nLPoints+gd->nMPoints+gd->nHPoints; i++)
   {
      printer(stream,"<Point>");
      printer(stream," %f %f ",
              gd->points[i][1],gd->points[i][2]);
      printer(stream,"</Point>");
   }
   printer(stream,"</Holepointlist>\n");
}

} 


static void
emitBolts(Geometrydata * gd, PrintFunc printer, void * stream) {


   int i;
   
   printer(stream,"<Boltlist>\n");
   for(i=1;i<=gd->nFPoints;i++)
   {
      printer(stream,"<Bolt>");
      printer(stream," %f %f %f %f ",
              gd->rockbolts[i][1],gd->rockbolts[i][2],
              gd->rockbolts[i][3],gd->rockbolts[i][4]);
      printer(stream,"</Bolt>");
   }
   printer(stream,"</Boltlist>\n");


}  /* close emitBolts() */


static void 
emitMatlines(Geometrydata * gd, PrintFunc printer, void * stream) {


   int i;
   
   printer(stream,"<Matlinelist>\n");
   for(i=1;i<=gd->nFPoints;i++) {


      printer(stream,"<Line>");
      printer(stream," %f %f %f %f %d",
              gd->matlines[i][1],gd->matlines[i][2],
              gd->matlines[i][3],gd->matlines[i][4], 
              (int)gd->matlines[i][5]);
      printer(stream,"</Line>");
   }
   printer(stream,"</Matlinelist>\n");

}



void
gdata_write_ddaml(Geometrydata * gd, PrintFunc printer, char * outfilename) {


   FILE * outfile;

  /* FIXME: Return an error if this fails. */
   outfile = fopen(outfilename,"w");

  

  /* xml header */
   printer(outfile,"<?xml version=\"1.0\" standalone=\"no\"?>\n");
   printer(outfile,"<!DOCTYPE DDA SYSTEM \"geometry.dtd\">\n");
   printer(outfile,"<Berkeley:DDA xmlns:Berkeley=\"http://www.tsoft.com/~bdoolin/dda\">\n");

  /* FIXME:  This is a bogosity.  The parser code is broken
   * because it requires a comment before the <Geometry>
   * or <Analysis> tags.
   */
   printer(outfile,"<!-- Bogus comment to keep ddaml tree-stripping\n");
   printer(outfile,"from seg faulting on bad child node. -->\n\n");
   printer(outfile,"<Geometry>\n");
   printer(outfile,I1"<Edgenodedist distance=\"%f\"/>\n",gd->e00);

   emitJoints(gd, printer, outfile);
   emitPoints(gd, printer, outfile);


   if (gd->nBolts > 0) 
      emitBolts(gd, printer, outfile);
   if (gd->nMatLines > 0)
      emitMatlines(gd, printer, outfile);


   printer(outfile,"</Geometry>\n");
   printer(outfile,"</Berkeley:DDA>\n");

   fclose(outfile);

}  /* close dumpDDAMLGeometryFile() */


static void
deleteBlock(Geometrydata * gd, int blocknumber)
{
   int i,j,i0,i1,i2;
   //int nPoints = gd->nPoints;
   int psize2 = gd->pointsize2;
   int nBolts = gd->nBolts;
   int nfp = gd->nFPoints;
   int nlp = gd->nLPoints;
   int nmp = gd->nMPoints;
   int ** vindex = gd->vindex;
   double ** vertices = gd->vertices;
   double ** points = gd->points;
   double ** rockbolts = gd->rockbolts;
  


   FILE * ofp;

   ofp = fopen ("point.log","w");


/* Need a print points here */
//printPoints(gd, "deleteblocks", ofp);

   i0=0;  /* block counter  */
   i1=0;  /* node  counter  */
   for (i=1; i<=gd->nBlocks; i++)
   {
     /* Basically, what this says is that if the 
      * block number matches the block number we
      * want to delete, then don't count 
      * that block, ie delete it.
      */
      if (i == blocknumber)
         continue;

     /* If not the particular block number, then 
      * copy forward in the vertex array.  
      */
      i0 += 1;     /* block number+1 */
      i2  = i1+1;  /* start of block */
      for (j=vindex[i][1]; j<= vindex[i][2]+4; j++)
      {
         i1+=1;
         vertices[i1][0]=vertices[j][0];
         vertices[i1][1]=vertices[j][1];
         vertices[i1][2]=vertices[j][2];
      } /* j */
     /* Copy the block material over. */
      vindex[i0][0] = vindex[i][0];
      vindex[i0][1]=i2;
      vindex[i0][2]=i1-4;

   } /* i */

   gd->nBlocks--;


  /* Now we have to reset the points associated with each block.
   * This appears to work pretty much correctly, other than not handling
   * points outside of a block.
   */
   for(i=1;i<=nfp+nmp+nlp;i++)
   {
      for (j=1;j<=gd->nBlocks;j++)
      {
          if(pointinpoly(j,points[i][1],points[i][2],vindex, vertices))
          {
             points[i][3] = j;
             break;
          }
          else
          { 
             points[i][3] = 0;
             //break;
          }
      }
   }


//printPoints(gd, "deleteblocks",ofp);

  /*------------------------------------------------*/
  /* case of having points without carry block      */
  /* FIXME: Remove such a point from the point list */
  /* FIXME: This code is duplicated in dc11().  Turn it 
   * into its own function.
   */
   i0 = 0;  // counts deletions, start with 0 deletions
   for (i= 1; i<= gd->nFPoints+gd->nLPoints+gd->nMPoints+1; i++)
   {
    
      if ( (int)points[i][3] == 0 )
      {
         switch ((int)points[i][0])
         {
            case 0://fixed:
               gd->nFPoints--; 
               break;
            case 2://load: 
               gd->nLPoints--;
               break;
            case 1://measured:
               gd->nMPoints--;
               break;
            default:
               break;
         }
         i0++;  // counts deletions   
         //fprintf(ddacutlog,"fixed load measured point outside all blocks %d \n",i);
         //fprintf(ddacutlog,"%f , %f \n",points[i][1],points[i][2]);
         continue;
      }

      for (j=0;j<psize2;j++)
      {
         points[i-i0][j] = points[i][j];
      }

   } /* i */

/* And another print points here. */
//printPoints(gd, "deleteblocks",ofp);


  /* Next, reset the bolt endpoints... 
   * FIXME: Bolt endpoints outside of blocks are not handled.
   * The easiest thing to do about such a case is to throw
   * an exception.
   */
  /* WARNING!!!  Indexed from 0, not 1 as usual */
   for(i=0;i<nBolts;i++)
   {
      for (j=1;j<=gd->nBlocks;j++)
      {
          if(pointinpoly(j,rockbolts[i][1],rockbolts[i][2],vindex, vertices))
             rockbolts[i][5] = j;
          if(pointinpoly(j,rockbolts[i][3],rockbolts[i][4],vindex, vertices))
             rockbolts[i][6] = j;
      }
   }


   fclose(ofp);

}  /* close deleteBlock() */


/* The simplex area function depends on having the vertex coordinates
 * in a certain order so that the coordinates can be traversed in 
 * a CCW direction.  vertexInit performs this reordering at the outset,
 * for all subsequent calls to area.  Since the area function is 
 * called everytime step, initializing ahead of time saves a lot of
 * overhead.
 */
/**
 * @todo  Change the algorithm to use modular arithmetic so 
 * this loop does not have to be computed.
 */
void
gdata_vertex_init(Geometrydata *gd) {

  /* change vertices to have previous vertex of 1st vertex */
  /* This is a convenience for the simplex integration
   * in the area function.
   */
   int i, i1, i2;

   for (i=1; i<= gd->nBlocks; i++)
   {
      i1 = gd->vindex[i][1];
      i2 = gd->vindex[i][2];
      gd->vertices[i1-1][0]=gd->vertices[i2][0];
      gd->vertices[i1-1][1]=gd->vertices[i2][1];
      gd->vertices[i1-1][2]=gd->vertices[i2][2]; 
   }  /*  i  */

}


/* Code to specifically free the geometry data structure 
 * passed in to the analysis procedure.  This code should 
 * be changed to take advantage of 2D matrix freeing code.
 */
void
gdata_delete(Geometrydata * gd) {

   if (gd == NULL)
      return;

   /* FIXME: At some place in the code, there is a memory overwrite
    * on block masses that is producing an out of bounds error when
    * attempting to free the memory.
    */
   //freeBlockMasses(gd);

   if (gd->vertices)
      free2DMat((void **)gd->vertices, gd->vertexsize1);
   if (gd->origvertices)
      free2DMat((void **)gd->origvertices, gd->vertexsize1);
   if (gd->vindex)
      free2DMat((void **)gd->vindex, gd->vindexsize1);
   if (gd->joints)
      free2DMat((void **)gd->joints, gd->jointsize1);
   if (gd->points)
      free2DMat((void **)gd->points, gd->pointsize1);
   if (gd->prevpoints)
      free2DMat((void **)gd->prevpoints, gd->pointsize1);
   if (gd->origpoints)
      free2DMat((void **)gd->origpoints, gd->pointsize1);
   if (gd->matlines)
      free2DMat((void **)gd->matlines, gd->matlinesize1);


   /** @todo Change these */
   if (gd->nBolts > 0) {


      free2DMat((void **)gd->rockbolts, gd->rockboltsize1);
      free2DMat((void **)gd->origbolts, gd->rockboltsize1);
   }


// Wasn't freeing the moments...

/*
   if (gd->moments) {

      free2DMat((void **)gd->moments, gd->momentsize1);

   }
*/


   if (gd->porepres)
      free2DMat((void **)gd->porepres, gd->porepressize1);
   if (gd->watertable)
      free2DMat((void **)gd->watertable, gd->wtablesize1);
   if (gd->nn0)
      free(gd->nn0);


  /* Shred it! */
   memset((void*)gd,0xdd,sizeof(Geometrydata));
   free (gd);
   gd = NULL;

   return;
}  



void
gdata_read_input_file(Geometrydata * geomdata, char * geomfile ) {

   IFT gfv;
 
   assert(geomfile != NULL);

   gfv = ddafile_get_type(geomfile);

   switch(gfv) {

       case ddaml:
          ddaml_read_geometry_file(geomdata,geomfile);
          break;

       case extended: 
          dda_display_warning("Extended geometry file format obsolete");
          exit(0);
          //geomdata = geometryReader2(geomfile);
          //if (!geomdata) 
          //{
          //   iface->displaymessage("Error in geometry file" /* Error */);
          //   return NULL;
          //}
          break;
       case original:
       default:
          dda_display_warning("Obsolete geometry file detected");
          geometryReader1(geomdata,geomfile);  
          break;
   }
}  




/** FIXME: This needs to be set as an object method.
 */
Geometrydata *
gdata_clone(Geometrydata * gdn) {

   int i;
   /* There should be a way to initialize this
    * in the type declaration and pass back the
    * address, but it does not seem to work on the 
    * MS compiler, so initialize manually.  :(
    */

   Geometrydata * gdo;

   gdo = gdata_new();

   memcpy((void*)gdo,(void*)gdn,sizeof(Geometrydata));


   //gdo->getblocknumber = gdn->getblocknumber;
   
  /* scale and w0 initialized in dc02  */
   for (i=0;i<4;i++)
      gdo->scale[i] = gdn->scale[i];

   if (gdn->points)
      gdo->points = clone2DMatDoub(gdn->points,gdn->pointsize1,gdn->pointsize2);

   if (gdn->prevpoints)
      gdo->prevpoints = clone2DMatDoub(gdn->prevpoints,gdn->pointsize1,gdn->pointsize2);
   else  /* copy the points over */
      gdo->prevpoints = clone2DMatDoub(gdn->points,gdn->pointsize1,gdn->pointsize2);

   if (gdn->origpoints)
      gdo->origpoints = clone2DMatDoub(gdn->origpoints,gdn->pointsize1,gdn->pointsize2);
   else  /* copy the points over */
      gdo->origpoints = clone2DMatDoub(gdn->points,gdn->pointsize1,gdn->pointsize2);
   if (gdn->joints)
      gdo->joints = clone2DMatDoub(gdn->joints,gdn->jointsize1,gdn->jointsize2);
   if (gdn->matlines)
      gdo->matlines = clone2DMatDoub(gdn->matlines,gdn->matlinesize1,gdn->matlinesize2);

   if (gdn->nBolts > 0)
   {
      gdo->rockboltsize1 = gdn->rockboltsize1;
      gdo->rockboltsize2 = gdn->rockboltsize2;
      if(gdn->rockbolts)
         gdo->rockbolts = clone2DMatDoub(gdn->rockbolts,gdn->rockboltsize1,gdn->rockboltsize2);
      if(gdn->origbolts)
         gdo->origbolts = clone2DMatDoub(gdn->origbolts,gdn->rockboltsize1,gdn->rockboltsize2);
      else  /* copy the points over */
         gdo->origbolts = clone2DMatDoub(gdn->rockbolts,gdn->pointsize1,gdn->pointsize2);
   }

   if (gdn->vertices)
      gdo->vertices = clone2DMatDoub(gdn->vertices,gdn->vertexsize1,gdn->vertexsize2);
   if (gdn->origvertices)
      gdo->origvertices = clone2DMatDoub(gdn->origvertices,gdn->vertexsize1,gdn->vertexsize2);
   else
      gdo->origvertices = clone2DMatDoub(gdn->vertices,gdn->vertexsize1,gdn->vertexsize2);

   if (gdn->vindex)
      gdo->vindex = clone2DMatInt(gdn->vindex,gdn->vindexsize1,gdn->vindexsize2);

  /* WARNING!!!  nn0 does not get cloned from the geometry data passed
   * in to an analysis.  It is computed later.
   */
   if(gdn->nn0)
      gdo->nn0 = clone1DMatInt(gdn->nn0, gdn->nn0size);

  /* Vertices for the pore pressure polygon. */
   if (gdn->porepres)
      gdo->porepres = clone2DMatDoub(gdn->porepres,gdn->porepressize1,gdn->porepressize2);
   else
      gdo->porepres = NULL;

  /* Points for water table surface.  */
   if (gdn->watertable)
      gdo->watertable = clone2DMatDoub(gdn->watertable,gdn->wtablesize1,gdn->wtablesize2);
   else 
      gdo->watertable = NULL;

   /* These may or may not be useful. */
   if (gdn->moments)
      gdo->moments = clone2DMatDoub(gdn->moments,gdn->momentsize1,gdn->momentsize2);
   else
      gdo->moments = NULL;

   //cloneBlockMasses(gdn,gdo);

   return gdo;

} 




/**************************************************/
/* dc02: draw lines and points             */
/**************************************************/
/*  dc02() really just sets the scale of the drawing area,
 * as near as I can tell right now. 
 * FIXME: This is almost the same function as df01().  Get rid of 
 * df01() if possible.
 */
//void dc02(Geometrydata *gd)
/* FIXME: Combine this with df01() to make one function out
 * these 2 very similar functions.  In fact, they are almost 
 * identical.  And both are the same as computeBoundingBox,
 [which is deleted]
 * so there are 3 [2] functions doing the same task here.
 * This function is called from geomdriver.  The way to do this
 * is to call the bounding box function from here, then set
 * the scale from the results of the bounding box computation.
 */
void computeDomainscale(Geometrydata *gd)

{
   int i;
   int j;
   double w0;
   double ** joints = gd->joints;

  /* The variables named w1-w4 need to renamed min/max x/y.  They are for 
   * determining the scale of the problem.
   */
   //double  w1;  /* min x */
   //double  w2;  /* max x */
   //double  w3;  /* min y */
   //double  w4;  /* max y */
   //double  w7;
   //double  w8;
   double minx, maxx;
   double miny, maxy;
   double height, width;

  /* compute min x  max x  min y  max y             
   */
   minx = joints[1][1];
   maxx = joints[1][1];
   miny = joints[1][2];
   maxy = joints[1][2];

   
   for (i=1; i<= gd->nJoints; i++)
   {
      for (j=2; j<= 4; j+=2)
      {
         if ( minx > joints[i][j-1] )
            minx = joints[i][j-1];       // w1 = min x

         if ( maxx < joints[i][j-1] )
            maxx = joints[i][j-1];       // w2 = max x

         if ( miny > joints[i][j  ] )
            miny = joints[i][j];         // w3 = min y

         if ( maxy < joints[i][j  ] )
            maxy = joints[i][j];         // w4 = max y
      } 
   } 
      
    gd->scale[0] = minx;
    gd->scale[1] = maxx;
    gd->scale[2] = miny;
    gd->scale[3] = maxy;

  /* form window for screen and ps */
  /* Variables w5-w8 probably don't need
   * to be in here either.
   */
   //w5=(w1+w2)/2;
   //w6=(w3+w4)/2;
   width=(maxx-minx)/2;
   height=(maxy-miny)/2;
   w0=height;

  /* This is a very peculiar construction.  
   * Eliminating this goto might have a nasty 
   * side effect on the value of w0.  Leave it 
   * in for now.  It has something to do with the 
   * aspect ratio of the problem.
   */
   //if ( w7 <= (1.3*w0) ) 
      //goto a205;
   if ( width > (1.3*w0) ) 
      w0=width/1.3;

//a205:

  /* FIXME: Get rid of the 1.02 factor. */
   w0=w0*1.02;
   gd->w0 = w0;

}  /* Close dc02().  */

  
int 
gdata_get_block_number(Geometrydata * gd, double x, double y) {

  int i;
  int flag;

  for (i=1; i<=gd->nBlocks; i++) {

      flag = pointinpoly(i, x, y, gd->vindex, gd->vertices);

      if (flag == 1) {
         return i;
      }
  }

  return 0;
}


int
gdata_get_number_of_blocks(Geometrydata * gd) {
   return gd->nBlocks;
}




/* The Geometrydata struct is built in the 
 * geometry code and contains blocks, points
 * vertices needed by the analysis code to 
 * run a numerical computation.  One of these 
 * should be declared in the gui or command 
 * line wrapper that drives an analysis.
 */
Geometrydata *
gdata_new(void) {

   Geometrydata * gdo;

  /* Change to a malloc, memset everything to garbage */
   gdo = (Geometrydata *)calloc(1,sizeof(Geometrydata));


   gdo->seispoints = NULL;


   gdo->deleteblock = deleteBlock;
   gdo->dumptofile = gdata_write_ddaml;

   return gdo;

} 


/** @todo Write a unit test for this then replace the 
 * inner loop in computeMoments() with this function.
 */
/** This function computes all the relevant moments for
 * a single block.
 */
static void
moments_compute(double * moments, double ** vertices, int * vindex) {

   int i;
   double f1,x2,x3,y2,y3;

   for (i=1; i<=8; i++) {
      moments[i] = 0;
   }    

   for (i=vindex[1]; i<=vindex[2]; i++) {

      x2 = vertices[i][1];
      y2 = vertices[i][2];
      x3 = vertices[i+1][1];
      y3 = vertices[i+1][2];

      f1 = (x2*y3-x3*y2);
      moments[1] += f1/2;

     /* Evidently, the 0 represents the value of
      * x1 and y1.
      */
      moments[2] += f1*(0+x2+x3)/6;
      moments[3] += f1*(0+y2+y3)/6;
      moments[4] += f1*(x2*x2+x3*x3+x2*x3)/12;
      moments[5] += f1*(y2*y2+y3*y3+y2*y3)/12;
      moments[6] += f1*(2*x2*y2+2*x3*y3+x2*y3+x3*y2)/24;
   }

  /* Compute current centroids. */   
   moments[7] = moments[2]/moments[1];
   moments[8] = moments[3]/moments[1];
}



/**************************************************/
/* area: compute s0 sx sy sxx syy sxy        0010 */
/**************************************************/
/* Looks like **vertices and **vindex going in and returning **moments.
 * This is also called from df25.
 */
/* i  j  x2  y2  x3  y3  f1  moments[][] */
/* FIXME: This needs to be written to call a function 
 * that takes a block number.
 */
double 
gdata_compute_moments(Geometrydata * gd) {

  /* FIXME: Need this to track the block 
   * centroids.
   */

   int j;
   int nBlocks = gd->nBlocks;
   double ** vertices = gd->vertices;
   int ** vindex = gd->vindex;
   double ** moments = gd->moments;
   double avgArea;

   int block;
   int vertex;
   double x2, y2, x3, y3, f1;



   for (block=1; block<=nBlocks; block++) {

     /** @todo replace the following inner loops with a function call
      * which is much easier to test.  But don't replace it until it
      * is tested!
      */     
      //moments_compute(moments[block], vertices, vindex[block]);

     /* This loop to zero the moments matrix may or may not be 
      * necessary.  Leave it in for now, as it is not the 
      * problem affecting the areas.  This could also be moved
      * outside the loop and zeroed with the subroutine call.
      */
     /* @todo move this into the function above. */
      for (j=1; j<=8; j++) {
         moments[block][j] = 0;
      }       
      

      for (vertex=vindex[block][1]; vertex<=vindex[block][2]; vertex++) {

         x2 = vertices[vertex][1];
         y2 = vertices[vertex][2];
         x3 = vertices[vertex+1][1];
         y3 = vertices[vertex+1][2];
      
         f1 = (x2*y3-x3*y2);
         moments[block][1] += f1/2;
        /* Evidently, the 0 represents the value of
         * x1 and y1.
         */
         moments[block][2] += f1*(0+x2+x3)/6;
         moments[block][3] += f1*(0+y2+y3)/6;
         moments[block][4] += f1*(x2*x2+x3*x3+x2*x3)/12;
         moments[block][5] += f1*(y2*y2+y3*y3+y2*y3)/12;
         moments[block][6] += f1*(2*x2*y2+2*x3*y3+x2*y3+x3*y2)/24;
      }



     /* Compute current centroids. */   
      moments[block][7] = moments[block][2]/moments[block][1];
      moments[block][8] = moments[block][3]/moments[block][1];

      if (moments[block][1] <= 0) {
         char mess[80];
         sprintf(mess,"Block %d has negative area, probably divide by zero error",block);
         gd->display_error(mess);
      }
   }

	avgArea=0;
   for (block=1; block<= nBlocks; block++) {
	   avgArea += moments[block][1];
   }
      
   avgArea  = avgArea/nBlocks;
   
   return avgArea;
}  



void 
gdata_get_centroid(double * moments, double * x0, double * y0) {

  *x0 = moments[7];   
  *y0 = moments[8];
}


double
gdata_get_block_area(Geometrydata * gd, int block) {

   double ** moments = gd->moments;

  /* Need a function that will just compute moments for a single block. */
   gdata_compute_moments(gd);

   return moments[block][1]; 

}


void
gdata_rockbolt_init(Geometrydata * gd, int numbolts) {

   gd->maxSegmentsPerBolt = 24;  // hardwired
   gd->nBolts = numbolts;
   
   // mmm: make the array extra huge for potential segments
   // similar to what's done for the fixed lines (even if it's bad coding)
   gd->rockboltsize1 = gd->maxSegmentsPerBolt * numbolts+1;

   gd->rockboltsize2 = 17;
   gd->rockbolts = DoubMat2DGetMem(gd->rockboltsize1, gd->rockboltsize2);
}
