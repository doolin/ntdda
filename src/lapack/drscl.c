#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)        ( ((a)<0.0) ? -(a) : (a))
#include <math.h>

/* Subroutine */ int drscl_(int *n, double *sa, double *sx, 
	int *incx)
{
/*  -- LAPACK auxiliary routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    DRSCL multiplies an n-element real vector x by the real scalar 1/a.   
    This is done without overflow or underflow as long as   
    the final result x/a does not overflow or underflow.   

    Arguments   
    =========   

    N       (input) INTEGER   
            The number of components of the vector x.   

    SA      (input) DOUBLE PRECISION   
            The scalar a which is used to divide each component of x.   
            SA must be >= 0, or the subroutine will divide by zero.   

    SX      (input/output) DOUBLE PRECISION array, dimension   
                           (1+(N-1)*ABS(INCX))   
            The n-element vector x.   

    INCX    (input) INTEGER   
            The increment between successive values of the vector SX.   
            > 0:  SX(1) = X(1) and SX(1+(i-1)*INCX) = x(i),     1< i<= n 
  

   ===================================================================== 
  


       Quick return if possible   

    
   Parameter adjustments   
       Function Body */
    static double cden;
    static long int done;
    static double cnum, cden1, cnum1;
    extern /* Subroutine */ int dscal_(int *, double *, double *, 
	    int *), dlabad_(double *, double *);
    extern double dlamch_(char *);
    static double bignum, smlnum, mul;


#define SX(I) sx[(I)-1]


    if (*n <= 0) {
	return 0;
    }

/*     Get machine parameters */

    smlnum = dlamch_("S");
    bignum = 1. / smlnum;
    dlabad_(&smlnum, &bignum);

/*     Initialize the denominator to SA and the numerator to 1. */

    cden = *sa;
    cnum = 1.;

L10:
    cden1 = cden * smlnum;
    cnum1 = cnum / bignum;
    if (ABS(cden1) > ABS(cnum) && cnum != 0.) {

/*        Pre-multiply X by SMLNUM if CDEN is large compared to CNUM. 
*/

	mul = smlnum;
	done = 0;
	cden = cden1;
    } else if (ABS(cnum1) > ABS(cden)) {

/*        Pre-multiply X by BIGNUM if CDEN is small compared to CNUM. 
*/

	mul = bignum;
	done = 0;
	cnum = cnum1;
    } else {

/*        Multiply X by CNUM / CDEN and return. */

	mul = cnum / cden;
	done = 1;
    }

/*     Scale the vector X by MUL */

    dscal_(n, &mul, &SX(1), incx);

    if (! done) {
	goto L10;
    }

    return 0;

/*     End of DRSCL */

} /* drscl_ */

