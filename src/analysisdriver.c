/* 
 * analysisdriver.c 
 *
 * Drives the dda forward analysis. The functions
 * invoked from this driver comprise the set of 
 * functions from the "df", or "dda forward" 
 * written by GHS.
 * 
 * $Author: doolin $
 * $Date: 2001/06/23 23:14:03 $
 * $Source: /cvsroot/dda/ntdda/src/analysisdriver.c,v $
 * $Revision: 1.4 $
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


/* externs, necessary evil right now.
 * This MUST be cleaned up later.
 */
extern Geometrydata * geomdata;
extern Geometrydata * geometry2draw;
extern Analysisdata * ad;

extern InterFace * iface;
extern FILEPOINTERS fp;

DATALOG * DLog;
DDAError ddaerror;

void compilecontrol(Analysisdata * AData);


/* TODO: Change parameter list to take a geometry data 
 * pointer and a char * for the path and file name of
 * the analysis file.  End goal: No externs, no memory
 * leaks.
 */
int	
ddanalysis(FILEPATHS * filepath, GRAPHICS * gg)
{
   Geometrydata * GData;
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

  /* FIXME: Find a way to get rid of this and have only one
   * geometry data structure at a time.
   */
   GData = cloneGeometrydata(geomdata);
   geometry2draw = GData;

   doublesize = sizeof(double);

   AData = analysisInput(filepath->afile, GData);   
   if (AData == NULL)
      return 0;

  /* Hardwired parameters that need to go into 
   * user input files in the future.
   */
   compilecontrol(AData);

  /* Copy this pointer to extern so that we can access the 
   * analysis data from the analysis dialog box.
   * FIXME: Find a way to remove this crappy extern.
   */
   ad = AData;

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

   //writeBlockVertices(GData, 4);
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
      //AData->first_openclose_iteration = TRUE;


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
        /* The "classical" DDA derived in GHS 1988 used a central
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

           /* saveState() handles copying loops from df20(). */
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
   
           /* displacement ratio and iteration drawing  */
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
      * FIXME: Put a conditional in front of this.  No need to 
      * call it every time if there are no measured points.
      */
      writeMeasuredPoints(GData, AData);
      //writeFixedPoints(GData, AData);
      //writeSolutionVector(AData->F, kk, k1, n, GData->nBlocks);
      //writeBlockMasses(AData, GData);
      //writeBlockStresses(e0,4);

     /* MacLaughlin, 1997: Chapter 3, Section 3, p. 26-30. */
      if (AData->gravityflag == 1)
         checkGravityConvergence(AData->gravity, GData, AData);

     /* Draw some stuff to the screen. */ 
     ///*iface->*/display(hwMain, hdc, GData, AData, gg);
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
   //postProcess(hwMain, GData, AData,gg);
   postProcess(GData, AData,gg);

   writeBlockVertices(GData, 4);

  /* Everything after here is clean up code.  All memory 
   * should be freed, all open files closed. etc etc etc.
   * FIXME: Ram all of the rest of this into a "cleanup"
   * function as soon as the memory debacle with the 
   * geometry data is fixed.
   */
   deallocateAnalysisArrays(kk,k1,c0,e0,U,n);

  /* This is weird stuff to update the stupid extern that we
   * need to interact with win32 to draw on the screen.  Hopefully,
   * this does not leak any memory.
   */
   freeGeometrydata(geomdata);
   geomdata = cloneGeometrydata(GData);
   geometry2draw = geomdata;
   freeGeometrydata(GData);
   AData = freeAnalysisData(AData);
   freeDatalog(DLog);

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
