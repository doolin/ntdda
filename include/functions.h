
#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


#ifndef TRANSMAPFUNC
#define TRANSMAPFUNC
typedef void (*TransMap)         (double * moments, 
                                  double T[7][7], 
                                  double x, 
                                  double y);
#endif


#ifndef TRANS_APPLY_FUNC
#define TRANS_APPLY_FUNC
typedef void (*TransApply)        (double T[][7], 
                                   double * D, 
                                   double * u1, 
                                   double * u2);
#endif


#ifndef MASS_MATRIX_FUNC
#define MASS_MATRIX_FUNC
typedef void (*MassMatrix)        (double T[][7],
                                   double m,                                   
                                   const double S0,
                                   const double S1,
                                   const double S2,
                                   const double S3);
#endif


void transplacement_linear        (double * moments, 
                                   double T[7][7], 
                                   const double x, 
                                   const double y);

void transplacement_finite        (double * moments, 
                                   double T[7][7], 
                                   const double x, 
                                   const double y);

void transplacement_apply_linear  (double T[][7], 
                                   double * D, 
                                   double * u1, 
                                   double * u2);

void transplacement_apply_2dorder (double T[][7], 
                                   double * D, 
                                   double * u1, 
                                   double * u2);

void transplacement_apply_exact   (double T[][7], 
                                   double * D, 
                                   double * u1, 
                                   double * u2);


/**
 * 
 * @param double T[7][7] stack based work array for mass matrix
 *  for linear infinitesial motion.
 *
 * @param double block density.
 *
 * @param double S0 block volume: int_{V}\,dV.
 *
 * @param double S1 second mass moment X_1 \[ \int_V X_1^2\,dV \]
 *
 * @param double S2 second mass moment X_2 \[ \int_V X_2^2\,dV \]
 *
 * @param double S3 second mass moment X_1X_2 \[ \int_V X_1X_2\,dV \]
 *
 * @warning  T[][] is assumed to have been initialized to 0 
 *  before entering this function.  Only the non-zero terms
 *  are set.  
 *
 */
void massmatrix_linear            (double T[][7],
                                   double density,
                                   const double S0,
                                   const double S1,
                                   const double S2,
                                   const double S3);

/**
 * 
 * @param double T[7][7] stack based work array for mass matrix
 *  for finite homegeneous motion.
 *
 * @param double block density.
 *
 * @param double S0 block volume: int_{V}\,dV.
 *
 * @param double S1 second mass moment X_1 \[ \int_V X_1^2\,dV \]
 *
 * @param double S2 second mass moment X_2 \[ \int_V X_2^2\,dV \]
 *
 * @param double S3 second mass moment X_1X_2 \[ \int_V X_1X_2\,dV \]
 *
 * @warning  T[][] is assumed to have been initialized to 0 
 *  before entering this function.  Only the non-zero terms
 *  are set.  
 *
 */
void massmatrix_finite            (double T[][7],
                                   double density,
                                   const double S0,
                                   const double S1,
                                   const double S2,
                                   const double S3);


/** Computing the kinetic energy depends on the 
 * form of the transplacement map.
 */
double energy_kinetic             (const double * v, 
                                   const double rho, 
                                   const double S0, 
                                   const double S1, 
                                   const double S2, 
                                   const double S3);




#ifdef __cplusplus
}
#endif

#endif  /* __FUNCTIONS_H__ */