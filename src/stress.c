
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
              int numblocks, BoundCond apply_boundary_cond, StrainModel strain_compute) {

   int i, i1;
   double strain[4] = {0.0};

   for (i=1; i<= numblocks; i++) {

      i1 = k1[i];
      strain_compute(D[i1], strain);
      apply_boundary_cond(e0[i], strain);
   }
}


/** A strain identity, assumes D has the small strain 
 * elements already and does nothing.
 *
 * @warning DO NOT DELETE THIS FUNCTION.  Deleting 
 *  this function will break the callback structure, 
 *  and will require one or more if statements.
 */
void
strain_linear_elastic(double * D, double strain[4]) {

   strain[1] = D[4];
   strain[2] = D[5];
   strain[3] = D[6];
}


/** Assume that D carries elements of the tangent map.
 *
 * @param D carrying F1, F3, F2, F4.  This is column oriented.
 *
 * @return strains in last argument
 *
 * @todo Write a unit test for this.
 */
void 
strain_green_lagrange(double * D, double strain[4]) {

   double h11 = D[3];
   double h12 = D[5];
   double h21 = D[4];
   double h22 = D[6];

   strain[1] = 0.5*( (h11+1)*(h11+1) + h21*h21 - 1);
   strain[2] = 0.5*( h12*h12 + (h22+1)*(h22+1) - 1);
   strain[3] = 0.5*((h11+1)*h12 + (h22+1)*h21);

}




                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
/**
 * Plain strain from MacLaughlin 1997, p. 20
 */
void
stress_planestrain(double * stress, double strain[4]) {

   double a1;
   double E  = stress[_E_];
   double nu = stress[_nu_];

   a1             = E/(1 + nu);
   stress[_s11_] += a1*( ((strain[1]*(1-nu))/(1-2*nu)) + ((strain[2]*nu)/(1-2*nu)) );
   stress[_s22_] += a1*((strain[1]*nu)/(1-2*nu) + (strain[2]*(1-nu)/(1-2*nu)));
   stress[_s12_] += a1*strain[3]/2.0;

  //stress_rotate(e0[i],D[i1][3]);

  /* Now compute e_zz, which should be 0 if we are in plane
   * strain, and will be used for computing mass if in 
   * plane stress.  This is for density correction.  See
   * TCK, p. 213, Fung, p. 267.
   * Note: we have to accumulate e_z to use for calculating
   * current thickness of block when calculating mass.
   */
   stress[7] += -(nu/(1-nu))*(strain[1] + strain[2]);
}


