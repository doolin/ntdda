
// header file for graphics routines used in Windows version of DDA

#ifndef _WINGRAPH_H_
#define _WINGRAPH_H_

#include "winmain.h"
#include "datalog.h"
#include "graphics.h"
#include "geometrydata.h"

/* New functions to change how the graphics are handled.
 */
int setScale(HWND, 
             HDC, 
             GRAPHICS *, 
             double []);
double setPrScale(HWND, HDC, GRAPHICS *, double []);

//int drawLines(HDC, HPEN [9], double, POINT, Joint *, int, int);
int drawJoints(HDC, HPEN[9], GRAPHICS *, Geometrydata *, double ** vertices, int);

int drawPoints(HDC, GRAPHICS *, Geometrydata *, double **);
int drawBlocks(HDC, HBRUSH [6], GRAPHICS *, Geometrydata *);
int drawBolts(HDC, HPEN [6], Geometrydata *, GRAPHICS *, double **);
int drawCentroids(HDC, HPEN [9], Geometrydata *, GRAPHICS *, Datalog *);
int drawStresses(HDC, HPEN [9], Geometrydata *, GRAPHICS *);
void printTime(HDC, GRAPHICS *);


#endif

