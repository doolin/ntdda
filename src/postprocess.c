
/*
 * postprocess.c
 *
 * Handle a number of postprocessing chores.
 * 
 * $Author: doolin $
 * $Date: 2002/05/26 15:56:06 $
 * $Source: /cvsroot/dda/ntdda/src/postprocess.c,v $
 * $Revision: 1.7 $
 */

#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "analysis.h"
#include "math.h"
#include "ddamemory.h"
#include "interface.h"
#include "graphics.h"
#include "printdebug.h"
#include "postprocess.h"
#include "dda.h"

extern InterFace * iface;


/* necessary evil right now. */
extern FILEPOINTERS fp;
extern Datalog * DLog;

void writeHTMLLogFile(Geometrydata *, Analysisdata *, Datalog *, GRAPHICS *, FILE *);
void writeDataLog(Geometrydata *, Analysisdata *, Datalog *);

/* This will eventually handle all of the postprocessing 
 * requested by the user through using flags, etc.  For 
 * now, just define a struct in a header file with all the 
 * flags in the struct.
 */
void 
//postProcess(HWND hw, Geometrydata * GData, Analysisdata * AData, GRAPHICS * g)
postProcess(Geometrydata * GData, Analysisdata * AData, GRAPHICS * g)
{
   char temp[100], mess[1000];
   //int length;

   DLog->analysis_stop = clock();
   DLog->analysis_runtime = DLog->analysis_stop - DLog->analysis_start;

   g->analysis_runtime = DLog->analysis_runtime/(double)CLOCKS_PER_SEC;
   g->contact_runtime = DLog->contact_runtime/(double)CLOCKS_PER_SEC;
   g->update_runtime = DLog->update_runtime/(double)CLOCKS_PER_SEC;
   g->assemble_runtime = DLog->assemble_runtime/(double)CLOCKS_PER_SEC;
   g->solve_runtime = DLog->solve_runtime/(double)CLOCKS_PER_SEC;
   g->integration_runtime = DLog->integration_runtime/(double)CLOCKS_PER_SEC;
   g->openclose_runtime = DLog->openclose_runtime/(double)CLOCKS_PER_SEC;

   g->totaloc_count = AData->n9;


   sprintf(mess, "\nAnalysis complete.\n");
   sprintf(temp, "CPU run time: %2.3f seconds\n", g->analysis_runtime);
   strcat(mess, temp);
   sprintf(temp, "Assembly run time: %2.3f seconds\n", g->assemble_runtime);
   strcat(mess, temp);
   sprintf(temp, "Solve run time: %2.3f seconds\n", g->solve_runtime);
   strcat(mess, temp);
   sprintf(temp, "Penalty run time: %2.3f seconds\n", g->openclose_runtime-g->solve_runtime);
   strcat(mess, temp);

   dda_display_info(mess);


  /* writeMeasuredPoints must called after df25().
   * We could remainder arithmetic to save this every
   * n steps, or to not save at all at user request.
   * This would just cost a conditional.
   */
   //writeMeasuredPoints(GData, AData);
   //exportfig(GData);

   //writeAvgArea(AData);
   //writeCentroids(AData);
   writeTimeInfo(AData);
   /* Both the following functions write to `parfile',
    * which should be reserved for analysis parameters.
    */
   //writeSpringStiffness(AData);
   //writeFriction(DLog);

   writeDataLog(GData, AData, DLog);

   writeHTMLLogFile(GData, AData, DLog, g, fp.htmlfile);
   //computeStresses(GData, AData, e0, blockArea);



} /* close postProcess() */


