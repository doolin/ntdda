
/*
 * geometry.h
 *
 * General header file for the numerical geometry routines.
 *
 * $Author: doolin $
 * $Date: 2003/12/17 23:36:36 $
 * $Revision: 1.5 $
 */

#include "geometrydata.h"

/*
 * double e00 : minimum edge-node distance
 * mm1   : number of input joints
 * mm2   : number of boundary joints in begining
 * nBlMat : number of block material lines
 * nBolt : number of bolt elements
 * nFixed : number of fixed points
 * nLoad : number of loading points
 * nMeas : number of measured points
 * nHole : number of hole points
 * n0   : number of blocks
 * n2   : number of intersection points no merge
 * n3   : number of intersection points    merge
 * o    : total length of block vertices data
 */


void countIntersections(Geometrydata *);  // was dc03()
void allocateGeometryArrays(Geometrydata *, int *** aa, int *** k,
                            int *** r, int *** m, int ** h,
                            int *** vindexdc, double *** q, double *** c,
                            double *** verticesdc);


void deallocateGData(Geometrydata *, int **, int **, int **, int **, int **, int *,
                     double **, double **, double **);


int crr(Geometrydata *, double, double, int **, double **, double **);

int lns(double, double, double, double, double,
         double, double, double, double, double *, double *);

int rlt(double w0, int, double, double, int **, double **, double **);

Geometrydata * geometryToReturn(Geometrydata *, int **, double **);



