
/*
 * loghtml.c
 *
 * Provides nicely formatted html output from DDA 
 *
 * $Author: doolin $
 * $Date: 2001/08/26 00:21:21 $
 * $Revision: 1.2 $
 * $Source: /cvsroot/dda/ntdda/src/loghtml.c,v $
 */

/* We need to pass in the file name. *
/* Probably need to include dda.h or some
 * thing also. 
 */
/* This eventually needs to go into its own
 * library, which will be a pain in the butt
 * with respect to headers.
 */


//#include "compilecontrol.h"
#include <stdio.h>
#include <string.h>
#include "dda.h"
#include "datalog.h"
#include "analysisdata.h"
#include "geometrydata.h"
#include "graphics.h"
#if DDA_FOR_WINDOWS
#include "winmain.h"
#endif
#include "ddaerror.h"

static void writeHTMLHeader(FILE * htmlfile);
static void writeHTMLBody(GRAPHICS *, Analysisdata *, FILE * htmlfile);
static void writeHTMLFooter(FILE * htmlfile);
static void writeGnuplotFile();
static void printBarTable(char *, FILE *);


/* variable for html output */
static char bgcolor[20];

extern FILEPATHS filepath;
extern FILEPOINTERS fp;
#if DDA_FOR_WINDOWS
extern OPTIONS options;
#endif
extern DDAError ddaerror;


/* One way to get information into this function is
 * to pass in lists of info, each list corresponding 
 * to a table or cell, and function to write that 
 * table or cell.
 */
int
writeHTMLLogFile(Geometrydata * gd, Analysisdata * ad,
                 DATALOG * dlog, GRAPHICS * g, FILE * htmlfile)
{

   writeHTMLHeader(htmlfile);

  /* Write the data */
   writeHTMLBody(g, ad, htmlfile);

   writeHTMLFooter(htmlfile);

   //fclose(htmlfile);

  /* Now write out a gnuplot file. */
   writeGnuplotFile(gd);


   return 0;

}  /* close writeLogHTML() */


static void
writeHTMLHeader(FILE * htmlfile)
{
   
   fprintf(htmlfile,"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\"\n"); 
   fprintf(htmlfile,"\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"); 

   fprintf(htmlfile,"<html>\n<head>\n");

   fprintf(htmlfile,"<meta name=\"creator\" content=\"DDA for Windows v. 1.6\">\n");
  /* This can be changed in the future for unicode support. */
   fprintf(htmlfile,"<meta http-equiv=\"charset\" content=\"iso-8859-1\">\n");
  
  /* This should be set as either geometry base file name or project
   * geometry base file name.
   */
   fprintf(htmlfile,"<title>DDA Run Log: %s</title>\n", filepath.gfile);

  /* Let's get fancy... */
   fprintf(htmlfile,"<style type=\"text/css\">\n");
   fprintf(htmlfile,"H2 { color: blue; font-style: italic; font: arial; }\n");
   fprintf(htmlfile,"BODY {bgcolor:white;\nmargin-left:10%%;\nmargin-right:10%%;}\n");
   fprintf(htmlfile,"SPAN.error {color:red;}\n");
   fprintf(htmlfile,"</style>\n");

  /* A good candidate would be the cpu run time of the total 
   * program and various functions.
   */

   fprintf(htmlfile,"</head>\n\n");




}  /* close writeHTMLHeader() */


