
/*
 * post processing helper functions.
 */

#ifndef __POSTPROCESS_H__
#define __POSTPROCESS_H__


#include "datalog.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


/* Set the mask for the stuff we want to look at in an
 * output file.
 */
#define VERTICES        1 << 0
#define FIXEDPOINTS     1 << 1
#define SOLUTIONVECTOR  1 << 2
#define BLOCKMASSES     1 << 3
#define BLOCKSTRESSES   1 << 4  
#define PENALTYFORCES   1 << 5
#define MOMENTS         1 << 6  
#define FRICTIONFORCES  1 << 7  
#define MEASPOINTS      1 << 8
#define BOLTS			   1 << 9


/******************** Post-processing code ******************/
void writeCentroids(Analysisdata *, GRAPHICS *, int numblocks);
void writeSpringStiffness(Analysisdata *);
void writeFriction(Datalog *);
void writeBlocks(Geometrydata *, char *);
void writeReplayFile(Geometrydata *, Analysisdata *);
void writeAvgArea(Analysisdata * ad);
void writeTimeInfo(Analysisdata *);
void writeMeasuredPoints(Geometrydata *, Analysisdata *);
void writeFixedPoints(Geometrydata *, Analysisdata *);
void writeMoments(Geometrydata *, int timestep, int numtimesteps);
void writeBlockMasses(Analysisdata *,Geometrydata *);
void writeMasses(Analysisdata *,Geometrydata *);
void writeSpyfile(int ** n, int * kk, int numblocks, FILE * spyfile1);
void writeBlockVerticesLog(Geometrydata *, int timestep, int block);
void writeAllBlockVerticesMatrix(Geometrydata *, Analysisdata *);


//void writeBoltLog(Geometrydata *, Analysisdata *);


void writeBoltLog (double ** rockbolts, 
                   int numbolts, 
                   int current_time_step, 
                   double elapsed_time);


void writeBoltMatrix(Geometrydata *, Analysisdata *);


void writeBlockStresses(double ** e0, int block);

/* FIXME: Derive a testing interface for the numerical scheme,
 * and a matlab subsystem for this interface.
 */
void writeMFile(double ** K, double ** F, double ** D,
                int * kk, int * k1, int ** n, int numblocks);


void writeSolutionVector(double ** D, int * kk, 
                         int * k1, int ** n, int nBlocks);

void writeSX0();


#ifdef __cplusplus
}
#endif

#endif /* __POSTPROCESS_H__ */