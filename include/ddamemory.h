/* 
 *  ddamemory.h
 * 
 *  $Author: doolin $
 *  $Date: 2001/05/20 21:03:43 $
 *  $Log: ddamemory.h,v $
 *  Revision 1.1  2001/05/20 21:03:43  doolin
 *  Initial revision
 *
 */

#ifndef _DDAMEMORY_H_
#define _DDAMEMORY_H_

#include <stdlib.h>
#include <memory.h>

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


#endif