static void
writeHTMLBody(GRAPHICS * g, Analysisdata * ad, FILE * htmlfile)
{

  /* FIXME: Bad division for breaking into functions.
   * All the following belongs in the body, but I need info
   * from the analysis data struct and the constants struct 
   * as well.
   */
   fprintf(htmlfile,"<body>\n");// bgcolor=\"white\">\n\n");

   fprintf(htmlfile,"<h2>DDA Run Log: %s</h2>\n",filepath.gfile);

  /* Open 1 row table and set left column */
   //fprintf(htmlfile,"<table border=0 cellpadding=0>\n<tr>\n <td width=\"10%%\"></td>\n");
   
  /* Open middle column */
   //fprintf(htmlfile,"<td>\n\n");

  /* Now have some tables or something to organize 
   * errors, run time stats, etc.
   */

  /* List errors */
   strcpy(bgcolor,"#FF3333");
   printBarTable("Run-time errors",htmlfile);

  /* FIXME: Move this switch into ddaerror.c for handling. */
   if (ddaerror.error)
   {
      fprintf(htmlfile,"<span class=\"error\">");
      switch (ddaerror.error)
      {
         case __ZERO_MOMENT_ERROR__:
            fprintf(htmlfile,"Negative or zero block volume (numerical instability)");
            break;

         default:
            fprintf(htmlfile,"FIXME: unhandled error condition triggered\n");
            break;
      }
      fprintf(htmlfile,"</span>\n");
   }



  /* List warnings */
   strcpy(bgcolor,"#FFFF33");

   printBarTable("Run-time warnings",htmlfile);
   fprintf(htmlfile, "\n<p>\n<ul>\n");
   fprintf(htmlfile, "<li>Add tension and cohesion joint warnings</li>\n");
   fprintf(htmlfile, "<li>Add warning if the number of joint and block materials "
                     "differ between geometry and analysis files.</li>\n");
   fprintf(htmlfile, "</ul>\n</p>\n");

   fprintf(htmlfile, "\n<hr>\n");

   fprintf(htmlfile, "\n<p>\n<ul>\n");
   fprintf(htmlfile, "<li>Add graphs for max and min velocity, displacement, etc.</li>\n");
   fprintf(htmlfile, "</ul>\n</p>\n");


  /* The plots probably will have to not go with the 
   * bar tables, but for now it is a handy reference.
   */
   strcpy(bgcolor,"#99FF66");


   fprintf(htmlfile,"<table>\n<tr align=\"top\">\n");


   fprintf(htmlfile,"<td align=\"top\">\n");
   fprintf(htmlfile,"<h3>Analysis timing (seconds)</h3>\n");
   fprintf(htmlfile,"<ul>\n");
   fprintf(htmlfile,"<li>Total: %f</li>\n",g->analysis_runtime);
   fprintf(htmlfile,"<li>Contacts: %f</li>\n",g->contact_runtime);
   fprintf(htmlfile,"<li>Assembly: %f</li>\n",g->assemble_runtime);
   fprintf(htmlfile,"<li>Time integration: %f</li>\n",g->integration_runtime);
   fprintf(htmlfile,"<li>Solve: %f</li>\n",g->solve_runtime);
   fprintf(htmlfile,"<li>Open-close: %f</li>\n",g->openclose_runtime);
   fprintf(htmlfile,"<li>Update: %f</li>\n",g->update_runtime);
   fprintf(htmlfile,"</ul>\n");
   fprintf(htmlfile,"</td>\n");
   
   fprintf(htmlfile,"<td align=\"top\">\n");
   fprintf(htmlfile,"<h3>Flag states</h3>\n");
   fprintf(htmlfile,"<ul>\n");
   fprintf(htmlfile,"<li>Auto time step: %d</li>\n",ad->autotimestepflag);   
   fprintf(htmlfile,"<li>Auto penalty: %d</li>\n",ad->autopenaltyflag);
   fprintf(htmlfile,"<li>Max disp.: %f</li>\n",ad->maxdisplacement);
   fprintf(htmlfile,"<li>Max time step: %f</li>\n",ad->maxtimestep);
   fprintf(htmlfile,"<li>Penalty value: %f</li>\n",ad->contactpenalty);
   fprintf(htmlfile,"</ul>\n");
   fprintf(htmlfile,"</td>\n");

   fprintf(htmlfile,"</tr>\n</table>\n");  

   fprintf(htmlfile,"<h3>Open-close count</h3>\n");
   fprintf(htmlfile,"Total number of open-close iterations was %d over %d time steps",
           g->totaloc_count,ad->nTimeSteps);

#if DDA_FOR_WINDOWS
   if (options.spyplots)
   {
      printBarTable("Spy plots",htmlfile);
      fprintf(htmlfile,"<img src=\"spy1.png\">");
      //printBarTable("Spy plot after reordering",htmlfile); 
      fprintf(htmlfile,"<img src=\"spy2.png\">\n\n");
   }
#endif

   printBarTable("Kinetic energy",htmlfile);
   fprintf(htmlfile,"<img src=\"ke.png\">\n\n");

   printBarTable("Virtual work, initial stresses",htmlfile);
   fprintf(htmlfile,"<img src=\"istress.png\">\n\n");

   printBarTable("Spring stiffness",htmlfile);
   fprintf(htmlfile,"<img src=\"spring.png\">\n\n");

   printBarTable("Time increment",htmlfile);
   fprintf(htmlfile,"<img src=\"deltat.png\">\n\n");

   printBarTable("Open-close iterations",htmlfile);
   fprintf(htmlfile,"<img src=\"openclose.png\">\n\n");

   printBarTable("Open-close failures",htmlfile);
   fprintf(htmlfile,"<img src=\"openclosecut.png\">\n\n");



  /* Set right column, then close top-level table */
   //fprintf(htmlfile,"</td>\n <td width=\"10%%\"></td>\n\n</tr>\n</table>\n\n");


} /* close writeHTMLBody() */



