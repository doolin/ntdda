/* 
 * blockreader.c
 *
 * The data contained in the file "block.in" is read in
 * this subroutine.  No error checking is performed.
 * The "block.in" file is written by dc19.  This piece of code
 * will be maintained for third party purposes, but the actual
 * blocks will be passed in a data structure returned from the 
 * geometry routine in the DDA for Windows code.
 *
 * $Author: doolin $
 * $Date: 2001/05/20 20:58:45 $
 * $Source: /cvsroot/dda/ntdda/src/blockreader.c,v $
 * $Revision: 1.1 $
 */
#include "compilecontrol.h"
#include"analysis.h"
#include<stdlib.h>
#include<assert.h>

/* FIXME: this function for reading block data from a file
 * will have to be completely rewritten.
 */
Geometrydata * 
blockreader(FILEPATHS *filepath)
{
   FILE * blocksInFile;
   Geometrydata * bdn;  /*  i.e., block data in */
   int ** vIndex = NULL;
   double ** points = NULL;
   double ** vertices = NULL;

  /**************************************************/
  /* nBlocks   number of blocks                     */
  /* nBolts   number of bolts                       */
  /* vertexCount   number of total vertices of d    */
  /*------------------------------------------------*/
  /* nFPoints   fixed points                        */
  /* nLPoints   loading points                      */
  /* nMPoints   measured points                     */
  /* pointCount   total number of points (?)        */
  /**************************************************/
   //int nBlocks, nBolts, vertexCount, nFPoints, nLPoints, nMPoints, pointCount;
   int nPoints;
   //int nBolts;
   int n7, n8, i;

   double ** rockBolts;  /* was hb  */

  /* this function needs to be rewritten!!! */
   return NULL;

   //bdn = (BLOCKDATA *)malloc(sizeof(BLOCKDATA));

  /* We make the dangerous assumption that the 
   * blockfile has already been initialized in the
   * dc19 function of the geometry part.
   */
   blocksInFile = fopen (filepath->blockfile,"r"); 
   

#if DDA_FOR_WINDOWS
#else
   printf("Blockfile name (blockreader):  %s\n", filepath->blockfile);
#endif

  /* These next comments help to document the format 
   * of the block file, and are intentionally redundant 
   * because the data is not parsed in any way. Also,
   * Since there is no documentation for this format,
   * and no comments are allowed in the file, it gets 
   * very tiring to keep relearning the same crap.
   * The data is simply read in and assumed to be correct.
   */
  /* Read in line 1 of the block file.
   * number of blocks,  number of bolts,  number of vertices.
   */
   fscanf(blocksInFile,"%d %d %d",&bdn->nBlocks,&bdn->nBolts,&bdn->vertexCount);
  /* Read in the second line of the file.
   * number of fixed points, number of loading points, number of measured points
   */
   fscanf(blocksInFile,"%d %d %d",&bdn->nFPoints,&bdn->nLPoints,&bdn->nMPoints);
   
   assert(bdn->nBlocks > 0);
   assert(bdn->nBolts == 0);

  /* g : x y n u v of fixed measured load points    */
  /* g[pointCount+1][6]                                     */
 	(bdn->pointCount) = (bdn->nFPoints) + (bdn->nLPoints) + (bdn->nMPoints);
 	nPoints = bdn->pointCount;
   n7 = bdn->pointCount + 1;
   n8 = 6;
   points = DoubMat2DGetMem(n7, n8);

  /*------------------------------------------------*/
  /* k0: block index of d     0 material number     */
  /* k0: 1 block start        2 block end           */
  /* k0[nBlocks+1][3]                                    */
   n7=bdn->nBlocks+1;
   n8=3;
   vIndex = IntMat2DGetMem(n7, n8);

  
  /*------------------------------------------------*/
  /* d : 0 number of joint material of edge i i+1   */
  /* d : 1 x  2 y of coordinates of vertices        */
  /* d[vertexCount+1][3]                                     */
   n7=bdn->vertexCount+1;
   n8=3;
   vertices = DoubMat2DGetMem(n7, n8);


  /* rockBolts : rock bolt matrix.                  */
  /* rockBolts : x1 y1 x2 y2 n1 n2 e0 t0 f0 of bolt. */
  /* n1, n2 carry (?) block number (i.e., are the 
   * block numbers of start and stop of bolt?)  
   */
  /* rockBolts[nBolts+1][10]  */
   rockBolts = DoubMat2DGetMem(bdn->nBolts+1, 10);


  /* nBlocks   number of blocks                          */
  /* nBolts   number of bolts                           */
  /* vertexCount   number of total vertices of d             */
  /* nFPoints   fixed points                              */
  /* nLPoints   loading points                            */
  /* nMPoints   measured points                           */
  /* material number  block start  block end  index */
  /* The way this seems to work is that each set of block
   * vertices is stored in "slots" of the d matrix.  The
   * following input reads in the material number into the
   * the first position of each slot, followed by the index
   * of the starting vertex, followed by the index of the 
   * last unique vertex associated with that block.  The 
   * "block.in" file repeats the starting index after this,
   * in position 'block end' +1.  The reason for this is 
   * explained below.  The block vertices are stored in 
   * CCW order.
   */
 		for (i=1; i<= bdn->nBlocks; i++)
   {
      fscanf(blocksInFile,"%d %d %d",
         &vIndex[i][0], &vIndex[i][1], &vIndex[i][2]);
   }  /*  i  */
   
 
  /* 0:joint material   1:x   2:y  of block vertices */
  /* The "number of vertices" is stored in vertexCount, but note that 
   * this number is more than the actual number of vertices.
   * Each block has one repeated vertex, to "close the loop".
   * The block vertices are stored in CCW order.  Note that 
   * each set of block vertices indexed in k0 is followed
   * by a repeat of the first, the three more vertices whose
   * purpose is not known right now.
   */
	 	for (i=1; i<= bdn->vertexCount; i++)
   {
      fscanf(blocksInFile,"%lf %lf %lf",
         &vertices[i][0],&vertices[i][1],&vertices[i][2]);
   }  /*  i  */
   
  /* h: x1  y1  x2  y2  nBlocks  n2  e0  t0  f0  of bolt */
  /* nBlocks n2 carry block number        f0 pre-tension */
  /* Bolts are indexed from zero.  This will need to be changed
   * to index from 1 to make this stuff actually work.
   */
   if (bdn->nBolts > 0)
   {
      for (i=0; i< bdn->nBolts; i++)
      {
         fscanf(blocksInFile,"%lf %lf %lf",&rockBolts[i][1],&rockBolts[i][2],&rockBolts[i][3]);
         fscanf(blocksInFile,"%lf %lf %lf",&rockBolts[i][4],&rockBolts[i][5],&rockBolts[i][6]);
         fscanf(blocksInFile,"%lf %lf %lf",&rockBolts[i][7],&rockBolts[i][8],&rockBolts[i][9]);
      }  /*  i  */
   }

  /* x  y  n  of fixed measured loading points      */
	 	for (i=1; i<= bdn->pointCount; i++)
   {
		    fscanf(blocksInFile,"%lf %lf %lf %lf",
         &points[i][0],&points[i][1],
         &points[i][2],&points[i][3]);
   }  /*  i  */

   fclose(blocksInFile);

   bdn->points = points;
   bdn->vertices = vertices;
   bdn->vindex = vIndex;

   return bdn;

}  /* Close blockreader()  */
