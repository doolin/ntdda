/* FIXME: Move the blas functions into a blas.h */
/* This is just a prototype to keep the compiler 
 * from complaining.
 */
extern double dlansy_(char *norm, char *uplo, int *n, double *a, int 
                      *lda, double *work);
int daxpy_(int *n, double *da, double *dx, int *incx, double *dy, int *incy);
double ddot_(int *n, double *dx, int *incx, double *dy, int *incy);
double ddaddot_(int *n, double *dx, int *incx, double *dy, int *incy);

int dsymv_(char *uplo, int *n, double *alpha, double *a, int *lda, double *x, int *incx, double *beta, double *y, int *incy);
void ddadsymv(double ** A, double ** p, double ** q, int * colnums, int * k1, int ** colindex, int nblocks);
//int dgemv_(char *trans, int *m, int *n, double * alpha, double *a, int *lda, double *x, int *incx, double *beta, double *y, int *incy);
/* FIXME: change name to block_daxpy() */
void sp_mult(double * Arow, double * x, double * y, int blocksize, int index);

int dgemv_(char *trans, int *m, int *n, double * alpha, double *a, int *lda, double *x, int *incx, double *beta, double *y, int *incy);
int dgemm_(char *transa, char *transb, int *rows_a, int * cols_b, int *rowcol, double *alpha, double *a, int *lda, double *b, int *ldb, double *beta, double *c, int *ldc);
int dgetrf_(int *m, int *n, double *a, int * lda, int *ipiv, int *info);
int dgetri_(int *n, double *a, int *lda, int *ipiv, double *work, int *lwork, int *info);
/* FIXME: Start a blas.h and a lapack.h */
/* This is just a prototype to keep the compiler 
 * from complaining.
 */
double dlansy_(char *norm, char *uplo, int *n, double *a, int *lda, double *work);
int daxpy_(int *n, double *da, double *dx, int *incx, double *dy, int *incy);
double ddot_(int *n, double *dx, int *incx, double *dy, int *incy);
int dsymv_(char *uplo, int *n, double *alpha, double *a, int *lda, double *x, int *incx, double *beta, double *y, int *incy);
//void ddadsymv(double ** K, double ** F, int * colnums, int ** colindex, int nblocks);
/* y gets overwritten */
void ddadaxpy(double ALPHA, double ** x, double ** y);

