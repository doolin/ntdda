
#ifndef __STRESS_H__
#define __STRESS_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


#ifndef __PRINTFUNC__
typedef int (*PrintFunc)(void * stream, const char * format, ...);
#define __PRINTFUNC__
#endif

/**
 *
 *
 * @param D
 * 
 * @param e0
 *
 * @param k1 stores the index for the ith block deformation 
 *  values in D.
 *
 * @param numblocks is used for looping over e0 and k1.
 *
 * @param planestrainflag = 1 indicates using plane strain 
 *  boundary conditions, plane stress otherwise.
 *
 * @return void.
 */
void stress_update   (double ** e0, 
                      double ** D, 
                      int * k1, 
                      int numblocks,
                      int planestrainflag);



/** For this function, the looping is done elsewhere.
 * Ideally, this function should be in the inner loop
 * of something similar to the previous function which 
 * takes k1 as an argument.
 *
 * @param D
 * 
 * @param e0
 *
 * @param planestrainflag = 1 indicates using plane strain 
 *  boundary conditions, plane stress otherwise.
 *
 * @return void.
 */
 void stress_update_a (double * e0, 
                       double * D, 
                       int planestrainflag);

void  stress_print    (double * s, 
                       PrintFunc printer, 
                       void * stream);



#ifdef __cplusplus
}
#endif



#endif  /* __STRESS_H__ */