/* print.c 
 * 
 * Routine takes a handle to the window, pointer string (?)
 * to a document name (not sure what this means exactly. 
 * Should be general enough that any routine with a graphics
 * to print can call it.  Any options for the printer or 
 * graphics are set in a global OPTIONS struct (see dda.h)
 * and should be set before this function is called.  This 
 * function should define a return variable to indicate a 
 * successful call to the printer.  The basic function is 
 * to make successive calls to draw blocks, lines and points.
 * Extensions should include drawing scales and borders as
 * options from this routine.
 *
 * The basic functionality of this procedure can be extended 
 * write a windows metafile to disk instead of writing to the
 * printer.  This can be done by passing in the device context.
 * 
 * $Author: doolin $
 * $Date: 2003/12/17 23:36:36 $
 * $Source: /cvsroot/dda/ntdda/src/print.c,v $
 * $Revision: 1.5 $
 */


#define STRICT
#include <windows.h>

#include "dda.h"
#include "winmain.h"
#include "resource.h"
#include "wingraph.h"

//extern Geometrydata * geomdata;

void printGeom(HWND hwMain, PSTR szDocName, Geometrydata * geomdata, double scale_params[4], 
               GRAPHICS * g)
{

   extern HPEN printPen;
   extern HBRUSH hBr;
   extern int showOrig;


   int npoi = geomdata->nPoints;
   int nblo = geomdata->nBlocks;
   int nbolts = geomdata->nBolts;


	  PRINTDLG pd;
	  DOCINFO di;
	  //POINT offset;
	  //double radius;
	  double scale;
	  //int i;

	 /* All of this needs to be handled elsewhere, in a separate 
	  * routine.  Eventually, windows metafile saving will be able
	  * to be implemented.
	  */
	  memset (&pd, 0, sizeof(PRINTDLG));
	  pd.lStructSize = sizeof(PRINTDLG);
	  pd.hwndOwner = hwMain;
	  pd.Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION | PD_HIDEPRINTTOFILE;
	  // pd.Flags = PD_RETURNDC | PD_PRINTSETUP;
	  if(!PrintDlg(&pd))
		    return;  /* No printer perhaps.  Ought to return an error message. */
	  di.cbSize = sizeof(DOCINFO);
	  di.lpszDocName = szDocName;
	  di.lpszOutput = NULL;
	  StartDoc(pd.hDC, &di);
	  StartPage(pd.hDC);

	  scale = setPrScale(NULL, pd.hDC, g, scale_params);

	  drawBlocks(pd.hDC, &hBr, g, geomdata);

	  //for(i=0; i<nblo; i++) 
   //{

      if (showOrig)
         //drawLines(pd.hDC, &printPen, scale, offset, porigblocks[i].jp, porigblocks[i].nSides, FALSE);
         drawJoints(pd.hDC, &printPen, g, geomdata, geomdata->origvertices, TRUE);
      //drawLines(pd.hDC, &printPen, scale, offset, pblocks[i].jp, porigblocks[i].nSides, TRUE);
      drawJoints(pd.hDC, &printPen, g, geomdata, geomdata->vertices, FALSE);

      //} /* end for */
				
   if(showOrig) 
      drawPoints(pd.hDC, g, geomdata, geomdata->origpoints);
	  drawPoints(pd.hDC, g, geomdata, geomdata->points);

   if (showOrig)
      drawBolts(pd.hDC, &printPen, geomdata, g, geomdata->origbolts, TRUE);
   drawBolts(pd.hDC, &printPen, geomdata, g, geomdata->rockbolts, FALSE);

	  EndPage(pd.hDC);
	  EndDoc(pd.hDC);
	  DeleteDC(pd.hDC);
	  if(pd.hDevMode != NULL) 
      GlobalFree(pd.hDevMode);
	  if(pd.hDevNames != NULL) 
      GlobalFree(pd.hDevNames);
	 /* return(SuccessMessage);  */

}  /*  Close printGeom().  */





