/* 
 * analysisdriver.c 
 *
 * Drives the dda forward analysis. The functions
 * invoked from this driver comprise the set of 
 * functions from the "df", or "dda forward" 
 * written by GHS.
 * 
 * $Log: analysisdriver.c,v $
 * Revision 1.13  2001/11/02 13:38:41  doolin
 * Gravitational acceleration now a user option, specifically to
 * allow zero gravity.  Not sure if the tag has any effect at the moment,
 * because setting density to zero blows up the code while setting unit mass to zero works fine.  Having unit mass non zero doesn't work.  The code should not blow up for zero density, this is a bug.  Also, having both density and unit
 * mass specified should not be allowed.  Specify one or the other, but not both.  This will be enforced soon.
 *
 * Revision 1.12  2001/09/15 14:14:34  doolin
 * This has been a very long week.  It is hard to recall exactly
 * what was done last weekend, after the events on Tues. Sept. 11 2001, and
 * hard to really care.   At the minimum, TCKs 1995 contact damping has been
 * *fully* implemented, from the code in the forcing vector to the requisite
 * DDAML file handling changes.  As usual, there is not a clear (to me) way
 * of checking the correctness of the code other than single stepping through
 * and checking values.  This has _not_ yet been done.  The contact
 * damping code is only in the development version, and will not be released
 * in the next release version.  There will probably be a couple more commits
 * after this (small) as the rest of last weeks changes come to light.  Also,
 * changes made in the release branch are still propagating into the dev
 * branch, which is ugly and will have to be fixed.
 *
 * Revision 1.11  2001/09/03 03:45:37  doolin
 * REALLY IMPORTANT:  This set of commits will be
 * tagged, then version 1.6 release candidate 1 will be branched
 * from the main code.  Development will then proceed in parallel
 * between the development version and the release version.
 *
 * Revision 1.10  2001/08/26 03:16:26  doolin
 * One extern out, 2 more to go.  Lots of small API argument
 * changes.  Also, the formerly externally declared var was a struct,
 * so we are now leaking sizeof(Geometrydata) for each geometry, possibly
 * for each run.  This will be dealt with from the console after all the
 * externs are done.
 *
 * Revision 1.9  2001/08/26 02:15:49  doolin
 * Some minor changes leading up to a major memory overhaul to eliminate externs.  
 * This commit will be tagged as 1.5.126
 *
 * Revision 1.8  2001/08/26 00:21:20  doolin
 * Major output format change for block areas to handle all
 * the moments for each block at each time step.  The downside of this is
 * that the output file will require more postprocessing, which can be
 * trivially handled in matlab, octave or a simple perl script, etc.
 *
 * Revision 1.7  2001/08/24 22:56:22  doolin
 * Started work on abstracting data output code to
 * make it easier to handle different types of output data
 * depending on the geometry and the analysis file.  This will be
 * tedious because of the messy code interfacing windows to the
 * numerical back end.
 *
 * Revision 1.6  2001/08/17 03:29:19  doolin
 * Found a bug in namespace handling for geometry files.
 *
 * Revision 1.5  2001/07/23 12:54:49  doolin
 * Added stuff to the contact handling code.
 * The whole build is broken, don't know why.  Examples don't
 * work correctly, and they run very slowly.  Will probably
 * need to back out several weeks, possibly months worth of
 * changes to figure it out.
 *
 */



#include <stdlib.h>
#include <time.h>
#include "analysis.h"
#include <math.h>
#include <direct.h>
#include <assert.h>
#include <string.h>
#include "errorhandler.h"
#include "ddamemory.h"
#include "interface.h"
#include "ddaerror.h"
#include "printdebug.h"
#include "gravity.h"
#include "contacts.h"
#include "postprocess.h"


/* This should be the only allowable extern. */
extern InterFace * iface;



extern FILEPOINTERS fp;

DATALOG * DLog;
DDAError ddaerror;



/* TODO: Change parameter list to take a geometry data 
 * pointer and a char * for the path and file name of
 * the analysis file.  End goal: No externs, no memory
 * leaks.
 */
