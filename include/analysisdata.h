
/* There should be no windows dependencies in this file. */

#ifndef __ANALYSISDATA_H__
#define __ANALYSISDATA_H__

#ifdef WIN32
#pragma warning( disable : 4115 )        
#endif

#include "ddatypes.h"
#include "gravity.h"
#include "timehistory.h"
#include "loadpoint.h"
#include "constants.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



typedef struct _analysisdata_tag Analysisdata;



/* Holds data for a particular instance of an 
 * analysis.  If any members are added to this 
 * struct, make sure and add the appropropriate 
 * code to the init, clone and free functions.
 */
struct _analysisdata_tag {


  /* Save a lot of memory on these flags, most
   * of which are boolean anyway. 
   */
   unsigned int options;

   DisplayFunc display_warning;
   DisplayFunc display_error;

  /* Analysis state information.  This is useful for handling 
   * more sophisticated user interface features.
   */
  /* FIXME: should be boolean */
   int isRunning;
   int isPaused;

  /* These will eventually get moved out to a more
   * appropriate structure.  In the original GHS code,
   * the array indices are started from 1 instead of
   * zero. 
   */
   int index;   //  1 in GHS code.
   int blocksize;  //  6 in GHS code, 3 in DDAD

  /* Any of the these variables that stay boolean
   * need to go into a FLAG struct.  Variables such as
   * the rotationflag will eventually have more than
   * two values to reflect many different types of 
   * rotation correction.
   */
   ddaboolean rotationflag;
   ddaboolean gravityflag;
   ddaboolean autotimestepflag;
   ddaboolean autopenaltyflag;
   ddaboolean planestrainflag;
   ddaboolean verticesflag;

  /* This is a desparate kludge just to get some matlab
   * output.
   */
   int writemfile;

  /* static = 0, dynamic = 1;  problem type 
   * This may be variable k01 in GHS code.  Should
   * also probably be a flag, but keep it here for 
   * now.
   */
   int analysistype;  

  /* pfactor is what to multiply the Young's modulus by to get 
   * a penalty number for an analysis.  Putting it here is a 
   * kludge.  Later, some sort of penalty control struct will
   * need to be derived.
   */
   double pfactor;

   enum rotationtype {linear, secondorder, exact} rotationtype;
   enum solvetype {lu, cg, sor} solvetype;
   enum contactmethod {penalty, auglagrange} contactmethod;
   enum frictionlaw {tpmc = 0, negexp, voight, druckerprager, bartonjrc} frictionlaw; 
   enum integrator {constant, newmark, symplectic} integrator;
   enum units {si, english} units;
   int fileformat; //  Should be an enum  {original, extended, ddaml} fileformat;


  /**************************************************/
  /* k5: index of u0               >=2 time points  */
  /* k5: 0 start of point i   1 end of point i      */
  /* k5[nFPoints+nLPoints+1][2]                     */
  /* k5 is the index into time history of loading array,
   * which is now called "TimeDeps".  
   */
   int k5size1;
   int k5size2;
   //int ** k5;
   int ** tindex;
  /* Now we need something to provide an extra record
   * of load versus fixed points:
   */
  /* Throw stuff against the wall, see what sticks. 
   * This will have size nLPoints 
   */
   Loadpoint * loadpoints;
  /* The time history stuff has to play with the LoadPoints
   * for the time being.
   */
   TimeHistory * timehistory;

  /* From pp. 86-88 GHS 1988, we have a viscosity formulation. */
   double mu;

  /* FIXME:   This is a horrible kludge */
   int pointCount;
   int nLPoints;
  /* c does several things.  What it is needed for in this struct 
   * is for storing fixed point ``correction'' displacements.  
   * Each fixed point will move a little bit because the penalty 
   * methods does not prohibit movement, just makes the resultant 
   * displacement small in comparison with unconstrained motion.
   * So after a time step, the displacement of the fixed point 
   * (computed in df25()) is stored in c and used to provide
   * a ``correction'' when the fixed point matrices are computed
   * in df12().  The correction forces are computed from the 
   * displacements, then added to the load vector.
   */
   int csize1;
   int csize2;
   double ** c;

  /* FIXME: add comments */
   int timedepsize1;
   int timedepsize2;
   int nTDPoints;
   double ** timeDeps;

  /* FIXME: Add comments */
   int materialpropsize1;
   int materialpropsize2;
   double ** materialProps;   // e0
 
  /* FIXME: Add comments */
   int phicohesionsize1;
   int phicohesionsize2;
   double ** phiCohesion;

  /* Previously, all bolt properties were given in the 
   * geometry file.  Splitting bolt property types from 
   * the bolt geometry (endpoints) brings bolt handling
   * into parallel with block and joint property handling.
   * FIXME: What is size for these?
   */
   int boltmatsize1;
   int boltmatsize2;
   double ** boltmats;

