
#include <stdio.h>
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
static const int _s1_    = 4;
static const int _s2_    = 5;
static const int _s12_   = 6;
static const int _t1_    = 7;
static const int _t2_    = 8;
static const int _t3_    = 9;
static const int _v1_    = 10;
static const int _v2_    = 11;
static const int _v3_    = 12;

void
stress_update(double ** e0, double ** D, int * k1, int numblocks, int planestrainflag) {

   int i, i1;
   double a1;
   double gamma_0; 
   double c,s;
   double sigmaxx, sigmayy, tauxy;


  /* (GHS: compute updating stresses)  */
   for (i=1; i<= numblocks; i++) {

      double E = e0[i][_E_];
      double nu = e0[i][_nu_];

      i1 = k1[i];
      gamma_0 = D[i1][3];
      c = cos(gamma_0);
      s = sin(gamma_0);

      if(planestrainflag == 1) {

         a1 = E/(1 + nu);
         e0[i][_s1_]  += a1*( ((D[i1][4]*(1-nu))/(1-2*nu)) + ((D[i1][5]*nu)/(1-2*nu)) );
         e0[i][_s2_]  += a1*((D[i1][4]*nu)/(1-2*nu) + (D[i1][5]*(1-nu)/(1-2*nu)));
         e0[i][_s12_] += a1*D[i1][6]/2.0;

      } else { /* Plane stress */
      
         a1        = E/(1-(nu*nu));
         e0[i][_s1_]  += a1*(D[i1][4]+D[i1][5]*nu);
         e0[i][_s2_]  += a1*(D[i1][4]*nu+D[i1][5]);
         e0[i][_s12_] += a1*D[i1][6]*(1-nu)/2;
      }  


     /* TCK stress rotation correction, Eq. 17, p. 324,
      * ICADD 1 proceedings.  See also Eqs. 18 and 19 for
      * formulas for updating the deformation rates.
      */
     /* TODO: verify that doing the updating here is 
      * mathematically correct.  It might need to be done 
      * before adding to the existing block stresses which
      * are (presumably) already in referential coordinates.
      */
      if (1) {

         sigmaxx = e0[i][_s1_];
         sigmayy = e0[i][_s2_];
         tauxy   = e0[i][_s12_];
         e0[i][_s1_]  = c*c*sigmaxx - 2*c*s*tauxy + s*s*sigmayy;
         e0[i][_s2_]  = s*s*sigmaxx - 2*c*s*tauxy + c*c*sigmayy;
         e0[i][_s12_] = c*s*(sigmaxx-sigmayy) + (c*c - s*s)*tauxy;
      }
      
     /* Now compute e_zz, which should be 0 if we are in plane
      * strain, and will be used for computing mass if in 
      * plane stress.  This is for density correction.  See
      * TCK, p. 213, Fung, p. 267.
      * Note: we have to accumulate e_z to use for calculating
      * current thickness of block when calculating mass.
      */
      e0[i][7] += -(nu/(1-nu))*(D[i1][4] + D[i1][5]);

   }  

}



void
stress_update_a(double * stress, double * D, int planestrainflag) {

   double a1;
   double gamma_0; 
   double c,s;
   double sigmaxx, sigmayy, tauxy;


   double E  = stress[_E_];
   double nu = stress[_nu_];

   gamma_0 = D[3];
   c = cos(gamma_0);
   s = sin(gamma_0);

   if(planestrainflag == 1) {

      a1 = E/(1 + nu);
      stress[_s1_]  += a1*( ((D[4]*(1-nu))/(1-2*nu)) + ((D[5]*nu)/(1-2*nu)) );
      stress[_s2_]  += a1*((D[4]*nu)/(1-2*nu) + (D[5]*(1-nu)/(1-2*nu)));
      stress[_s12_] += a1*D[6]/2.0;

   } else { /* Plane stress */
      
      a1            = E/(1-(nu*nu));
      stress[_s1_]  += a1*(D[4]    + D[5]*nu);
      stress[_s2_]  += a1*(D[4]*nu + D[5]);
      stress[_s12_] += a1* D[6]*(1-nu)/2;
   }  


  /* TCK stress rotation correction, Eq. 17, p. 324,
   * ICADD 1 proceedings.  See also Eqs. 18 and 19 for
   * formulas for updating the deformation rates.
   */
  /* TODO: verify that doing the updating here is 
   * mathematically correct.  It might need to be done 
   * before adding to the existing block stresses which
   * are (presumably) already in referential coordinates.
   */
   if (0) {

      sigmaxx = stress[_s1_];
      sigmayy = stress[_s2_];
      tauxy   = stress[_s12_];
      stress[_s1_]  = c*c*sigmaxx - 2*c*s*tauxy + s*s*sigmayy;
      stress[_s2_]  = s*s*sigmaxx - 2*c*s*tauxy + c*c*sigmayy;
      stress[_s12_] = c*s*(sigmaxx-sigmayy) + (c*c - s*s)*tauxy;
   }

      
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
stress_print(double * s, PrintFunc printer, void * stream) {


   printer(stream,"rho = %f;  gamma = %f; E = %f; nu = %f;\n",
                   s[_rho_],s[_gamma_],s[_E_],s[_nu_]);

   printer(stream,"sigma = [ %f %f  \n          %f %f ];\n", 
                   s[_s1_],s[_s12_],s[_s12_],s[_s2_]);


}


#ifdef __cplusplus
}
#endif
