/*
 * dda.c is the "controller" module for running 
 * an analysis.  It has to deal with both geometry
 * and analysis data.
 */

#include "dda.h"


/* This doesn't work yet, because the get/sets aren't 
 * written.
 */
/*
struct _dda_tag  {

   Geometrydata * geometry;
   Analysisdata * analysis;
   FILEPATHS * filepaths;
   int menustate;
   BOOL toolbarvis;
   BOOL statusbarvis;
   BOOL popupvis;
   BOOL tooltipvis;

};
*/

ddaboolean
dda_get_toolbarvis(DDA * dda)
{
   return dda->toolbarvis;
}

void
dda_set_toolbarvis(DDA * dda, ddaboolean vis)
{
   dda->toolbarvis = vis;
}


ddaboolean
dda_get_statusbarvis(DDA * dda)
{
   return dda->statusbarvis;
}

void
dda_set_statusbarvis(DDA * dda, ddaboolean vis)
{
   dda->statusbarvis = vis;
}

ddaboolean
dda_get_popupvis(DDA * dda)
{
   return dda->popupvis;
}

void
dda_set_popupvis(DDA * dda, ddaboolean vis)
{
   dda->popupvis = vis;
}


ddaboolean
dda_get_tooltipvis(DDA * dda)
{
   return dda->tooltipvis;
}

void
dda_set_tooltipvis(DDA * dda, ddaboolean vis)
{
   dda->tooltipvis = vis;
}


Analysisdata * 
dda_get_analysisdata(DDA * dda)
{
   return dda->analysis;
}

void
dda_set_analysisdata(DDA * dda, Analysisdata * ad)
{
   dda->analysis = ad;
}

Geometrydata * 
dda_get_geometrydata(DDA * dda)
{ 
   return dda->geometry;
}

void 
dda_set_geometrydata(DDA * dda, Geometrydata *gd)
{
   dda->geometry = gd;
}

DDA *
dda_new()
{
   DDA * dda;
   dda = (DDA*)malloc(sizeof(DDA));
   memset(dda,0xda,sizeof(DDA));
   return dda;
}

dda_destroy(DDA * dda)
{
   /* These functions have not yet been written. */
   adata_destroy(dda->analysis);
   gdata_destroy(dda->geometry);
   //filepaths_destroy(dda->filepaths);
   memset(dda,0xdd,sizeof(DDA));
   free(dda);
}

int 
dda_run(DDA * dda)
{

   return 0;
}


ddaboolean
dda_test()
{
   DDA * dda;

   dda = dda_new();

   dda_destroy(dda);
   
   return FALSE;
}


#ifdef STANDALONE
int 
main()
{
   if(dda_test())
      printf("Passed DDA test\n");
   else
      printf("Failed DDA test\n");

return 0;
}
#endif /* STANDALONE */