  /* The timing information for ith step consists of:
   * globalTime[i][0]: cumulative time from 0 seconds;
   * globalTime[i][1]: displacement ratios???
   * globalTime[i][2]: the time increment \Delta t 
   *                   for the ith step
   */
  /**************************************************/
  /* q0: 0 accumulated real time utill this step    */
  /* q0: 1 real max. dspl./ defined max. dspl.      */
  /* q0: 2 real time interval of this step          */
  /* q0[nTimeSteps+1][3]                            */
   int globaltimesize1;
   int globaltimesize2;
   double ** globalTime;

  /* Gravity turn-on stuff.  Most of this will
   * probably get moved into its own struct later.
   * Then it can be grabbed when needed, and 
   * deallocated immediately after the gravity
   * turn on phase is complete.
   */
  /* Accerelation of gravity.  This is currently handled in 
   * unit weight of block materials, but it may be easier
   * to handle unit conversions by explicitly defining 
   * this value.
   */
   double gravaccel;    
  /* current gravity step value, compared with 
   * gravTurnonLimit. 
   */

   Gravity * gravity;

   int ksize1;
   int ksize2;
   double ** K;  /* global stiffness matrix, was `a' */
  /* b is "old K", a copy of K.  It will be in the 
   * code as a copy of a.
   */
   double ** Kcopy;

  /* Vector of knowns, i.e., boundary conditions.  Need a copy of
   * this too.
   */ 
   int Fsize1;
   int Fsize2;
  /**************************************************/
  /* f : u v r ex ey gxy     of load terms          */
  /* f : free terms and solution of equations       */
  /* f[nBlocks+1][7]                                */
   //double **f;
  /*------------------------------------------------*/
  /* r : u v r ex ey gxy of load terms              */
  /* r : save f  before equation solving            */
  /* rr[nBlocks+1][7]                               */
   //double **rr;
   double ** F;
   double ** Fcopy;

   int abortedtimestep;
   int nTimeSteps;
   int nBlockMats;
   int nJointMats;
   int nBoltMats;

  /* g0 is spring stiffness at contacts */
   //double g0;
   double contactpenalty;
  /* Spring stiffness may need to be reset, say after 
   * running gravity stuff.  Another way to do this would
   * be to call the function initContactSpring() again (maybe).
   */
   double g0initial;
  /* TCKs contact damping parameters... */
   double contact_damping;  // change to cn, cs

  /* WARNING: Array must be allocated for nTimeSteps
   * + 1 to catch the initial value.  Move this out of
   * here and put it into the datalog struct.
   */
   int springstiffsize;
   double * springstiffness;
   
  /* g1 is max time step size   */
   double maxtimestep; /* was g1 */
   double max_delta_t;  /* was g1 */
   double min_delta_t;  /* previously hard-coded to 0.001 */

  /* FIXME: Explain what this means.  */
   double maxdisplacement;  // was g2


  /* FIXME: What is this? */
   //double g3;

  /* g4 seems to be a value used for spring stiffness for 
   * fixed point only.  Replaced with FPointSpring;
   * See GHS 1988, Chapter 2, p. 93.
   */
   //double g4;
   double FPointSpring;
  /* g7 is copy of normal spring stiffness g0, used to compute
   * joint normal force.
   */
   //double g7;
   double JointNormalSpring; 
   //int m4;
  /* nt is used in analysis reader code to set the number 
   * of time dependent points.  FIXME: It would be good to get 
   * rid of this variable completely
   * it is used to malloc and is set in blockdata.
   * I need it here to free the struct.
   */
   int nt;  
  /* n3 is the number of total block-to-block contacts, not
   * the total possible v-e/v-v contacts (nPrevContacts?),
   * or the total v-e/v-v actual contacts (nCurrContacts?).
   * n3 is used to allocate the memory for K.
   */
   int n3;
  /* w6 is related to penalty parameter or spring stiffness
   * somehow.
   */
   double w6;
   /*int m6;*/  /* m6 is the number of current contacts  */
   int nCurrentContacts;
  /* Distance contacts */
   int nPrevContacts;
  /* Save a replay file every saveTimeSteps */
   //int saveTimeSteps;
  /* Warning: this duplicates nBolts in the geometry data
   * structure.
   */
   int nBoltContacts;
  /* m9 counts the number of open-close iterations for a 
   * particular time step.  Reset to zero at the start of 
   * everytime step, or if open-close doed not converge and
   * the time step has to be cut.  Set to m9 = -1 when 
   * open-close iteration converges.
   */
   int m9;
  /* These variables are not used yet, but will be needed
   * in the future.
   */
  /* This needs to be a boolean.  Initialized to FALSE,
   * set to TRUE when open-close iteration converges.
   */
   int OCConverged;
  /* Counts the number of open close iterations during a 
   * time step.
   */
   int OCCount;
  /* If the number of open-close iterations gets to the 
   * value of OCLimit, then cut time step, query user,
   * write to log file, or do whatever else is appropriate.
   */
   int OCLimit;
  /* Another open-close flag... n9 counts the total number of 
   * open-close iterations for a particular analysis.
   */
   int n9;
  /* FIXME: What is k00? */
   int k00;   /*  Probably some sort of flag.  */
   //ddaboolean first_openclose_iteration;
   //int  n6;   /* n6 is previous number of contacts. */
  /* nn00 appears to track the number of contacts used 
   * in mallocing for the a (K) and b (old K) arrays.
   * Thus if the number of contacts increases, then the 
   * size of the K array is increased as well.  nn00 is
   * initialized in df08().
   */
   int nn00;  
  /* Array of average areas, 1 per time step.
   */
   double * avgArea;
   int avgareasize;

