
/**  Description: (add here)
 *
 *
 * Parts of the code in this file are used for 
 * DDAML processing.  Details of DDAML are 
 * available from the author.  
 *
 * @author David M. Doolin dave@mail.gonesilent.com
 */


#ifndef __BOLT_H__
#define __BOLT_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#ifndef PRINTFUNC
#define PRINTFUNC
typedef int (*PrintFunc)(void * stream,const char * format,...);
#endif


#ifndef TRANSMAPFUNC
typedef void (*TransMap)(double * moments, double T[7][7], double x, double y);
#define TRANSMAPFUNC
#endif


#ifndef TRANS_APPLY_FUNC
#define TRANS_APPLY_FUNC
typedef void (*TransApply)(double T[][7], double * D, 
                            double * u1, double * u2);
#endif
typedef struct _bolt Bolt;
typedef struct _boltlist Boltlist; 
typedef struct _boltmat Boltmat;





Bolt *     bolt_new            (void);
Bolt *     bolt_new_1          (double x1,
                                double y1,
                                double x2,
				                    double y2);

/**
 * This is primarily used for unit testing to ensure that 
 * the bolt append and retrieve functions work correctly.
 *
 * @return int 1 for passing, 0 for failing.
 */
int        bolt_equals           (Bolt * b1,
                             	    Bolt * b2);

void       bolt_delete           (Bolt *);

int        bolt_test             (void);

/**
 * @todo  Write the API documentation for this function.
 */
int        bolt_get_type         (Bolt *);

void       bolt_set_type         (Bolt *,
                                  int type);

void       bolt_set_endpoints    (Bolt * b, 
                                  double x1,
                                  double y1,
                                  double x2,
                                  double y2);

void       bolt_get_endpoints    (Bolt * b, 
                                  double * x1,
                                  double * y1,
                                  double * x2,
                                  double * y2);

double     bolt_get_length       (Bolt * b);


/** All of the function with an appended "_a" indicate that 
 * they operate on the traditional array style structure.
 * In the future, the incomplete type that is defined as 
 * a bolt struct should be able to be implemented fairly
 * easily.  The API should be really similar between the 
 * bolt array and bolt struct methods, and if there are any 
 * problems that appear with the array based API, they can be 
 * rectified when the struct is implemented.
 *
 * The API presented below for arrays will probably change as 
 * well.  Checking and saving state on primitives is difficult.
 * Therefore, methods in the API must be invoked in the appropriate
 * order.  This is inconvenient from a programming point of view,
 * but fast to implement.  More work needs to be done here.
 */

/** How to simulate a constructor when the object is an 
 * array?  One way is to split the memory allocation
 * for the double **.  Then each bolt can be allocated
 * as a "bolt".  This leaves the problem of allocating
 * the pointer array.
 */
// Not implemented
double *   bolt_new_a            (double size);

// Not implemented
double *   bolt_new_a_1          (double size,
                                  double x1,
                                  double y1,
                                  double x2,
                                  double y2);

// Not implemented
void       bolt_set_endpoints_a  (double * b, 
                                  double x1,
                                  double y1,
                                  double x2,
                                  double y2);

// Not implemented
void       bolt_get_endpoints_a  (double * b, 
                                  double * x1,
                                  double * y1,
                                  double * x2,
                                  double * y2);

/** bolt_set_length_a should probably go away,
 * and have the bolt length be private.  Then 
 * just have an accessor method for retrieving 
 * length for unit testing.
 */
void       bolt_set_length_a     (double * bolt);
double     bolt_get_length_a     (double * bolt);

void       bolt_log_a            (double ** rockbolts, 
                                  int numbolts, 
                                  int current_time_step, 
                                  double elapsed_time,
                                  PrintFunc printer,
                                  void * stream);
/**
 * When a bolt is given an initial pretension force,
 * an unstrained length of the bolt must be computed 
 * before the analysis proceeds, so that subsequent 
 * deformation will induce the correct forces.
 * Since we can't rewrite, use the array in lieu of 
 * bolt struct.
 *
 * @todo This method needs to idempotent 
 *  per each bolt.
 *
 * @param double * an array to various values 
 *  needed for a bolt.
 *
 * @return double dl the length differential
 *  computed by dl = f/k;       
 */
double     bolt_set_ref_length_a (double * bolt);


/** This method assumes that the reference length is 
 * already computed.
 *
 * @param double * to an array of properties for a bolt.
 *
 * @return double unstrained length of bolt.
 */
double     bolt_get_ref_length_a (double * bolt);


void       bolt_set_pretension_a (double * bolt);

double     bolt_get_pretension_a (double * bolt);


void       bolt_get_dir_cosine_a (double * bolt, 
                                  double * lx, 
                                  double * ly);

void       bolt_update_a         (double ** bolts, 
                                  int numbolts, 
                                  double ** F,
                                  double ** moments, 
                                  TransMap transmap,
                                  TransApply transapply);

void       bolt_stiffness_a      (double ** bolts, 
                                  int numbolts, 
                                  double ** K, 
                                  int * k1, 
                                  int * kk, 
                                  int ** n, 
                                  double ** blockArea,  
                                  double ** F, 
                                  TransMap transmap);



/** Boltmat methods, which may go into their own header
 * file in the future.
 */
Boltmat *  boltmat_new         (void);

void       boltmat_set_props   (Boltmat * bm, 
                                double stiffness, 
                                double strength, 
                                double pretension);

void       boltmat_get_props   (Boltmat * bm, 
                                double * stiffness, 
                                double * strength, 
                                double * pretension);


/********************************************************/

/** Boltlist methods, which may go into their own
 * header file in the future.
 */

Boltlist * boltlist_new        (void);

/** Free all of the memory associated with a list 
 * of bolts.
 *
 * @warning This function deletes all the bolts in the
 * boltlist as well.  It breaks symmetry in the API, 
 * unfortunately, in c, there is no easy way around 
 * that fact.
 */
void       boltlist_delete     (Boltlist *);

/** Add a Bolt to the end of the list of bolts.
 */
void       boltlist_append     (Boltlist *, 
                                Bolt *);

/** Helper method, useful for transferring bolt data into 
 * arrays, which need to be allocated before the list is 
 * traversed.
 *
 * @return int number of bolts in the list.
 */
int        boltlist_length     (Boltlist *);

/** Internally, DDA uses arrays for everything so this 
 * allows transforming the bolt data into arrays.  Arrays
 * are very inconvenient to work with for dialog boxes, 
 * xml input, drawing etc.  The array of doubles ** are 
 * allocated in the calling function to help maintain 
 * symmetry in the memory handling.
 */
void       boltlist_get_array  (Boltlist *, 
                                double ** boltarray);

void       boltlist_print      (Boltlist *,
				                    PrintFunc printer,
				                    void * stream);



#ifdef __cplusplus
}
#endif

#endif /* __BOLT_H__ */
