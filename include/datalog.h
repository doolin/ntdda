
#ifndef __DATALOG_H__
#define __DATALOG_H__

#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif




/** Start changing this over to use an incomplete type.
 * It will be much easier to maintain.
 */
typedef struct _datalog Datalog;




/* Track energy consumption per time step.  Later,
 * each block can grab one of these structs for itself.
 */
typedef struct _energy_struct {

   double KEcentroid;
   double KEdeform;
   double ke;
   double contact;
   double gravitational;
   double bolt;
   double fpoint;
   double lpoint;
   double istress;

} Energy;

/* This struct records a large amount of data
 * collected during run time for an analysis.  
 * FIXME: Think about adding an options struct in here
 * that controls what data to record.  This will help
 * with post-processing.
 */
struct _datalog {

  /* Clocking variables.  These will eventually go into 
   * their own struct.
   */
   clock_t analysis_start, analysis_stop;

   clock_t analysis_runtime;
   clock_t assemble_runtime;
   clock_t integration_runtime;
   clock_t solve_runtime;
   clock_t openclose_runtime;
   //clock_t contact_runtime;
   //clock_t update_runtime;

  /* track changing friction angles for disp dep
   * properties.
   */
   int phisize1;
   int phisize2;
   double ** phi;

  /* Records state changes from open-to-close, i7 in df22 */
   int open_to_close;

  /* Records state changes from close-to-open, i8 in df22 */
   int close_to_open;

  /* Records the number of open/close iterations per time step. */
   int openclosesize1;
   int * openclosecount;

  /* Records the number of times the time step was cut to reach 
   * a convergent time step.
   */
   int timestepcutsize1;
   int * timestepcut;

  /* Records the number of distance admissable contacts each 
   * time step.
   */
   int numdistcontactsize1;
   int * numdistcontact;

  /* Records the number of actual contacts used to solve analysis
   * at each time step.  This differs from distcontact by admitting
   * angle constraints as well as distance constraints.
   */
   int numanglecontactsize1;
   int * numanglecontact;

  /* Records number of sides per block.  This will eventually help with 
   * determining contact allocation.
   */
   int numsidessize1;
   int * numsidesperblock;

  /* Records number of vertex-vertex contacts in each time step. */
   int num_v_v_contactsize1;
   int * num_v_v_contacts;

  /* Records number of vertex-side contacts in each time step. */
   int num_v_s_contactsize1;
   int * num_v_s_contacts;

  /* Records convex-convex vertex contacts. */
   int num_cvex_cvex_contactsize1;
   int * num_cvex_cvex_contacts;

  /* Records number of convex-concave angle contacts in each time step. */
   int num_cvex_ccave_contactsize1;
   int * num_cvex_ccave_contacts;

  /* Record the memory allocation size.  This is different from the 
   * contact sizes, because it has to handle the diagonals, off-diagonals,
   * and any produced zeros that will be used in LDLT decomposition.
   */
   int memallocsize1;
   int * memallocsize;

  /* Track energy production and consumption per time step.
   */
   int energysize1;
   Energy * energy;


  /* Record contact forces on a per-time step basis.  This is a bit
   * tricky.  The memory will have to be allocated and freed along 
   * with the memory for the K matrix.  This probably needs to be 
   * handled with a struct *.
   */
   
};



Datalog *  datalog_new     (int numtimesteps, 
                            int numjointmats, 
                            int numblocks);

void       datalog_delete  (Datalog *);


#ifdef __cplusplus
}
#endif

#endif /* __DATALOG_H__ */

