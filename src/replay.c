#include "dda.h"
#include "winmain.h"
#include <stdio.h>
#include "ddamemory.h"
#include <assert.h>
#include <math.h>
#include "resource.h"
#include "replay.h"


extern int showOrig;

extern FILEPOINTERS fp;
extern FILEPATHS filepath;



int
handleResultsViewReplay(HWND hwMain, LPARAM lParam, GRAPHICS * g)
{
   OPENFILENAME ofn;

   LPCSTR szFilter[] = {"Replay files (*.replay)\0*.replay\0All files (*.*)\0*.*\0\0"};

   dda_display_info(filepath.vfile);

   fileBrowse(hwMain, &ofn, szFilter, filepath.vpath, filepath.vfile, "replay");
                   
                    
         
                   
                        
       if( !GetOpenFileName(&ofn) ) 
       {
          strcpy(filepath.vpath, filepath.oldpath);
          return 0;  // user pressed cancel.
       }    
       else // Replay something.
       {
         /* Set up to call the replay function with an arbitrary
          * replay file.  It is not currently known whether 
          * the replay file has enough information to do the 
          * replay without first invoking the appropriate file 
          * containing the geometry information.  Eliminate all
          * this if possible.
          */
          strcpy(filepath.replayfile, ofn.lpstrFileTitle);
          strcpy(filepath.rootname, strtok(ofn.lpstrFileTitle, "."));
          strcpy(filepath.gfile, filepath.rootname);
          strcat(filepath.gfile, ".geo");

          dda_display_info(filepath.gfile);

          SendMessage(hwMain, WM_COMMAND, GEOM_APPLY, lParam);

          replay_analysis(hwMain, g, filepath.replayfile);

       }  /* Close if-else  */

  /* FIXME: Check the return value */
   return 0;

}  /*  close handleResultsViewReplay() */

/* There will probably be a few other bits of info 
 * that will written here.  Good place to write 
 * file info, header comments, etc.
 */
void 
initReplayFile(Geometrydata * gd, Analysisdata * ad)
{
   int i;
   int numblocks = gd->nBlocks;
   int oo;
   int ** k0 = gd->vindex;
   int numstepssaved;

  /* FIXME: This crashes the xml analysis input files! */
   numstepssaved = (int)floor(ad->nTimeSteps/ad->tsSaveInterval);

  /* This function can just write out the header data. */

  /* n0   number of blocks   nBolt number of bolts   */
  /* nFPoints fixed points       nLPoints loading points    */
  /* nMPoints measured points                           */
   oo = k0[numblocks][2]+4;
  /* Writes to "filename.replay" */
   fprintf(fp.replayfile,"%d %d %d %d \n",  numstepssaved, numblocks,gd->nBolts,  oo);
   fprintf(fp.replayfile,"%d %d %d \n", gd->nFPoints,gd->nLPoints,gd->nMPoints);


  /* material number  block start  block end  index */
  /* The first (0th) array element should be the block 
   * material number.  I don't believe this is currently
   * implemented.  Also, wherever this is determined, there
   * needs to be some argument checking.
   */
   for (i=0; i<= numblocks; i++)
   {
      fprintf(fp.replayfile,"%d %d %d \n", k0[i][0], k0[i][1], k0[i][2]);
   } /* i */

}  /* close initReplayFile() */



/* Do not write any header data!  Just vertices, 
 * bolts and points.  Should also write time step
 * number, delta t and elapsed time.
 */