int	
ddanalysis(DDA * dda, FILEPATHS * filepath, GRAPHICS * gg)
{
   Geometrydata * GData = dda_get_geometrydata(dda);

   Analysisdata * AData;
   Contacts * CTacts;

   clock_t start, stop;
   int doublesize;   // 8 bytes on a 32 bit machine

  /**************************************************/
  /* k : index of a    2*block contact + 20*nBlocks */
  /* k : reserved positions for non-zero storage    */
  /* kk[nBlocks*40+1]                               */
  /* WARNING!!!  kk is overloaded between contact
   * finding in df04,05 and the sparse storage function.
   */
   int *kk;
  /*------------------------------------------------*/
  /* k1: i old block number k1[i] new block number  */
  /* k1[nBlocks+1]                                  */
   int *k1;
  /*------------------------------------------------*/
  /* c0: xl xu yl yu block box in 3000              */
  /* c0: u v r ex ey gxy of sliding friction force  */
  /* c0[nBlocks+1][7]                               */
   double **c0;

  /* FIXME: Move matprops to geometry structure */
  /**************************************************/
  /* e0: block material constants                   */
  /* e0: ma we e0 u0 c11 c22 c12 t-weight           */
  /* e0[nBlocks+1][8]                               */
   double ** e0; //matprops; // was e0
  /*------------------------------------------------*/
  /*  u v  or x+u y+v  of vertices in the `vertices'*/
  /* matrix,                                        */
  /* u[vertexCount+1][3]                            */
  /* WARNING!!!  U is overloaded in df04, df05 for 
   * storing the inside and outside block angles.
   */
  	double **U;
  /**************************************************/
  /* n : 1 begin number of k a b for i-th row       */
  /* n : 2 number of k a b       for i-th row       */
  /* n : 3 limit of k element number of i row       */
  /* n[nBlocks+1][4]                                */
  /* In other words, n[i][1] stores the location in kk
   * that has the starting column number (block j) 
   * associated with row i (block i).  n[i][2] contains
   * the total number of columns (contacts) of ith row,
   * and each column number is stored in successive 
   * locations in kk.  The diagonal ends up at 
   * n[i][1]+n[i][2]-1.  But these aren't really column 
   * numbers, they are locations in Kij!!!!!  What happens
   * is that kk stores column number associated with a 
   * certain value between n[i][1] and n[i][1]+n[i][2]-1,
   * which is used for matching block number in kk.
   * When the block number is matched in kk, the value
   * given from the n[][] matrix is used to extract or
   * insert values into the stiffness matrix K.  That is,
   * n has the index locations in K of various columns,
   * where the index value is matched against kk.  To extract 
   * a specific element, traverse a row i in n[][], checking 
   * to see whether that element is stored in kk.  If so,
   * the travseral counter from n[][] will match the index
   * for that particular row and column in the stiffness
   * matrix K.  Whew.
   */
   int **n;

   doublesize = sizeof(double);

   AData = analysisInput(filepath->afile, GData);   
   if (AData == NULL)
      return 0;
   dda_set_analysisdata(dda,AData);



/* The invoking program, whether windows or not, is going to 
 * have to grab an AData and send it various signals.  Here,
 * we set the output options directly.  This stuff is not in 
 * compile control because I want to deal with it asap.  And 
 * I want to get rid of compilecontrol asap.
 */
   //adata_set_output_flag(AData, VERTICES);
   adata_set_output_flag(AData, FIXEDPOINTS);
   adata_set_output_flag(AData, MEASPOINTS);
   adata_set_output_flag(AData, SOLUTIONVECTOR);
   adata_set_output_flag(AData, BLOCKMASSES);
   //adata_set_output_flag(AData, BLOCKSTRESSES);
   adata_set_output_flag(AData, PENALTYFORCES);
   adata_set_output_flag(AData, FRICTIONFORCES);
  /* FIXME: This is a horrible bogosity: moments need to 
   * be written from the geometry data, not the analysis
   * data.  
   */
   adata_set_output_flag(AData, MOMENTS);

  /* Allocating arrays in a function removes lots of superfluous
   * code. 
   */
   allocateAnalysisArrays(GData, &kk, &k1, &c0, &e0, &U, &n);

  /* FIXME: document function. */
   initNewAnalysis(GData, AData, e0, filepath,gg);

  /* New contact handling struct */
   CTacts = getNewContacts(GData->nBlocks);

  /* Draw some stuff to the screen. */ 
   display(GData, AData, gg);

  /* Check to see how the forces look. */
   //printForces(GData, AData->F, k1, "Before main loop");
      
   //{FILE*fp=NULL;printLoadPointStruct(AData,fp);}


/* All this stuff gets put elsewhere at some point in the 
 * future.
 */
   if (AData->options & VERTICES)
      writeBlockVertices(GData, 1);      
   
   if (AData->options & MOMENTS)
      writeMoments(GData, AData->currTimeStep, AData->nTimeSteps);

   //writeBlockMasses(AData,GData);
  /* If arg 2 is greater than the number of blocks, 
   * we get a crash.
   */
   //writeBlockStresses(e0,4);

  /* START THE MAIN ANALYSIS LOOP  */
   for (AData->currTimeStep=1; AData->currTimeStep<=AData->nTimeSteps; AData->currTimeStep++)
   {
     /* Compute the size of the next time step.
      * FIXME :Give a precis how it it works.
      */
      computeTimeStep(GData,AData); 
     /* Contains subfunctions df04, df05, df06, df07.
      * Contact finding takes a list of blocks, finds the 
      * number of vertex-vertex (v-v) or vertex-edge (v-e) contacts 
      * possible in the forthcoming time step, then determines
      * which of those contacts are possible using the corner
      * reference diagram, establishes locks (?), determines
      * length of contact for friction and cohesion computation,
      * then determines the number of block-to-block contacts
      * using the actual v-e/v-v  count (from df05).  Contact 
      * finding is described in Shi 1988, Chapter 4, pp. 139-188
      * (for df04, df05, df06, df07).  Note that U is overloaded
      * here, being used to temporarily store inside and outside
      * angles at each vertex.
      */
      findContacts(GData,AData, CTacts, kk,k1,n,c0);
      //printContactLengths(GData, contactlength, "From main loop");

     /* Positions of non-zero storage.
      * Constructing sparse storage (df08) is described 
      * in Shi 1988, Chapter 5, Section 5.3-8, pp. 199-230.
      */
      sparsestorage(GData,AData, CTacts, kk,k1, GData->nn0,n);

     /* For first time step, get the memory for the 
      * stiffness matrix.  For all other time steps,
      * check to see if the number of current block-to-block 
      * contacts is the same as the previous number of 
      * block-to-block contacts.  If not, reallocate.
      */
      allocateK(AData); 


         /**************  TEST FROM ABOVE THE DO LOOP  ************/
     /* k00 controls open/close iteration.  Appears to 
      * be a flag instead of an incremented value.  k00
      * is reset when the time step is incremented. This 
      * allows tracking values from previous time increments
      * when the time step is cut during an increment.
      */
      AData->k00=0;


      /************END TEST ***********************/		   
      
      /* (GHS: new time interval   k00=0:locks[i][1]=0 df07) */


     /* newtimestep means that the value of the time has been changed,
      * so that the analysis is re-run for the same displacements,
      * and the same time step number, but for a different time
      * difference between the current and last time value.
      */
      do //newtimestep:  /* the label formerly known as a001: */
      {

         start = clock();
   	  /* FIXME: m9 appears to be the iteration
         * counter for a particular time step.
         */
         AData->m9=0;
        /* Construct the diagonals K_ii of the `stiffness' matrix K.
         * The off-diagonals represent contacts, and are constructed
         * in df18() (?).  The analysis is in Shi 1988, Chapter 2,
         * pp. 60-96.  df10()-df16() are called from assemble().
         */
         assemble(GData, AData,get_locks(CTacts),e0,k1,kk,n, U);
        /* The "classical" DDA derived in GHS 1988 used a forward
         * difference formulation to integrate over time.  This 
         * code currently uses a forward difference expansion
         * outlined in the 1996 1st DDA Int forum.
         */
         timeintegration(GData, AData,e0,k1, n, U);

         start = clock();

        /* OPEN-CLOSE ITERATION */
         do  //openclose:  /* the label formerly known as a002: */
         {
           /* Add and subtract contact matrix(ces?) */
            df18(GData, AData, CTacts, kk,k1,c0,n);
            //printKForIthBlock(AData, 2, 1, kk, n, "After sparsestorage");
            //printKK(kk,n,GData->nBlocks,"Analysis driver");

           /* saveState() handles copying loops from df20(). 
            * This function saves a copy of K and F because the 
            * solver overwrites both.
            */
            saveState(AData, c0, GData->nBlocks);
           /* U = K^{-1}F  df20() and df21() are still called from solve(). 
            * U is overwritten into F as a result of the LU solver.
            */
            //printForces(GData, AData->F, k1, "analysis driver before solve");
            solve(AData, AData->K, AData->F,kk,k1,n,GData->nBlocks);

            //printForces(GData, AData->F, k1, "analysis driver after solve");
            //fflush(fp.logfile);
           /* set open-close    k00=1:locks[i][1]=locks[i][2] df22 */
           /* FIXME: Rename these flags to something a little more descriptive. */
            AData->k00=1; /* k00 seems to be a flag */
            //AData->first_openclose_iteration = FALSE;
            AData->m9++;
            AData->n9++;  // total oc count
#if DDA_FOR_WINDOWS
            iface->setoc_count(AData->m9);
#endif
           /* (GHS: Contact judge after iteration.)*/
           /* df22 is where open-close convergence is checked.  To implement
            * augmented lagrangian, there will need to be some code modified
            * in this function as well.  See Shi 1988, Chapter 4, Section 
            * 4.3, p. 157 for details.
            */
            df22(GData, AData, CTacts, k1);
   
           /* displacement ratio and iteration drawing.
            * This function also restores K and F by copying
            * the saved value (from saveState) back into 
            * K and F.
            */
            df24(GData, AData, k1);

        /* close do-while loop for open-close iteration */
        /* while ( HAVETENPEN && ( AData->iterationcount < MaxOpenCloseCount) ) */
         }  while (0<(AData->m9)  && (AData->m9)< (6+2) );          

        /* PASSED OPEN CLOSE ITERATION */

         stop = clock();
         DLog->openclose_runtime += (stop - start);

     /* Check the iteration count.  If too high and not converged,
      * recompute time step size or spring stiffness.  Then
      * run the open-close iteration again with new parameters
      */
      }  while(checkParameters(GData, AData, CTacts));  

     /* PASSED PARAMETER CHECKS */

      //printForces(GData, AData->F, k1, "After OC convergence");

     /* Compute step displacements.  FIXME: See if this function 
      * can be renamed "updateGeometry()".
      */
      df25(GData, AData,k1,e0,U);

     /* writeMeasuredPoints must be called after df25().
      * We could remainder arithmetic to save this every
      * n steps, or to not save at all at user request.
      * This would just cost a conditional. 
      * FIXME: Buffer this to be written out after the analysis.
      */

      if (AData->options & MEASPOINTS)
         writeMeasuredPoints(GData, AData); 
  
      if (AData->options & FIXEDPOINTS)
         writeFixedPoints(GData, AData);

      if (AData->options & SOLUTIONVECTOR)
         writeSolutionVector(AData->F, kk, k1, n, GData->nBlocks);

      if (AData->options & BLOCKMASSES)
         writeBlockMasses(AData, GData);

      if (AData->options & BLOCKSTRESSES)
         writeBlockStresses(e0,4);

      if (AData->options & MOMENTS)
         writeMoments(GData, AData->currTimeStep, AData->nTimeSteps);

     /* MacLaughlin, 1997: Chapter 3, Section 3, p. 26-30. */
      if (AData->gravityflag == 1)
         checkGravityConvergence(AData->gravity, GData, AData);

     /* Draw some stuff to the screen. */ 
      display(GData, AData, gg);

   } /* END OF MAIN ANALYSIS LOOP  */


   if (AData->writemfile)
   {
      writeMFile(AData->K, AData->Fcopy, AData->F, kk, k1, n, GData->nBlocks);
      //printK(AData->K, AData->ksize1, "Analysis driver");
      fflush(fp.mfile);
      //fflush(fp.logfile);  
      //exit(0);    
      //congrad(AData->K, AData->F, kk, k1, n, GData->nBlocks);

      AData->writemfile = FALSE;
   }


  /* All of the post-processing crapola goes in here.
   * Set up a global struct in a special header file 
   * with flags for what to emit in post-processing 
   * phase.
   */
   postProcess(GData, AData,gg);

   if (AData->options & VERTICES)
      writeBlockVertices(GData, 1);

  /* Everything after here is clean up code.  All memory 
   * should be freed, all open files closed. etc etc etc.
   * FIXME: Ram all of the rest of this into a "cleanup"
   * function as soon as the memory debacle with the 
   * geometry data is fixed.
   */
   deallocateAnalysisArrays(kk,k1,c0,e0,U,n);

  /* This should probably be freed in the calling function. */
   //AData = freeAnalysisData(AData);  //adata_destroy(AData);
   AData = adata_delete(AData);
   freeDatalog(DLog); //datalog_destroy(DLog);

  /* New contacts structure */
   CTacts = freeContacts(CTacts);

  /* Keep this as the last function call so that there will 
   * no ambiguity about which files are open.  
   */
   closeAnalysisFiles();   

  /* Eventually, this will return a struct that points at various parts of the
   * analysis.  That way, the analysis only has to be run one time, and all relevant
   * variables etc can be retained in the struct for further (and faster) 
   * examination.  What is happening currently is there is 
   * an extern pointing at it from winmain.
   */  
   return TRUE;  
   
} /* Close ddanalysis().  */
