
/**  Most of the functions in here need to be 
 *  put somewhere else.
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

/* FIXME: Turn this into a macro and test it. */
double sign         (double);

void   computeMass  (double * mass, 
                     double ** moments, 
                     double ** e0, 
                     int numblocks);

#ifdef __cplusplus
}
#endif

#endif  /* __UTILS_H__ */