
/*  
 * geometry.h  
 * 
 * General header file for the numerical geometry routines.
 *
 * $Author: doolin $
 * $Date: 2001/05/20 21:03:43 $
 * $Source: /cvsroot/dda/ntdda/include/geometry.h,v $
 * $Revision: 1.1 $
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


void dc04(Geometrydata *, int **, /* int **, */ double **, double **);
void dc05(Geometrydata *, int **, int **, int **, int **, double **, double **, double **);
void dc06(Geometrydata *, int **, int **, double **);
void dc07(Geometrydata *, int **, int **, int **, int **, double **, double **);

/* Part of what dc08 does is determine the 
 * number of blocks in the problem domain.
 * Need to free **aa after this call.
 */
void dc08(Geometrydata *, int **, int **, int **, int **, int **, 
          double **, double **, double **);

void dc09(Geometrydata *, int **, int *, double **);
void dc10(Geometrydata *, int **, double **);
void dc11(Geometrydata *, int **, double **, double **);
void dc12(Geometrydata *, int **, double **);
void dc13(Geometrydata *, int **, double **, double **);
void dc14(Geometrydata *, int **, double **, double **);
void dc16(Geometrydata *, int **, double **, double **);
void dc17(Geometrydata *, int **, double **);
void dc19(Geometrydata *, int **, double **);


void deallocateGData(Geometrydata *, int **, int **, int **, int **, int **, int *, 
                     double **, double **, double **);
//void setGZero(Geometrydata *);

//int CancelOrAbort(HWND, MSG);

int crr(Geometrydata *, double, double, int **, double **, double **);
//int crr(double w0, double, double, int **, double **, double **);

int lns(double, double, double, double, double,
         double, double, double, double, double *, double *);

//int rlt(Geometrydata *pc, int, double, double, int **, double **, double **);
int rlt(double w0, int, double, double, int **, double **, double **);
//int pointinpoly(int blocknumber, double x11, double y11,int ** vindex, double ** vertices);



Geometrydata * geometryToReturn(Geometrydata *, int **, double **);



