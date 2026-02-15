

#include "graphics.h"
#include "ddamemory.h"


GRAPHICS *
initGraphicStruct()
{
   //int i;
   GRAPHICS * g;


  /* This is never freed, so we have a pretty big
   * memory leak here.  FIXME: Write an initialization
   * function for this struct.
   */
   g = (GRAPHICS *)calloc(1,sizeof(GRAPHICS));
   //g->numpoints = 0;
   //g->numblocks = 0;
   //g->numbolts = 0;
   //g->numjoints = 0;
   //g->points = NULL;
   //g->origpoints = NULL;
   //g->joints = NULL;
   //g->blocks = NULL;
   //g->origblocks = NULL;
   //g->bolts = NULL;
   //g->origbolts = NULL;

			g->R[0] = 0.0; g->G[0] = 0.0; g->B[0] = 0.0;			// black
			g->R[1] = 0.0; g->G[1] = 0.0; g->B[1] = 0.0;			// black
			g->R[2] = 1.0; g->G[2] = 0.0; g->B[2] = 0.0;			// red
			g->R[3] = 0.0; g->G[3] = 0.0; g->B[3] = 1.0;			// blue
			g->R[4] = 0.0; g->G[4] = 1.0; g->B[4] = (float)(64/255);	// green
			g->R[5] = 1.0; g->G[5] = 0.0; g->B[5] = 1.0;			// purple
			g->R[6] = 1.0; g->G[6] = 1.0; g->B[6] = 0.0;			// yellow
			g->R[7] = (float)(128/255); g->G[7] = (float)(64/255); g->B[7] = 0.0;	// brown
			g->R[8] = 1.0; g->G[8]= (float)(128/255); g->B[8] = 0.0;	// olive
			g->R[9] = (float)(192/255); g->G[9] = (float)(192/255); g->B[9] = (float)(192/255);	// gray

   //g->initstresses = initStresses;
   //g->freestresses = freeStresses;

   return g;

}  /* Close initializeGraphStruct() */




int
freeGraphicStruct(GRAPHICS * g)
{

  /* Boy, this is a good way to lose a lot of memory...
   * FIXME: Warn or throw if g is NULL on entry.
   */
   if (g == NULL)
      return 0;
   //g->freestresses(g);

   free(g);

   return 0;
}  /* Close freeGraphicStruct() */




