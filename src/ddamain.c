/*
 *  ddamain.c
 *
 *  "console" driver for dda.  This is mostly intended to be used
 * on the unix side.
 *
 * $Author: doolin $
 * $Date: 2002/05/26 23:47:24 $
 * $Source: /cvsroot/dda/ntdda/src/ddamain.c,v $
 * $Revision: 1.3 $
 */

#define OPTIONS 0
#define XML 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#if OPTIONS
#include <opt.h>
#endif
#ifdef XML
#include <gnome-xml/parser.h>
#endif
#include "dda.h"
#include "ddamemory.h"
#include <direct.h>
#include "geometrydata.h"
#include "analysisdata.h"
#include "graphics.h"


/* This function will eventually get fixed to take only a
 * JSYSTEM struct, and be called by the options handling 
 * function.
 */
int rundda(int argc, char ** argv);
/* Accompanies main to handle the options, then pass a 
 * jsystem struct to the function that actually produces
 * the fracture geometry.
 */
static int handleOpts(int, char **);
static void fileInit (char[]);
void closeAFiles (void);
int exportfig(Geometrydata *);


/* TODO: Find a way to get rid of these globals.
 */
FILEPATHS filepath;
FILEPOINTERS fp;

Geometrydata *geomdata;
Geometrydata *geometry2draw;
Analysisdata *ad;
GRAPHICS *g;
InterFace * iface;

#define BUFSIZE 256

#ifdef OPTIONS
typedef struct _options
{
   int ats;
   int gravityflag;
   int planestrain;
   int rotation;
   char infile[256];
   char outfile[256];
   int plot;
} Options;
#endif


int
rundda(int argc, char ** argv)
{
   //HWND hwmain = 0;
   //HDC hdc = 0;

   //char * cwd;

  /* What I need to do here is load the entire path and 
   * the root of the file name into filepath->rootname.
   * This will match the windows side.  Eventually, a
   * rootpath field needs to be added to the filepath struct
   * so that these two names can be kept separate.
   */
   //chdir("../..");
   //cwd = _getcwd(pathbuf,BUFSIZE);
   //fprintf(stdout,"%s\n",cwd);

  /* The path information will need to be handled manually.
   * The win32 calls handle paths more or less transparently
   * to the programmer, which is very convenient, unless 
   * porting to a non-windows application.
   */
   strcpy (filepath.rootname, "../../../../examples/ddaml/squarexml");
   strcat (filepath.gfile, filepath.rootname);
   strcat (filepath.gfile, ".geo");
   strcat (filepath.afile, filepath.rootname);
   strcat (filepath.afile, ".ana");

   g = initGraphicStruct ();

   geomdata = ddacut (&filepath, g);
   
   ddanalysis (&filepath, g);

   exportfig(geomdata);

   //freeGeometrydata (geomdata);
   gdata_delete(geomdata);

   freeGraphicStruct (g);

   return 0;

}				// Close main()


int
main (int argc, char ** argv)
{

#if OPTIONS
   Options * options;
   options = (Options *)calloc(1,sizeof(Options));

   optrega(&options->ats,OPT_INT,'t',"auto-time-step","Adaptive time stepping");
   optrega(options->infile,OPT_STRING,'i',"input-file","Input file name");
   optrega(options->outfile,OPT_STRING,'o',"output-file","Output file root name");
   optrega(&options->rotation,OPT_INT,'r',"rotation", "Use exact post-correction");
   optrega(&options->gravityflag,OPT_INT,'g',"gravity","Initial gravity phase");
   optrega(&options->planestrain,OPT_DOUBLE,'p',"plane-strain","Use plain strain");
   optrega(&options->plot,OPT_FLAG,'x',"xfig","XFig graphics file");

   optMain(rundda);

   opt(&argc,&argv);

  /* TODO: put a function between main and the driver code
   * that handles all of the options parsing.  The result 
   * with using libopt then means that instead of a single
   * main method to drive a code, there is a main method
   * that handles option registration, which then calls 
   * an options handler, which then calls the functions
   * to produce the output.  This second function is what 
   * will be called as a library function from external 
   * programs.
   */
#endif /* OPTIONS */
   rundda(argc,argv);

  /* FIXME: These need to be freed somehow because I am losing
   * memory here.
   */
   //free(options->infile);
   //printf("Here\n");
   //free(options->outfile);

   //free(options);

   return 0;

}				/* close main() */


#if OPTIONS
/* This is the layer between the main function and the 
 * function to actually produce joints.  It will load
 * a struct using the options code.  In an external function,
 * assume that the jsystem struct is loaded elsewhere, then
 * this function is bypassed and the joint generating 
 * function is called directly.
 */
static int
handleOpts(int argc, char ** argv)
{

   return 0;

}  /* close handleOpts() */
#endif  /* OPTIONS */


