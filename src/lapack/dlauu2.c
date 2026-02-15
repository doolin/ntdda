#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)        ( ((a)<0.0) ? -(a) : (a))
#include <math.h>

/* Subroutine */ int dlauu2_(char *uplo, int *n, double *a, int *
	lda, int *info)
{
/*  -- LAPACK auxiliary routine (version 2.0) --
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
       Courant Institute, Argonne National Lab, and Rice University
       February 29, 1992


    Purpose
    =======

    DLAUU2 computes the product U * U' or L' * L, where the triangular
    factor U or L is stored in the upper or lower triangular part of
    the array A.

    If UPLO = 'U' or 'u' then the upper triangle of the result is stored,

    overwriting the factor U in A.
    If UPLO = 'L' or 'l' then the lower triangle of the result is stored,

    overwriting the factor L in A.

    This is the unblocked form of the algorithm, calling Level 2 BLAS.

    Arguments
    =========

    UPLO    (input) CHARACTER*1
            Specifies whether the triangular factor stored in the array A

            is upper or lower triangular:
            = 'U':  Upper triangular
            = 'L':  Lower triangular

    N       (input) INTEGER
            The order of the triangular factor U or L.  N >= 0.

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
            On entry, the triangular factor U or L.
            On exit, if UPLO = 'U', the upper triangle of A is
            overwritten with the upper triangle of the product U * U';
            if UPLO = 'L', the lower triangle of A is overwritten with
            the lower triangle of the product L' * L.

    LDA     (input) INTEGER
            The leading dimension of the array A.  LDA >= MAX(1,N).

    INFO    (output) INTEGER
            = 0: successful exit
            < 0: if INFO = -k, the k-th argument had an illegal value

    =====================================================================



       Test the input parameters.


   Parameter adjustments
       Function Body */
    /* Table of constant values */
    static double c_b7 = 1.;
    static int c__1 = 1;

    /* System generated locals */
    int  i__1, i__2, i__3;
    /* Local variables */
    extern double ddot_(int *, double *, int *, double *,
	    int *);
    static int i;
    extern /* Subroutine */ int dscal_(int *, double *, double *,
	    int *);
    extern long int lsame_(char *, char *);
    extern /* Subroutine */ int dgemv_(char *, int *, int *,
	    double *, double *, int *, double *, int *,
	    double *, double *, int *);
    static long int upper;
    extern /* Subroutine */ int xerbla_(char *, int *);
    static double aii;




#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]

    *info = 0;
    upper = lsame_(uplo, "U");
    if (! upper && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < MAX(1,*n)) {
	*info = -4;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DLAUU2", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

    if (upper) {

/*        Compute the product U * U'. */

	i__1 = *n;
	for (i = 1; i <= *n; ++i) {
	    aii = A(i,i);
	    if (i < *n) {
		i__2 = *n - i + 1;
		A(i,i) = ddot_(&i__2, &A(i,i), lda, &A(i,i), lda);
		i__2 = i - 1;
		i__3 = *n - i;
		dgemv_("No transpose", &i__2, &i__3, &c_b7, &A(1,i+1), lda, &A(i,i+1), lda, &aii,
			 &A(1,i), &c__1);
	    } else {
		dscal_(&i, &aii, &A(1,i), &c__1);
	    }
/* L10: */
	}

    } else {

/*        Compute the product L' * L. */

	i__1 = *n;
	for (i = 1; i <= *n; ++i) {
	    aii = A(i,i);
	    if (i < *n) {
		i__2 = *n - i + 1;
		A(i,i) = ddot_(&i__2, &A(i,i), &c__1, &
			A(i,i), &c__1);
		i__2 = *n - i;
		i__3 = i - 1;
		dgemv_("Transpose", &i__2, &i__3, &c_b7, &A(i+1,1),
			lda, &A(i+1,i), &c__1, &aii, &A(i,1), lda);
	    } else {
		dscal_(&i, &aii, &A(i,1), lda);
	    }
/* L20: */
	}
    }

    return 0;

/*     End of DLAUU2 */

} /* dlauu2_ */

