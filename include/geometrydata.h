
/* There should be no windows dependencies in this file. */
#ifndef __GEOMETRYDATA_H__
#define __GEOMETRYDATA_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#ifdef WIN32
#pragma warning( disable : 4115 )        
#endif

#include "ddadlist.h"
#include "ddatypes.h"
#include "graphics.h"
#include "ddafile.h"




typedef struct _geo_data_tag Geometrydata;

/* The Geometrydata struct is built in the 
 * geometry code and contains blocks, points
 * vertices needed by the analysis code to 
 * run a numerical computation.  One of these 
 * should be declared in the gui or command 
 * line wrapper that drives an analysis.
 */
struct _geo_data_tag {

   DisplayFunc display_warning;
   DisplayFunc display_error;

  /* These will eventually get moved out to a more
   * appropriate structure.  In the original GHS code,
   * the array indices are started from 1 instead of
   * zero.  This is the same in both the geometry struct
   * and the analysis struct.
   */
   int index;   //  1 in GHS code.
   int blocksize;  //  6 in GHS code, 3 in DDAD

  /* Initialized in the geometry reader code. */
   double e00;  /* minimum edge-node distance */
   int nJoints;   
   int nBoundaryLines;     
   int nMatLines;
   int nBolts;
   int nFPoints;
   int nLPoints;
   int nMPoints;
   int nHPoints; 
   int nSPoints;  // Seismic loading points
  /* FIXME: Where is nPoints set and does it include
   * the hole points.
   */
   int nPoints;
   int nPorePres;
   int nWtable;
   int nIntersectionPoints;
   int nContacts;
  /* pointCount is a "compatibility" value.  
   * Eliminate this as soon as possible.  This 
   * probably corresponds to the `np' variable 
   * in GHS code.
   */
   int pointCount;
 
  /* nBlocks initialized where dc08 */
   int nBlocks;
  /* Initialized in geometryToReturn(). */
   int vertexCount;  /*  The `oo' variable in dc19 */
  /* Not sure what its for... */
   int n2;
  /* n3 initialized in dc05 */
   int n3;  
  /* scale and w0 initialized in dc02  */   
   // w1;  /* min x */
   // w2;  /* max x */
   // w3;  /* min y */
   // w4;  /* max y */
   double scale[4];
   double w0;  /* scale parameter */
  /* And this is the way it should be done... */
   DDARect boundbox;
  /* FIXME: What is this? */
   //int nt;

  /* Handy values for malloc, free and loop counting
   * over arrays.  This will also help when the code is 
   * ported to c++.
   */
   int pointsize1;
   int pointsize2;
   double ** points;
   double ** prevpoints;
   double ** origpoints;

  /* Try to separate out the fixed from the loading points.
   */
   DList * fpoints;
   DList * lpoints;
   DList * mpoints;
   DList * hpoints;
   DList * dirpoints;
   DList * dispoints;
   DList * seispoints;

   int jointsize1;
   int jointsize2;
   double ** joints;

   int matlinesize1;
   int matlinesize2;
   double ** matlines;

  /*------------------------------------------------*/
  /* h: x1  y1  x2  y2  n1  n2  e0  t0  f0  of bolt */
	 /* n1 n2 carry block number        f0 pre-tension */
  /* hb[nBolts+1][10]                                    */
  //double **hb;
   int rockboltsize1;
   int rockboltsize2;
   double ** rockbolts;
   double ** origbolts;

  /* Copied from d in geometryToReturn(). */
   int vertexsize1;
   int vertexsize2;
   double ** vertices;
   double ** origvertices;

  /* Copied from k0 in geometryToReturn(). */
   int vindexsize1;
   int vindexsize2;
   int ** vindex;

  /**************************************************/
  /* nn0:   block number of block vertices in vertices[]   */
   int nn0size;
   int * nn0;

   int porepressize1;
   int porepressize2;
  /* Vertices for the pore pressure polygon. */
   double **porepres; 

