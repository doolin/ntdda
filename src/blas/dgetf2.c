
#include <math.h>
#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )

/* Subroutine */ int dgetf2_(int *m, int *n, double *a, int *
	lda, int *ipiv, int *info)
{
/*  -- LAPACK routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1992   


    Purpose   
    =======   

    DGETF2 computes an LU factorization of a general m-by-n matrix A   
    using partial pivoting with row interchanges.   

    The factorization has the form   
       A = P * L * U   
    where P is a permutation matrix, L is lower triangular with unit   
    diagonal elements (lower trapezoidal if m > n), and U is upper   
    triangular (upper trapezoidal if m < n).   

    This is the right-looking Level 2 BLAS version of the algorithm.   

    Arguments   
    =========   

    M       (input) INT   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INT   
            The number of columns of the matrix A.  N >= 0.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the m by n matrix to be factored.   
            On exit, the factors L and U from the factorization   
            A = P*L*U; the unit diagonal elements of L are not stored.   

    LDA     (input) INT   
            The leading dimension of the array A.  LDA >= MAX(1,M).   

    IPIV    (output) INT array, dimension (MIN(M,N))   
            The pivot indices; for 1 <= i <= MIN(M,N), row i of the   
            matrix was interchanged with row IPIV(i).   

    INFO    (output) INT   
            = 0: successful exit   
            < 0: if INFO = -k, the k-th argument had an illegal value   
            > 0: if INFO = k, U(k,k) is exactly zero. The factorization   
                 has been completed, but the factor U is exactly   
                 singular, and division by zero will occur if it is used 
  
                 to solve a system of equations.   

    ===================================================================== 
  


       Test the input parameters.   

    
   Parameter adjustments   
       Function Body */
    /* Table of constant values */
    static int c__1 = 1;
    static double c_b6 = -1.;
    
    /* System generated locals */
    int  i__1, i__2, i__3;
    double d__1;
    /* Local variables */
    extern /* Subroutine */ int dger_(int *, int *, double *, 
	    double *, int *, double *, int *, double *, 
	    int *);
    static int j;
    extern /* Subroutine */ int dscal_(int *, double *, double *, 
	    int *), dswap_(int *, double *, int *, double 
	    *, int *);
    static int jp;
    extern int idamax_(int *, double *, int *);
    extern /* Subroutine */ int xerbla_(char *, int *);



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
	xerbla_("DGETF2", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*m == 0 || *n == 0) {
	return 0;
    }

    i__1 = MIN(*m,*n);
    for (j = 1; j <= MIN(*m,*n); ++j) {

/*        Find pivot and test for singularity. */

	i__2 = *m - j + 1;
	jp = j - 1 + idamax_(&i__2, &A(j,j), &c__1);
	IPIV(j) = jp;
	if (A(jp,j) != 0.) {

/*           Apply the interchange to columns 1:N. */

	    if (jp != j) {
		dswap_(n, &A(j,1), lda, &A(jp,1), lda);
	    }

/*           Compute elements J+1:M of J-th column. */

	    if (j < *m) {
		i__2 = *m - j;
		d__1 = 1. / A(j,j);
		dscal_(&i__2, &d__1, &A(j+1,j), &c__1);
	    }

	} else if (*info == 0) {

	    *info = j;
	}

	if (j < MIN(*m,*n)) {

/*           Update trailing submatrix. */

	    i__2 = *m - j;
	    i__3 = *n - j;
	    dger_(&i__2, &i__3, &c_b6, &A(j+1,j), &c__1, &A(j,j+1), lda, &A(j+1,j+1), lda);
	}
/* L10: */
    }
    return 0;

/*     End of DGETF2 */

} /* dgetf2_ */

