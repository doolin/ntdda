
/*  
 * geometry.h  
 * 
 * General header file for the numerical geometry routines.
 *
 * $Author: doolin $
 * $Date: 2002/05/25 14:49:39 $
 * $Source: /cvsroot/dda/ntdda/include/geometry.h,v $
 * $Revision: 1.2 $
 */

#include"dda.h"
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


//void computeDomainscale(Geometrydata *);  // was dc02()


void countIntersections(Geometrydata *);  // was dc03()
void allocateGeometryArrays(Geometrydata *, int *** aa, int *** k,
                            int *** r, int *** m, int ** h, 
                            int *** vindexdc, double *** q, double *** c,
                            double *** verticesdc);


void deallocateGData(Geometrydata *, int **, int **, int **, int **, int **, int *, 
                     double **, double **, double **);


int crr(Geometrydata *, double, double, int **, double **, double **);
//int crr(double w0, double, double, int **, double **, double **);

int lns(double, double, double, double, double,
         double, double, double, double, double *, double *);

//int rlt(Geometrydata *pc, int, double, double, int **, double **, double **);
int rlt(double w0, int, double, double, int **, double **, double **);
//int pointinpoly(int blocknumber, double x11, double y11,int ** vindex, double ** vertices);



Geometrydata * geometryToReturn(Geometrydata *, int **, double **);



