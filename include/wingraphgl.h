// header file for graphics routines used in Windows version of DDA


/* New functions to change how the graphics are handled.
 */
int setScaleGL( GRAPHICS *, double [], int, int);
/* This function moved to printgl.c */
//double setPrScaleGL(HWND, HDC, GRAPHICS *, double []);

int drawPointsGL( GRAPHICS *, GEOMETRYDATA *, double **);
int drawBlocksGL( GRAPHICS *, GEOMETRYDATA *);
int drawJointsGL( GRAPHICS *, GEOMETRYDATA *, double ** vertices);
int drawBoltsGL( GRAPHICS *,  GEOMETRYDATA *, double **);
int drawCentroidsGL(GRAPHICS *, DATALOG *, GEOMETRYDATA *);
int drawStressesGL(GRAPHICS *, DATALOG *, GEOMETRYDATA *);
void printTimeGL( GRAPHICS *, GEOMETRYDATA *);




