#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)        ( ((a)<0.0) ? -(a) : (a))
#define SIGN(a,b)          ( ((b)>=0) ? (a) : -(a) )
#include <math.h>
#define P_nint(x)      ((int)(x+0.5))

/* Subroutine */ int dlacon_(int *n, double *v, double *x, 
	int *isgn, double *est, int *kase)
{
/*  -- LAPACK auxiliary routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    DLACON estimates the 1-norm of a square, real matrix A.   
    Reverse communication is used for evaluating matrix-vector products. 
  

    Arguments   
    =========   

    N      (input) INTEGER   
           The order of the matrix.  N >= 1.   

    V      (workspace) DOUBLE PRECISION array, dimension (N)   
           On the final return, V = A*W,  where  EST = norm(V)/norm(W)   
           (W is not returned).   

    X      (input/output) DOUBLE PRECISION array, dimension (N)   
           On an intermediate return, X should be overwritten by   
                 A * X,   if KASE=1,   
                 A' * X,  if KASE=2,   
           and DLACON must be re-called with all the other parameters   
           unchanged.   

    ISGN   (workspace) INTEGER array, dimension (N)   

    EST    (output) DOUBLE PRECISION   
           An estimate (a lower bound) for norm(A).   

    KASE   (input/output) INTEGER   
           On the initial call to DLACON, KASE should be 0.   
           On an intermediate return, KASE will be 1 or 2, indicating   
           whether X should be overwritten by A * X  or A' * X.   
           On the final return from DLACON, KASE will again be 0.   

    Further Details   
    ======= =======   

    Contributed by Nick Higham, University of Manchester.   
    Originally named SONEST, dated March 16, 1988.   

    Reference: N.J. Higham, "FORTRAN codes for estimating the one-norm of 
  
    a real or complex matrix, with applications to condition estimation", 
  
    ACM Trans. Math. Soft., vol. 14, no. 4, pp. 381-396, December 1988.   

    ===================================================================== 
  


    
   Parameter adjustments   
       Function Body */
    /* Table of constant values */
    static int c__1 = 1;
    static double c_b11 = 1.;
    
    /* System generated locals */
    int i__1;
    double d__1;
    /* Builtin functions */
    /* Local variables */
    static int iter;
    static double temp;
    static int jump, i, j;
    extern double dasum_(int *, double *, int *);
    static int jlast;
    extern /* Subroutine */ int dcopy_(int *, double *, int *, 
	    double *, int *);
    extern int idamax_(int *, double *, int *);
    static double altsgn, estold;



#define ISGN(I) isgn[(I)-1]
#define X(I) x[(I)-1]
#define V(I) v[(I)-1]


    if (*kase == 0) {
	i__1 = *n;
	for (i = 1; i <= *n; ++i) {
	    X(i) = 1. / (double) (*n);
/* L10: */
	}
	*kase = 1;
	jump = 1;
	return 0;
    }

    switch (jump) {
	case 1:  goto L20;
	case 2:  goto L40;
	case 3:  goto L70;
	case 4:  goto L110;
	case 5:  goto L140;
    }

/*     ................ ENTRY   (JUMP = 1)   
       FIRST ITERATION.  X HAS BEEN OVERWRITTEN BY A*X. */

L20:
    if (*n == 1) {
	V(1) = X(1);
	*est = ABS(V(1));
/*        ... QUIT */
	goto L150;
    }
    *est = dasum_(n, &X(1), &c__1);

    i__1 = *n;
    for (i = 1; i <= *n; ++i) {
	X(i) = SIGN(c_b11, X(i));
	ISGN(i) = P_nint(X(i));
/* L30: */
    }
    *kase = 2;
    jump = 2;
    return 0;

/*     ................ ENTRY   (JUMP = 2)   
       FIRST ITERATION.  X HAS BEEN OVERWRITTEN BY TRANDPOSE(A)*X. */

L40:
    j = idamax_(n, &X(1), &c__1);
    iter = 2;

/*     MAIN LOOP - ITERATIONS 2,3,...,ITMAX. */

L50:
    i__1 = *n;
    for (i = 1; i <= *n; ++i) {
	X(i) = 0.;
/* L60: */
    }
    X(j) = 1.;
    *kase = 1;
    jump = 3;
    return 0;

/*     ................ ENTRY   (JUMP = 3)   
       X HAS BEEN OVERWRITTEN BY A*X. */

L70:
    dcopy_(n, &X(1), &c__1, &V(1), &c__1);
    estold = *est;
    *est = dasum_(n, &V(1), &c__1);
    i__1 = *n;
    for (i = 1; i <= *n; ++i) {
	d__1 = SIGN(c_b11, X(i));
	if (P_nint(d__1) != ISGN(i)) {
	    goto L90;
	}
/* L80: */
    }
/*     REPEATED SIGN VECTOR DETECTED, HENCE ALGORITHM HAS CONVERGED. */
    goto L120;

L90:
/*     TEST FOR CYCLING. */
    if (*est <= estold) {
	goto L120;
    }

    i__1 = *n;
    for (i = 1; i <= *n; ++i) {
	X(i) = SIGN(c_b11, X(i));
	ISGN(i) = P_nint(X(i));
/* L100: */
    }
    *kase = 2;
    jump = 4;
    return 0;

/*     ................ ENTRY   (JUMP = 4)   
       X HAS BEEN OVERWRITTEN BY TRANDPOSE(A)*X. */

L110:
    jlast = j;
    j = idamax_(n, &X(1), &c__1);
    if (X(jlast) != (d__1 = X(j), ABS(d__1)) && iter < 5) {
	++iter;
	goto L50;
    }

/*     ITERATION COMPLETE.  FINAL STAGE. */

L120:
    altsgn = 1.;
    i__1 = *n;
    for (i = 1; i <= *n; ++i) {
	X(i) = altsgn * ((double) (i - 1) / (double) (*n - 1) + 1.);
	altsgn = -altsgn;
/* L130: */
    }
    *kase = 1;
    jump = 5;
    return 0;

/*     ................ ENTRY   (JUMP = 5)   
       X HAS BEEN OVERWRITTEN BY A*X. */

L140:
    temp = dasum_(n, &X(1), &c__1) / (double) (*n * 3) * 2.;
    if (temp > *est) {
	dcopy_(n, &X(1), &c__1, &V(1), &c__1);
	*est = temp;
    }

L150:
    *kase = 0;
    return 0;

/*     End of DLACON */

} /* dlacon_ */

