

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


void massmatrix_linear            (double T[][7],
                                   const double S0,
                                   const double S1,
                                   const double S2,
                                   const double S3);


void massmatrix_linear            (double T[][7],
                                   const double S0,
                                   const double S1,
                                   const double S2,
                                   const double S3);






#ifdef __cplusplus
}
#endif


#endif  /* __FUNCTIONS_H__ */