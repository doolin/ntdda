#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)        ( ((a)<0.0) ? -(a) : (a))
#include <math.h>

/* Subroutine */ int dgetri_(int *n, double *a, int *lda, int 
	*ipiv, double *work, int *lwork, int *info)
{
/*  -- LAPACK routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    DGETRI computes the inverse of a matrix using the LU factorization   
    computed by DGETRF.   

    This method inverts U and then computes inv(A) by solving the system 
  
    inv(A)*L = inv(U) for inv(A).   

    Arguments   
    =========   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the factors L and U from the factorization   
            A = P*L*U as computed by DGETRF.   
            On exit, if INFO = 0, the inverse of the original matrix A.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= MAX(1,N).   

    IPIV    (input) INTEGER array, dimension (N)   
            The pivot indices from DGETRF; for 1<=i<=N, row i of the   
            matrix was interchanged with row IPIV(i).   

    WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK) 
  
            On exit, if INFO=0, then WORK(1) returns the optimal LWORK.   

    LWORK   (input) INTEGER   
            The dimension of the array WORK.  LWORK >= MAX(1,N).   
            For optimal performance LWORK >= N*NB, where NB is   
            the optimal blocksize returned by ILAENV.   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   
            > 0:  if INFO = i, U(i,i) is exactly zero; the matrix is   
                  singular and its inverse could not be computed.   

    ===================================================================== 
  


       Test the input parameters.   

    
   Parameter adjustments   
       Function Body */
    /* Table of constant values */
    static int c__1 = 1;
    static int c_n1 = -1;
    static int c__2 = 2;
    static double c_b20 = -1.;
    static double c_b22 = 1.;
    
    /* System generated locals */
    int  i__1, i__2, i__3;
    /* Local variables */
    static int i, j;
    extern /* Subroutine */ int dgemm_(char *, char *, int *, int *, 
	    int *, double *, double *, int *, double *, 
	    int *, double *, double *, int *),
	     dgemv_(char *, int *, int *, double *, double *, 
	    int *, double *, int *, double *, double *, 
	    int *);
    static int nbmin;
    extern /* Subroutine */ int dswap_(int *, double *, int *, 
	    double *, int *), dtrsm_(char *, char *, char *, char *, 
	    int *, int *, double *, double *, int *, 
	    double *, int *);
    static int jb, nb, jj, jp, nn;
    extern /* Subroutine */ int xerbla_(char *, int *);
    extern int ilaenv_(int *, char *, char *, int *, int *, 
	    int *, int *, long int, long int);
    static int ldwork;
    extern /* Subroutine */ int dtrtri_(char *, char *, int *, double 
	    *, int *, int *);
    static int iws;



#define IPIV(I) ipiv[(I)-1]
#define WORK(I) work[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]

    *info = 0;
    WORK(1) = (double) MAX(*n,1);
    if (*n < 0) {
	*info = -1;
    } else if (*lda < MAX(1,*n)) {
	*info = -3;
    } else if (*lwork < MAX(1,*n)) {
	*info = -6;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DGETRI", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Form inv(U).  If INFO > 0 from DTRTRI, then U is singular,   
       and the inverse is not computed. */

    dtrtri_("Upper", "Non-unit", n, &A(1,1), lda, info);
    if (*info > 0) {
	return 0;
    }

/*     Determine the block size for this environment. */

    nb = ilaenv_(&c__1, "DGETRI", " ", n, &c_n1, &c_n1, &c_n1, 6L, 1L);
    nbmin = 2;
    ldwork = *n;
    if (nb > 1 && nb < *n) {
/* Computing MAX */
	i__1 = ldwork * nb;
	iws = MAX(i__1,1);
	if (*lwork < iws) {
	    nb = *lwork / ldwork;
/* Computing MAX */
	    i__1 = 2, i__2 = ilaenv_(&c__2, "DGETRI", " ", n, &c_n1, &c_n1, &
		    c_n1, 6L, 1L);
	    nbmin = MAX(i__1,i__2);
	}
    } else {
	iws = *n;
    }

/*     Solve the equation inv(A)*L = inv(U) for inv(A). */

    if (nb < nbmin || nb >= *n) {

/*        Use unblocked code. */

	for (j = *n; j >= 1; --j) {

/*           Copy current column of L to WORK and replace with zer
os. */

	    i__1 = *n;
	    for (i = j + 1; i <= *n; ++i) {
		WORK(i) = A(i,j);
		A(i,j) = 0.;
/* L10: */
	    }

/*           Compute current column of inv(A). */

	    if (j < *n) {
		i__1 = *n - j;
		dgemv_("No transpose", n, &i__1, &c_b20, &A(1,j+1), lda, &WORK(j + 1), &c__1, &c_b22, &A(1,j), &c__1);
	    }
/* L20: */
	}
    } else {

/*        Use blocked code. */

	nn = (*n - 1) / nb * nb + 1;
	i__1 = -nb;
	for (j = nn; -nb < 0 ? j >= 1 : j <= 1; j += -nb) {
/* Computing MIN */
	    i__2 = nb, i__3 = *n - j + 1;
	    jb = MIN(i__2,i__3);

/*           Copy current block column of L to WORK and replace wi
th   
             zeros. */

	    i__2 = j + jb - 1;
	    for (jj = j; jj <= j+jb-1; ++jj) {
		i__3 = *n;
		for (i = jj + 1; i <= *n; ++i) {
		    WORK(i + (jj - j) * ldwork) = A(i,jj);
		    A(i,jj) = 0.;
/* L30: */
		}
/* L40: */
	    }

/*           Compute current block column of inv(A). */

	    if (j + jb <= *n) {
		i__2 = *n - j - jb + 1;
		dgemm_("No transpose", "No transpose", n, &jb, &i__2, &c_b20, 
			&A(1,j+jb), lda, &WORK(j + jb), &
			ldwork, &c_b22, &A(1,j), lda);
	    }
	    dtrsm_("Right", "Lower", "No transpose", "Unit", n, &jb, &c_b22, &
		    WORK(j), &ldwork, &A(1,j), lda);
/* L50: */
	}
    }

/*     Apply column interchanges. */

    for (j = *n - 1; j >= 1; --j) {
	jp = IPIV(j);
	if (jp != j) {
	    dswap_(n, &A(1,j), &c__1, &A(1,jp), &c__1);
	}
/* L60: */
    }

    WORK(1) = (double) iws;
    return 0;

/*     End of DGETRI */

} /* dgetri_ */

