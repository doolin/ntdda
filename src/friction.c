/* 
 * friction.c
 * 
 * $Author: doolin $
 * $Date: 2002/10/14 16:02:47 $
 * $Source: /cvsroot/dda/ntdda/src/friction.c,v $
 * $Revision: 1.5 $
 *
 */


#include <math.h>
#include <assert.h>

#include "analysis.h"




/* This function is written to provide a displacement dependent
 * friction angle for one and only one measured point.  The
 * point can be located in any block.  The results are valid
 * only under strict translation.  Rotation of measured point
 * will produce spurious results.
 */
/* BUG BUG BUG BUG
 * This frictional decay function provides 
 * a friction angle based on the value of the 
 * displacement of a measured point.  If there is 
 * no measured point, the call will produce spurious
 * results due to garbage in the over-allocated arrays.
 */
double
computeFriction(Geometrydata *gd, /*Analysisdata * ad,*/ int joint_type)
{ 
   static double phi;
   double x;
  /* probably better to dereference this in the calling 
   * function and pass the array in directly instead of 
   * making multiple dereferences.
   */
   double ** points = gd->points;
  
   double phi0 = 12;
   double phiR = 6;
   double k = 0.01;
  
  /* x, y are the cumulative displacements
   * FIXME: This next line of code sets the frictional 
   * decay from the x displacement of the first 
   * measured point.  This is hideously fragile,
   * non-extensible, and barely maintainable.
   */
   //x = fabs(points[gd->nFPoints+gd->nLPoints+1][7]);

  /* KLUDGE:  For a block on a plane inclined 30 degrees,
   * displacement needs to be parallel to the slope.
   */
   x = (fabs(points[gd->nFPoints+gd->nLPoints+1][7]))/cos(30.0*PI/180.0);

  
   phi = (phi0-phiR)*(exp(-k*x)) + phiR;

   return phi;

}  /* Close computeFriction() */


/* computeVFriction uses the Voight (1982) heat reduction model.
 * WARNING!!! The values used for initialization have not been checked
 * for consistency with the units used in the Vaoint geometry
 * file.
 */
/* FIXME: Don't need the analysis data here. */
double
computeVFriction(Geometrydata *gd, /*Analysisdata * ad,*/ int units, double mu0)
{ 
   static double phi;
   double x;
   double lambda;
  /* This value should either be changed or
   * passed in as a parameter.
   */
   double lambda0;

  /* kinetic friction coefficient */
   double mu_k;

  /* thickness of shearing zone. This is given in Voight 
   * in meters.  For Vaiont, we need it in feet.
   */
   double h;
   static double b = 0;
   double bnum, bdenom;
  /* porosity */
   double n0;
  /* porosity */
   double n;
  /* linear coefficient of thermal expansion */
   double alpha;
  /* linear compressibility */
   double beta;
  /* dpdt change in pressure wrt to temperature */
   double dpdt; /* units of bar, need to change units */
  /* change in pressure wrt to change in density */
   double dpdrho;
  /* initial density ??? */
   double rho0;
  /* density of rock, should be passed in */
   double rho_r; /* check this value */
  /* density of water */
   double rho_w;  /* units g/cm^3 */
  /* specific heat of rock */
   double Cr;
  /* specific heat of water */
   double Cw;
   
  /* cohesion, zero for now, probably always zero in 
   * Voight's model
   */
   double cohesion;

  /* probably better to dereference this in the calling 
   * function and pass the array in directly instead of 
   * making multiple dereferences.
   */
   double ** points = gd->points;

  /* TODO: This is a kludge because `b' does not change 
   * over the duration of a problem, and could (should)
   * be computed elsewhere as part of an initialization
   * sequence.  Anyway, once b is computed, this code no
   * longer needs to be evaluated.
   */
   if(!b)
   {
     /* System of units matter */
      //if (ad->units == si)
      if (units == si) {

         lambda0 = 0;
         h = 0.333;
         n0 = 0.2;
         n = 0.2;
         alpha = 0.000008;
         beta = 10000.0;
         dpdt = 15.0; /* units of bar, need to change units */
         dpdrho = 20000.0;
         rho0 = 2.4;
         rho_r = 2.4; /* check this value */
         rho_w = 1.0;  /* units g/cm^3 */
         Cr = 1000.0;
         Cw = 4200.0;
         cohesion = 0;
      }
      else  /* ad->units == english */
      {
        /* TODO: Set these all up for english units. */
         dda_display_warning("English units are not yet implemented");
      }

     /* Once the values are set according to the system
      * of units...
      */
      bnum = (dpdt - (alpha*rho0)*dpdrho)*mu0;
      bdenom = (rho_r*Cr*(1-n) + rho_w*Cw*n)*(1+(beta*rho0/n0)*dpdrho);
      assert(bnum > 0 && bdenom > 0);
      b = bnum/bdenom;
      //fprintf(fp.logfile,"Voight's b value: %.f\n", b);
   }

  /* x, y are the cumulative displacements, which 
   * computed in ???
   */
  /* Note that this assumes there is only 1 measured point
   * that is important in the problem, and that the x direction
   * displacement is the total, or at least only displacement 
   * that we care about.
   */
   x = fabs(points[gd->nFPoints+gd->nLPoints+1][7]);
 
   //fprintf(fp.logfile,"Disp x: %lf\n", x);

   assert(x>=0);
  
  /* Hardwire from Voight, 1982 */
   b = 0.000187;

   lambda = lambda0 + (1-lambda0)*(1-exp(-(b*x)/h));

  /* Equation 16 in Voight 1982 */
   mu_k  = (1-lambda)*tan(PI*mu0/180.0);   
   //fprintf(fp.logfile,"mu_k: %lf\n", mu_k);

   phi = atan(mu_k)*180.0/PI;
   //fprintf(fp.logfile,"phi: %lf\n", phi);

   assert(phi >= 0);

   return phi;

}  

