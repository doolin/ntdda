
/* 
 * analysis.h
 * 
 * This is the main header file for the analysis routines
 * in DDA.
 *
 * $Author: doolin $
 * $Date: 2003/12/17 23:36:36 $
 * $Revision: 1.24 $
 */

#ifndef __ANALYSIS_H__
#define __ANALYSIS_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



#include "dda.h"
#include "contacts.h"
#include "functions.h"

#include "stress.h"


/* These are the old variable names, retained for 
 * historical purposes.
 */
/*------------------------------------------------*/
/* n1: number of blocks                           */
/* n2: number of contacts                         */
/* n3: number of 6*6 matrices of a                */
/* n4: number of bolt or cable elements           */
/* n5: number of time steps                       */
/* n6: number of previous contacts                */
/* n7: number of first dimension                  */
/* n8: number of second dimension                 */
/* n9: number of iterations                       */
/*------------------------------------------------*/
/* nf: number of fixed points                     */
/* nl: number of load  points                     */
/* nm: number of measured points                  */
/* nb: number of block materials                  */
/* nj: number of joint materials                  */
/* nt: number of input time points                */
/* np: number of special points   = nf+nl+nm      */
/*------------------------------------------------*/

/* pre-defined changeble constants                */
/* s0 : criteria of open-close                    */
/* f0 : criteria of opening                       */
/* g3 : allowable normal spring penetration ratio */
/* d0 : normal external    distance for contact   */
/* d9 : normal penetration distance for contact   */
/* h1 : angle overlapping degrees   for contact   */
/* h2 : ratio for shear spring versus normal one  */


/**************************************************/
/* df01: input geometric data                     */
/* df02: input physical data                      */
/* df26: draw deformed blocks                     */
/*------------------------------------------------*/
/* df03: mechanical parameters of blocks          */
/* df04: contact finding by distance criteria     */
/* df05: contact finding by angle criteria        */
/* df06: contact transfer                         */
/* df07: contact initialization                   */
/* df08: positions of non-zero storage            */
/* df08: block order                              */
/* df09: time interpolation                       */
/*------------------------------------------------*/
/* df10: initiate coefficient and load matrix     */
/* df11: submatrix of inertia                     */
/* df12: submatrix of fixed points                */
/* df13: submatrix of stiffness                    */
/* df14: submatrix of initial stress              */
/* df15: submatrix of point loading               */
/* df16: submatrix of volume force                */
/*------------------------------------------------*/
/* df18: iteration start                          */
/* df18: add and subtract submatrix of contact    */
/* df20: triangle discomposition equation solver  */
/* df21: backward and forward substitution        */
/* df22: contact judge after iteration            */
/* df22: iteration output                         */
/* df24: displacement ratio and iteration drawing */
/*------------------------------------------------*/
/* df25: compute step displacements               */
/* df26: draw deformed blocks                     */
/* df27: save results to file                     */
/*------------------------------------------------*/
/* dspl: block displacement matrix           0001 */
/* invr: inverse of 6*6 matrix               0002 */
/* mult: muliplication of 6*6 matrices       0003 */
/* sign: return sign                         0004 */
/* cons: set control constants               0005 */
/* step: compute next time interval          0006 */
/* spri: compute stiffness of contact spring 0007 */
/* exti: extent 2-d integer arrays           0008 */
/* extd: extent 2-d double  arrays           0009 */
/* area: compute s0 sx sy sxx syy sxy        0010 */
/* mblo: material constants of blocks        0011 */
/* mjoi: material constants of joints        0012 */
/* angl: material constants of bolts         0013 */
/* dist: distance of a segment to a node     0014 */
/* proj: projection of a edge to other edge  0015 */
/* graf: graph mode                          0016 */
/* fill: fill blocks with colors             0017 */
/* move: move                                0018 */
/* draw: draw                                0019 */
/* fild: fill deformed blocks with colors    0020 */
/* writ: write charactors                    0021 */
/**************************************************/
/* call subroutines            n9:total iteration */


void allocateAnalysisArrays(Geometrydata * GData, 
                            int ** kk, 
                            int ** k1, 
                            double *** c0, 
                            //double *** e0,
                            double *** angles, 
                            int *** n);


void deallocateAnalysisArrays(int * kk, 
                              int * k1, 
                              double ** c0, 
                              //double ** e0, 
                              double ** U, 
                              int ** n);






double df01  (double ** vertices, 
              int ** vindex, 
              int numblocks);







void setInitialLocks(Geometrydata * gd, Analysisdata * ad, int **contacts, int **locks);

void df06(Geometrydata *, Analysisdata *, int **, int **, int **, int **, double **,
            int *, int *, int *);

void sparsestorage(Geometrydata *, Analysisdata *, Contacts *, 
                   int *, int *, 
                   int *, int **);




/******************  Solver *********************/
/* FIXME:  All this needs to go into its own header file. */
void saveState(double ** K, double ** Kcopy, int n3,
                  double ** F, double ** Fcopy, int numblocks,
                  double ** c0);

void restoreState(double ** K, double ** Kcopy, int n3,
                  double ** F, double ** Fcopy, int numblocks);


