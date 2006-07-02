
/*  
 *  Mohr-Coulomb criteria
 */

#ifndef __MOHRCOULOMB_H__
#define __MOHRCOULOMB_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


/**
 * Add explanatory comment here.
 */
void   invar       (void);


/**
 * Add explanatory comment here.
 */
double mocouf      (double c,
		    double phi);


/**
 * Add explanatory comment here.
 */
void   mocopl      (double phi,
		    double psi,
		    double nu,
		    double ymod);


/**
 * Add explanatory comment here.
 */
void   mohrcoulomb (double c,
		    double phi,
		    double psi,
		    double nu,
		    double ymod,
		    double eps[],
		    double * stress,
		    double sigma[]);


#endif /* __MOHRCOULOMB_H__ */
