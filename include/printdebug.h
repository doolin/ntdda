
/*
 * These functions are used as printf wrappers for debugging
 * and verification.
 */



#include "analysis.h"



/** @todo Almost all of these functions can be prototyped
 * with primitives to cut the dependency on the dda
 * derived types.  After this is done, it will be easier
 * to abstract some printing functions, callbacks,
 * etc.
 */
void printBlockAreas(Geometrydata *, Analysisdata *, double **, char *);
void printBlockWeights(Geometrydata *, double **, double **, char *);


void printAngles(Geometrydata *, double **, char *);

void printTimeDeps(double **, int, char *);
void printKForIthBlock(Analysisdata *, int blocki, int blockj,
                       int * kk, int ** n, char * location);
void printKK(int * kk, int ** n, int numblocks, char * location);
void printK3(int *, char *);
void printK1(int *, char *);
void printParameters(Analysisdata *, char *);


void printN(int ** n, char * location);
void printM1(int ** m1, char * location);

void printContactForces(char *);


void printContacts(int **, char *);
void printContactIndex(int **, char *);
void printContactLengths     (Geometrydata *, double **, char *);
void printPreviousContacts(int ** m1, char * location);
void printLocks(int ** locks, char * location);


void printVertices(Geometrydata *, double **, int **, char *);

void printForces             (int numblocks,
                              double ** F,
                              int * k1,
                              char * calling_location);

void print2DMat(double [][7], int, int, char *);
void print2DArray(double **, int, int, FILE *, char *);
void print1DIntArray(int *, int n, char *);
void print1DDoubleArray(double *, int n, char *);
void printLockStates(int lockstates[3][5], char * location);
void printVindex(Geometrydata *, int ** vindex, char *);
void printK(double ** K, int k1size, char * location);
void printDinv(double ** dinv, int block, char * location);



void printPoints(Geometrydata * gd, char * location, FILE *);