   //int currTimeStep;
   int cts;
  /* curretTime will be changed to "elapsedTime", which should 
   * be easier to understand.
   */
	//double currentTime;
   double elapsedTime;
   int tsSaveInterval;
   int saveTimeStep;
   int extraIteration;
  /* timestep will be changed to delta_t, which should make the 
   * code much easier to read.
   */
   //double timestep;
   //double pts;  /* previous time step. */
   double delta_t;
  /* Various constant used for various purposes
   * for controlling an analysis.  These were in a 
   * separate struct, but are moved into this 
   * struct because some of these will be changeable in 
   * the future from the analysis dialog box, or read in
   * from a file.
   * FIXME: Figure out any of these are changed during an 
   * analysis, and if so, record them in an array for 
   * later examination.
   */
   Constants * constants;
   Constants * initialconstants;

  /* Letting an analysis run unattended may produce 
   * amazingly bad results.  Previously, "bad" values 
   * were trapped by using assertions.  This is not clean 
   * from a user's point of view.  Checking critical values
   * against user-defined threshold values allows the 
   * analysis to shut down "nicely" if bad things such as
   * negative block area or angles out of range occur.
   */
   //Thresholds * threshold;


  /* Private functions accessed through function pointers. */
   void (*abort)(Analysisdata *);
   Analysisdata * (*newanalysis)(void);
//   void (*initialize)(void);
   void (*validate)(void);
//   void (*run)(Analysisdata *);

  /* This function is a bit problematic.  Part of the problem 
   * is deciding where to cut the geometry and analysis functions
   * apart.  Deleting blocks falls in between the two.  Sometimes
   * the blocks need to be deleted from purely a geometric reason,
   * other times, a block and all of its material parameters need to
   * be deleted.  
   * WARNING!!!  This function should only be invoked immediately 
   * after deleteBlocks.
   */
   void (*deletematerial)(Analysisdata *, int blocknumber);
  /* This might need to send back an integer. */
   void (*free)(Analysisdata *);

};



Analysisdata * adata_new                 (void);

Analysisdata * adata_init                (void);

void           adata_delete              (Analysisdata *);


Analysisdata * adata_clone               (Analysisdata *);

void           adata_validate            (Analysisdata *);


void           adata_set_output_flag     (Analysisdata *, 
                                          int flag);

void           adata_clear_output_flag   (Analysisdata *, 
                                          int flag);

double         adata_get_contact_damping (Analysisdata *);
void           adata_set_contact_damping (Analysisdata *, 
                                          double);


double         adata_get_grav_accel      (Analysisdata *);
void           adata_set_grav_accel      (Analysisdata *, 
                                          double grav);





/**
 * adata_read_input_file wraps all of the file format ugliness
 * behind a single call.  If there is a recoverable problem,
 * the Analysisdata * will come back NULL, and the memory freed
 * internally somewhere.  The reason why there are so many arguments
 * is that GHS original file format splits geometry from properties
 * so some quantities are read from different files using fscanf
 * in for loops.
 *
 * @param Analysisdata * points to a preallocated struct.
 * @param void * filestream needs to be cast in the actual
 *        file reading function to a FILE *.
 *
 * @return NULL Analysisdata * (with freed memory).
 */
void           adata_read_input_file     (Analysisdata *, 
                                          char * filename,
                                          int numfixedpoints,
                                          int pointcount,
                                          int numloadpoints);

void           adata_write_ddaml          (Analysisdata * ad, 
                                           PrintFunc printer, 
                                           void * stream);



#ifdef __cplusplus
}
#endif


#endif /* __ANALYSISDATA_H__ */