   int wtablesize1;
   int wtablesize2;
  /* Points for water table surface. */
   double **watertable;  


   /****************************************************/
  /* FIXME: The next several variables need to go into a 
   * block structure.
   */
  /* These may or may not be useful. */
   int momentsize1;
   int momentsize2;
   double ** moments;

  /* Need to keep original areas to conserve mass.  
   * See TCK p. 213.
   */
   int origareasize;  // num blocks + 1
   double * origarea;

  /* Need to conserve mass:  TCK p. 213 */
   int origdensitysize;  // numblocks + 1
   double * origdensity; 

  /* For conserving mass.. */
   int masssize;
   double * mass;
   double * origmass;

  /*********************************************************/

   void (*computeBBox)(Geometrydata * this);
  /* FIXME: Add zbuffering to this later. 
   * Function returns block number where cursor 
   * is located.  Units are physical, so need to 
   * translate device->logical->physical.
   */
   //int (*getblocknumber)(Geometrydata * this, double x, double y);
   
  /* Add comments and code to initialize and colne geometry structs */
   void (*deleteblock)(Geometrydata * this, int blocknum);
   void (*deletepoint)(Geometrydata * this, int pointnum);
  /* Use this to easily control which output function gets
   * called.
   */
   void (*dumptofile)(Geometrydata *, char *);

};  /* End of struct geometry data definition */



/* These two input parsers need to be static in some 
 * driver function.
 */
void geometryReader1(Geometrydata *,char *);
//Geometrydata * geometryReader2(char *);


Geometrydata * XMLparseDDA_Geometry_File(char *filename);

//Geometrydata * cloneGeometrydata(Geometrydata *);
//int freeGeometrydata(Geometrydata *);

void ddacut(Geometrydata *);

void initBlockMasses(Geometrydata *);
void cloneBlockMasses(Geometrydata *, Geometrydata *);
void freeBlockMasses(Geometrydata *);

//Geometrydata * initGeometrydata(void);


void dumpGeometrydata(Geometrydata *, void *);


/* Moved from analysisdata.h */
double computeMoments(Geometrydata *);  //, double ** moments);

/* Moved from dc02 in blocks.c.  Duplicates df01 */
/* FIXME: Move the bounding box code out of here, then 
 * rewrite this function to handle scaling from the 
 * already computed bounding box.  This will allow the 
 * domain bounding box to be computed in only one place,
 * and will even allow for a unit test!  w00h00!
 */
void computeDomainscale(Geometrydata *);  // was dc02()


/* All the new code added will follow the Peter Mattis
 * convention for syntactic structure.  It's about the 
 * most readable way to write c code.
 */
Geometrydata * gdata_new                   (void);
Geometrydata * gdata_clone                 (Geometrydata *);

void           gdata_delete                (Geometrydata *);

int            gdata_get_number_of_blocks  (Geometrydata *);

int            gdata_get_block_number      (Geometrydata *, 
                                            double x, 
                                            double y);

void           gdata_get_block_centroid    (Geometrydata * gd, 
                                            int block, 
                                            double [2]);

double         gdata_get_block_area        (Geometrydata * gd, 
                                            int block);

void           gdata_rockbolt_init         (Geometrydata * gd, 
                                            int numbolts);
/**
 * gdata_read_input_file wraps all of the file format ugliness
 * behind a single call.  If there is a recoverable problem,
 * the Geometrydata * will come back NULL, and the memory freed
 * internally somewhere.  
 *
 * @param Geometrydata * points to a preallocated struct.
 * @param void * filestream needs to be cast in the actual
 *        file reading function to a FILE *.
 *
 * @return NULL Geometrydata * (with freed memory).
 */
void           gdata_read_input_file       (Geometrydata *, 
                                            char * filename);


#ifdef __cplusplus
}
#endif


#endif /* __GEOMETRYDATA_H__ */