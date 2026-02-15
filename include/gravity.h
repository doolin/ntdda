
/*
 * gravity.h
 *
 * Everything to do with gravity turn-on.
 */


#ifndef __GRAVITY_H__
#define __GRAVITY_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


/* Typedefs first.  The struct definition may later be
 * privatized in the source file.
 */
typedef struct _gravity_tag Gravity;


/* FIXME: Eliminate all dependency on Analysisdata and
 * Geometrydata types for the next four functions.  This
 * needs to be done to allow the gravity stuff to be
 * handled cleanly with property sheets, output files, etc.
 */
void checkGravityConvergence(Gravity *, void * Geometrydata, void * Analysisdata);
void initGravity(Gravity *, int nBlocks, double delta_t);

/* The calling function is responsible for
 * initializing all of the appropriate values.
 */
Gravity * newGravity();
void destroyGravity(Gravity *);
Gravity * cloneGravity(Gravity *);

struct _gravity_tag {

   int contactfnsize1;  // = n7;
   int contactfnsize2;  // = 4;
   double ** contactFN;

   int contactfssize1;  // = n7;
   int contactfssize2;  // = 4;
   double ** contactFS;

  /* FIXME: Rename this to prevForces */
   int prevforcesize1;  // = nBlocks+1;
   int prevforcesize2;  // = 7;
   double ** prevforces;

  /* Turns out we might need this to separate the
   * gravity code from the analysis code.
   */
   double delta_t;

   int gravTimeStep;
   int gravTurnonLimit;    // = 5000;
   //double gravTolerance;   // = 0.01;
   double nforcetolerance; // = 0.01;
   double sforcetolerance; // = 0.01;
  /* This is control value for unknown vectors, checked after
   * each time step.
   */
   double residual_tol;    // = 0.001;


  /* Need a gravity file name for recording gravity
   * results.
   */
   //char * gravfilename;

  /* Add a file pointer in here for dumping the
   * gravity data.
   */
   //FILE * gravfp;

};




#ifdef __cplusplus
}
#endif


#endif  /* __GRAVITY_H__ */

