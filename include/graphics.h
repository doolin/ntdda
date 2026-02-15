
#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include  "ddatypes.h"


#ifdef WIN32
  #ifndef STRICT
  #define STRICT
  #endif
#include <windows.h>
#pragma hdrstop
#include <windowsx.h>
#endif

typedef struct _graphics_tag GRAPHICS;


struct _graphics_tag {

   float R[9],G[9],B[9];
   double offsetx;
   double offsety;
   double scale;

#ifdef WIN32
   POINT offset;
#endif
   double radius;

   int numtimesteps;
   int timestep;

   int centroidsize1;
   int centroidsize2;
   DPoint ** centroids;

   int stressessize1;
   int stressessize2;
   PStress ** stresses;

};


GRAPHICS * initGraphicStruct();
int freeGraphicStruct(GRAPHICS *);

#endif /* __GRAPHICS_H__ */

