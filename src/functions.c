
/** Every function in this file should be used as
 * a callback. NEVER call these functions directly.
 */

#include <math.h>

#include "functions.h"

/**************************************************/
/* dspl: block displacement matrix           0001 */
/*************************************************/
/* x10  y10  u2  v2  t[][]               i0  x  y */
/* FIXME: Change the arguments for this function so that
 * it will be easier to test:
 * transplacement_linear(double T[][7], double tX, tY)
 * where T is the "return value", and tX,tY are
 * \tilde X and \tilde Y respectively.  This way the
 * moments array does not need to be continually
 * dereferenced in this function.  In fact, computing
 * the centroid could be done anytime the block is
 * already in memory (i.e., the array is being dereferenced
 * elsewhere for any other reason.
 */
void
transplacement_linear(double * moments, double T[][7],
                      const double x, const double y) {

   double xb, yb;
   double u2, v2;

  /* i0 is block number                         */
  /* x10,y10:center of gravity of the block     */
   //xb = moments[i0][2]/moments[i0][1];
   //yb = moments[i0][3]/moments[i0][1];
   xb = moments[7];
   yb = moments[8];

   //moments_get_base_point(moments,&x0,&y0);

  /* u2 and v2 are temporary variables to handle linear
   * rotation and strains.
   */
   u2      = x-xb;
   v2      = y-yb;

  /* Translations. */
   T[1][1] = 1;
   T[2][1] = 0;
   T[1][2] = 0;
   T[2][2] = 1;

  /* Linear rotations. */
   T[1][3] = -v2;
   T[2][3] = u2;

  /* Principal strains. */
   T[1][4] = u2;
   T[2][4] = 0;
   T[1][5] = 0;
   T[2][5] = v2;

  /* Shear strains. */
   T[1][6] = v2/2;
   T[2][6] = u2/2;
}





void

transplacement_finite(double * moments, double T[][7],
                      const double x1, const double x2) {

   double x1b, x2b;
   //double u2, v2;
   double tX1, tX2;

  /* i0 is block number                     */
  /* xb,yb: base point for directors        */
   //xb = moments[i0][2]/moments[i0][1];
   //yb = moments[i0][3]/moments[i0][1];
   x1b = moments[7];
   x2b = moments[8];
   //moments_get_base_point(moments,&xb,&yb);

   tX1 = x1-x1b;
   tX2 = x2-x2b;

  /* An amusing layout. */
   T[1][1] = 1;   T[1][2] = 0;   T[1][3] = tX1;   T[1][4] = 0;    T[1][5] = tX2;  T[1][6] = 0;
   T[2][1] = 0;   T[2][2] = 1;   T[2][3] = 0;    T[2][4] = tX1;   T[2][5] = 0;   T[2][6] = tX2;
}





void
transplacement_apply_linear(double T[][7], double * D,
                            double * u1, double * u2) {

   int j;

   *u1 = 0;
   *u2 = 0;

   for (j=1; j<= 6; j++) {
     *u1 += T[1][j]*D[j];
     *u2 += T[2][j]*D[j];
   }
}


void
transplacement_apply_2dorder(double T[][7], double * D,
                             double * u1, double * u2) {

   int j;

  *u1 = 0;
  *u2 = 0;

   for (j=1; j<= 6; j++) {

      if (j==3) {

         j++;
      }

     *u1 += T[1][j]*D[j];
     *u2 += T[2][j]*D[j];
   }

  *u1 += (-T[2][3]*D[3]*D[3]/2.0) + (T[1][3]*D[3]);
  *u2 += (T[2][3]*D[3]) + (T[1][3]*D[3]*D[3]/2.0);
}


void
transplacement_apply_exact(double T[][7], double * D,
                             double * u1, double * u2) {

   int j;

  *u1 = 0;
  *u2 = 0;

   for (j=1; j<= 6; j++) {

      if (j==3) {

         j++;
      }

     *u1 += T[1][j]*D[j];
     *u2 += T[2][j]*D[j];
   }

  *u1 += (T[2][3] * (cos(D[3])-1) + T[1][3]*sin(D[3]));
  *u2 += (T[2][3] * sin(D[3]) - T[1][3]*(cos(D[3])-1));
}




/** Build the mass matrix using Eq. 2.57, p. 85,
 * Chapter 2, Shi 1988.
 */
void
massmatrix_linear (double T[7][7], double m, const double S0,
                   const double S1, const double S2, const double S3) {

   T[1][1] = m*S0;
   T[2][2] = m*S0;
   T[3][3] = m*(S1+S2);
   T[3][4] = -m*S3;
   T[4][3] = T[3][4];
   T[3][5] = m*S3;
   T[5][3] = T[3][5];
   T[3][6] = m*((S1-S2)/2);
   T[6][3] = T[3][6];
   T[4][4] = m*S1;
   T[4][6] = m*(S3/2);
   T[6][4] = T[4][6];
   T[5][5] = m*S2;
   T[5][6] = m*(S3/2);
   T[6][5] = T[5][6];
   T[6][6] = m*((S1+S2)/4);

}



void
massmatrix_finite (double T[][7], double m, const double S0, const double S1,
                   const double S2, const double S3) {

   /* Diagonals */
   T[1][1] = m*S0;
   T[2][2] = m*S0;
   T[3][3] = m*S1;
   T[4][4] = m*S1;
   T[5][5] = m*S2;
   T[6][6] = m*S2;

   /* Off-diagonals */
   T[3][5] = m*S3;
   T[5][3] = m*S3;
   T[4][6] = m*S3;
   T[6][4] = m*S3;

}




/** This function depends on the structure of the
 * transplacement map, it should be passed into
 * the time integrator as a callback.
 */
double
energy_kinetic(const double * v, const double rho, const double S0,
               const double S1, const double S2, const double S3) {

   double ke;
   double d1, d2, d3, d4, d5, d6;
   double E33, E34, E35, E36, E44, E46, E55, E56, E66;

   d1 = v[1];
   d2 = v[2];
   d3 = v[3];
   d4 = v[4];
   d5 = v[5];
   d6 = v[6];
   E33 = (S1+S2);
   E44 = S1;
   E55 = S2;
   E66 = E33/4.0;
   E34 = -S3;
   E35 = S3;
   E36 = (S1-S2)/2.0;
   E46 = S3/2.0;
   E56 = S3/2.0;

   ke = rho*S0*( d1*d1 + d2*d2)
        + rho*((E33*d3*d3) + (E44*d4*d4) + (E55*d5*d5)
        + (E66*d6*d6) + (2*E34*d3*d4) + (2*E35*d3*d5)
        + (2*E36*d3*d6) + (2*E46*d4*d6) + (2*E56*d5*d6));

   return ke/2;
}



