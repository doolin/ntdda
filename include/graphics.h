
#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

//#include "dda.h"
//#include "compilecontrol.h"
#include  "ddatypes.h"


#ifdef WIN32
#ifndef STRICT
#define STRICT
#endif
#include <windows.h>
/* Take the pragma out if not using VC++
 */
#pragma hdrstop
#include <windowsx.h>
#else
//typedef long HWND;
//typedef long HDC;
//typedef long MSG;
#endif

/* Collect all objects into one struct to allow platform 
 * independent graphics.
 */

typedef struct _graphics_tag GRAPHICS;


struct _graphics_tag {

   float R[9],G[9],B[9];
   double offsetx;
   double offsety;
   double scale;
 
/*
   int totaloc_count;
   double analysis_runtime;
   double assemble_runtime;
   double integration_runtime;
   double solve_runtime;
   double openclose_runtime;
   double contact_runtime;
   double update_runtime;
 */

#ifdef WIN32
   POINT offset;
#endif
   double radius;

   int numtimesteps;
   int timestep;

#if 0
   //int numcontacts;
   //double currenttime;
   //double elapsedtime;
   //int openclosecount;
#endif


   int centroidsize1;
   int centroidsize2;
   DPoint ** centroids;

   int stressessize1;
   int stressessize2;
   PStress ** stresses;

   //void (*initstresses)(Analysisdata *, GRAPHICS *, int);
  //void (*freestresses)(GRAPHICS *);
};


//GRAPHICS * initGraphicStruct(Geometrydata *, Analysisdata *, GRAPHICS *);
GRAPHICS * initGraphicStruct();
int freeGraphicStruct(GRAPHICS *);

#endif /* _GRAPHICS_H_ */