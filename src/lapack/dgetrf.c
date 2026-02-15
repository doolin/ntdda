#include <math.h>
#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )

/* Subroutine */ int dgetrf_(int *m, int *n, double *a, int *
	lda, int *ipiv, int *info)
{
/*  -- LAPACK routine (version 2.0) --
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
       Courant Institute, Argonne National Lab, and Rice University
       March 31, 1993


    Purpose
    =======

    DGETRF computes an LU factorization of a general M-by-N matrix A
    using partial pivoting with row interchanges.

    The factorization has the form
       A = P * L * U
    where P is a permutation matrix, L is lower triangular with unit
    diagonal elements (lower trapezoidal if m > n), and U is upper
    triangular (upper trapezoidal if m < n).

    This is the right-looking Level 3 BLAS version of the algorithm.

    Arguments
    =========

    M       (input) INT
            The number of rows of the matrix A.  M >= 0.

    N       (input) INT
            The number of columns of the matrix A.  N >= 0.

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
            On entry, the M-by-N matrix to be factored.
            On exit, the factors L and U from the factorization
            A = P*L*U; the unit diagonal elements of L are not stored.

    LDA     (input) INT
            The leading dimension of the array A.  LDA >= MAX(1,M).

    IPIV    (output) INT array, dimension (MIN(M,N))
            The pivot indices; for 1 <= i <= MIN(M,N), row i of the
            matrix was interchanged with row IPIV(i).

    INFO    (output) INT
            = 0:  successful exit
            < 0:  if INFO = -i, the i-th argument had an illegal value
            > 0:  if INFO = i, U(i,i) is exactly zero. The factorization

                  has been completed, but the factor U is exactly
                  singular, and division by zero will occur if it is used

                  to solve a system of equations.

    =====================================================================



       Test the input parameters.


   Parameter adjustments
       Function Body */
    /* Table of constant values */
    static int c__1 = 1;
    static int c_n1 = -1;
    static double c_b16 = 1.;
    static double c_b19 = -1.;

    /* System generated locals */
    int  i__1, i__2, i__3, i__4, i__5;
    /* Local variables */
    static int i, j;
    extern /* Subroutine */ int dgemm_(char *, char *, int *, int *,
	    int *, double *, double *, int *, double *,
	    int *, double *, double *, int *);
    static int iinfo;
    extern /* Subroutine */ int dtrsm_(char *, char *, char *, char *,
	    int *, int *, double *, double *, int *,
	    double *, int *), dgetf2_(
	    int *, int *, double *, int *, int *, int
	    *);
    static int jb, nb;
    extern /* Subroutine */ int xerbla_(char *, int *);
    extern int ilaenv_(int *, char *, char *, int *, int *,
	    int *, int *, long int, long int);
    extern /* Subroutine */ int dlaswp_(int *, double *, int *,
	    int *, int *, int *, int *);



#define IPIV(I) ipiv[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]

    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < MAX(1,*m)) {
	*info = -4;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DGETRF", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*m == 0 || *n == 0) {
	return 0;
    }

/*     Determine the block size for this environment. */

    nb = ilaenv_(&c__1, "DGETRF", " ", m, n, &c_n1, &c_n1, 6L, 1L);
    if (nb <= 1 || nb >= MIN(*m,*n)) {

/*        Use unblocked code. */

	dgetf2_(m, n, &A(1,1), lda, &IPIV(1), info);
    } else {

/*        Use blocked code. */

	i__1 = MIN(*m,*n);
	i__2 = nb;
	for (j = 1; nb < 0 ? j >= MIN(*m,*n) : j <= MIN(*m,*n); j += nb) {
/* Computing MIN */
	    i__3 = MIN(*m,*n) - j + 1;
	    jb = MIN(i__3,nb);

/*           Factor diagonal and subdiagonal blocks and test for e
xact
             singularity. */

	    i__3 = *m - j + 1;
	    dgetf2_(&i__3, &jb, &A(j,j), lda, &IPIV(j), &iinfo);

/*           Adjust INFO and the pivot indices. */

	    if (*info == 0 && iinfo > 0) {
		*info = iinfo + j - 1;
	    }
/* Computing MIN */
	    i__4 = *m, i__5 = j + jb - 1;
	    i__3 = MIN(i__4,i__5);
	    for (i = j; i <= MIN(*m,j+jb-1); ++i) {
		IPIV(i) = j - 1 + IPIV(i);
/* L10: */
	    }

/*           Apply interchanges to columns 1:J-1. */

	    i__3 = j - 1;
	    i__4 = j + jb - 1;
	    dlaswp_(&i__3, &A(1,1), lda, &j, &i__4, &IPIV(1), &c__1);

	    if (j + jb <= *n) {

/*              Apply interchanges to columns J+JB:N. */

		i__3 = *n - j - jb + 1;
		i__4 = j + jb - 1;
		dlaswp_(&i__3, &A(1,j+jb), lda, &j, &i__4, &
			IPIV(1), &c__1);

/*              Compute block row of U. */

		i__3 = *n - j - jb + 1;
		dtrsm_("Left", "Lower", "No transpose", "Unit", &jb, &i__3, &
			c_b16, &A(j,j), lda, &A(j,j+jb), lda);
		if (j + jb <= *m) {

/*                 Update trailing submatrix. */

		    i__3 = *m - j - jb + 1;
		    i__4 = *n - j - jb + 1;
		    dgemm_("No transpose", "No transpose", &i__3, &i__4, &jb,
			    &c_b19, &A(j+jb,j), lda, &A(j,j+jb), lda, &c_b16, &A(j+jb,j+jb), lda);
		}
	    }
/* L20: */
	}
    }
    return 0;

/*     End of DGETRF */

} /* dgetrf_ */