void
writeReplayFile(Geometrydata * gd, Analysisdata * ad)
{
   int i;
   double ** d = gd->vertices;
   int ** k0 = gd->vindex;
   int numblocks = gd->nBlocks;
   double ** gg1 = gd->rockbolts;
   double ** g = gd->points;

  /* Write the time step, delta t and elapsed time */
   //fprintf(fp.replayfile,"%d %f %f\n",ad->currTimeStep,ad->currentTime,ad->delta_t);

  /* x  y of each block  0  s0  sx  sy  0  0        */
   for (i=0; i<= k0[numblocks][2]+4; i++)
   {
      fprintf(fp.replayfile,"%f %f %f\n",d[i][0],d[i][1],d[i][2]);
   } /* i */


  /* gg1: x1  y1  x2  y2  n1  n2  e00  t0  f0    bolt */
  /* n1 n2 carry block number        f0 pre-tension */
  /* This has to be indexed from 0!!!!
   */
   for (i=0; i< gd->nBolts; i++)
   {
     /* Write to the "rootname.blk" file. */
      fprintf(fp.replayfile, "%f %f %f ", gg1[i][1],gg1[i][2],gg1[i][3]);
		    fprintf(fp.replayfile, "%f %d  %d ", gg1[i][4],(int)gg1[i][5],(int)gg1[i][6]);
      fprintf(fp.replayfile, "%f %f %f\n", gg1[i][7],gg1[i][8],gg1[i][9]);
   }   /* i */

      
  /* x  y  of fixed loading measured points         */
   for (i=0; i<= gd->nFPoints+gd->nLPoints+gd->nMPoints; i++)
   {
    		fprintf(fp.replayfile,"%f %f %f %f \n", 
              g[i][0],g[i][1],g[i][2],g[i][3]);
   } /* i */
}  /* writeReplayFile() */



/* FIXME: Need to catch a running analysis error and not replay
 * while an analysis is running.
 */
