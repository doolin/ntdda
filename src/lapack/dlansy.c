#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)        ( ((a)<0.0) ? -(a) : (a))
#include <math.h>

double dlansy_(char *norm, char *uplo, int *n, double *a, int 
	*lda, double *work)
{
/*  -- LAPACK auxiliary routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       October 31, 1992   


    Purpose   
    =======   

    DLANSY  returns the value of the one norm,  or the Frobenius norm, or 
  
    the  infinity norm,  or the  element of  largest absolute value  of a 
  
    real symmetric matrix A.   

    Description   
    ===========   

    DLANSY returns the value   

       DLANSY = ( MAX(ABS(A(i,j))), NORM = 'M' or 'm'   
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
            Specifies the value to be returned in DLANSY as described   
            above.   

    UPLO    (input) CHARACTER*1   
            Specifies whether the upper or lower triangular part of the   
            symmetric matrix A is to be referenced.   
            = 'U':  Upper triangular part of A is referenced   
            = 'L':  Lower triangular part of A is referenced   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.  When N = 0, DLANSY is   
            set to zero.   

    A       (input) DOUBLE PRECISION array, dimension (LDA,N)   
            The symmetric matrix A.  If UPLO = 'U', the leading n by n   
            upper triangular part of A contains the upper triangular part 
  
            of the matrix A, and the strictly lower triangular part of A 
  
            is not referenced.  If UPLO = 'L', the leading n by n lower   
            triangular part of A contains the lower triangular part of   
            the matrix A, and the strictly upper triangular part of A is 
  
            not referenced.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= MAX(N,1).   

    WORK    (workspace) DOUBLE PRECISION array, dimension (LWORK),   
            where LWORK >= N when NORM = 'I' or '1' or 'O'; otherwise,   
            WORK is not referenced.   

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
    static double absa;
    static int i, j;
    static double scale;
    extern long int lsame_(char *, char *);
    static double value;
    extern /* Subroutine */ int dlassq_(int *, double *, int *, 
	    double *, double *);
    static double sum;



#define WORK(I) work[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]

    if (*n == 0) {
	value = 0.;
    } else if (lsame_(norm, "M")) {

/*        Find MAX(ABS(A(i,j))). */

	value = 0.;
	if (lsame_(uplo, "U")) {
	    i__1 = *n;
	    for (j = 1; j <= *n; ++j) {
		i__2 = j;
		for (i = 1; i <= j; ++i) {
/* Computing MAX */
		    d__2 = value, d__3 = (d__1 = A(i,j), ABS(d__1))
			    ;
		    value = MAX(d__2,d__3);
/* L10: */
		}
/* L20: */
	    }
	} else {
	    i__1 = *n;
	    for (j = 1; j <= *n; ++j) {
		i__2 = *n;
		for (i = j; i <= *n; ++i) {
/* Computing MAX */
		    d__2 = value, d__3 = (d__1 = A(i,j), ABS(d__1))
			    ;
		    value = MAX(d__2,d__3);
/* L30: */
		}
/* L40: */
	    }
	}
    } else if (lsame_(norm, "I") || lsame_(norm, "O") || *(
	    unsigned char *)norm == '1') {

/*        Find normI(A) ( = norm1(A), since A is symmetric). */

	value = 0.;
	if (lsame_(uplo, "U")) {
	    i__1 = *n;
	    for (j = 1; j <= *n; ++j) {
		sum = 0.;
		i__2 = j - 1;
		for (i = 1; i <= j-1; ++i) {
		    absa = (d__1 = A(i,j), ABS(d__1));
		    sum += absa;
		    WORK(i) += absa;
/* L50: */
		}
		WORK(j) = sum + (d__1 = A(j,j), ABS(d__1));
/* L60: */
	    }
	    i__1 = *n;
	    for (i = 1; i <= *n; ++i) {
/* Computing MAX */
		d__1 = value, d__2 = WORK(i);
		value = MAX(d__1,d__2);
/* L70: */
	    }
	} else {
	    i__1 = *n;
	    for (i = 1; i <= *n; ++i) {
		WORK(i) = 0.;
/* L80: */
	    }
	    i__1 = *n;
	    for (j = 1; j <= *n; ++j) {
		sum = WORK(j) + (d__1 = A(j,j), ABS(d__1));
		i__2 = *n;
		for (i = j + 1; i <= *n; ++i) {
		    absa = (d__1 = A(i,j), ABS(d__1));
		    sum += absa;
		    WORK(i) += absa;
/* L90: */
		}
		value = MAX(value,sum);
/* L100: */
	    }
	}
    } else if (lsame_(norm, "F") || lsame_(norm, "E")) {

/*        Find normF(A). */

	scale = 0.;
	sum = 1.;
	if (lsame_(uplo, "U")) {
	    i__1 = *n;
	    for (j = 2; j <= *n; ++j) {
		i__2 = j - 1;
		dlassq_(&i__2, &A(1,j), &c__1, &scale, &sum);
/* L110: */
	    }
	} else {
	    i__1 = *n - 1;
	    for (j = 1; j <= *n-1; ++j) {
		i__2 = *n - j;
		dlassq_(&i__2, &A(j+1,j), &c__1, &scale, &sum);
/* L120: */
	    }
	}
	sum *= 2;
	i__1 = *lda + 1;
	dlassq_(n, &A(1,1), &i__1, &scale, &sum);
	value = scale * sqrt(sum);
    }

    ret_val = value;
    return ret_val;

/*     End of DLANSY */

} /* dlansy_ */

