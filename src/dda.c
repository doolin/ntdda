/*
 * dda.c is the "controller" module for running 
 * an analysis.  It has to deal with both geometry
 * and analysis data.
 */


#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#define getcwd _getcwd
#endif


#include "dda.h"

/** @todo These are included to handle toggling 
 * code, get rid of them as soon as possible.
 */
#include "statusbar.h"
#include "toolbar.h"



struct _dda_tag  {

   Geometrydata * geometry;
   Analysisdata * analysis;
   Filepaths * filepaths;
   int menu_state;

   ddaboolean popupvis;
   ddaboolean tooltipvis;
   short xcur;
   short ycur;
};

int 
dda_get_menu_state(DDA * dda) {
   return dda->menu_state;
}

void 
dda_set_menu_state(DDA * dda, int state) {
   dda->menu_state = state;
}


int 
dda_get_xcur(DDA * dda) {
   return dda->xcur;
}

void 
dda_set_xcur(DDA * dda, int xcur) {
   dda->xcur = xcur;
}

int 
dda_get_ycur(DDA * dda) {
   return dda->ycur;
}

void 
dda_set_ycur(DDA * dda, int ycur) {
   dda->ycur = ycur;
}

ddaboolean
dda_get_toolbarvis(DDA * dda) {
   return (ddaboolean)toolbar_get_visibility();
}

void
dda_set_toolbarvis(DDA * dda, ddaboolean vis) {
   toolbar_set_visibility((int)vis);
}


/** Kinda kludgy, need to get rid of the DDA struct
 * layer for toggling status bar visibility.
 */
ddaboolean
dda_get_statusbarvis(DDA * dda) {
   return (ddaboolean)statusbar_get_visibility();
}

void
dda_set_statusbarvis(DDA * dda, ddaboolean vis) {
   statusbar_set_visibility((int)vis);
}

ddaboolean
dda_get_popupvis(DDA * dda) {
   return dda->popupvis;
}

void
dda_set_popupvis(DDA * dda, ddaboolean vis) {
   dda->popupvis = vis;
}


ddaboolean
dda_get_tooltipvis(DDA * dda) {
   return dda->tooltipvis;
}

void
dda_set_tooltipvis(DDA * dda, ddaboolean vis) {
   dda->tooltipvis = vis;
}


Analysisdata * 
dda_get_analysisdata(DDA * dda) {
   return dda->analysis;
}

void
dda_set_analysisdata(DDA * dda, Analysisdata * ad) {
   dda->analysis = ad;
}

Geometrydata * 
dda_get_geometrydata(DDA * dda){ 
   return dda->geometry;
}

void 
dda_set_geometrydata(DDA * dda, Geometrydata * gd) {
   dda->geometry = gd;
}

DDA *
dda_new() {
   DDA * dda;
   dda = (DDA*)malloc(sizeof(DDA));
   memset(dda,0xda,sizeof(DDA));

   //dda->geometry = gdata_new();

   return dda;
}

void
dda_delete(DDA * dda) {
   /* These functions have not yet been written. */
   //adata_delete(dda->analysis);
   //gdata_delete(dda->geometry);
   //filepaths_delete(dda->filepaths);
   memset(dda,0xdd,sizeof(DDA));
   free(dda);
}


int 
dda_run(DDA * dda) {

   return 0;
}


void
dda_set_output_directory(const char * dirname, size_t dirnamesize) {

#define BUFSIZE 1024

   int checkval;
   char wdbuf[BUFSIZE];
   char * outdir;
   char * outdirenv;

   outdir = malloc(BUFSIZE);
   outdirenv = malloc(BUFSIZE);

   getcwd(wdbuf,BUFSIZE);


   strncpy(outdir,wdbuf,sizeof(wdbuf));
   strncat(outdir,"\\",sizeof("\\"));
   strncat(outdir,dirname,dirnamesize);

/** This is really crappy. Should use stat or here. 
 * MS doesn't list errno for stat, so checking to 
 * see whether the directory already exists will be 
 * done with mkdir.  Smells like a kludge.
 */
   checkval = _mkdir(outdir);
 

 /* @todo rewrite all of this because EEXIST
  * doesn't matter.  Need only check for ENOENT.
  */
#if 0
   if (checkval == 0) {  // 0 is success for this call

      strncpy(outdirenv,"DDA_OUT_DIR",sizeof("DDA_OUT_DIR"));
      strncat(outdirenv,outdir,sizeof(outdir));
      _putenv(outdirenv);
      //_chdir(outdir);
   } else {
      
      dda_display_error("Problem with setting environment");
   }
#endif

      strncpy(outdirenv,"DDA_OUT_DIR",sizeof("DDA_OUT_DIR"));
      strncat(outdirenv,outdir,sizeof(outdir));
      _putenv(outdirenv);


   free(outdir);

}


ddaboolean
dda_test() {

   DDA * dda;
   dda = dda_new();
   dda_delete(dda);
   return FALSE;
}


#ifdef STANDALONE
int 
main() {

   if(dda_test())
      printf("Passed DDA test\n");
   else
      printf("Failed DDA test\n");

return 0;
}
#endif /* STANDALONE */







