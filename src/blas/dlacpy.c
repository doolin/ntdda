
#include <math.h>
#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )

/* Subroutine */ int dlacpy_(char *uplo, int *m, int *n, double *
	a, int *lda, double *b, int *ldb)
{
/*  -- LAPACK auxiliary routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    DLACPY copies all or part of a two-dimensional matrix A to another   
    matrix B.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            Specifies the part of the matrix A to be copied to B.   
            = 'U':      Upper triangular part   
            = 'L':      Lower triangular part   
            Otherwise:  All of the matrix A   

    M       (input) INT   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INT   
            The number of columns of the matrix A.  N >= 0.   

    A       (input) DOUBLE PRECISION array, dimension (LDA,N)   
            The m by n matrix A.  If UPLO = 'U', only the upper triangle 
  
            or trapezoid is accessed; if UPLO = 'L', only the lower   
            triangle or trapezoid is accessed.   

    LDA     (input) INT   
            The leading dimension of the array A.  LDA >= MAX(1,M).   

    B       (output) DOUBLE PRECISION array, dimension (LDB,N)   
            On exit, B = A in the locations specified by UPLO.   

    LDB     (input) INT   
            The leading dimension of the array B.  LDB >= MAX(1,M).   

    ===================================================================== 
  


    
   Parameter adjustments   
       Function Body */
    /* System generated locals */
    int  i__1, i__2;
    /* Local variables */
    static int i, j;
    extern long int lsame_(char *, char *);



#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]
#define B(I,J) b[(I)-1 + ((J)-1)* ( *ldb)]

    if (lsame_(uplo, "U")) {
	i__1 = *n;
	for (j = 1; j <= *n; ++j) {
	    i__2 = MIN(j,*m);
	    for (i = 1; i <= MIN(j,*m); ++i) {
		B(i,j) = A(i,j);
/* L10: */
	    }
/* L20: */
	}
    } else if (lsame_(uplo, "L")) {
	i__1 = *n;
	for (j = 1; j <= *n; ++j) {
	    i__2 = *m;
	    for (i = j; i <= *m; ++i) {
		B(i,j) = A(i,j);
/* L30: */
	    }
/* L40: */
	}
    } else {
	i__1 = *n;
	for (j = 1; j <= *n; ++j) {
	    i__2 = *m;
	    for (i = 1; i <= *m; ++i) {
		B(i,j) = A(i,j);
/* L50: */
	    }
/* L60: */
	}
    }
    return 0;

/*     End of DLACPY */

} /* dlacpy_ */

