
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mohrcoulomb.h"

/* sigm:      mean stress invariant
 * dsbar:     deviatoric stress invariant
 * fnew or f: value of failure function
 * phi:       friction angle
 * c:         cohesion
 * psi:       dilation angle
 * tnph:      tan(phi)
 * snph:      sin(phi) 
 * theta:     lode angle
 */
  
double sigm,dsbar,theta,cs[5],pl[5][5];

int i,j;

/*
 *  first invariant "sigm", second invariant "dsbar" 
 *  and lode angle "theta" for a stress tensor "stress"
 */ 
void invar()
{
  double sx,sy,sz,txy,dx,dy,dz,xj3,sine;

  sx = cs[1];
  sy = cs[2];
  txy = cs[3];
  sz = cs[4];
  sigm = (sx+sy+sz)/3;
  dsbar = sqrt(pow((sx-sy),2)+pow((sy-sz),2)+pow((sz-sx),2)+6.*pow(txy,2))/sqrt(2.);
  if(dsbar < 1.e-10)
  {
     theta = .0;
  }
  else
  {
     dx = (2*sx-sy-sz)/3;
	 dy = (2*sy-sz-sx)/3;
	 dz = (2*sz-sx-sy)/3;
     xj3 = dx*dy*dz-dz*pow(txy,2);
     sine = -13.5*xj3/pow(dsbar,3);
     if(sine > 1) sine = 1;
     if(sine < -1) sine = -1;
     theta = asin(sine)/3;
  }

}

/*
 * Mohr-Coulomb yield function F from invariants "sigm"
 * and "dsbar" lode angle "theta (radian) of friction "phi" 
 * and cohesion "c"
 */
double mocouf(double c,double phi)
{  
  double phir,snph,csph,csth,snth,f;
  phir = phi*4*atan(1)/180;
  snph = sin(phir);
  csph = cos(phir);
  csth = cos(theta);
  snth = sin(theta);
  f = snph*sigm+dsbar*(csth/sqrt(3.)-snth*snph/3.)-c*csph;
  return f;
}

/*
 * plastic stress-strain matrix "pl" from stresses "stress" 
 * angle of friction "phi", dilation "psi" and elastic stiffness "E"
 * ,"nu" for a Mohr-Coulomb material
 */
void mocopl(double phi,double psi,double nu, double ymod)
{

   double row[5],col[5],sx,sy,txy,sz,pi,phir,psir;
   double dx,dy,dz,d2,d3,th,snth,sig,rph,rps,cps,snps,sq3;
   double cc,cph,alp,ca,sa,dd,snph,ee,s1,s2;

   sx = cs[1];
   sy = cs[2];
   txy = cs[3];
   sz = cs[4];
   pi = 4*atan(1);
   phir = phi*pi/180;
   psir = psi*pi/180;
   snph = sin(phir);
   snps = sin(psir);
   sq3 = sqrt(3);
   cc = 1-2*nu;
   dx = (2*sx-sy-sz)/3;
   dy = (2*sy-sz-sx)/3;
   dz = (2*sz-sx-sy)/3;
   d2 = sqrt(-dx*dy-dy*dz-dz*dx+txy*txy);
   d3 = dx*dy*dz-dz*txy*txy;
   th = -3*sq3*d3/(2*pow(d2,3));
   if(th > 1)th = 1;
   if(th < -1)th = -1;
   th = asin(th)/3;
   snth = sin(th);
   if(fabs(snth) > 0.49)
   {
      sig = -1;
      if(snth < 0)sig = 1;
      rph = snph*(1+nu)/3;
	  rps = snps*(1.+nu)/3;
	  cps = 0.25*sq3/d2*(1+sig*snps/3);
      cph = 0.25*sq3/d2*(1+sig*snph/3);

      col[1] = rph+cph*((1-nu)*dx+nu*(dy+dz));
	  col[2] = rph+cph*((1-nu)*dy+nu*(dz+dx));
      col[3] = cph*cc*txy;
	  col[4] = rph+cph*((1-nu)*dz+nu*(dx+dy));

      row[1] = rps+cps*((1-nu)*dx+nu*(dy+dz));
	  row[2] = rps+cps*((1-nu)*dy+nu*(dz+dx));
      row[3] = cps*cc*txy;
	  row[4] = rps+cps*((1-nu)*dz+nu*(dx+dy));

      ee = ymod/((1+nu)*cc*(rph*snps+2*cph*cps*d2*d2*cc));
   }
  else
  {
      alp = atan(fabs((sx-sy)/(2*txy)));
	  ca = cos(alp);
	  sa = sin(alp);
      dd = cc*sa;
	  s1=1;
	  s2=1;
      if((sx-sy) < 0)s1 = -1;
      if(txy < 0)s2 = -1;
      col[1] = snph+s1*dd;
	  col[2] = snph-s1*dd;
	  col[3] = s2*cc*ca;
	  col[4] = 2*nu*snph;

      row[1] = snps+s1*dd;
	  row[2] = snps-s1*dd; 
	  row[3] = s2*cc*ca;
	  row[4] = 2*nu*snps;

      ee = ymod/(2*(1.+nu)*cc*(snph*snps+cc));
  }
  for(i=1; i<=4; i++)
  {
	  for(j=1; j<=4; j++)
	  {
		  pl[i][j] = ee*row[i]*col[j];
	  }
  }
}

/*
 * main function in order to obtain the 
 * correct stress using Mohr-Coulomb criteria
 */
void mohrcoulomb(double c,double phi,double psi,double nu,double ymod,
				 double eps[4],double * stress,double sigma[4])
{

	 double elso[5];
     double fnew;

	 for(i = 1; i <= 4; i++) elso[i] = 0;
	 for(i = 1; i <= 4; i++) cs[i] = stress[i+3] + sigma[i];

     invar();                            
     fnew = mocouf(c,phi);

	 if(fnew > 0)
	 {
      mocopl(phi,psi,nu,ymod);

	  elso[1] = pl[1][1] * eps[1] + pl[1][2] * eps[2] + pl[1][3] * eps[3];
      elso[2] = pl[2][1] * eps[1] + pl[2][2] * eps[2] + pl[2][3] * eps[3];
      elso[3] = pl[3][1] * eps[1] + pl[3][2] * eps[2] + pl[3][3] * eps[3];
      elso[4] = pl[4][1] * eps[1] + pl[4][2] * eps[2] + pl[4][3] * eps[3];
	 }
     
	 for(i = 1; i <= 4; i++) stress[i+3] = stress[i+3] + sigma[i] - elso[i];
}
