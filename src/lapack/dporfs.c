#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)        ( ((a)<0.0) ? -(a) : (a))
#include <math.h>

/* Subroutine */ int dporfs_(char *uplo, int *n, int *nrhs, 
	double *a, int *lda, double *af, int *ldaf, 
	double *b, int *ldb, double *x, int *ldx, double *
	ferr, double *berr, double *work, int *iwork, int *
	info)
{
/*  -- LAPACK routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    DPORFS improves the computed solution to a system of linear   
    equations when the coefficient matrix is symmetric positive definite, 
  
    and provides error bounds and backward error estimates for the   
    solution.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            = 'U':  Upper triangle of A is stored;   
            = 'L':  Lower triangle of A is stored.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrices B and X.  NRHS >= 0.   

    A       (input) DOUBLE PRECISION array, dimension (LDA,N)   
            The symmetric matrix A.  If UPLO = 'U', the leading N-by-N   
            upper triangular part of A contains the upper triangular part 
  
            of the matrix A, and the strictly lower triangular part of A 
  
            is not referenced.  If UPLO = 'L', the leading N-by-N lower   
            triangular part of A contains the lower triangular part of   
            the matrix A, and the strictly upper triangular part of A is 
  
            not referenced.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= MAX(1,N).   

    AF      (input) DOUBLE PRECISION array, dimension (LDAF,N)   
            The triangular factor U or L from the Cholesky factorization 
  
            A = U**T*U or A = L*L**T, as computed by DPOTRF.   

    LDAF    (input) INTEGER   
            The leading dimension of the array AF.  LDAF >= MAX(1,N).   

    B       (input) DOUBLE PRECISION array, dimension (LDB,NRHS)   
            The right hand side matrix B.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= MAX(1,N).   

    X       (input/output) DOUBLE PRECISION array, dimension (LDX,NRHS)   
            On entry, the solution matrix X, as computed by DPOTRS.   
            On exit, the improved solution matrix X.   

    LDX     (input) INTEGER   
            The leading dimension of the array X.  LDX >= MAX(1,N).   

    FERR    (output) DOUBLE PRECISION array, dimension (NRHS)   
            The estimated forward error bound for each solution vector   
            X(j) (the j-th column of the solution matrix X).   
            If XTRUE is the true solution corresponding to X(j), FERR(j) 
  
            is an estimated upper bound for the magnitude of the largest 
  
            element in (X(j) - XTRUE) divided by the magnitude of the   
            largest element in X(j).  The estimate is as reliable as   
            the estimate for RCOND, and is almost always a slight   
            overestimate of the true error.   

    BERR    (output) DOUBLE PRECISION array, dimension (NRHS)   
            The componentwise relative backward error of each solution   
            vector X(j) (i.e., the smallest relative change in   
            any element of A or B that makes X(j) an exact solution).   

    WORK    (workspace) DOUBLE PRECISION array, dimension (3*N)   

    IWORK   (workspace) INTEGER array, dimension (N)   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    Internal Parameters   
    ===================   

    ITMAX is the maximum number of steps of iterative refinement.   

    ===================================================================== 
  


       Test the input parameters.   

    
   Parameter adjustments   
       Function Body */
    /* Table of constant values */
    static int c__1 = 1;
    static double c_b12 = -1.;
    static double c_b14 = 1.;
    
    /* System generated locals */
    int  i__1, i__2, i__3;
    double d__1, d__2, d__3;
    /* Local variables */
    static int kase;
    static double safe1, safe2;
    static int i, j, k;
    static double s;
    extern long int lsame_(char *, char *);
    extern /* Subroutine */ int dcopy_(int *, double *, int *, 
	    double *, int *), daxpy_(int *, double *, 
	    double *, int *, double *, int *);
    static int count;
    static long int upper;
    extern /* Subroutine */ int dsymv_(char *, int *, double *, 
	    double *, int *, double *, int *, double *, 
	    double *, int *);
    extern double dlamch_(char *);
    extern /* Subroutine */ int dlacon_(int *, double *, double *,
	     int *, double *, int *);
    static double xk;
    static int nz;
    static double safmin;
    extern /* Subroutine */ int xerbla_(char *, int *), dpotrs_(
	    char *, int *, int *, double *, int *, double 
	    *, int *, int *);
    static double lstres, eps;



#define FERR(I) ferr[(I)-1]
#define BERR(I) berr[(I)-1]
#define WORK(I) work[(I)-1]
#define IWORK(I) iwork[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]
#define AF(I,J) af[(I)-1 + ((J)-1)* ( *ldaf)]
#define B(I,J) b[(I)-1 + ((J)-1)* ( *ldb)]
#define X(I,J) x[(I)-1 + ((J)-1)* ( *ldx)]

    *info = 0;
    upper = lsame_(uplo, "U");
    if (! upper && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*nrhs < 0) {
	*info = -3;
    } else if (*lda < MAX(1,*n)) {
	*info = -5;
    } else if (*ldaf < MAX(1,*n)) {
	*info = -7;
    } else if (*ldb < MAX(1,*n)) {
	*info = -9;
    } else if (*ldx < MAX(1,*n)) {
	*info = -11;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DPORFS", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0 || *nrhs == 0) {
	i__1 = *nrhs;
	for (j = 1; j <= *nrhs; ++j) {
	    FERR(j) = 0.;
	    BERR(j) = 0.;
/* L10: */
	}
	return 0;
    }

/*     NZ = maximum number of nonzero elements in each row of A, plus 1 */

    nz = *n + 1;
    eps = dlamch_("Epsilon");
    safmin = dlamch_("Safe minimum");
    safe1 = nz * safmin;
    safe2 = safe1 / eps;

/*     Do for each right hand side */

    i__1 = *nrhs;
    for (j = 1; j <= *nrhs; ++j) {

	count = 1;
	lstres = 3.;
L20:

/*        Loop until stopping criterion is satisfied.   

          Compute residual R = B - A * X */

	dcopy_(n, &B(1,j), &c__1, &WORK(*n + 1), &c__1);
	dsymv_(uplo, n, &c_b12, &A(1,1), lda, &X(1,j), &c__1, 
		&c_b14, &WORK(*n + 1), &c__1);

/*        Compute componentwise relative backward error from formula 
  

          MAX(i) ( ABS(R(i)) / ( ABS(A)*ABS(X) + ABS(B) )(i) )   

          where ABS(Z) is the componentwise absolute value of the matr
ix   
          or vector Z.  If the i-th component of the denominator is le
ss   
          than SAFE2, then SAFE1 is added to the i-th components of th
e   
          numerator and denominator before dividing. */

	i__2 = *n;
	for (i = 1; i <= *n; ++i) {
	    WORK(i) = (d__1 = B(i,j), ABS(d__1));
/* L30: */
	}

/*        Compute ABS(A)*ABS(X) + ABS(B). */

	if (upper) {
	    i__2 = *n;
	    for (k = 1; k <= *n; ++k) {
		s = 0.;
		xk = (d__1 = X(k,j), ABS(d__1));
		i__3 = k - 1;
		for (i = 1; i <= k-1; ++i) {
		    WORK(i) += (d__1 = A(i,k), ABS(d__1)) * xk;
		    s += (d__1 = A(i,k), ABS(d__1)) * (d__2 = X(i,j), ABS(d__2));
/* L40: */
		}
		WORK(k) = WORK(k) + (d__1 = A(k,k), ABS(d__1)) * 
			xk + s;
/* L50: */
	    }
	} else {
	    i__2 = *n;
	    for (k = 1; k <= *n; ++k) {
		s = 0.;
		xk = (d__1 = X(k,j), ABS(d__1));
		WORK(k) += (d__1 = A(k,k), ABS(d__1)) * xk;
		i__3 = *n;
		for (i = k + 1; i <= *n; ++i) {
		    WORK(i) += (d__1 = A(i,k), ABS(d__1)) * xk;
		    s += (d__1 = A(i,k), ABS(d__1)) * (d__2 = X(i,j), ABS(d__2));
/* L60: */
		}
		WORK(k) += s;
/* L70: */
	    }
	}
	s = 0.;
	i__2 = *n;
	for (i = 1; i <= *n; ++i) {
	    if (WORK(i) > safe2) {
/* Computing MAX */
		d__2 = s, d__3 = (d__1 = WORK(*n + i), ABS(d__1)) / WORK(i);
		s = MAX(d__2,d__3);
	    } else {
/* Computing MAX */
		d__2 = s, d__3 = ((d__1 = WORK(*n + i), ABS(d__1)) + safe1) / 
			(WORK(i) + safe1);
		s = MAX(d__2,d__3);
	    }
/* L80: */
	}
	BERR(j) = s;

/*        Test stopping criterion. Continue iterating if   
             1) The residual BERR(J) is larger than machine epsilon, a
nd   
             2) BERR(J) decreased by at least a factor of 2 during the
   
                last iteration, and   
             3) At most ITMAX iterations tried. */

	if (BERR(j) > eps && BERR(j) * 2. <= lstres && count <= 5) {

/*           Update solution and try again. */

	    dpotrs_(uplo, n, &c__1, &AF(1,1), ldaf, &WORK(*n + 1), n, 
		    info);
	    daxpy_(n, &c_b14, &WORK(*n + 1), &c__1, &X(1,j), &c__1)
		    ;
	    lstres = BERR(j);
	    ++count;
	    goto L20;
	}

/*        Bound error from formula   

          norm(X - XTRUE) / norm(X) .le. FERR =   
          norm( ABS(inv(A))*   
             ( ABS(R) + NZ*EPS*( ABS(A)*ABS(X)+ABS(B) ))) / norm(X)   

          where   
            norm(Z) is the magnitude of the largest component of Z   
            inv(A) is the inverse of A   
            ABS(Z) is the componentwise absolute value of the matrix o
r   
               vector Z   
            NZ is the maximum number of nonzeros in any row of A, plus
 1   
            EPS is machine epsilon   

          The i-th component of ABS(R)+NZ*EPS*(ABS(A)*ABS(X)+ABS(B)) 
  
          is incremented by SAFE1 if the i-th component of   
          ABS(A)*ABS(X) + ABS(B) is less than SAFE2.   

          Use DLACON to estimate the infinity-norm of the matrix   
             inv(A) * diag(W),   
          where W = ABS(R) + NZ*EPS*( ABS(A)*ABS(X)+ABS(B) ))) */

	i__2 = *n;
	for (i = 1; i <= *n; ++i) {
	    if (WORK(i) > safe2) {
		WORK(i) = (d__1 = WORK(*n + i), ABS(d__1)) + nz * eps * WORK(
			i);
	    } else {
		WORK(i) = (d__1 = WORK(*n + i), ABS(d__1)) + nz * eps * WORK(
			i) + safe1;
	    }
/* L90: */
	}

	kase = 0;
L100:
	dlacon_(n, &WORK((*n << 1) + 1), &WORK(*n + 1), &IWORK(1), &FERR(j), &
		kase);
	if (kase != 0) {
	    if (kase == 1) {

/*              Multiply by diag(W)*inv(A'). */

		dpotrs_(uplo, n, &c__1, &AF(1,1), ldaf, &WORK(*n + 1), 
			n, info);
		i__2 = *n;
		for (i = 1; i <= *n; ++i) {
		    WORK(*n + i) = WORK(i) * WORK(*n + i);
/* L110: */
		}
	    } else if (kase == 2) {

/*              Multiply by inv(A)*diag(W). */

		i__2 = *n;
		for (i = 1; i <= *n; ++i) {
		    WORK(*n + i) = WORK(i) * WORK(*n + i);
/* L120: */
		}
		dpotrs_(uplo, n, &c__1, &AF(1,1), ldaf, &WORK(*n + 1), 
			n, info);
	    }
	    goto L100;
	}

/*        Normalize error. */

	lstres = 0.;
	i__2 = *n;
	for (i = 1; i <= *n; ++i) {
/* Computing MAX */
	    d__2 = lstres, d__3 = (d__1 = X(i,j), ABS(d__1));
	    lstres = MAX(d__2,d__3);
/* L130: */
	}
	if (lstres != 0.) {
	    FERR(j) /= lstres;
	}

/* L140: */
    }

    return 0;

/*     End of DPORFS */

} /* dporfs_ */