void
writeDataLog(Geometrydata * gd, Analysisdata * ad, Datalog * dlog)
{
   int i;
   char  * matlabcomment = "%% ";
   char  * gnuplotcomment = "## ";
   char * comment;
   int matlab = 1;

  /* matlab style comments */
  /*
   fprintf(fp.parfile,"%% Spring stiffness for each time\n"
                    "%% step (col. 1) is recorded in column 2.\n"
                    "%% FIXME: which variable does this really\n"
                    "%% represent in the original GHS code.\n");
   */
  /* gnuplot-style comments... */

   if (matlab == 1)
      comment = matlabcomment;
   else 
      comment = gnuplotcomment;

   fprintf(fp.datafile,
      "%s step  springstiff delta_t OC_count  KE Istress OC_cuts\n\n", 
      comment);

   if (matlab == 1)
      fprintf(fp.datafile,"runtime = [\n");

   for (i=0;i<=ad->nTimeSteps; i++)
   {
     fprintf(fp.datafile,"%d  %f %f %d %f %f %d\n",
		                   i, 
						   ad->springstiffness[i],
                           ad->globalTime[i][2], 
						   DLog->openclosecount[i],
                           DLog->energy[i].ke, 
						   DLog->energy[i].istress,
                           DLog->timestepcut[i]);
   }


   if (matlab == 1)  
      fprintf(fp.datafile,"];\n");


}  /* close writeDataLog() */



/* This should be changed to track the normalized
 * maximum area change in any block in any time step.  
 * The actual block might change, but that probably 
 * doesn't matter.  This should write to the area file 
 * also instead of the logfile.
 */
void 
writeAvgArea(Analysisdata * AData)
{
   int i;
   
   fprintf(fp.momentfile, "\naveragearea = [\n");
   for (i = 1; i <= AData->nTimeSteps; i++)
   {
      fprintf(fp.momentfile, "avgArea %d: %f\n", i, AData->avgArea[i]);
   }
   fprintf(fp.momentfile, "];\n");

}   /*  Close writeAvgArea  */



void 
writeMoments(Geometrydata * gd, int currtimestep, int numtimesteps)
{
   int i;
   double x0, y0;
   double S0, S1, S2, S3;
   double ** moments = gd->moments;

   if (currtimestep == 0)
   {
      fprintf(fp.momentfile,"%% Block moments for each time step\n");
      fprintf(fp.momentfile,"%% timestep block_number S0 S1 S2 S3\n");
      fprintf(fp.momentfile,"blockmoments = [\n");
   }

   for (i=1;i<=gd->nBlocks;i++)
   {
     /* (GHS: non-zero terms of mass matrix)  */
     /* Compute centroids.   These are also computed in 
      * the time step function. 
      */
      x0=moments[i][2]/moments[i][1];  // x0 := x centroid
      y0=moments[i][3]/moments[i][1];  // y0 := y centroid

     /* S1, S2, S3 result from integrals derived on 
      * pp. 83-84, Chapter 2, Shi 1988.
      */
      S0=moments[i][1];  // block area/volume
      //S0=e0[i][9]; //moments[i][1];  // block area/volume
      S1=moments[i][4]-x0*moments[i][2];
      S2=moments[i][5]-y0*moments[i][3];
      S3=moments[i][6]-x0*moments[i][3];

      fprintf(fp.momentfile, "%d  %d  %21.16f   %21.16f   %21.16f   %21.16f\n",
                            currtimestep, i, S0, S1, S2, S3);
   }
   //fprintf(fp.areafile,"\n");

   if (currtimestep == numtimesteps)
      fprintf(fp.momentfile,"];\n");

}  


void 
writeBlockMasses(Analysisdata *ad, Geometrydata * gd)
{
   int i;
   double * mass = gd->mass;

   if (ad->currTimeStep == 0)
   {
      fprintf(fp.massfile,"%% Block areas for each time step\n");
      fprintf(fp.massfile,"%% Starts at the zeroth time step\n\n");
      fprintf(fp.massfile,"blockmasses = [\n");
   }

  /* WARNING: indexed from 1 instead of 0 */
   for (i=1;i<=gd->nBlocks;i++)
   {
      fprintf(fp.massfile, "%20.16f ",mass[i]);
   }
   fprintf(fp.massfile,"\n");

   if (ad->currTimeStep == ad->nTimeSteps)
      fprintf(fp.massfile,"];\n");

}  /* close writeBlockMasses() */



/* FIXME: Find a way to handle writing a matlab input 
 * file without checking to see that we are on the last 
 * iteration to close the array.  
 * FIXME: This function is obsolete.
 */
void 
writeMasses(Analysisdata *ad,
                Geometrydata * gd)
{
   int i;

  /* WARNING: indexed from 1 instead of 0 */
   for (i=1;i<=gd->nBlocks;i++)
   {
      fprintf(fp.logfile, "%20.16f ",gd->mass[i]);
   }
   fprintf(fp.logfile,"\n");

/*
   if (ad->currTimeStep == ad->nTimeSteps)
      fprintf(fp.areafile,"];\n");
*/

}  /* close writeMasses() */


