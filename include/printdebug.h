
/*
 * These functions are used as printf wrappers for debugging
 * and verification.
 */

/********************** Debugging code *******************/
/* These functions are testing the internal working of DDA.
 * The convention is to use "printStuff" for testing code,
 * and "writeStuff" for DDA output.
 */
void printBlockAreas(Geometrydata *, Analysisdata *, double **, char *);
void printBlockWeights(Geometrydata *, double **, double **, char *);
void printContactLengths(Geometrydata *, double **, char *);
void printAngles(Geometrydata *, double **, char *);
void printContacts(int **, char *);
void printContactIndex(int **, char *);
void printTimeDeps(double **, int, char *);
void printKForIthBlock(Analysisdata *, int blocki, int blockj, 
                       int * kk, int ** n, char * location);
void printKK(int * kk, int ** n, int numblocks, char * location);
void printK3(int *, char *);
void printK1(int *, char *);
void printParameters(Analysisdata *, char *);
void printGravNormForce(Analysisdata *, char * location);
void printGravShearForce(Analysisdata *, char * location);
void printContactForces(char *);
void printN(int ** n, char * location);
void printM1(int ** m1, char * location);
void printPreviousContacts(int ** m1, char * location);
void printVertices(Geometrydata *, double **, int **, char *);
void printForces(Geometrydata *, double **, int *, char *);
void print2DMat(double [][7], int, int, char *);
void print2DArray(double **, int, int, FILE *, char *);
void print1DIntArray(int *, int n, char *);
void print1DDoubleArray(double *, int n, char *);
void printLocks(int ** locks, char * location);
void printVindex(Geometrydata *, int ** vindex, char *);
void printK(double ** K, int k1size, char * location);
void printDinv(double ** dinv, int block, char * location);
void printLoadPointStruct (Analysisdata * ad, FILE * bfp);
void printPoints(Geometrydata * gd, char * location, FILE *);