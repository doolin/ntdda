#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)        ( ((a)<0.0) ? -(a) : (a))
#include <math.h>

/* Subroutine */ int dtrtri_(char *uplo, char *diag, int *n, double *
	a, int *lda, int *info)
{
/*  -- LAPACK routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       March 31, 1993   


    Purpose   
    =======   

    DTRTRI computes the inverse of a real upper or lower triangular   
    matrix A.   

    This is the Level 3 BLAS version of the algorithm.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            = 'U':  A is upper triangular;   
            = 'L':  A is lower triangular.   

    DIAG    (input) CHARACTER*1   
            = 'N':  A is non-unit triangular;   
            = 'U':  A is unit triangular.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the triangular matrix A.  If UPLO = 'U', the   
            leading N-by-N upper triangular part of the array A contains 
  
            the upper triangular matrix, and the strictly lower   
            triangular part of A is not referenced.  If UPLO = 'L', the   
            leading N-by-N lower triangular part of the array A contains 
  
            the lower triangular matrix, and the strictly upper   
            triangular part of A is not referenced.  If DIAG = 'U', the   
            diagonal elements of A are also not referenced and are   
            assumed to be 1.   
            On exit, the (triangular) inverse of the original matrix, in 
  
            the same storage format.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= MAX(1,N).   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument had an illegal value   
            > 0: if INFO = i, A(i,i) is exactly zero.  The triangular   
                 matrix is singular and its inverse can not be computed. 
  

    ===================================================================== 
  


       Test the input parameters.   

    
   Parameter adjustments   
       Function Body */
    /* Table of constant values */
    static int c__1 = 1;
    static int c_n1 = -1;
    static double c_b18 = 1.;
    static double c_b22 = -1.;
    
    /* System generated locals */
    int  i__1, i__3, i__4, i__5;
    char ch__1[2];
    /* Builtin functions   */
    /* Local variables */
    static int j;
    extern long int lsame_(char *, char *);
    extern /* Subroutine */ int dtrmm_(char *, char *, char *, char *, 
	    int *, int *, double *, double *, int *, 
	    double *, int *), dtrsm_(
	    char *, char *, char *, char *, int *, int *, double *
	    , double *, int *, double *, int *);
    static long int upper;
    extern /* Subroutine */ int dtrti2_(char *, char *, int *, double 
	    *, int *, int *);
    static int jb, nb, nn;
    extern /* Subroutine */ int xerbla_(char *, int *);
    extern int ilaenv_(int *, char *, char *, int *, int *, 
	    int *, int *, long int, long int);
    static long int nounit;




#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]

    *info = 0;
    upper = lsame_(uplo, "U");
    nounit = lsame_(diag, "N");
    if (! upper && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (! nounit && ! lsame_(diag, "U")) {
	*info = -2;
    } else if (*n < 0) {
	*info = -3;
    } else if (*lda < MAX(1,*n)) {
	*info = -5;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DTRTRI", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Check for singularity if non-unit. */

    if (nounit) {
	i__1 = *n;
	for (*info = 1; *info <= i__1; ++(*info)) {
	    if (A(*info,*info) == 0.) {
		return 0;
	    }
/* L10: */
	}
	*info = 0;
    }

/*     Determine the block size for this environment.   

   Writing concatenation */
    /* i__2[0] = 1, a__1[0] = uplo;
    i__2[1] = 1, a__1[1] = diag;
    s_cat(ch__1, a__1, i__2, &c__2, 2L); */
    ch__1[0] = *uplo; ch__1[1] = *diag; ch__1[2] = 0;
    nb = ilaenv_(&c__1, "DTRTRI", ch__1, n, &c_n1, &c_n1, &c_n1, 6L, 2L);
    if (nb <= 1 || nb >= *n) {

/*        Use unblocked code */

	dtrti2_(uplo, diag, n, &A(1,1), lda, info);
    } else {

/*        Use blocked code */

	if (upper) {

/*           Compute inverse of upper triangular matrix */

	    i__1 = *n;
	    i__3 = nb;
	    for (j = 1; nb < 0 ? j >= *n : j <= *n; j += nb) {
/* Computing MIN */
		i__4 = nb, i__5 = *n - j + 1;
		jb = MIN(i__4,i__5);

/*              Compute rows 1:j-1 of current block column */

		i__4 = j - 1;
		dtrmm_("Left", "Upper", "No transpose", diag, &i__4, &jb, &
			c_b18, &A(1,1), lda, &A(1,j), lda);
		i__4 = j - 1;
		dtrsm_("Right", "Upper", "No transpose", diag, &i__4, &jb, &
			c_b22, &A(j,j), lda, &A(1,j), 
			lda);

/*              Compute inverse of current diagonal block */

		dtrti2_("Upper", diag, &jb, &A(j,j), lda, info);
/* L20: */
	    }
	} else {

/*           Compute inverse of lower triangular matrix */

	    nn = (*n - 1) / nb * nb + 1;
	    i__3 = -nb;
	    for (j = nn; -nb < 0 ? j >= 1 : j <= 1; j += -nb) {
/* Computing MIN */
		i__1 = nb, i__4 = *n - j + 1;
		jb = MIN(i__1,i__4);
		if (j + jb <= *n) {

/*                 Compute rows j+jb:n of current block co
lumn */

		    i__1 = *n - j - jb + 1;
		    dtrmm_("Left", "Lower", "No transpose", diag, &i__1, &jb, 
			    &c_b18, &A(j+jb,j+jb), lda, &A(j+jb,j), lda);
		    i__1 = *n - j - jb + 1;
		    dtrsm_("Right", "Lower", "No transpose", diag, &i__1, &jb,
			     &c_b22, &A(j,j), lda, &A(j+jb,j), lda);
		}

/*              Compute inverse of current diagonal block */

		dtrti2_("Lower", diag, &jb, &A(j,j), lda, info);
/* L30: */
	    }
	}
    }

    return 0;

/*     End of DTRTRI */

} /* dtrtri_ */