/* Find a different file to write this information to.
 * logfile is gettting tired.
 */
void
writeCentroids(Analysisdata * AData, GRAPHICS * g, int numblocks)
{
   int i,j;
   fprintf(fp.logfile,"\nBegin Centroids\n");
   for (i=1;i<=AData->nTimeSteps; i++)
   {
      fprintf(fp.logfile,"Timestep %d\n",i);
      for (j=1;j<=numblocks;j++)
      {
         fprintf(fp.logfile,"%d  %f   %f\n",
            j, g->centroids[i][j].x,g->centroids[i][j].y);
      }
      fprintf(fp.logfile,"\n");
   }

   fprintf(fp.logfile,"End Centroids\n\n");

}  /* close writeCentroids() */


/* The spring stiffness appears to be the same for 
 * every block during an analysis, but it does change
 * over time steps.  TODO:  Explain what variables
 * affect the spring stiffness.
 */
void
writeSpringStiffness(Analysisdata * ad)
{
   int i;
  /* matlab style comments */
   fprintf(fp.parfile,"%% Spring stiffness for each time\n"
                    "%% step (col. 1) is recorded in column 2.\n"
                    "%% FIXME: which variable does this really\n"
                    "%% represent in the original GHS code.\n");
  /* matlab variable name and opening bracket */
   fprintf(fp.parfile,"\nspringstiffness = [\n");
   for (i=0;i<ad->springstiffsize; i++)
   {
     fprintf(fp.parfile,"%d  %f\n",i, ad->springstiffness[i]);
   }
  /* matlab closing bracket */   
   fprintf(fp.parfile,"];\n");
 
}  /* close writeStiffness()  */

/* Displacement dependent properties can investigated
 * by examining the value of the friction angle computed
 * according to the friction relationship.
 */
void
writeFriction(Datalog * data)
{
   int i,j;
  /* matlab style comments */
   fprintf(fp.parfile,"%% Friction angle for each joint type for each time\n"
                    "%% step (col. 1) is recorded in column 2-n.\n");
 
  /* matlab variable name and opening bracket */
   fprintf(fp.parfile,"\nfrictionangles = [\n");
   for (i=1;i<data->phisize1; i++)
   {
     fprintf(fp.parfile,"%d ",i);
     for (j=1;j<data->phisize2;j++)
        fprintf(fp.parfile," %f ",data->phi[i][j]);
     fprintf(fp.parfile,"\n");
   }
  /* matlab closing bracket */   
   fprintf(fp.parfile,"];\n");
 
}  /* close writeFriction()  */


/* The timing information for ith step consists of:
 * globalTime[i][0]: cumulative time from 0 seconds;
 * globalTime[i][1]: displacement ratios???
 * globalTime[i][2]: the time increment \Delta t 
 *                   for the ith step
 * It would be very convenient to examine this data
 * graphically.  But instead of just dumping the array
 * through the print2DArray() function, we can do 
 * better: format the output for Matlab input.
 */
