
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


void invar();
double mocouf(double c,double phi);
void mocopl(double phi,double psi,double nu, double ymod);
void mohrcoulomb(double c, double phi, double psi,double nu,double ymod,
				 double eps[],double * stress,double sigma[]);


#endif /* __MOHRCOULOMB_H__ */
