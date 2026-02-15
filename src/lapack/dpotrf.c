#include <math.h>

#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )

/* Subroutine */ int dpotrf_(char *uplo, int *n, double *a, int *
	lda, int *info)
{
/*  -- LAPACK routine (version 2.0) --
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
       Courant Institute, Argonne National Lab, and Rice University
       March 31, 1993


    Purpose
    =======

    DPOTRF computes the Cholesky factorization of a real symmetric
    positive definite matrix A.

    The factorization has the form
       A = U**T * U,  if UPLO = 'U', or
       A = L  * L**T,  if UPLO = 'L',
    where U is an upper triangular matrix and L is lower triangular.

    This is the block version of the algorithm, calling Level 3 BLAS.

    Arguments
    =========

    UPLO    (input) CHARACTER*1
            = 'U':  Upper triangle of A is stored;
            = 'L':  Lower triangle of A is stored.

    N       (input) INT
            The order of the matrix A.  N >= 0.

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
            On entry, the symmetric matrix A.  If UPLO = 'U', the leading

            N-by-N upper triangular part of A contains the upper
            triangular part of the matrix A, and the strictly lower
            triangular part of A is not referenced.  If UPLO = 'L', the
            leading N-by-N lower triangular part of A contains the lower

            triangular part of the matrix A, and the strictly upper
            triangular part of A is not referenced.

            On exit, if INFO = 0, the factor U or L from the Cholesky
            factorization A = U**T*U or A = L*L**T.

    LDA     (input) INT
            The leading dimension of the array A.  LDA >= MAX(1,N).

    INFO    (output) INT
            = 0:  successful exit
            < 0:  if INFO = -i, the i-th argument had an illegal value
            > 0:  if INFO = i, the leading minor of order i is not
                  positive definite, and the factorization could not be
                  completed.

    =====================================================================



       Test the input parameters.


   Parameter adjustments
       Function Body */
    /* Table of constant values */
    static int c__1 = 1;
    static int c_n1 = -1;
    static double c_b13 = -1.;
    static double c_b14 = 1.;

    /* System generated locals */
    int  i__1, i__2, i__3, i__4;
    /* Local variables */
    static int j;
    extern /* Subroutine */ int dgemm_(char *, char *, int *, int *,
	    int *, double *, double *, int *, double *,
	    int *, double *, double *, int *);
    extern long int lsame_(char *, char *);
    extern /* Subroutine */ int dtrsm_(char *, char *, char *, char *,
	    int *, int *, double *, double *, int *,
	    double *, int *);
    static long int upper;
    extern /* Subroutine */ int dsyrk_(char *, char *, int *, int *,
	    double *, double *, int *, double *, double *,
	     int *), dpotf2_(char *, int *,
	    double *, int *, int *);
    static int jb, nb;
    extern /* Subroutine */ int xerbla_(char *, int *);
    extern int ilaenv_(int *, char *, char *, int *, int *,
	    int *, int *, long int, long int);




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
	xerbla_("DPOTRF", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Determine the block size for this environment. */

    nb = ilaenv_(&c__1, "DPOTRF", uplo, n, &c_n1, &c_n1, &c_n1, 6L, 1L);
    if (nb <= 1 || nb >= *n) {

/*        Use unblocked code. */

	dpotf2_(uplo, n, &A(1,1), lda, info);
    } else {

/*        Use blocked code. */

	if (upper) {

/*           Compute the Cholesky factorization A = U'*U. */

	    i__1 = *n;
	    i__2 = nb;
	    for (j = 1; nb < 0 ? j >= *n : j <= *n; j += nb) {

/*              Update and factorize the current diagonal bloc
k and test
                for non-positive-definiteness.

   Computing MIN */
		i__3 = nb, i__4 = *n - j + 1;
		jb = MIN(i__3,i__4);
		i__3 = j - 1;
		dsyrk_("Upper", "Transpose", &jb, &i__3, &c_b13, &A(1,j), lda, &c_b14, &A(j,j), lda);
		dpotf2_("Upper", &jb, &A(j,j), lda, info);
		if (*info != 0) {
		    goto L30;
		}
		if (j + jb <= *n) {

/*                 Compute the current block row. */

		    i__3 = *n - j - jb + 1;
		    i__4 = j - 1;
		    dgemm_("Transpose", "No transpose", &jb, &i__3, &i__4, &
			    c_b13, &A(1,j), lda, &A(1,j+jb), lda, &c_b14, &A(j,j+jb), lda);
		    i__3 = *n - j - jb + 1;
		    dtrsm_("Left", "Upper", "Transpose", "Non-unit", &jb, &
			    i__3, &c_b14, &A(j,j), lda, &A(j,j+jb), lda);
		}
/* L10: */
	    }

	} else {

/*           Compute the Cholesky factorization A = L*L'. */

	    i__2 = *n;
	    i__1 = nb;
	    for (j = 1; nb < 0 ? j >= *n : j <= *n; j += nb) {

/*              Update and factorize the current diagonal bloc
k and test
                for non-positive-definiteness.

   Computing MIN */
		i__3 = nb, i__4 = *n - j + 1;
		jb = MIN(i__3,i__4);
		i__3 = j - 1;
		dsyrk_("Lower", "No transpose", &jb, &i__3, &c_b13, &A(j,1), lda, &c_b14, &A(j,j), lda);
		dpotf2_("Lower", &jb, &A(j,j), lda, info);
		if (*info != 0) {
		    goto L30;
		}
		if (j + jb <= *n) {

/*                 Compute the current block column. */

		    i__3 = *n - j - jb + 1;
		    i__4 = j - 1;
		    dgemm_("No transpose", "Transpose", &i__3, &jb, &i__4, &
			    c_b13, &A(j+jb,1), lda, &A(j,1),
			    lda, &c_b14, &A(j+jb,j), lda);
		    i__3 = *n - j - jb + 1;
		    dtrsm_("Right", "Lower", "Transpose", "Non-unit", &i__3, &
			    jb, &c_b14, &A(j,j), lda, &A(j+jb,j), lda);
		}
/* L20: */
	    }
	}
    }
    goto L40;

L30:
    *info = *info + j - 1;

L40:
    return 0;

/*     End of DPOTRF */

} /* dpotrf_ */

