#define MIN(a,b)      ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)      ( ((a)<(b)) ? (b) : (a) )
#define ABS(a)        ( ((a)<0.0) ? -(a) : (a))
#include <math.h>

/* Subroutine */ int dpoequ_(int *n, double *a, int *lda, 
	double *s, double *scond, double *amax, int *info)
{
/*  -- LAPACK routine (version 2.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       March 31, 1993   


    Purpose   
    =======   

    DPOEQU computes row and column scalings intended to equilibrate a   
    symmetric positive definite matrix A and reduce its condition number 
  
    (with respect to the two-norm).  S contains the scale factors,   
    S(i) = 1/sqrt(A(i,i)), chosen so that the scaled matrix B with   
    elements B(i,j) = S(i)*A(i,j)*S(j) has ones on the diagonal.  This   
    choice of S puts the condition number of B within a factor N of the   
    smallest possible condition number over all possible diagonal   
    scalings.   

    Arguments   
    =========   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    A       (input) DOUBLE PRECISION array, dimension (LDA,N)   
            The N-by-N symmetric positive definite matrix whose scaling   
            factors are to be computed.  Only the diagonal elements of A 
  
            are referenced.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= MAX(1,N).   

    S       (output) DOUBLE PRECISION array, dimension (N)   
            If INFO = 0, S contains the scale factors for A.   

    SCOND   (output) DOUBLE PRECISION   
            If INFO = 0, S contains the ratio of the smallest S(i) to   
            the largest S(i).  If SCOND >= 0.1 and AMAX is neither too   
            large nor too small, it is not worth scaling by S.   

    AMAX    (output) DOUBLE PRECISION   
            Absolute value of largest matrix element.  If AMAX is very   
            close to overflow or very close to underflow, the matrix   
            should be scaled.   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   
            > 0:  if INFO = i, the i-th diagonal element is nonpositive. 
  

    ===================================================================== 
  


       Test the input parameters.   

    
   Parameter adjustments   
       Function Body */
    /* System generated locals */
    int  i__1;
    double d__1, d__2;
    /* Builtin functions */
    /* Local variables */
    static double smin;
    static int i;
    extern /* Subroutine */ int xerbla_(char *, int *);


#define S(I) s[(I)-1]

#define A(I,J) a[(I)-1 + ((J)-1)* ( *lda)]

    *info = 0;
    if (*n < 0) {
	*info = -1;
    } else if (*lda < MAX(1,*n)) {
	*info = -3;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DPOEQU", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	*scond = 1.;
	*amax = 0.;
	return 0;
    }

/*     Find the minimum and maximum diagonal elements. */

    S(1) = A(1,1);
    smin = S(1);
    *amax = S(1);
    i__1 = *n;
    for (i = 2; i <= *n; ++i) {
	S(i) = A(i,i);
/* Computing MIN */
	d__1 = smin, d__2 = S(i);
	smin = MIN(d__1,d__2);
/* Computing MAX */
	d__1 = *amax, d__2 = S(i);
	*amax = MAX(d__1,d__2);
/* L10: */
    }

    if (smin <= 0.) {

/*        Find the first non-positive diagonal element and return. */

	i__1 = *n;
	for (i = 1; i <= *n; ++i) {
	    if (S(i) <= 0.) {
		*info = i;
		return 0;
	    }
/* L20: */
	}
    } else {

/*        Set the scale factors to the reciprocals   
          of the diagonal elements. */

	i__1 = *n;
	for (i = 1; i <= *n; ++i) {
	    S(i) = 1. / sqrt(S(i));
/* L30: */
	}

/*        Compute SCOND = MIN(S(I)) / MAX(S(I)) */

	*scond = sqrt(smin) / sqrt(*amax);
    }
    return 0;

/*     End of DPOEQU */

} /* dpoequ_ */