void
stress_planestress(double * stress, double strain[4]) {

   double a1;
   double E  = stress[_E_];
   double nu = stress[_nu_];

   a1             = E/(1-(nu*nu));
   stress[_s11_] += a1*(strain[1]    + strain[2]*nu);
   stress[_s22_] += a1*(strain[1]*nu + strain[2]);
   stress[_s12_] += a1* strain[3]*(1-nu)/2;

  //stress_rotate(e0[i],D[i1][3]);

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
stiffness_accumulate_firstorder_2d(double * K, double e[7][7]) {

   int j,j1,l;

   for (j=1; j<= 6; j++) {

      for (l=1; l<= 6; l++) {

         j1 = 6*(j-1)+l;  /* set index to global matrix */
         K[j1] += e[j][l];  /* add elastic coefficients */
      }  
   }
}



void
stiffness_linear_2d_planestrain(double e[7][7], const double S0,
                                const double E, const double nu) {

   double a2;
   double E11, E12, E21, E22, E33;

   a2 = S0*E/(1-nu);
   E11 = a2*(1-nu)/(1-2*nu);
   E22 = a2*(1-nu)/(1-2*nu);
   E12 = a2*(nu)/(1-2*nu);
   E21 = a2*(nu)/(1-2*nu);
   E33 = a2/2;

   e[4][4] = E11;
   e[4][5] = E12;
   e[5][4] = E21;
   e[5][5] = E22;
   e[6][6] = E33;
}


void
stiffness_finite_2d_planestrain(double e[7][7], const double S0,
                                const double E, const double nu) {

   double a2;
   double E11, E12, E21, E22, E33;

   a2 = S0*E/(1-nu);
   E11 = a2*(1-nu)/(1-2*nu);
   E22 = a2*(1-nu)/(1-2*nu);
   E12 = a2*(nu)/(1-2*nu);
   E21 = a2*(nu)/(1-2*nu);
   E33 = a2/2;

   e[3][3] = E11;
   e[4][4] = E33;
   e[5][5] = E33;
   e[6][6] = E22;

   e[4][5] = E33;
   e[5][4] = E33;

   e[3][6] = E12;
   e[6][3] = E21;

}

void
stiffness_linear_2d_planestress(double e[7][7], const double S0,
                                const double E, const double nu) {

   double a2;
   double E11, E12, E21, E22, E33;

   a2 = S0*E/(1-(nu*nu));
   E11 = a2;
   E12 = a2*nu;
   E21 = a2*nu;
   E22 = a2;
   E33 = a2*(1-nu)/2;

   e[4][4] = E11;
   e[4][5] = E12;
   e[5][4] = E21;
   e[5][5] = E22;
   e[6][6] = E33;
}


void
stiffness_finite_2d_planestress(double e[7][7], const double S0,
                                const double E, const double nu) {

   double a2;
   double E11, E12, E21, E22, E33;

   a2 = S0*E/(1-(nu*nu));
   E11 = a2;
   E12 = a2*nu;
   E21 = a2*nu;
   E22 = a2;
   E33 = a2*(1-nu)/2;
   
   e[3][3] = E11;
   e[4][4] = E33;
   e[5][5] = E33;
   e[6][6] = E22;

   e[4][5] = E33;
   e[5][4] = E33;

   e[3][6] = E12;
   e[6][3] = E21;
}




/**
 * Future callback.  This function might also disappear,
 * or could be used to wrap a couple of calls that are
 * interdependent, with this function being the single 
 * callback invoked instead or two callbacks from the 
 * function that loops over the block list which would 
 * have to be compatible.
 */
void
stiffness_compute(double * K, const double E, const double nu, 
                  const double S0,int planestrainflag) {

   //int i;
   //int j,j1,l;
   //double a2;
   double e[7][7] = {{0.0}};

   /* Since e is only used once per call, this 
    * loop should be unnecessary.
    */
   /*
   for (i=1; i<= 6; i++) {
      for (j=1; j<= 6; j++) {
         e[i][j]=0;
      }
   } 
   */

  /* The stiffness function needs to come in as a callback. */
#if 1
   if (planestrainflag == 1) {
      stiffness_linear_2d_planestrain(e,S0,E,nu);
   } else {
      stiffness_linear_2d_planestress(e,S0,E,nu);
   }
#else
   if (planestrainflag == 1) {
      stiffness_finite_2d_planestrain(e,S0,E,nu);
   } else {
      stiffness_finite_2d_planestress(e,S0,E,nu);
   }
#endif


#if 1
   stiffness_accumulate_firstorder_2d(K,e);
#else
   for (j=1; j<= 6; j++) {

      for (l=1; l<= 6; l++) {

         j1 = 6*(j-1)+l;  /* set index to global matrix */
         K[j1] += e[j][l];  /* add elastic coefficients */
      }  
   }
#endif

}


/**************************************************/
/* df13: submatrix of stiffness                    */
/**************************************************/
void 
stress_stiffness(int numblocks, double ** K, const int *k1, double **e0, 
                 double ** moments, int **n, int planestrainflag) {

   int i2, i3;
   int block;

   double S0;  // block volume, was a1
   double E,nu;

  /*
   int i,j,j1,l;
   double a2;

   double e[7][7]; 

   for (i=1; i<= 6; i++) {
      for (j=1; j<= 6; j++) {
         e[i][j]=0;
      }
   } 
   */
   for (block=1; block<=numblocks; block++) {

      E  = e0[block][2];
      nu = e0[block][3];
      S0 = moments[block][1];

     /* i2, i3 is location of Kii in global matrix. */
      i2=k1[block];
      i3=n[i2][1]+n[i2][2]-1;

#if 1
      stiffness_compute(K[i3],E,nu,S0,planestrainflag);
#else
      if (planestrainflag == 1) {

         a2 = S0*E/(1-nu);
         e[4][4] = a2*(1-nu)/(1-2*nu);
         e[5][5] = a2*(1-nu)/(1-2*nu);
         e[4][5] = a2*(nu)/(1-2*nu);
         e[5][4] = a2*(nu)/(1-2*nu);
         e[6][6] = a2/2;

      } else {  /* PLANESTRESS */

         a2 = S0*E/(1-(nu*nu));
         e[4][4] = a2;
         e[4][5] = a2*nu;
         e[5][4] = a2*nu;
         e[5][5] = a2;
         e[6][6] = a2*(1-nu)/2;
      }


      for (j=1; j<= 6; j++) {

         for (l=1; l<= 6; l++) {

            j1 = 6*(j-1)+l;  /* set index to global matrix */
            K[i3][j1] += e[j][l];  /* add elastic coefficients */
         }  
      } 
#endif

   }  
}  



/**************************************************/
/* df14: submatrix of initial stress              */
/**************************************************/
void stress_initial(int nBlocks, const int *k1, double **F, const double **e0,
                    const double ** moments) {

   int i, i1;
   double S0;

   for (i=1; i<=nBlocks; i++) {

      S0 = moments[i][1];
      i1 = k1[i];

     /* Compute virtual work from last iteration. */
     /* Add the stresses. e0 is updated in df25() */
      F[i1][4] += -S0*e0[i][_s11_];
      F[i1][5] += -S0*e0[i][_s22_];
      F[i1][6] += -S0*e0[i][_s12_];
   }  
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
