
#include <stdio.h>

#include <stdlib.h>
#include <string.h>  /* memcpy */
#include <math.h>

#include "stress.h"



#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

/**
\begin{center}
\begin{tabular}{llc}\\\hline
  & meaning  & admissible values$^1$\\\hline
e0[][0] & unit mass ($\rho$)  & $0\leq\rho$\\
e0[][1] & unit weight ($\gamma$) & $0\leq\gamma$ \\
e0[][2] & Young Mod. ($E$) & $0<E$ \\
e0[][3] & Poisson ratio ($\nu$) & $-1<\nu<0.5$ \\
e0[][4] & $\sigma_1$ & \\
e0[][5] & $\sigma_2$ & \\
e0[][6] & $\sigma_{12}$ & \\
e0[][7] & $\tau_1$& \\
e0[][8] & $\tau_2$& \\
e0[][9] & $\tau_{12}$ &\\
e0[][10] & $v_1$ & \\
e0[][11]  & $v_2$ & \\
e0[][12] & $v_r$  & \\\hline \multicolumn{3}{p{3.0in}}{Material
properties $e0$ is initialized in the analysis reading functions.
\variable{e0} is typed as a double **, but in the future it should
be handled as an array of material structs.  This allocates all
the memory into one block.}\\\hline
\end{tabular}
\end{center}

*/


static const int _rho_   = 0;
static const int _gamma_ = 1; 
static const int _E_     = 2;
static const int _nu_    = 3; 
static const int _s11_   = 4;
static const int _s22_   = 5;
static const int _s12_   = 6;
static const int _t1_    = 7;
static const int _t2_    = 8;
static const int _t3_    = 9;
static const int _v1_    = 10;
static const int _v2_    = 11;
static const int _v3_    = 12;

static const int _size_  = 13;


int
stress_equals(double * d1, double * d2, double tol) {

  int i;

  for (i=0; i<_size_; i++) {
    if (fabs(d1[i]-d2[i]) > tol) {
      return 0;
    }
  }
  return 1;
}


double *
stress_clone(const double * s1) {

   double * s2;

   s2 = (double*)(malloc(sizeof(double)*_size_));
   memcpy(s2,s1,sizeof(double)*_size_);
   return s2;
}


void
stress_update(double ** e0, double ** D, int * k1, 
              int numblocks, ConstModel apply_const_model) {

   int i, i1;

   for (i=1; i<= numblocks; i++) {

      i1 = k1[i];
      apply_const_model(e0[i], D[i1]);
      //stress_update_a(e0[i], D[i1], planestrainflag);
      //stress_rotate(e0[i],D[i1][3]);
   }
}


/**
 * Plain strain from MacLaughlin 1997, p. 20
 */
void
stress_planestrain(double * stress, double * D) {

   double a1;
   double E  = stress[_E_];
   double nu = stress[_nu_];

   a1             = E/(1 + nu);
   stress[_s11_] += a1*( ((D[4]*(1-nu))/(1-2*nu)) + ((D[5]*nu)/(1-2*nu)) );
   stress[_s22_] += a1*((D[4]*nu)/(1-2*nu) + (D[5]*(1-nu)/(1-2*nu)));
   stress[_s12_] += a1*D[6]/2.0;

  /* Now compute e_zz, which should be 0 if we are in plane
   * strain, and will be used for computing mass if in 
   * plane stress.  This is for density correction.  See
   * TCK, p. 213, Fung, p. 267.
   * Note: we have to accumulate e_z to use for calculating
   * current thickness of block when calculating mass.
   */
   stress[7] += -(nu/(1-nu))*(D[4] + D[5]);
}


void
stress_planestress(double * stress, double * D) {

   double a1;
   double E  = stress[_E_];
   double nu = stress[_nu_];

   a1             = E/(1-(nu*nu));
   stress[_s11_] += a1*(D[4]    + D[5]*nu);
   stress[_s22_] += a1*(D[4]*nu + D[5]);
   stress[_s12_] += a1* D[6]*(1-nu)/2;

  /* Now compute stress_zz, which should be 0 if we are in plane
   * stress.
   */
   //stress[7] += 0; //-(nu/(1-nu))*(D[4] + D[5]);
}



/** TCK stress rotation correction, Eq. 17, p. 324,
 * ICADD 1 proceedings.  See also Eqs. 18 and 19 for
 * formulas for updating the deformation rates.
 */
/* TODO: verify that doing the updating here is 
 * mathematically correct.  It might need to be done 
 * before adding to the existing block stresses which
 * are (presumably) already in referential coordinates.
 */
void 
stress_rotate(double * stress, double r0) {

   double sigmaxx, sigmayy, tauxy;
   double c,s;

   c = cos(r0);
   s = sin(r0);

   sigmaxx = stress[_s11_];
   sigmayy = stress[_s22_];
   tauxy   = stress[_s12_];
   stress[_s11_]  = c*c*sigmaxx - 2*c*s*tauxy + s*s*sigmayy;
   stress[_s22_]  = s*s*sigmaxx - 2*c*s*tauxy + c*c*sigmayy;
   stress[_s12_] = c*s*(sigmaxx-sigmayy) + (c*c - s*s)*tauxy;
}



void
stress_print(double * s, PrintFunc printer, void * stream) {

   printer(stream,"rho = %f;  gamma = %f; E = %f; nu = %f;\n",
                   s[_rho_],s[_gamma_],s[_E_],s[_nu_]);
   stress_print_stresses(s,printer,stream);
}


void 
stress_print_stresses(double * s, PrintFunc printer, void * stream) {

   printer(stream,"sigma = [ %f %f  \n          %f %f ];\n", 
                   s[_s11_],s[_s12_],s[_s12_],s[_s22_]);
}



#ifdef __cplusplus
}
#endif
