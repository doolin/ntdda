#include <math.h>

#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )

/* Subroutine */ int dpotrs_(char *uplo, int *n, int *nrhs,
	double *a, int *lda, double *b, int *ldb, int *
	info)
{
/*  -- LAPACK routine (version 2.0) --
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
       Courant Institute, Argonne National Lab, and Rice University
       March 31, 1993


    Purpose
    =======

    DPOTRS solves a system of linear equations A*X = B with a symmetric
    positive definite matrix A using the Cholesky factorization
    A = U**T*U or A = L*L**T computed by DPOTRF.

    Arguments
    =========

    UPLO    (input) CHARACTER*1
            = 'U':  Upper triangle of A is stored;
            = 'L':  Lower triangle of A is stored.

    N       (input) INT
            The order of the matrix A.  N >= 0.

    NRHS    (input) INT
            The number of right hand sides, i.e., the number of columns
            of the matrix B.  NRHS >= 0.

    A       (input) DOUBLE PRECISION array, dimension (LDA,N)
            The triangular factor U or L from the Cholesky factorization

            A = U**T*U or A = L*L**T, as computed by DPOTRF.

    LDA     (input) INT
            The leading dimension of the array A.  LDA >= MAX(1,N).

    B       (input/output) DOUBLE PRECISION array, dimension (LDB,NRHS)
            On entry, the right hand side matrix B.
            On exit, the solution matrix X.

    LDB     (input) INT
            The leading dimension of the array B.  LDB >= MAX(1,N).

    INFO    (output) INT
            = 0:  successful exit
            < 0:  if INFO = -i, the i-th argument had an illegal value

    =====================================================================



       Test the input parameters.


   Parameter adjustments
       Function Body */
    /* Table of constant values */
    static double c_b9 = 1.;

    /* System generated locals */
    int  i__1;
    /* Local variables */
    extern long int lsame_(char *, char *);
    extern /* Subroutine */ int dtrsm_(char *, char *, char *, char *,
	    int *, int *, double *, double *, int *,
	    double *, int *);
    static long int upper;
    extern /* Subroutine */ int xerbla_(char *, int *);




#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]
#define B(I,J) b[(I)-1 + ((J)-1)* ( *ldb)]

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
    } else if (*ldb < MAX(1,*n)) {
	*info = -7;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DPOTRS", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0 || *nrhs == 0) {
	return 0;
    }

    if (upper) {

/*        Solve A*X = B where A = U'*U.

          Solve U'*X = B, overwriting B with X. */

	dtrsm_("Left", "Upper", "Transpose", "Non-unit", n, nrhs, &c_b9, &A(1,1), lda, &B(1,1), ldb);

/*        Solve U*X = B, overwriting B with X. */

	dtrsm_("Left", "Upper", "No transpose", "Non-unit", n, nrhs, &c_b9, &
		A(1,1), lda, &B(1,1), ldb);
    } else {

/*        Solve A*X = B where A = L*L'.

          Solve L*X = B, overwriting B with X. */

	dtrsm_("Left", "Lower", "No transpose", "Non-unit", n, nrhs, &c_b9, &
		A(1,1), lda, &B(1,1), ldb);

/*        Solve L'*X = B, overwriting B with X. */

	dtrsm_("Left", "Lower", "Transpose", "Non-unit", n, nrhs, &c_b9, &A(1,1), lda, &B(1,1), ldb);
    }

    return 0;

/*     End of DPOTRS */

} /* dpotrs_ */

