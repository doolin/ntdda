#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)        ( ((a)<0.0) ? -(a) : (a))
#include <math.h>

/* Subroutine */ int dlauum_(char *uplo, int *n, double *a, int *
	lda, int *info)
{
/*  -- LAPACK auxiliary routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    DLAUUM computes the product U * U' or L' * L, where the triangular   
    factor U or L is stored in the upper or lower triangular part of   
    the array A.   

    If UPLO = 'U' or 'u' then the upper triangle of the result is stored, 
  
    overwriting the factor U in A.   
    If UPLO = 'L' or 'l' then the lower triangle of the result is stored, 
  
    overwriting the factor L in A.   

    This is the blocked form of the algorithm, calling Level 3 BLAS.   

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
    static int c__1 = 1;
    static int c_n1 = -1;
    static double c_b15 = 1.;
    
    /* System generated locals */
    int  i__1, i__2, i__3, i__4;
    /* Local variables */
    static int i;
    extern /* Subroutine */ int dgemm_(char *, char *, int *, int *, 
	    int *, double *, double *, int *, double *, 
	    int *, double *, double *, int *);
    extern long int lsame_(char *, char *);
    extern /* Subroutine */ int dtrmm_(char *, char *, char *, char *, 
	    int *, int *, double *, double *, int *, 
	    double *, int *);
    static long int upper;
    extern /* Subroutine */ int dsyrk_(char *, char *, int *, int *, 
	    double *, double *, int *, double *, double *,
	     int *), dlauu2_(char *, int *, 
	    double *, int *, int *);
    static int ib, nb;
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
	xerbla_("DLAUUM", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Determine the block size for this environment. */

    nb = ilaenv_(&c__1, "DLAUUM", uplo, n, &c_n1, &c_n1, &c_n1, 6L, 1L);

    if (nb <= 1 || nb >= *n) {

/*        Use unblocked code */

	dlauu2_(uplo, n, &A(1,1), lda, info);
    } else {

/*        Use blocked code */

	if (upper) {

/*           Compute the product U * U'. */

	    i__1 = *n;
	    i__2 = nb;
	    for (i = 1; nb < 0 ? i >= *n : i <= *n; i += nb) {
/* Computing MIN */
		i__3 = nb, i__4 = *n - i + 1;
		ib = MIN(i__3,i__4);
		i__3 = i - 1;
		dtrmm_("Right", "Upper", "Transpose", "Non-unit", &i__3, &ib, 
			&c_b15, &A(i,i), lda, &A(1,i), 
			lda);
		dlauu2_("Upper", &ib, &A(i,i), lda, info);
		if (i + ib <= *n) {
		    i__3 = i - 1;
		    i__4 = *n - i - ib + 1;
		    dgemm_("No transpose", "Transpose", &i__3, &ib, &i__4, &
			    c_b15, &A(1,i+ib), lda, &A(i,i+ib), lda, &c_b15, &A(1,i), 
			    lda);
		    i__3 = *n - i - ib + 1;
		    dsyrk_("Upper", "No transpose", &ib, &i__3, &c_b15, &A(i,i+ib), lda, &c_b15, &A(i,i), lda);
		}
/* L10: */
	    }
	} else {

/*           Compute the product L' * L. */

	    i__2 = *n;
	    i__1 = nb;
	    for (i = 1; nb < 0 ? i >= *n : i <= *n; i += nb) {
/* Computing MIN */
		i__3 = nb, i__4 = *n - i + 1;
		ib = MIN(i__3,i__4);
		i__3 = i - 1;
		dtrmm_("Left", "Lower", "Transpose", "Non-unit", &ib, &i__3, &
			c_b15, &A(i,i), lda, &A(i,1), lda);
		dlauu2_("Lower", &ib, &A(i,i), lda, info);
		if (i + ib <= *n) {
		    i__3 = i - 1;
		    i__4 = *n - i - ib + 1;
		    dgemm_("Transpose", "No transpose", &ib, &i__3, &i__4, &
			    c_b15, &A(i+ib,i), lda, &A(i+ib,1), lda, &c_b15, &A(i,1), lda);
		    i__3 = *n - i - ib + 1;
		    dsyrk_("Lower", "Transpose", &ib, &i__3, &c_b15, &A(i+ib,i), lda, &c_b15, &A(i,i),
			     lda);
		}
/* L20: */
	    }
	}
    }

    return 0;

/*     End of DLAUUM */

} /* dlauum_ */

