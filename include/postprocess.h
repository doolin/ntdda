
/*
 * post processing helper functions.
 */

#ifndef _POSTPROCESS_H_
#define _POSTPROCESS_H_

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
void writeBlockVertices(Geometrydata *, int block);


void writeBlockStresses(double ** e0, int block);

/* FIXME: Derive a testing interface for the numerical scheme,
 * and a matlab subsystem for this interface.
 */
void writeMFile(double ** K, double ** F, double ** D,
                int * kk, int * k1, int ** n, int numblocks);
void writeSolutionVector(double ** D, int * kk, 
                         int * k1, int ** n, int nBlocks);

void writeSX0();


#endif /* _POSTPROCESS_H_ */