/* 
 *  ddamemory.h
 * 
 *  $Author: doolin $
 *  $Date: 2002/10/14 16:02:46 $
 *  $Log: ddamemory.h,v $
 *  Revision 1.3  2002/10/14 16:02:46  doolin
 *  Lots of small clean up done.  No major changes in this commit.
 *
 *  Revision 1.2  2002/10/09 01:46:38  doolin
 *  Lots of clean up and fix it work on this commit.
 *  Details are in the diffs  ;)
 *
 *  Revision 1.1.1.1  2001/05/20 21:03:43  doolin
 *  This import corresponds to development version 1.5.124
 *
 */

#ifndef __DDAMEMORY_H__
#define __DDAMEMORY_H__

#include <stdlib.h>
#include <memory.h>


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

int ** IntMat2DGetMem(int, int);
double ** DoubMat2DGetMem(int, int);
void free2DMat(void **, int);

void free2Darray(void **, int);
void free2DIntArray(int **, int, int);
void free2DDoubleArray(double **, int, int);

double ** clone2DMatDoub(double **,int, int);
int ** clone2DMatInt(int **,int, int);
int * clone1DMatInt(int * mat,int n);
void freeSolverTempArrays(void);
void freeStorageTempArrays(void);

void copy2DArray(void ** in, void ** out, int n, int m);

int copy2DDoubMat(double **, double **, int, int); 
int copy2DIntMat(int **, int **, int, int); 
/* This function needs to be turned into a macro. */
void setMatrixToZero(double **, int, int);


#ifdef __cplusplus
}
#endif

#endif  /* __DDAMEMORY_H__ */