void
writeTimeInfo(Analysisdata * ad)
{
  /* Kludge.  Output file formats will eventually have 
   * their own struct.
   */
   int matlab = 1;
   char * matlabcomment = "%% ";
   char * gnuplotcomment = "## ";
   char * comment;
   double ** globalTime = ad->globalTime;
   int m = ad->globaltimesize1;
   int n = ad->globaltimesize2;

   if (matlab == 1)
      comment = matlabcomment;
   else 
      comment = gnuplotcomment;

  /* Here is a reason to have nice wrapper functions
   * around ordinarily pedestrian array output.  Since 
   * post-processing is traditionally a major PITA, we
   * can use Matlab (or octave) to slurp up named arrays
   * and plot large arrays.  Consequently, we need to 
   * name the arrays, and it is always helpful to add a few
   * comments in the matlab script explaining the data in 
   * the arrays.
   */
  /* Matlab comments are preceded by a "%" sign.  We need
   * two because the C compiler uses a single % sign for
   * formatting data.
   */
   fprintf(fp.timefile,"%s Global time data.\n"
                    "%s globaltime:\n"
                    "%s    col 1: cumulative time\n"
                    "%s    col 2: displacement ratio??\n"
                    "%s    col 3: time step \\Delta t\n\n",
                     comment, comment, comment, comment, comment);
  
   if (matlab == 1)
      fprintf(fp.timefile,"globaltime = [");

  /* Now, for another reason to have wrapper functions,
   * consider this: The `globalTime' array is not
   * initialized when it is allocated.  Then, no
   * values are stored in the zero position of the 
   * "2d" dimension, but the first dimension of the 
   * array DOES index from zero.  So, what we do is 
   * the following...
   */
   globalTime[0][0] = 0;
   globalTime[0][1] = 0;
   globalTime[0][2] = 0;
  /* Otherwise, the uninitialized contents of the second
   * dim leading may contain spurious values, on the order 
   * of say -10^{66}. Now, pass it to the function which 
   * actually writes it to a file.
   */
   print2DArray(globalTime, m,n,fp.timefile, "");
  /* And add the trailing bracket for the Matlab 
   * matrix notation:
   */
   fprintf(fp.timefile,"];\n\n");

  /* So, when the analysis concludes, these data stored in
   * the globalTime array are formatted for direct Matlab 
   * input in an appropriate `m' file.  Spiffy, no?
   */
}  /* close writeTimeInfo() */



/* Wrapper for writing out measured points.  Since we
 * need information on each measured point for every time
 * step, writing the info to disk saves ram at the expense
 * of run-time.  Writing to disk also requires some file
 * pointer stunts to ensure that the data can be read by 
 * matlab or octave.  The technique used is to name each 
 * measured point as "measpoint1", "measpoint2", etc etc.,
 * then write the values as an array following the variable.
 * This requires computing the size of the array in bytes
 * ahead of time, the jumping from array to array using 
 * random file access functions fseek, then saving locations
 * using ftell.  The access points are stored in an array
 * "fp_pos" which holds the locations of the end of the 
 * ith array block.  
 *
 * Note the horrible index kludgery that is necessary because
 * GHS stores all of the points (fixed, measured, load and
 * hole) in the same array, then does index arithmetic to 
 * access individual points.  This technique turns straight-
 * forward ideas, such as computing velocities of certain 
 * points, into painfully obtuse implementation.
 */
/* FIXME: Add a header documenting the output variables,
 * with a comment marker in front:
 * % Time step, current time, delta t, xpos, ypos, vx, vy, v
 */
