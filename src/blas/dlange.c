
#include <math.h>
#define MIN(a,b)           ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)           ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)             ( ((a)<0.0)   ? -(a) : (a) )


double dlange_(char *norm, int *m, int *n, double *a, int 
	*lda, double *work)
{
/*  -- LAPACK auxiliary routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       October 31, 1992   


    Purpose   
    =======   

    DLANGE  returns the value of the one norm,  or the Frobenius norm, or 
  
    the  infinity norm,  or the  element of  largest absolute value  of a 
  
    real matrix A.   

    Description   
    ===========   

    DLANGE returns the value   

       DLANGE = ( MAX(ABS(A(i,j))), NORM = 'M' or 'm'   
                (   
                ( norm1(A),         NORM = '1', 'O' or 'o'   
                (   
                ( normI(A),         NORM = 'I' or 'i'   
                (   
                ( normF(A),         NORM = 'F', 'f', 'E' or 'e'   

    where  norm1  denotes the  one norm of a matrix (maximum column sum), 
  
    normI  denotes the  infinity norm  of a matrix  (maximum row sum) and 
  
    normF  denotes the  Frobenius norm of a matrix (square root of sum of 
  
    squares).  Note that  MAX(ABS(A(i,j)))  is not a  matrix norm.   

    Arguments   
    =========   

    NORM    (input) CHARACTER*1   
            Specifies the value to be returned in DLANGE as described   
            above.   

    M       (input) INT   
            The number of rows of the matrix A.  M >= 0.  When M = 0,   
            DLANGE is set to zero.   

    N       (input) INT   
            The number of columns of the matrix A.  N >= 0.  When N = 0, 
  
            DLANGE is set to zero.   

    A       (input) DOUBLE PRECISION array, dimension (LDA,N)   
            The m by n matrix A.   

    LDA     (input) INT   
            The leading dimension of the array A.  LDA >= MAX(M,1).   

    WORK    (workspace) DOUBLE PRECISION array, dimension (LWORK),   
            where LWORK >= M when NORM = 'I'; otherwise, WORK is not   
            referenced.   

   ===================================================================== 
  


    
   Parameter adjustments   
       Function Body */
    /* Table of constant values */
    static int c__1 = 1;
    
    /* System generated locals */
    int  i__1, i__2;
    double ret_val, d__1, d__2, d__3;
    /* Builtin functions */
    /* Local variables */
    static int i, j;
    static double scale;
    extern long int lsame_(char *, char *);
    static double value;
    extern /* Subroutine */ int dlassq_(int *, double *, int *, 
	    double *, double *);
    static double sum;



#define WORK(I) work[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]

    if (MIN(*m,*n) == 0) {
	value = 0.;
    } else if (lsame_(norm, "M")) {

/*        Find MAX(ABS(A(i,j))). */

	value = 0.;
	i__1 = *n;
	for (j = 1; j <= *n; ++j) {
	    i__2 = *m;
	    for (i = 1; i <= *m; ++i) {
/* Computing MAX */
		d__2 = value, d__3 = (d__1 = A(i,j), ABS(d__1));
		value = MAX(d__2,d__3);
/* L10: */
	    }
/* L20: */
	}
    } else if (lsame_(norm, "O") || *(unsigned char *)norm == '1') {

/*        Find norm1(A). */

	value = 0.;
	i__1 = *n;
	for (j = 1; j <= *n; ++j) {
	    sum = 0.;
	    i__2 = *m;
	    for (i = 1; i <= *m; ++i) {
		sum += (d__1 = A(i,j), ABS(d__1));
/* L30: */
	    }
	    value = MAX(value,sum);
/* L40: */
	}
    } else if (lsame_(norm, "I")) {

/*        Find normI(A). */

	i__1 = *m;
	for (i = 1; i <= *m; ++i) {
	    WORK(i) = 0.;
/* L50: */
	}
	i__1 = *n;
	for (j = 1; j <= *n; ++j) {
	    i__2 = *m;
	    for (i = 1; i <= *m; ++i) {
		WORK(i) += (d__1 = A(i,j), ABS(d__1));
/* L60: */
	    }
/* L70: */
	}
	value = 0.;
	i__1 = *m;
	for (i = 1; i <= *m; ++i) {
/* Computing MAX */
	    d__1 = value, d__2 = WORK(i);
	    value = MAX(d__1,d__2);
/* L80: */
	}
    } else if (lsame_(norm, "F") || lsame_(norm, "E")) {

/*        Find normF(A). */

	scale = 0.;
	sum = 1.;
	i__1 = *n;
	for (j = 1; j <= *n; ++j) {
	    dlassq_(m, &A(1,j), &c__1, &scale, &sum);
/* L90: */
	}
	value = scale * sqrt(sum);
    }

    ret_val = value;
    return ret_val;

/*     End of DLANGE */

} /* dlange_ */