void 
replay_analysis(HWND hwMain, GRAPHICS * g, char *replayfilename) {

   FILE * rpfile;
   int i,j;
   int nsteps_saved;
   int ** vindex;
   double ** vertices;
   double ** points;
   double ** rockbolts;
   Geometrydata * gd;
   DDA * dda;

   dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);

   gd = gdata_new();
   gdata_read_input_file(gd,filepath.gfile);

   if (gd == NULL) {
      dda_display_error("NULL geometry");
   }

   dda_set_geometrydata(dda,gd);
   ddacut(gd);

	InvalidateRect(hwMain, NULL, TRUE);
   UpdateWindow(hwMain);
  
  /* Open the replay file */
   rpfile = fopen(replayfilename,"r");

  /* Handle this as an error. Shouldn't even get here
   * because the menus should be disabled.  The error
   * should be something like "no replay file specified."
   * Also should lock the replay file until the replay
   * file stream is closed.
   */
   assert(rpfile != NULL);

  /* Read the first few lines to get the number of 
   * points, blocks, time steps, etc.
   */
   fscanf(rpfile,"%d %d %d %d",&nsteps_saved, &gd->nBlocks,&gd->nBolts,&gd->vertexCount);
  /* Read in the second line of the file.
   * number of fixed points, number of loading points, number of measured points
   */
   fscanf(rpfile,"%d %d %d",&gd->nFPoints,&gd->nLPoints,&gd->nMPoints);

  /* malloc enough memory to read in all the data. */
   gd->pointCount = gd->nFPoints + gd->nMPoints + gd->nLPoints;
   gd->pointsize1 = gd->pointCount + 1;
   gd->pointsize2 = 6;
   points = DoubMat2DGetMem(gd->pointsize1,gd->pointsize2);
   gd->points = points;

   gd->vindexsize1 = gd->nBlocks + 1;
   gd->vindexsize2 = 3;
   vindex = IntMat2DGetMem(gd->vindexsize1,gd->vindexsize2);
   gd->vindex = vindex;

   gd->vertexsize1 = gd->vertexCount + 1;
   gd->vertexsize2 = 3;
   vertices = DoubMat2DGetMem(gd->vertexsize1,gd->vertexsize2);
   gd->vertices = vertices;

   gd->rockboltsize1 = gd->nBolts + 1;
   gd->rockboltsize2 = 10;
   rockbolts = DoubMat2DGetMem(gd->rockboltsize1,gd->rockboltsize2);
   gd->rockbolts = rockbolts;

  /* Read in the first time step data, make the clones for 
   * original geometry.
   */
  /* nBlocks   number of blocks                          */
  /* nBolts   number of bolts                            */
  /* vertexCount   number of total vertices of d         */
  /* nFPoints   fixed points                             */
  /* nLPoints   loading points                           */
  /* nMPoints   measured points                          */
  /* material number  block start  block end  index      */
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


	for (i=0; i<= gd->nBlocks; i++)
   {
      fscanf(rpfile,"%d %d %d",
         &vindex[i][0], &vindex[i][1], &vindex[i][2]);
   }  /*  i  */


  /* Grab the time info */
  // fscanf(rpfile,"%d %lf %d", &g->timestep, &g->currenttime, &g->numcontacts);

  /* Loop over the number of time steps reading in 
   * geometry data for the rest of the data.
   */
  /* Suck up the first set of data */
  /* 0:joint material   1:x   2:y  of block vertices */
  /* The "number of vertices" is stored in vertexCount, but note that 
   * this number is more than the actual number of vertices.
   * Each block has one repeated vertex, to "close the loop".
   * The block vertices are stored in CCW order.  Note that 
   * each set of block vertices indexed in k0 is followed
   * by a repeat of the first, the three more vertices whose
   * purpose is not known right now.
   */
	 	for (i=0; i<= gd->vertexCount; i++)
   {
      fscanf(rpfile,"%lf %lf %lf",
         &vertices[i][0],&vertices[i][1],&vertices[i][2]);
   }  /*  i  */
   
  /* h: x1  y1  x2  y2  nBlocks  n2  e0  t0  f0  of bolt */
  /* nBlocks n2 carry block number        f0 pre-tension */
  /* Bolts are indexed from zero.  This will need to be changed
   * to index from 1 to make this stuff actually work.
   */
   if (gd->nBolts > 0)
   {
      for (i=0; i< gd->nBolts; i++)
      {
         fscanf(rpfile,"%lf %lf %lf",&rockbolts[i][1],&rockbolts[i][2],&rockbolts[i][3]);
         fscanf(rpfile,"%lf %lf %lf",&rockbolts[i][4],&rockbolts[i][5],&rockbolts[i][6]);
         fscanf(rpfile,"%lf %lf %lf",&rockbolts[i][7],&rockbolts[i][8],&rockbolts[i][9]);
      }  /*  i  */
   }

  /* x  y  n  of fixed measured loading points      */
 	for (i=0; i<= gd->pointCount; i++)
   {
      fscanf(rpfile,"%lf %lf %lf %lf",
         &points[i][0],&points[i][1],
         &points[i][2],&points[i][3]);
   }  


  /* Now that everything is initialized... */
   dda_set_geometrydata(dda,gd);


  /* This should have the first or original time step now. */
   for (j=0;j<nsteps_saved;j++)
   {

     /* Grab the time info */
      //fscanf(rpfile,"%d %f %d", &g->timestep, &g->currenttime, &g->numcontacts);

    	for (i=0; i<= gd->vertexCount; i++)
      {
         fscanf(rpfile,"%lf %lf %lf",
            &vertices[i][0],&vertices[i][1],&vertices[i][2]);
      }  /*  i  */
   
     /* h: x1  y1  x2  y2  nBlocks  n2  e0  t0  f0  of bolt */
     /* nBlocks n2 carry block number        f0 pre-tension */
     /* Bolts are indexed from zero.  This will need to be changed
      * to index from 1 to make this stuff actually work.
      */
      if (gd->nBolts > 0)
      {
         for (i=0; i< gd->nBolts; i++)
         {
            fscanf(rpfile,"%lf %lf %lf",&rockbolts[i][1],&rockbolts[i][2],&rockbolts[i][3]);
            fscanf(rpfile,"%lf %lf %lf",&rockbolts[i][4],&rockbolts[i][5],&rockbolts[i][6]);
            fscanf(rpfile,"%lf %lf %lf",&rockbolts[i][7],&rockbolts[i][8],&rockbolts[i][9]);
         }  /*  i  */
      }

     /* x  y  n  of fixed measured loading points      */
   	for (i=0; i<= gd->pointCount; i++)
      {
   	    fscanf(rpfile,"%lf %lf %lf %lf",
            &points[i][0],&points[i][1],
            &points[i][2],&points[i][3]);
      }  /*  i  */


     /* Make an update call to windows after every read. 
      */
		InvalidateRect(hwMain, NULL, TRUE);
		UpdateWindow(hwMain);


   }  /* end for loop over each saved time step */
				
  	//showOrig++;
	InvalidateRect(hwMain, NULL, TRUE);
	UpdateWindow(hwMain);
	//showOrig--;

   fclose(rpfile);

}  //  Close replay.