void
writeMeasuredPoints(Geometrydata * gd, Analysisdata * ad)
{
  /* For each measured point, the fp_pos array stores the 
   * location of the end of the ith measured point block.
   */
   static long * fp_pos = NULL;
  /* loop counters */
   int i,j,k;
  /* dereference some useful variables */
  /* int nTimeSteps = ad->nTimeSteps; */
   int nMPoints = gd->nMPoints;
   int nLPoints = gd->nLPoints;
   int nFPoints = gd->nFPoints;
  /* initial x, y position */
   static double * xinit = NULL, * yinit = NULL;
  /* current x,y positions */
   double xpos, ypos;
  /* velocities */
   double vx, vy, v;
  /* displacements at each time step */
   double dx, dy, d, dcum;
  /* This will declare the variable name in matlab or 
   * octave.  There is a problem here with the stupid
   * %%. 
   * FIXME: The comment header needs to be pasted in by the 
   * preprocessor or by a user option to control whether gnuplot
   * or matlab/octave output is desired.
   */
   char headerstring[] = {"\n%% %% Timestep, currenttime, deltat, xpos, ypos, dcum, vx, vy, v\n\n"
                          "measpoint%d=[\n"};  // Comma is for ^%#@!^%^#@! Excel importing
  /* After all the data for the array is written,
   * that is, at the last time step, close the array
   * with matlab array bracket delimiters.
   */
   static char trailerstring[] = {"];\n"};
  /* FIXME: redo the logic here to have the string size
   * variable and adjusted for run-time conditions.
   */
   char tempstring[256];
  /* size of headerstring, used for computing offset */
   long headersize;
  /* size in bytes of entire array */ 
   long blocksize;
  /* size in bytes of a single row of numbers in the array */
   long rowsize;
  /* size in bytes of bracket closing array */
   static long trailersize;
  /* These two values control field width for formatted 
   * output.  Later, this technique can be extended to have
   * field width and precision adjustable on the fly, from 
   * user specified parameters.
   */
   //int doublewidth = 10;
   int doublewidth = 20;
   int intwidth = 10;
  /* This formatting string will probably be rewritten as
   * a user specified precision, but it will have to have 
   * a fair bit of code to support it.  Do that later.
   */ 
   //char formatstring[] = {"%10d %10.4f %10.4f %10.4f %10.4f %10.5f %10.4f %10.4f %10.4f\n"};
   char formatstring[] = {"%10d %20e %20e %20e %20e %20e %20e %20e %20e\n"};
  /* Rowsize computation.  Doubles are 4 (8?) bytes, but writing out
   * to text requires one byte per character.  Note that there
   * is one non-printing character per line which is the 
   * new line, and (evidently) we need to add an extra byte
   * for null character.  FIXME: investigate the null(?)
   */
   rowsize = 8*doublewidth + 1*intwidth + 8*1/*spaces*/+ 1/* \n */+ 1/*\0*/;

  /* blocksize is basically the size of one row of output 
   * times the number of size (number of timesteps).
   * This should approximate the size of a 2D matrix of
   * numbers written out in ascii text.  FIXME: blocksize
   * is too big. Find out why and fix.
   */
   blocksize = rowsize*(ad->nTimeSteps+1);
  /* trailersize won't change like headersize does. */
   trailersize = strcspn(trailerstring, "\0");


  /* The first time we get to this function, we need 
   * set up the headers for each block of measured 
   * point data and set the positions of the end of
   * blocks.
   */ 
   if (ad->currTimeStep == 1)
   {
     /* Get memory for an array of pointers to the current end 
      * of each block.  Array size is the number of measured
      * points specified.  Warning: these may be initialized
      * from a gravity phase.
      */
      if(fp_pos == NULL)
         fp_pos = (long *)malloc(sizeof(long)*(gd->nMPoints));
     /* Need to store the initial values of x,y for each point. */
      if (xinit == NULL)
         xinit = (double *)malloc(sizeof(double)*(gd->nMPoints));
      if (yinit == NULL)
         yinit = (double *)malloc(sizeof(double)*(gd->nMPoints));
     /* Might need to save previous positions later on, if
      * all measured point post-processing gets moved into 
      * results file stuff. 
      */
      for (k=0,i=nFPoints+nLPoints+1; k<gd->nMPoints; k++, i++)
      {
        /* construct the variable name, e.g., "measpoint1" */
         sprintf(tempstring, headerstring, k+1);
        /* this size changes, e.g., might have "measpoint10" */
         headersize = strcspn(tempstring, "\0");
        /* at k = 0, we are at the start of the file.  Then
         * jump at k multiples of the block size.
         */
         fp_pos[k] =  k*(blocksize + trailersize);
        /* make the jump to new position in file */
         fseek(fp.measfile, fp_pos[k], SEEK_SET);
        /* write the variable name to the file */
         fprintf(fp.measfile, tempstring);
        /* save the current position of the file pointer cause we
         * will need it next time around.
         */
         fp_pos[k] = ftell(fp.measfile);
        /* Now save the initial position of the measured points.
         */
         xinit[k] = gd->origpoints[i][1];
         yinit[k] = gd->origpoints[i][2];
      }  /* k */
   }  /* end if first time step */

  /* Kludge.  This should go before the previous block of code,
   * but unfortunately causes a segfault b/c nothing gets
   * initialized otherwise.
   */
   if(ad->gravityflag == 1)
      return;

  /* Then, write the data to the file as we go. 
   * Shift indices over to start from 0 instead of some
   * number related to how many other points there are
   * in the analysis.  FIXME: velocities and possibly 
   * accelerations are already computed in other parts of 
   * the code.  Might be possible to remove some of this.
   */
   for (j=0,i=nFPoints+nLPoints+1; i<= nFPoints+nLPoints+nMPoints; i++, j++)
   {    
     /* Find the end of the jth block */      
      fseek(fp.measfile, fp_pos[j], SEEK_SET);
     /* find current position */
      xpos = gd->points[i][1];
      ypos = gd->points[i][2];
     /* grab displacements */
      dx = gd->points[i][5];
      dy = gd->points[i][6];
     /* resultant displacement this time step */
      d = sqrt(dx*dx + dy*dy);
     /* cumulative displacement */
      dcum = sqrt( (xpos-xinit[j])*(xpos-xinit[j]) 
                 + (ypos-yinit[j])*(ypos-yinit[j]) );
     /* compute velocities. */
      vx = dx/ad->delta_t;
      vy = dy/ad->delta_t;
      v = sqrt((vx*vx + vy*vy));
     /* Do what we came to do, viz. write data to file.
      * Note that repeated values are just place holders.
      */ 
      fprintf(fp.measfile, formatstring, ad->currTimeStep, 
               ad->elapsedTime, ad->delta_t, 
                xpos, ypos, dcum,
               vx, vy, v);
     /* Save the file pointer location after writing. */
      fp_pos[j] = ftell(fp.measfile);  
   }  /*  i,j  */

  /* After the data for the last time step is written to 
   * the file, we need to close the matlab braces.
   */
   if (ad->currTimeStep == ad->nTimeSteps)
   {
     /* Now close each block of numbers with the 
      * matlab matrix delimiters.
      */
      for (k=0; k<gd->nMPoints; k++)
      {  
        /* find current end of kth block */
         fseek(fp.measfile, fp_pos[k], SEEK_SET);
        /* write the closing array bracket */
         fprintf(fp.measfile, trailerstring);
      } /* k */
      
      //free(fp_pos);
      //free(xinit);
      //free(yinit);
   }  /* end if last time step */

}  /* Close writeMeasuredPoints()  */

   
void 
writeFixedPoints(Geometrydata * gd, Analysisdata * ad)
{
   int i;
   int nFPoints = gd->nFPoints;
      
   double xpos, ypos;

  /* This will declare the variable name in matlab or 
   * octave.
   */
   char headerstring[] = {"\n%% (x,y) pairs of fixed points\n\n"
                          "fixedpoints=[\n"};  // Comma is for ^%#@!^%^#@! Excel importing
  /* After all the data for the array is written,
   * that is, at the last time step, close the array
   * with matlab array bracket delimiters.
   */
   static char trailerstring[] = {"];\n"};

  /* Kludge. */
   if(ad->gravityflag == 1)
      return;

   if(ad->currTimeStep == 1)
      fprintf(fp.fpointfile, headerstring);

   for (i=1; i<=gd->nFPoints; i++)
   {
      xpos = gd->points[i][1];
      ypos = gd->points[i][2];
      fprintf(fp.fpointfile,"%.12f  %.12f  ",xpos,ypos);
   }
   fprintf(fp.fpointfile,"\n");

   if (ad->currTimeStep == ad->nTimeSteps)
   {
      fprintf(fp.fpointfile, trailerstring);
   }


}  /* close writeFixedPoints() */