void solve(Analysisdata *, double ** K, double ** F,
      int * kk, int * k1, int ** n, int numblocks);

void congrad(double ** A, double ** F, int * colnums, int * k1, int ** colindex, int numblocks);

void initCGTemp(Geometrydata * gd);

void df18(Geometrydata *, Analysisdata *, Contacts *, 
          int *, int *, double **,  int **,
          TransMap transmap);


void df22(Geometrydata *, Analysisdata *, Contacts *, 
            /*double **,*/ int * /*, double ** moments */,
            TransMap);

double df24(Geometrydata *, Analysisdata *, double ** F,
          int * k1, TransMap transmap, TransApply transapply);

void df25(Geometrydata *, Analysisdata *, int *, 
            double **, /* double ** moments,*/ double **,
            TransMap transmap, TransApply transapply, 

            BoundCond boundary_conditions, StrainModel strain_compute);




void computeTimeStep(Geometrydata *, Analysisdata *);


void proj(double ** vertices, double w0, double **, int *, 
            int, int, int, int, int /* , CONSTANTS *  */);
void invr(double [][7], double [][7]);
void mult(double [][7], double [][7], double [][7]);
void multnew(double [][7], double [][7]);






void findContacts(Geometrydata * GData, Analysisdata * AData, 
                  Contacts * Ctacts,
            /*double ** contactlength,*/ /*int ** m1,*/ int * kk, int * k1,
            int ** n, double ** c0);

void assemble(Geometrydata * GData, Analysisdata * AData,
            int ** locks, double ** e0, int * k1, int * kk,
            /* double ** moments, */int ** n, double ** U, TransMap transmap);

void timeintegration(Geometrydata * GData, 
                     Analysisdata * AData,
                     double ** e0, 
                     int * k1,
                     //double ** moments,
                     int ** n, 
                     double ** U, 
                     MassMatrix massmatrix);



void postProcess(Geometrydata * GData, Analysisdata * AData);


/*  Functions for handling sparse storage */
void initdf08(int ** n,int * kk,int nBlocks);





void blockNumberOfVertex(Geometrydata *);
void savePreviousContacts(Geometrydata *, Analysisdata *,int **, int **, int **, double **);


void closeAnalysisFiles();
int porepressure(Geometrydata *, int *, double **, double **, double **);
int inpoly(double **, int, double, double);

int viscosity(Geometrydata *, Analysisdata *, double **, 
              double **, double **);


/* Friction law stuff */
double computeFriction(Geometrydata * gd, /*Analysisdata *,*/ int);
double computeVFriction(Geometrydata * gd, /*Analysisdata *,*/ int units, double);


double getBlockMass(Geometrydata *, double ** moments, double ** e0, int blocknum);

/* Spring stiffness */
double computeSpringStiffness(Geometrydata *, Analysisdata *, int **, double **);
      
int checkParameters(Geometrydata * GData, Analysisdata * AData, Contacts *, FILE *);  





/******************** Initialization code *******************/
void initNewAnalysis(Geometrydata *, Analysisdata *, double **,
            /* double **, */ Filepaths * filepath);


/* density, unit weight etc, includes stress, strain */
void initBlockProperties(Geometrydata *, Analysisdata *, double **);
void initContactSpring(Analysisdata *);
//void initGravity(Analysisdata *, int);
void initVelocity(Geometrydata *, Analysisdata *, double **);
void initStorageTempArrays(int numblocks);
void initSolverTempArrays(Geometrydata *);
void initContactTempArrays(int numblocks, int vertexcount);
void initReplayFile(Geometrydata *, Analysisdata *);
void initIntTempArrays(int size1);
void initIntegrationArrays(Geometrydata *, Analysisdata *);

void congrad(double ** A, double ** F, int * colnums, int * k1, int ** colindex, int numblocks);


void freeContactTempArrays(void);
void freeIntTempArrays(void);
void freeIntegrationArrays(void);

void allocateK(Analysisdata *);


/* These defines are a first attempt to impose some 
 * rational memory handling on the dozens of scratch
 * arrays needed for an analysis run.  The values will
 * be used to set the sizes of the array dimensions, which 
 * can then be used again in deallocation routines. 
 * The next go-round will probably require using an 
 * array struct and function pointers. 
 */
#define ELEVEN 11
#define TWENTYONE 21
#define FORTY 40



/***************** Memory allocation globals *******/
/* These global ints are used in the ddanalysis function,
 * the deallocateAData function, and a yet-to-be-written
 * function that will initialize all of these variables.
 * In the future, this can be morphed into 
 * object oriented code.
 */
int __contactsize1, __contactsize2;
int __locksize1, __locksize2;
int __contactindexsize1, __contactindexsize2;
int __prevcontactsize1, __prevcontactsize2;
int __contactlengthsize1, __contactlengthsize2;
int __kksize1;
int __k1size1;
int __fsize1, __fsize2;
int __rrsize1, __rrsize2;
int __c0size1, __c0size2;


int __matpropsize1, __matpropsize2;  // was e0


int __v0size1, __v0size2;
int __Usize1, __Usize2;
int __nsize1, __nsize2;

#ifdef __cplusplus
}
#endif

#endif  /* __ANALYSIS_H__ */