static void
writeHTMLFooter(FILE * htmlfile)
{

  /* Put some extra fancy goodies in here before 
   * closing, maybe an hrule or something.
   */

   fprintf(htmlfile,"</body>\n");
   fprintf(htmlfile,"</html>\n");

}  /* close writeHTMLFooter() */


/* Extend this function to be able to take a table background
 * color, a font color, and a header background color.
 */
static void
printBarTable(char * title, FILE * ofp)
{

   fprintf(ofp,"<table border=\"0\" width=\"100%%\">\n");
   fprintf(ofp," <tr>\n  <td bgcolor=\"%s\">\n",bgcolor);
   //<img src="images/small_tri_right.gif" width="15" height="14" border="0" alt="&gt;">
   fprintf(ofp,"   <font color=darkblue face=\"Helvetica, Arial\">\n"
   "    <big><b><i>%s</i></b></big>\n    </font>\n   </td>\n  </tr>\n</table>\n\n",title);

}  /* close printBarTable() */


static void
writeGnuplotFile(Geometrydata * gd)
{

   fprintf(fp.gnuplotfile,"set term png small color\n");
   fprintf(fp.gnuplotfile,"set size .7,.5\n");

   
   fprintf(fp.gnuplotfile,"set logscale y\n");
   fprintf(fp.gnuplotfile,"set output \"spring.png\"\n");
   fprintf(fp.gnuplotfile,"plot \"%s\" using 1:2 with lines 1\n",filepath.datafile);

   fprintf(fp.gnuplotfile,"set nologscale y\n");
   fprintf(fp.gnuplotfile,"set output \"istress.png\"\n");
   fprintf(fp.gnuplotfile,"plot \"%s\" using 1:6 with lines 4\n",filepath.datafile);

   fprintf(fp.gnuplotfile,"set output \"deltat.png\"\n");
   fprintf(fp.gnuplotfile,"plot \"%s\" using 1:3 with lines 2\n",filepath.datafile);

   fprintf(fp.gnuplotfile,"set output \"openclose.png\"\n");
   fprintf(fp.gnuplotfile,"plot \"%s\" using 1:4 with lines 3\n",filepath.datafile);

  /* Fix range on openclose cuts so that they will be easier to 
   * see on the plots. 
   */
   //fprintf(fp.gnuplotfile,"set yrange [-1:%d]\n", maxcuts+1);
   fprintf(fp.gnuplotfile,"set output \"openclosecut.png\"\n");
   fprintf(fp.gnuplotfile,"plot \"%s\" using 1:7 with lines 3\n",filepath.datafile);

   fprintf(fp.gnuplotfile,"set output \"ke.png\"\n");
   fprintf(fp.gnuplotfile,"plot \"%s\" using 1:5 with lines 4\n",filepath.datafile);

   fprintf(fp.gnuplotfile,"set nokey\n");
   fprintf(fp.gnuplotfile,"set xrange [1:%d]\n", gd->nBlocks);
   fprintf(fp.gnuplotfile,"set yrange [%d:1]\n", gd->nBlocks);
   fprintf(fp.gnuplotfile,"set size square\n");
   fprintf(fp.gnuplotfile,"set output \"spy1.png\"\n");
   fprintf(fp.gnuplotfile,"plot \"%s\" with points\n",filepath.spyfile1);
   fprintf(fp.gnuplotfile,"set output \"spy2.png\"\n");
   fprintf(fp.gnuplotfile,"plot \"%s\" with points\n",filepath.spyfile2);



}  /* close writeGnuplotFile() */