void
writeSpyfile(int ** n, int * kk, int numblocks, FILE * spyfile)
{
   int i,i1,i2,i3;
   int j;

   for (i=1; i<=numblocks; i++)
   {
      i1 = n[i][1];  // start index of kk
      i2 = n[i][2];  // stop index of kk
      i3 = i1 + i2 - 1;  // number of entries in row

      for (j=i1;j<=i3;j++)
      {
         fprintf(spyfile, "%d %d\n",kk[j], i);
      }

   }



}  /* close WriteSpyfile() */


void
writeBlockVertices(Geometrydata * gd, int block)
{
   double ** vertices = gd->vertices;
   int ** vindex = gd->vindex;
   //int numvertices;
   int startindex=vindex[block][1];
   int stopindex=vindex[block][2];
   int i;

   fprintf(fp.logfile,"\nvertices = [\n");

   for (i=startindex; i<=stopindex; i++)
   {
      fprintf(fp.logfile,"%30.16f %30.16f \n", vertices[i][1], vertices[i][2]);
   }

   fprintf(fp.logfile,"]\n");

}  


void
writeBlockStresses(double ** e0, int block)
{

   fprintf(fp.stressfile,"%30.16f %30.16f %30.16f\n", 
      e0[block][4], e0[block][5], e0[block][6]);

}  /*  close writeBlockStress() */