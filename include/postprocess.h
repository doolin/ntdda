
/*
 * post processing helper functions.
 */

#ifndef _POSTPROCESS_H_
#define _POSTPROCESS_H_


/******************** Post-processing code ******************/
void writeCentroids(Analysisdata *, GRAPHICS *, int numblocks);
void writeSpringStiffness(Analysisdata *);
void writeFriction(DATALOG *);
void writeBlocks(Geometrydata *, char *);
void writeReplayFile(Geometrydata *, Analysisdata *);
void writeAvgArea(Analysisdata * ad);
void writeTimeInfo(Analysisdata *);
void writeMeasuredPoints(Geometrydata *, Analysisdata *);
void writeFixedPoints(Geometrydata *, Analysisdata *);
void writeMoments(Geometrydata *, double ** moments, int timestep);
void writeBlockareas(Analysisdata *,Geometrydata *);
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


#endif /* _POSTPROCESS_H_ */