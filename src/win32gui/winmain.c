/* 
 * winmain.c
 * 
 * DDA for Windows NT
 * 
 * This is the main gui message handling code to run the 
 * dda gui interface.
 * 
 * $Author: doolin $
 * $Date: 2006/03/07 05:39:54 $
 * $Source: /cvsroot/dda/ntdda/src/win32gui/winmain.c,v $
 * $Revision: 1.37 $
 */


#include "winmain.h"

#include <commdlg.h>
#include <commctrl.h>
#include <assert.h>
#include <assert.h>
#include <process.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <htmlhelp.h>
#include <tchar.h>
#include "math.h"


#include "win32utils.h"
#include "resource.h"
#include "wingraph.h"
#include "graphics.h"
#include "replay.h"
#include "toolbar.h"
#include "statusbar.h"
#include "runstates.h"


/* Win32 calling conventions. */
#ifndef CALLBACK
#define CALLBACK
#endif
/* These are used in a switch statement, to determine
 * the action of WM_PAINT.  
 */
#define NOTHING 0
#define LINES 1
#define BLOCKS 2
#define TITLE 3


/*  Whole bunch of global variables that need to disappear fast. */
char szAppName[] = "Berkeley DDA for Windows";
char mess[80];  /* mess appears to be a temporary variable.  Might be able to get rid of it. */

char mainWinTitle[120];


#define ABOUT "UC Berkeley DDA for Windows 95/NT(unstable),\n", \
              "$Id: winmain.c,v 1.37 2006/03/07 05:39:54 doolin Exp $\n", \
				  "by Mary M. MacLaughlin (Montana Tech), and Nicholas Sitar & David Doolin\n", \
              "Department of Civil Engineering, Geotechnical Group\n", \
              "University of California, Berkeley, CA 94720\n", \
              "This program is based on the original code by Dr. Gen-hua Shi.\n", \
              "Original development was supported in part by funding from the\n", \
              "U.S. Army Engineer Waterways Experiment Station, Vicksburg, MS.\n", \
              "This executable was compiled for internal use only."


int whatToDraw = TITLE;

int showOrig = FALSE;



/* Used by mouse handler to scroll around the screen. */
int xoff = 2500;
int yoff = 2500;
long zoom = 25000;



/* These set the cursor location on a mouse button down 
 * or mouse button up.  These should be moved into DDA 
 * struct so that the mouse handling can be moved into 
 * a different file.
 */
int xcursor;
int ycursor;




/* Unfortunately, several variables are better 
 * handled as externs for the time being.  Later
 * these can be handled in hash tables or something
 * to provide the ability for more than one analysis
 * at a time.
 */
Options options;
Filepaths filepath;


GRAPHICS * g = NULL;


/* For HTML Help */
DWORD dwHTMLCookie;


/* handleWMCommand has to be declared here to 
 * avoid namespace collision with the analysis
 * dialog box.  handleMouseMove collides with 
 * same in drawdialog().
 */
/* FIXME: Orthogonalize header files. */
//static int handleWMCommand(HWND, WPARAM wParam, LPARAM lParam);

static void displayBlockNumber(HWND hwMain, WPARAM wParam, LPARAM lParam);

static void deleteBlockNumber(HWND hwMain, WPARAM wParam, LPARAM lParam);

static void insertNewMainMenuItem(HWND hwMain);


void displayPhysicalCoordinates(HWND hwMain, WPARAM wParam, LPARAM lParam);
DPoint DPtoPP(HWND hwMain, int xpos, int ypos);
   
/* Move this to its own header file. */
int CreateTestPropSheet(HWND hwMain, Analysisdata *);


/* These 2 got to go. */
HWND mainwindow;
//InterFace * iface;



void
dda_display_error(const char * message) {

   MessageBox(NULL,message,"Error",MB_OK | MB_ICONERROR);
   exit(0);
}

void
dda_display_warning(const char * message) {

   MessageBox(NULL,message,"Warning",MB_OK | MB_ICONWARNING);
}


void
dda_display_info(const char * message) {

   MessageBox(NULL,message,"Info",MB_OK | MB_ICONINFORMATION);
}




static void 
initializeDDAForWindows(HWND hwMain, WPARAM wParam, LPARAM lParam) {

   HINSTANCE hInst;
   INITCOMMONCONTROLSEX icex;

   icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
   InitCommonControlsEx(&icex);


  /* This might be a problem too... */
   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   initializePens();
   initializeBrushes();
   
  /* FIXME: g is handling lots of things it ought not 
   * be handling.  The win32 gui control parts need to 
   * be abstracted out.
   */ 
   g = initGraphicStruct();

   statusbar_init(hwMain);
   statusbar_show();

   toolbar_init(hwMain);
   toolbar_show();


  /* Handle a possible command line argument from 
   * a file drag and drop.
   */
   if (__argc > 1) {
      handleCommandLine(hwMain, __argc, __argv, &filepath);
   }
} 




static void 
handleMouseMove(HWND hwMain, WPARAM wParam, LPARAM lParam) {

   POINTS mousepos;
   char mess[80];
   int wparamlo, wparamhi;
   int xnew, ynew;
   int deltax, deltay;

  /* Handles status bar updates */
   int blocknumber;
   char blocknumtext[STATUSBAR_TEXT_WIDTH];
   char xycoordstext[STATUSBAR_TEXT_WIDTH];
   DPoint p;
   DDA * dda = (DDA *)GetWindowLong(hwMain, GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);

   mousepos = MAKEPOINTS(lParam);
   xnew = mousepos.x;
   ynew = mousepos.y;

   dda_set_xcur(dda,mousepos.x);
   dda_set_ycur(dda,mousepos.y);

  /* There is a serious bug somewhere that allows a null vertex and 
   * vindex arrays to pass into this block.  Not sure where or why this
   * is happening. in the meantime, try this instead... 
   */
   if (dda_get_menu_state(dda) == (GEOM_STATE | FINISHED) ) {
      p = DPtoPP(hwMain, xnew,ynew);
      blocknumber = gdata_get_block_number(geomdata,p.x,p.y);
      sprintf(blocknumtext,"BN: %d",blocknumber);
      sprintf(xycoordstext,"\t(%.3f,%.3f)\t",p.x,p.y);
      statusbar_set_text((WPARAM)2,(LPARAM)blocknumtext);
      statusbar_set_text((WPARAM)5,(LPARAM)xycoordstext);
   }

   if (dda_get_menu_state(dda) == (ANA_STATE | FINISHED) ) {
      p = DPtoPP(hwMain, xnew,ynew);
      sprintf(xycoordstext,"\t(%.3f,%.3f)\t",p.x,p.y);
      statusbar_set_text((WPARAM)9,(LPARAM)xycoordstext);
   }

  /* FIXME:  This is a kludgy way to handle this. */
   if (whatToDraw == LINES || whatToDraw == BLOCKS)
   {
     /* Get the value of LOWORD and HIWORD, print them to a 
      * temp string, examine the temp string in a 
      * MessageBox.  The idea is to examine to be able
      * see how these various keys and mouse buttons may 
      * be combined.
      */
      wparamlo = LOWORD(wParam);
      wparamhi = HIWORD(wParam);
      sprintf(mess,"x: %d;  y: %d", wparamlo, wparamhi);
      switch (wparamlo) 
      {
         case MK_CONTROL: 
            break;

         case MK_SHIFT: 
            break;

         case (MK_LBUTTON+MK_CONTROL): 
      
            deltax = xnew - xcursor;
            deltay = ycursor - ynew;

            xoff = xoff + deltax;
            yoff = yoff + deltay;

			   InvalidateRect(hwMain, NULL, TRUE);
			   UpdateWindow(hwMain);

            break;

        /* WM_RBUTTONUP handles in WMCOMMAND.  
         * But RBUTTON pressed with a mouse 
         * move will work here.
         */
         case (MK_LBUTTON+MK_SHIFT):
           /* Zoom in for up, zoom out for down.  This is 
            * going to be tricky to catch a good scale to 
            * be useful.
            */            
            deltay = ycursor - ynew;
            zoom = zoom + deltay;

		      InvalidateRect(hwMain, NULL, TRUE);
	         UpdateWindow(hwMain);

            break;

         case (MK_RBUTTON+MK_SHIFT):
            break;

         case (MK_RBUTTON+'c'):
            //MessageBox(NULL,"c","c",MB_OK);
            break;

         case MK_MBUTTON: 
            break;

         default:
            break;
      }

   }

}  /* close handleMouseMove() */



static int
handleChar(HWND hwMain, WPARAM wParam, LPARAM lParam) {

   char message[80];
   int vkcode;
   int xnew, ynew;
   int blocknumber;
   DPoint p;
   DDA * dda = (DDA *)GetWindowLong(hwMain, GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);


   xnew = dda_get_xcur(dda);
   ynew = dda_get_ycur(dda);


   /* FIXME:  This is a kludgy way to handle this. */
   if (whatToDraw == LINES || whatToDraw == BLOCKS) {
      vkcode = wParam;

      switch (vkcode) {
      case 'c': {
            double x0, y0;
            //displayPhysicalCoordinates(hwMain,wParam,lParam);
            p = DPtoPP(hwMain, xnew,ynew);
            blocknumber = gdata_get_block_number(geomdata,p.x,p.y);
            if (blocknumber != 0) {
               gdata_get_centroid(geomdata->moments[blocknumber],&x0,&y0);
               sprintf(message,"Block %d centroid: (%lf,%lf)",blocknumber,x0,y0);
               dda_display_info(message);
               //MessageBox(NULL,mess,"Block centroid coordinates",MB_OK);
            }
       }
       break;

       case 'a': {
            double blockarea;
            p = DPtoPP(hwMain, xnew,ynew);
            blocknumber = gdata_get_block_number(geomdata,p.x,p.y);
            if (blocknumber != 0) {
               blockarea = gdata_get_block_area(geomdata,blocknumber);
               sprintf(message,"Block %d area: %lf",blocknumber,blockarea);
               dda_display_info(message);
               //MessageBox(NULL,mess,"Block centroid coordinates",MB_OK);
            }
       }
       break;

      case VK_F1:
         MessageBox(NULL,"foo","bar",MB_OK);
         break;

         default:
            break;
      }
   }

   return 0;

}  /* close handleChar() */



static int
handleSysChar(HWND hwMain, WPARAM wParam, LPARAM lParam) {


   return 0;

}  /* close handleSysChar() */


static int
handleKeydown(HWND hwMain, WPARAM wParam, LPARAM lParam) {

#if 0
   char mess[128];
   sprintf(mess, "nVirtKey: %d",wParam);
   MessageBox(NULL,mess,"keydown",MB_OK);
#endif

   return 0;

}  





/* Handlers follow, in approximate order of menu entry for 
 * convenience.
 */

static void
handleTitle(HWND hwMain) { 

  /* Just draw it once in lieu of splash screen */
  /* FIXME: Make this a handleSplash function. */
   static int draw = 1;
           

   if (draw == 0) {

      return;

   }
   draw = 0;

   handleMainAbout(hwMain);
   //drawTitle(draw_wnd, hdc);
}


/***************  PAINT HANDLERS **********************/
void
handleWinPaint(HWND hwMain, WPARAM wParam, LPARAM lParam, int width, int height)
{
   extern HPEN drawPen[10];
   extern HPEN screenPen[10];
   extern HBRUSH hBr[6];
   HWND draw_wnd;


   HDC hdc; 
   PAINTSTRUCT ps;
   RECT rectClient;
   Geometrydata * geomdata;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);

   draw_wnd = hwMain;

   hdc = BeginPaint(draw_wnd, &ps );


   switch (whatToDraw)
   {
      case LINES:
         geomdata = dda_get_geometrydata(dda);
         g->scale = setScale(draw_wnd, hdc, g, geomdata->scale);
			//drawLines(hdc, screenPen, g->scale, g->offset, g->joints, geomdata->nJoints, TRUE);
         drawJoints(hdc, screenPen, g, geomdata, geomdata->vertices, FALSE);
 			drawPoints(hdc, g, geomdata, geomdata->points);
	      break;

      case BLOCKS:
         geomdata = dda_get_geometrydata(dda);
         g->scale = setScale(draw_wnd, hdc, g, geomdata->scale);
         drawBlocks(hdc, hBr, g, geomdata);
        /* All this code here does is draw outlines of the
         * blocks in their original position using the dashed 
         * line pen.  FIXME: replace with a drawOBlocks or 
         * something.
         */
         if(showOrig) 
         {
            drawJoints(hdc, screenPen, g, geomdata, geomdata->origvertices, TRUE);
            //drawLines(hdc, screenPen, g->scale, g->offset, g->origblocks[i].jp, g->origblocks[i].nSides, FALSE);
         }
         drawJoints(hdc, screenPen, g, geomdata, geomdata->vertices, FALSE);
         //drawLines(hdc, screenPen, g->scale, g->offset, g->blocks[i].jp, g->blocks[i].nSides, TRUE);
                  
        /* BUG: Can't use showOrig until the analysis at least starts to 
         * run!
         */
         if(showOrig) 
         {
            drawPoints(hdc, g, geomdata, geomdata->origpoints);
			   drawBolts(hdc, screenPen, geomdata, g, geomdata->origbolts, TRUE);
         }
         drawPoints(hdc, g, geomdata, geomdata->points);
         drawBolts(hdc, screenPen, geomdata, g, geomdata->rockbolts, FALSE);
         if (0/* g->stresses */)
            drawStresses(hdc, screenPen, geomdata, g);
         break;

      case TITLE:	 	
         handleTitle(hwMain);
		   break;  // end case title
				
      case NOTHING:
		default:
         GetClientRect(draw_wnd , &rectClient );
			FillRect(hdc, &rectClient, GetStockObject(WHITE_BRUSH));

   } // end switch whatToDraw

   EndPaint(draw_wnd, &ps );
//return 0;
   //DeleteDC(hdc);
   //ReleaseDC(draw_wnd, hdc);


}  /* close handleWinPaint() */



/***************  GEOMETRY HANDLERS *******************/

int
handleGeomApply(HWND hwMain, double scale_params[]) {

   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = gdata_new(); // = dda_get_geometrydata(dda);
   geomdata->display_error = dda_display_error;
   geomdata->display_warning = dda_display_warning;

   dda_set_menu_state(dda,GEOM_STATE | RUNNING);

   whatToDraw = LINES;

   freeGraphicStruct(g);
   g = initGraphicStruct();

   gdata_read_input_file(geomdata,filepath.gfile);

  /* This handles having a bad geometry file.  An alternative 
   * would be to have it come back to the geometry dialog,
   * but we would then need to handle the `cancel' button 
   * event from the dialog.  Smarter than your average bear...
   */   
   if (geomdata == NULL)  {
       dda_display_error("Error in geometry file");
       whatToDraw = NOTHING;
	    InvalidateRect(hwMain, NULL, TRUE);
		 UpdateWindow(hwMain);
       return 0;   
   }

   dda_set_geometrydata(dda,geomdata);
   ddacut(geomdata);


   if(geomdata->nBlocks>0) {

      whatToDraw = BLOCKS;
      dda_set_menu_state(dda,GEOM_STATE | FINISHED);
      toolbar_set_state(GEOM_STATE | FINISHED);
      statusbar_set_state(GEOM_STATE | FINISHED);
      statusbar_update_geometry(geomdata->nBlocks);

	} else {

     /** Nothing wrong with the geometry file, but there
      *  are no blocks defined.
      *  FIXME: Define this case as an error, and move the 
      *  code for handling it into the geometry cutting 
      *  functions.
      */      
      whatToDraw = NOTHING;
		filepath.gfile[0] = '\0';
		sprintf(mainWinTitle, "%s for Windows 95/NT", (LPSTR) szAppName);
		SetWindowText(hwMain, (LPCTSTR) mainWinTitle);
      dda_set_menu_state(dda,GEOM_STATE | ABORTED);
   }

   InvalidateRect(hwMain, NULL, TRUE);
   UpdateWindow(hwMain);

	return 0; 
}  



void
handleGeomDraw(HWND hwMain)
{
   HINSTANCE hInst;

	strcpy(filepath.oldfile, filepath.gfile);
	filepath.gfile[0] = '\0';
 	sprintf(mainWinTitle, "%s for Windows 95/NT", (LPSTR) szAppName);
 	SetWindowText(hwMain, (LPCTSTR) mainWinTitle);

	hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

  /* See the man page on the DrawBox function.  The "DRAWDLG" parameter is 
   * specified in the resource file (?).
   */
	if(DialogBoxParam(hInst, "DRAWDLG", hwMain, (DLGPROC)DrawDlgProc, (LPARAM)12 ) )
   {
     	sprintf(mainWinTitle, "%s for Windows 95/NT ---  Geometry = %s", (LPSTR) szAppName, filepath.gfile);
      SetWindowText(hwMain, (LPCTSTR) mainWinTitle);
   }
	
}  /* close handleGeometryDraw() */



void
handleGeometryDialog(HWND hwMain, LPARAM lParam)
{
   HINSTANCE hInst;

  /* FIXME: GEtWindowsWord, MakeProcInstance are obsolete. */
	hInst = (HINSTANCE) GetWindowWord(hwMain, GWL_HINSTANCE);
	DialogBoxParam(hInst, "GEOMDLG1", hwMain, (DLGPROC)GeomDlgProc, (LPARAM)0);
   
  /* After we get back from the dialog box, we need to reset the current
   * display.  Just to test, comment out the following line, then hit
   * a subsystem that tries to display on the main window.
   */
   //iface->setdisplay((unsigned int)hwMain);

   if(filepath.gfile[0] != '\0') 
   {
      sprintf(mainWinTitle, "%s for Windows 95/NT  ---  Geometry = %s", (LPSTR) szAppName, filepath.gfile);
      SetWindowText(hwMain, (LPCTSTR) mainWinTitle);
      if (strcmp(filepath.gfile, filepath.oldfile) != 0 && MessageBox(hwMain, "Apply geometry?", "GEOMETRY", MB_YESNO) == IDYES) 
      {
         SendMessage(hwMain, WM_COMMAND, GEOM_APPLY, lParam);
      }
   }
} /* close handleGeometryDialog() */


static int 
handleGeomBrowse(HWND hwMain, LPARAM lParam)
{

   OPENFILENAME ofn;
   char temp[200];
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);

   LPCTSTR szFilter[] = {"Geometry files (*.geo)\0*.geo\0All files (*.*)\0*.*\0\0"};
   fileBrowse(hwMain, &ofn, szFilter, filepath.gpath, filepath.gfile, "geo");
   if( !GetOpenFileName(&ofn) ) 
   {
      strcpy(filepath.gpath, filepath.oldpath);
      return 0;  /* user pressed cancel */
   } 
   else 
   {
      sprintf(mainWinTitle, "%s for Windows 95/NT  ---  Geometry = %s", (LPSTR) szAppName, (LPSTR) filepath.gfile);

      SetWindowText(hwMain, (LPCTSTR) mainWinTitle);

      dda_set_menu_state(dda,GEOM_STATE);

     /* Initialize the path. Note that the .geo file is already loaded into 
      * the struct, so it doesn't have to be reloaded.
      */
      strcpy(temp, filepath.gfile);
      strcpy (filepath.rootname, strtok(temp, "."));

      dda_set_output_directory("output", sizeof("output"));

      switch (MessageBox( hwMain,"Edit geometry data before applying?","Geometry", MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2 ) )
      {   
         case IDCANCEL:
			   return 0;

         case IDYES:
            SendMessage(hwMain, WM_COMMAND, GEOM_VE, lParam);
            return 0;

        /* This is currently falling through but it would be much 
         * better to send a geom apply message back through the loop.
         */
         case IDNO:  /* fall through to apply!!  */
         default:
            SendMessage(hwMain, WM_COMMAND, GEOM_APPLY, lParam);
				break;
      } // end switch
   }  // end if

   return 1;

} /* close handleGeomBrowse() */



static int
handleGeomPrintGraphics(HWND hwMain)
{
   HINSTANCE hInst;
   //DLGPROC lpfnDlgProc;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);
   PSTR szDocName = "DDA: Original Configuration";
	//options.results = 0;

 	hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);


	//hInst = (HINSTANCE) GetWindowWord(hwMain, GWL_HINSTANCE);
	//lpfnDlgProc = (DLGPROC) MakeProcInstance( (FARPROC)UnitDlgProc, hInst);
	//if (!DialogBox(hInst, "UNITDLG", hwMain, (DLGPROC)lpfnDlgProc)) 
	if (!DialogBox(hInst, "UNITDLG", hwMain, (DLGPROC)UnitDlgProc)) 
      return 0;
  /* FIXME: &geomdata->nPoints should be passed by 
   * value instead of reference b/c it is not changed in 
   * the printGeom function.
   */
	printGeom(hwMain, szDocName, geomdata, geomdata->scale, g);

  /* FIXME: Check the return value */
   return 0;

}  /* close handleGeomPrintGraphics() */



/*******************  ANALYSIS HANDLERS  *****************/

static int 
handleAnalBrowse(HWND hwMain, LPARAM lParam)
{
   OPENFILENAME ofn;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);

	LPCTSTR szFilter[] = {"Analysis files (*.ana)\0*.ana\0All files (*.*)\0*.*\0\0"};
                 
   fileBrowse(hwMain, &ofn, szFilter, filepath.apath, filepath.afile, "ana");
       
  /* save intervals for time steps should be in the 
   * Analysisdata struct instead of sitting out here as
   * naked extern.  For now the code save ever time 
   * interval.  Alternate implementations can be considered
   * when the code is a bit more stable.
   */
  /*
   tsSaveInterval = 1;
   */
   if( !GetOpenFileName(&ofn) ) 
   {
      strcpy(filepath.apath, filepath.oldpath);
		return 0;  // user pressed cancel
   } 
   else 
   {
      sprintf(mainWinTitle, 
	         "%s for Windows 95/NT --- Geometry = %s, Analysis File = %s", 
	         (LPSTR) szAppName, (LPSTR) filepath.gfile, (LPSTR) filepath.afile);
	
      SetWindowText(hwMain, (LPCTSTR) mainWinTitle);
      //updateMainMenu(hwMain, anastate*readystate);
      //dda->menustate = ANA_STATE | READY_STATE;
      dda_set_menu_state(dda,ANA_STATE | READY_STATE);

      switch ( MessageBox( hwMain, "Edit analysis data before running?", "Analysis", MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2) )
		{
		   case IDYES:
	  		   SendMessage(hwMain, WM_COMMAND, ANAL_EDITPARAMD, lParam);
	         return 0;

         case IDNO:
		      SendMessage(hwMain, WM_COMMAND, ANAL_RUN, lParam);
            break;

         case IDCANCEL:
            default:
            return 0;
      } // end switch
			

   } // end if

   return 1;

} /* close handleAnalBrowse() */


static int 
handleAnalRun(HWND hwMain) {

   int retval;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Analysisdata * ad = adata_new();

   dda_set_menu_state(dda,ANA_STATE | RUNNING);
   toolbar_set_state(ANA_STATE | RUNNING);
   statusbar_set_state(ANA_STATE | RUNNING);

   whatToDraw = BLOCKS;

#if WINGRAPHICS
   InvalidateRect(hwMain, NULL, TRUE);
   UpdateWindow(hwMain);
#else
   SwapBuffers(hdc);
#endif /* WINGRAPHICS */


   ad->display_error = dda_display_error;
   ad->display_warning = dda_display_warning;
   dda_set_analysisdata(dda,ad);
   adata_read_input_file(ad,filepath.afile,0,0,0);

   /* So I really need to grab the analysis struct before calling
    * so that all the initialization can be done...  
    */
   retval = ddanalysis(dda, &filepath);

  /* FIXME: Check retval for error codes.  Here we set the 
   * analysis data pointer to NULL so that further menu stuff
   * like "abort" does not cause segfaults.  This should work
   * b/c the data pointed to by ad has been freed in the 
   * analysis function.
   */
   //ad = NULL;

  /* This handles having a bad geometry file.  An alternative 
   * would be to have it come back to the geometry dialog,
   * but we would then need to handle the `cancel' button 
   * event from the dialog.  Smarter than your average bear...
   */
   if (!retval) 
   {  
      InvalidateRect(hwMain, NULL, TRUE);
      UpdateWindow(hwMain);
     /* FIXME: remember what this was supposed to do and 
      * make sure it is doing it.
      */
      return 0;  /* maybe not so smart... */
   }

   dda_set_menu_state(dda,ANA_STATE | FINISHED);
   toolbar_set_state(ANA_STATE | FINISHED);

  /* showOrig is a global int masquerading as a boolean. */   
   showOrig = TRUE;  
   whatToDraw = BLOCKS;
  /* FALSE for openGL, TRUE for win32??? */
   InvalidateRect(hwMain, NULL, TRUE);
	UpdateWindow(hwMain);
	//sprintf(mess, "Analysis completed successfully.\n\nClick to continue.");
	//MessageBox( hwMain, mess, "Analysis", MB_ICONINFORMATION );
  /* Commenting out shwOrig handles redrawing for window
   * repaints after minimizing etc.  It will have to be reset 
   * to FALSE somewhere else otherwise it will produce a seg fault
   * from trying to draw non-existent original geometry.
   */
   showOrig = FALSE;

  /* end case anal_run  <- This comment wouldn't pass the
   * Australian net censorship laws against "profane and 
   * suggestive language".
   */
   return 0;

}  /* close handleAnalRun() */


static int 
handleAnalNew(HWND hwMain, LPARAM lParam)
{
   HINSTANCE hInst;
   //DLGPROC lpfnDlgProc;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);
   strcpy(filepath.oldfile, filepath.afile);
	filepath.afile[0] = '\0';
	sprintf(mainWinTitle, "%s for Windows 95/NT  ---  Geometry = %s", 
						(LPSTR) szAppName, filepath.gfile);

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   SetWindowText(hwMain, (LPCTSTR) mainWinTitle);

   if (!DialogBoxParam(hInst, "ANALDLG2", hwMain, (DLGPROC)AnalDlgProc,(LPARAM)geomdata )) 
   {
	   strcpy(filepath.afile, filepath.oldfile);
   }

	if(filepath.afile[0] != '\0') 
	{
      sprintf(mainWinTitle, 
              "%s for Windows 95/NT  ---  Geometry = %s, Analysis File = %s", 
				(LPSTR) szAppName, filepath.gfile, filepath.afile);

      SetWindowText(hwMain, (LPCTSTR) mainWinTitle);

      if (MessageBox(hwMain, "Run analysis?", "ANALYSIS", MB_YESNO) == IDYES) 
      {
         SendMessage(hwMain, WM_COMMAND, ANAL_RUN, lParam);
      }
   }

   return 1;

} /* close handleAnalNew() */


static void
handleAnalEditDialog(HWND hwMain, LPARAM lParam)
{
   HINSTANCE hInst;

   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   assert(geomdata != NULL);

   DialogBoxParam(hInst, "ANALDLG2", hwMain, (DLGPROC)AnalDlgProc, (LPARAM)geomdata );

   //iface->setdisplay((unsigned int)hwMain);

   if(filepath.afile[0] != '\0') 
   {
      sprintf(mainWinTitle, "%s for Windows 95/NT  ---  Geometry = %s, Analysis File = %s", 
							(LPSTR) szAppName, filepath.gfile, filepath.afile);
      SetWindowText(hwMain, (LPCTSTR) mainWinTitle);
      if (MessageBox(hwMain, "Run analysis?", "ANALYSIS", MB_YESNO) == IDYES) 
      {
         SendMessage(hwMain, WM_COMMAND, ANAL_RUN, lParam);
      }
   }

}  /* close handleAnalEditDialog() */



/*********************  RESULTS HANDLERS  ********************/

static int
handleResultViewText(HWND hwMain)
{   
  /* This may need to be declared static.
   */
   OPENFILENAME ofn;
   PARAMBLOCK pb;

   LPCSTR szFilter[] = {"Results files (*.txt)\0*.txt\0All files (*.*)\0*.*\0\0"};
   fileBrowse(hwMain, &ofn, szFilter, filepath.vpath, filepath.vfile, "txt");
					
	if( !GetOpenFileName(&ofn) ) 
   {
	   strcpy(filepath.vpath, filepath.oldpath);
		return 0;  // user pressed cancel.
   }  
   else 
   {
	   sprintf(mess, " %s", filepath.vfile);
	   sprintf(filepath.vfile, "%s", mess);
	   loadNotepad(&pb, mess);
   }				

   return 1;

}  /* close handleResultViewText() */


static int
handleResultsPrintGraphics(HWND hwMain, LPARAM lParam)
{
   HINSTANCE hInst;
   //DLGPROC lpfnDlgProc;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);
   PSTR szDocName = "DDA - Results";

	hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

	if (!DialogBoxParam(hInst, "UNITDLG", hwMain, (DLGPROC)UnitDlgProc,(LPARAM)0)) 
      return 0;

  /* FIXME: &geomdata->nPoints should be passed by value 
   * instead of by reference b/c printGeom doesn't change 
   * the value.
   */
   printGeom(hwMain, szDocName, geomdata, geomdata->scale, g);

  /* FIXME: Check the return value */
   return 0;

}  /* close handleResultsPrintGraphics()  */


void
handleMainAbout(HWND hwMain) {


   char about[1024];		    		
   char aText[9][256] = {ABOUT};

   sprintf(about, "%s\n%s%s%s\n%s\n%s%s%s", aText[0], aText[1], aText[2], 
   aText[3], aText[4], aText[5],  aText[6], aText[7]);
  	MessageBox( hwMain, about, "DDA for Windows", MB_OK | MB_ICONINFORMATION);

}  /* handleMainAbout() */


/***************** UTILITY FUNCTIONS *******************/
/*
 * Functions after this are called by handlers.
 */

/* This should be renamed to invokeTextEditor()
 * or something similar.
 */
void 
loadNotepad(PARAMBLOCK *pb, char * mess)
{
	WORD wCmdShow[2];
   DWORD returnvalue;

	pb->wEnvSeg = 0;
	pb->lpCmdLine = (LPSTR) mess;
	wCmdShow[0] = 2;
	wCmdShow[1] = SW_SHOWNORMAL;
	pb->lpCmdShow = wCmdShow;
	pb->dwReserved = 0;
  /* FIXME: Look for sc1 first.  If that isn't available,
   * settle for notepad.
   */
	returnvalue = LoadModule("sc1.exe ", (LPVOID) pb);
   if (returnvalue <= 31)  // 31 !!!!
      returnvalue = LoadModule("notepad.exe ", (LPVOID) pb);

   if (returnvalue <= 31)
      MessageBox(NULL,"No text editor in path",NULL,MB_OK);
}  /* close loadNotepad() */




int
handleAPWEDGE(HWND hwMain)
{
   HINSTANCE hInst;

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   DialogBoxParam(hInst, "APWEDGED", hwMain, (DLGPROC)APWEDGEDlgProc,(LPARAM)0);

   return 0;

}  /* close handleAPWEDGE() */


/* These two functions (LButtonDown and RButtonDown set
 * the global variables xcursor, ycursor.  This captures 
 * the current point of the cursor for use in scrolling 
 * and zooming.  Don't need wParam right now, but possibly 
 * later.
 */
static void 
handleLButtonDown(HWND hwMain, WPARAM wParam, LPARAM lParam)
{

   POINTS mousepos;
   
   mousepos = MAKEPOINTS(lParam);
   xcursor = mousepos.x;
   ycursor = mousepos.y;


}  /* close handleLButtonDown() */


static void 
handleRButtonDown(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   POINTS mousepos;

   mousepos = MAKEPOINTS(lParam);
   xcursor = mousepos.x;
   ycursor = mousepos.y;

}  /* close handleRButtonDown() */


static void 
handlePauseAnalysis(HWND hwMain, WPARAM wParam, LPARAM lParam)
{

   MessageBox(hwMain, "Pausing not yet implemented", "Pause Analysis", MB_OK | MB_ICONSTOP);

}  /* handlePauseAnalysis() */



/* If a running analysis needs to be stopped, for whatever
 * reason, using the Analysis->Abort menu entry will stop 
 * the analysis and write out the results at the stopped
 * time step.
 */
/* FIXME: Make this a private function of the analysis 
 * data structure, then have the windows call invoke it.
 */
static void 
handleAbortAnalysis(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   DDA * dda = (DDA *)GetWindowLong(hwMain, GWL_USERDATA);
   Analysisdata * ad = dda_get_analysisdata(dda);

  /* If for some reason we get here without having an analysis
   * running, just send it back.  This is kludge to compensate
   * for bad menu item enablng.
   */
   
   if (ad == NULL)
      return;
   

   ad->abort(ad);


  /* FIXME: Here is a good place to disable some menu 
   * items.
   */

}  /* close handleAbortAnalysis() */



static void 
handleGeomCancel(HWND hwMain, WPARAM wParam, LPARAM lParam)
{

   MessageBox(hwMain, "Geometry cancel not yet implemented", "Cancel Geometry", MB_OK);

}  /* close handleGeomCancel() */


/* Once this gets working, move the initialization code elsewhere,
 * then use this function to call the tracking and display of the
 * pop up menu.
 *
 * @todo Move popup code somewhere else.
 */
static void 
handleRButtonUp(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   HMENU hmenu, hpopupmenu;
   POINT pt;
   HINSTANCE hInst;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);

  /* kludgy */
   if(dda_get_popupvis(dda) == 0)
      return;

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);
   hmenu = LoadMenu(hInst, "IDR_CHILDCONTEXT");
   hpopupmenu = GetSubMenu(hmenu,0);


   switch(dda_get_menu_state(dda)) {

      case READY_STATE:
		   EnableMenuItem(hpopupmenu, POPUP_ANALYSIS, MF_BYCOMMAND | MF_GRAYED);
		   EnableMenuItem(hpopupmenu, POPUP_REPLAY,   MF_BYCOMMAND | MF_GRAYED);
		   EnableMenuItem(hpopupmenu, POPUP_ABORT,    MF_BYCOMMAND | MF_GRAYED);
         break;

      case (GEOM_STATE | FINISHED):
		   EnableMenuItem(hpopupmenu, POPUP_ANALYSIS, MF_BYCOMMAND | MF_ENABLED);
		   EnableMenuItem(hpopupmenu, POPUP_REPLAY,   MF_BYCOMMAND | MF_GRAYED);
		   EnableMenuItem(hpopupmenu, POPUP_ABORT,    MF_BYCOMMAND | MF_GRAYED);
         break;
      
      case (ANA_STATE | READY_STATE):
			EnableMenuItem(hpopupmenu, ANAL_EDITPARAMD, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hpopupmenu, ANAL_EDITPARAMN, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hpopupmenu, ANAL_RUN, MF_BYCOMMAND | MF_ENABLED);
         break;

      case (ANA_STATE | FINISHED):
		   EnableMenuItem(hpopupmenu, POPUP_ABORT,   MF_BYCOMMAND | MF_GRAYED);
         break;

      case (ANA_STATE | RUNNING):
  	      EnableMenuItem(hpopupmenu, POPUP_GEOMETRY, MF_BYCOMMAND | MF_GRAYED);
 	      EnableMenuItem(hpopupmenu, POPUP_ANALYSIS, MF_BYCOMMAND | MF_GRAYED);
		   EnableMenuItem(hpopupmenu, POPUP_REPLAY,   MF_BYCOMMAND | MF_GRAYED);
		   EnableMenuItem(hpopupmenu, POPUP_EXIT,     MF_BYCOMMAND | MF_GRAYED);
         break;

      default:
         //MessageBox(NULL,"Bad menu state",NULL,MB_OK);
         break;
   }


   pt.x = xcursor;
   pt.y = ycursor;
   
   ClientToScreen(hwMain, &pt);
   TrackPopupMenu(hpopupmenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hwMain, NULL);
   DestroyMenu(hmenu);
   DestroyMenu(hpopupmenu);

}  /* close handleRButtonUp() */



static void 
handleLButtonUp(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   int wparamlo, wparamhi;

   wparamlo = LOWORD(wParam);
   wparamhi = HIWORD(wParam);
   switch (wparamlo) 
   {
      case MK_CONTROL: 
         //displayBlockNumber(hwMain, wParam, lParam);
         break;

      case MK_SHIFT: 
         //displayPhysicalCoordinates(hwMain, wParam, lParam);
         break;

      case (MK_CONTROL+MK_SHIFT): 
         deleteBlockNumber(hwMain, wParam, lParam);
         break;

      case (MK_LBUTTON+MK_SHIFT):
         break;

      case MK_MBUTTON: 
         break;

      default:
      break;
   }

   return;

}  /* close handleLButtonUp() */



/*************  WINDOWS HELPERS ***************/
/* Windows helper functions.  These get put into their
 * own file in the very near future.
 */
void
displayPhysicalCoordinates(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
  /* Temporary, prints into messagebox */
   char mess[180];
   DPoint p;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);

  /* Segfaults if there is nothing to look at. 
   * FIXME: Add some error code in here to keep track
   * of whats going on.
   */
   if (geomdata == NULL)
      return;

   p = DPtoPP(hwMain, xcursor, ycursor);

   sprintf(mess,"x: %.3f, y: %.3f", p.x,p.y);
   MessageBox(hwMain, mess, "Physical coordinates", MB_OK);

}  /* close displayPhysicalCoordinates() */



/* We really need to get the physical coordinates from the 
 * device coordinates.
 */
DPoint
DPtoPP(HWND hwMain, int xpos, int ypos)
{
  /* Convenience variables set from Graphics struct. */
   double scale, offsetx, offsety;
  /* The local device context.  We will set this and release 
   * in this function.
   */
   HDC hdc;
  /* viewport rectangle */
   RECT vprect;
  /* Viewport origin */
   POINT vporg, worg;
  /* Coordinates of physical point */
   DPoint p;

  /* Coordinates of an arbitrary point in the logical
   * (window) coordinate system.
   */
   double xwindow, ywindow;
  /* Origin of logical coordinate system */
   int xwindoworg, ywindoworg;
  /* Size of logical system based on client area */
   int xwindowext, ywindowext;
  /* Coordinates of arbitrary device point, i.e.,
   * a cursor location, etc.
   */
   int xviewport, yviewport;
  /* Size of device based on client area */
   int xviewportext, yviewportext;
  /* Origin of viewport (device) coordinates. */
   int xviewportorg, yviewportorg;

  /* Segfaults if there is nothing to look at. 
   * FIXME: Add some error code in here to keep track
   * of whats going on.
   */
   //if (geomdata == NULL)
   //   return 0;

  /* Transfer to local variables for convience */
   scale = g->scale;
   offsetx = (double)g->offset.x;
   offsety = (double)g->offset.y;

  /* Not the recommended way to do this.  Note the
   * DC is released after we are done.
   */
   hdc = GetDC(hwMain);

  /* Find device origin */
   GetViewportOrgEx(hdc,&vporg);
   xviewportorg = vporg.x;
   yviewportorg = vporg.y;

  /* Find the origin of the logical coords. */
   GetWindowOrgEx(hdc, &worg);
   xwindoworg = worg.x;
   ywindoworg = worg.y;

  /* Grab the window size */
   GetClientRect(hwMain, &vprect);
   xviewportext = vprect.right;
   yviewportext = vprect.bottom;

  /* Convert the window size to logical units. */
   DPtoLP(hdc, (LPPOINT) &vprect, 2);
   xwindowext = vprect.right;
   ywindowext = -vprect.top;  

  /* For this particular function, I need the current
   * cursor position.
   */
   xviewport = xpos;
   yviewport = ypos;

  /* Translate from device to logical coordinates.
   * See discussion on p. 259, Rector & Newcomer.
   */
   xwindow = (xviewport-xviewportorg)*(xwindowext/xviewportext) + xwindoworg;
   ywindow = (yviewport-yviewportorg)*(ywindowext/yviewportext) + ywindoworg;

  /* (x,y) in physical coordinates... */
   p.x = (xwindow - offsetx)/scale;
   p.y = (ywindow - offsety)/scale;


   ReleaseDC(hwMain, hdc);

   return p;

}  /* close DPtoPP() */

/********* close Windows helpers block  **************/



static void 
displayBlockNumber(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
  /* Temporary, prints into messagebox */
   char mess[180];
   DPoint p;
   int blocknum;

   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);

  /* Segfaults if there is nothing to look at. 
   * FIXME: Add some error code in here to keep track
   * of whats going on.
   */
   if (geomdata == NULL)
      return;

   p = DPtoPP(hwMain, xcursor, ycursor);

   //blocknum = geomdata->getblocknumber(geomdata, p.x, p.y);
   blocknum = gdata_get_block_number(geomdata, p.x, p.y);

   sprintf(mess,"Block number %d", blocknum);
   MessageBox(hwMain, mess, "Block number", MB_OK);

}  /* displayBlockNumber() */


/* FIXME: Points associated with a given block will have to be 
 * removed when the block is removed, otherwise bad segfaults
 * when an analysis is run sans the points.
 */
static void 
deleteBlockNumber(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
  /* Temporary, prints into messagebox */
   char mess[180];
   DPoint p;
   int blocknum;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);


  /* Segfaults if there is nothing to look at. 
   * FIXME: Add some error code in here to keep track
   * of whats going on.
   */
   if (geomdata == NULL)
      return;

   p = DPtoPP(hwMain, xcursor, ycursor);

   blocknum = gdata_get_block_number(geomdata, p.x, p.y);

   /* FIXME: Add a cancel to this.  */
   sprintf(mess,"Confirm delete block number %d", blocknum);
   if (MessageBox(hwMain, mess, "Delete block", MB_OKCANCEL) == IDCANCEL) {
      return;
   }

   geomdata->deleteblock(geomdata, blocknum);

   
   statusbar_update_geometry(geomdata->nBlocks);

   InvalidateRect(hwMain, NULL, TRUE);
	UpdateWindow(hwMain);

}  /* deleteBlockNumber() */


static void 
handleWMSize(HWND hwMain, WPARAM wParam, LPARAM lParam) {

   statusbar_resize();
   toolbar_resize();
}  

		  
LRESULT
handleWMNotify(HWND hwMain, WPARAM wParam, LPARAM lParam)
{

   HINSTANCE hInst;
   LPTOOLTIPTEXT lpToolTipText;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);

  /* Huge Kludge!!! */
   static char *ttt[] = {"New DDA Geometry",
                         "New DDA Analysis",
                         "Open DDA Geometry",
                         "Select and run analysis",
                         "Abort running analysis",
                         "Reapply original geometry",
                         "Zoom in",
                         "Zoom out",
                         "Print" };

   //static char szBuffer[80];

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   lpToolTipText = (LPTOOLTIPTEXT)(lParam);
 
   if(lpToolTipText->hdr.code == TTN_NEEDTEXT  && 
      dda_get_tooltipvis(dda) == 1)
   {
     /* FIXME: Set up a winprint function */
      //fprintf(wintest,"From MsgNotify, tool tip ID: %d\n",lpToolTipText->hdr.idFrom);
     /* GAAK.  Uses the actual #defined value instead of tool tip id 
      * number.  Nasty.
      */
      switch(lpToolTipText->hdr.idFrom)
      {
         case GEOM_NEW:
            lpToolTipText->lpszText = ttt[0];
            break;

         case ANAL_NEW:
            lpToolTipText->lpszText = ttt[1];
            break;

         case GEOM_BROWSE:
            lpToolTipText->lpszText = ttt[2];
            break;

         case ANAL_BROWSE:
            lpToolTipText->lpszText = ttt[3];
            break;

         case ANAL_ABORT:
            lpToolTipText->lpszText = ttt[4];
            break;

         case GEOM_APPLY:
            lpToolTipText->lpszText = ttt[5];
            break;

         case TOOLBAR_ZOOMIN:
            lpToolTipText->lpszText = ttt[6];
            break;

         case TOOLBAR_ZOOMOUT:
            lpToolTipText->lpszText = ttt[7];
            break;

         case TOOLBAR_PRINT:
            lpToolTipText->lpszText = ttt[8];
            break;

         default:
            break;

      }  /* End switch over tool bar tool tip notify */

   }  /* End If NEED TOOL TIP TEXT */
 
   return 0;


}  /* close handleWMNotify() */



void
handle_toggle(DDA * dda,
              WPARAM wParam,
              SetToggleFunc set_toggle,  //callback
              GetToggleFunc get_toggle,  //callback
              HMENU hSubMenu,
              const char * menuchars) {

   if (get_toggle(dda)) {

      set_toggle(dda,FALSE);
     /* Very kludgy.  This should send a message to get 
      * the current text of the menu item.
      */
      ModifyMenu(hSubMenu, wParam, MF_BYCOMMAND | MF_UNCHECKED,wParam, menuchars);

   } else {

       set_toggle(dda,TRUE);
       ModifyMenu(hSubMenu, wParam, MF_BYCOMMAND | MF_CHECKED,wParam,menuchars);
   }
}



/* Toggles the view state of tool bar, status bar, etc. 
 * FIXME: All this stuff needs to go into the menu handling 
 * function for WM_INITMENUPOPUP.
 */
static void
handleViewToggles(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   HMENU hMainMenu, hSubMenu;
   int submenuposition;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
            
   hMainMenu = GetMenu(hwMain);

   submenuposition = findSubMenuPosition(hMainMenu, VIEW_TBAR);
   hSubMenu = GetSubMenu(hMainMenu, submenuposition);

   switch (wParam)
   {
      case VIEW_TBAR:
         handle_toggle(dda,wParam,dda_set_toolbarvis,
                       dda_get_toolbarvis,hSubMenu,"&Tool Bar");
         break;

      case VIEW_SBAR:
         handle_toggle(dda,wParam,dda_set_statusbarvis,
                       dda_get_statusbarvis,hSubMenu,"&Status Bar");
         break;
     
      case VIEW_POPUP:
         handle_toggle(dda,wParam,dda_set_popupvis,
                       dda_get_popupvis,hSubMenu,"&Enable Pop-up");
         break;

      case VIEW_TOOLTIPS:
         handle_toggle(dda,wParam,dda_set_tooltipvis,
                       dda_get_tooltipvis,hSubMenu,"T&ool Tips");
         break;      

      default:
         break;
   }

} /* close handleViewToggles() */






static void
handleMetafile(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   HDC hdcmain, mfdc;
   RECT mfrect;
   HENHMETAFILE hmf;
   extern HPEN drawPen[10];
   extern HPEN screenPen[10];
   extern HBRUSH hBr[6];
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);

   hdcmain = GetDC(hwMain);  
   GetClientRect(hwMain, &mfrect);

   //displayWindowRect(hwMain, wParam, lParam);


   mfdc = CreateEnhMetaFile(hdcmain,"C:\\mfile.emf",NULL,NULL);

#if _DEBUG
   if(!mfdc)
      showlasterror(GetLastError());
#endif
    
   g->scale = setScale(hwMain, mfdc, g, geomdata->scale);
   //drawBlocks(mfdc, hBr, g, geomdata);
   if(showOrig) {
      drawJoints(mfdc, screenPen, g, geomdata, geomdata->origvertices, TRUE);
      drawBolts(mfdc, screenPen, geomdata, g, geomdata->rockbolts, TRUE);
   }
   drawJoints(mfdc, screenPen, g, geomdata, geomdata->vertices, FALSE);
   drawPoints(mfdc, g, geomdata, geomdata->points);
   drawBolts(mfdc, screenPen, geomdata, g, geomdata->rockbolts, FALSE);

   hmf = CloseEnhMetaFile(mfdc);
   DeleteEnhMetaFile(hmf);
   ReleaseDC(hwMain, hdcmain);

}  /* close handleMetafile() */




void 
readDXF(FILE * fp1) {
   
   double *jx1,*jy1,*jx2,*jy2;
   char str[5000][50];
   FILE *fp2;
   int count1=0,count2=0,i,j;
   long *type,pn;
   int cabcou=0,dec,ii,pltype,ctype,n1;
   int fixn=0,measn=0,holen=0,loadn=0;
   double r0,se,u,v,x1,x2,x3,y1,y2,y3,x0,y0,deg;
   double fx[25],fy[25],lx[25],ly[25];
   double mx[25],my[25],hx[25],hy[25];
   int nline=0,npoly=0,narc=0,ncir=0,ntext=0;
   //OPENFILENAME ofn;
   //char temp[200];


      while(!feof(fp1)) {
		   count1 +=1;
		   fscanf(fp1,"%s",str[count1]);

		   if(strcmp(str[count1],"LINE")==0) 
            nline+=1; 
	    
         if(strcmp(str[count1],"LWPOLYLINE")==0) 
            npoly+=1;
		
         if(strcmp(str[count1],"ARC")==0) 
            narc+=1;
	    
         if(strcmp(str[count1],"CIRCLE")==0) 
            ncir+=1;
      }

      // Array Allocation
	   n1=nline+40*npoly+40*narc+40*ncir+1;
      jx1 =(double *)calloc(n1,sizeof(double));
	   jy1 =(double *)calloc(n1,sizeof(double));
	   jx2 =(double *)calloc(n1,sizeof(double));
	   jy2 =(double *)calloc(n1,sizeof(double));
	   type =(long *)calloc(n1,sizeof(long));

      for(i=1;i<=count1;i++) {

 	   // The dxf file will be read from this line and lines, polylines
	   // and arcs will be recognized from dxf file

		if(strcmp(str[i],"LINE")==0) {

          count2 +=1;
		    if(strcmp(str[i+9],"62")==0) {

			   type[count2]=atoi(str[i+10])+1;
			   dec=2;
          } else {
            type[count2]=1;
			   dec=0;
          }

          jx1[count2]=atof(str[i+12+dec]);
			 jy1[count2]=atof(str[i+14+dec]);
			 jx2[count2]=atof(str[i+18+dec]);
			 jy2[count2]=atof(str[i+20+dec]);
      
      }
		
      if(strcmp(str[i],"LWPOLYLINE")==0) {

         if(strcmp(str[i+9],"62")==0) {

			   pltype=atoi(str[i+10])+1;
			   dec=2;
         } else {
               
            pltype=1;
			   dec=0;
			}
		   pn=atoi(str[i+12+dec]);
		   for(j=0;j<pn-1;j++)  
		   {
			ii=4*j;
		    count2 +=1;
            jx1[count2]=atof(str[i+18+ii+dec]);
			jy1[count2]=atof(str[i+18+ii+2+dec]);
			jx2[count2]=atof(str[i+18+ii+4+dec]);
			jy2[count2]=atof(str[i+18+ii+6+dec]);
		    type[count2]=pltype;
		   }
		    count2 +=1;
		    if(strcmp(str[i+9],"62")==0)
			{
			   type[count2]=atoi(str[i+10])+1;
			   dec=2;
			}
		    else
			{
               type[count2]=1;
			   dec=0;
			}
			ii=4*3;
            jx1[count2]=atof(str[i+18+ii+dec]);
			jy1[count2]=atof(str[i+18+ii+2+dec]);
			jx2[count2]=atof(str[i+18+dec]);
			jy2[count2]=atof(str[i+20+dec]);
			type[count2]=pltype;
		}
         // From this line first, center and second points of an arc will be extracted from dxf
		 // and the lines of arc are created with a simple mathematical source 

          if(strcmp(str[i],"ARC")==0)
		    {
   			    if(strcmp(str[i+9],"62")==0)
			    {
			      ctype=atoi(str[i+10])+1;
			      dec=2;
			    }
		        else
			    {
                  ctype=1;
			      dec=0;
			    }
				se=30;
				deg=atof(str[i+24+dec]);
				r0=((3.1415926535)*deg)/(se*180);
                x1=atof(str[i+12+dec])+atof(str[i+18+dec]);
	            y1=atof(str[i+14+dec]);
				x2=atof(str[i+12+dec]);
				y2=atof(str[i+14+dec]);
				x0=x2;
				y0=y2;
				x3=atof(str[i+12+dec])-atof(str[i+18+dec]);
				y3=atof(str[i+14+dec]);
	            
                for(j=1;j<se;j++)
				{
                	u=((x1-x0)*(cos(r0)-1))-((y1-y0)*sin(r0));
                	v=((x1-x0)*sin(r0))+((y1-y0)*(cos(r0)-1));
					
	                x2=x1+u;
	                y2=y1+v;
	                count2 +=1;
					jx1[count2]=x1;
					jy1[count2]=y1;
					jx2[count2]=x2;
					jy2[count2]=y2;
					type[count2]=ctype;
					x1=x2;
					y1=y2;
                              
				}
				    
                    x2=x3;
	                y2=y3;
                    count2 +=1;
					jx1[count2]=x1;
					jy1[count2]=y1;
					jx2[count2]=x2;
					jy2[count2]=y2;
					type[count2]=ctype;
		  }
		  // From this line first, center and second points of an circle will be extracted from dxf
		  // and the lines of arc are created with a simple mathematical source 
		  if(strcmp(str[i],"CIRCLE")==0)
		    {
   			    if(strcmp(str[i+9],"62")==0)
			    {
			      ctype=atoi(str[i+10])+1;
			      dec=2;
			    }
		        else
			    {
                  ctype=1;
			      dec=0;
			    }
				se=30;
				deg=360;
				r0=((3.1415926535)*deg)/(se*180);
                x1=atof(str[i+12+dec])+atof(str[i+18+dec]);
	            y1=atof(str[i+14+dec]);
				x2=atof(str[i+12+dec]);
				y2=atof(str[i+14+dec]);
				x0=x2;
				y0=y2;
				x3=x1;
				y3=y1;
	            
                for(j=1;j<se;j++)
				{
                	u=((x1-x0)*(cos(r0)-1))-((y1-y0)*sin(r0));
                	v=((x1-x0)*sin(r0))+((y1-y0)*(cos(r0)-1));
					
	                x2=x1+u;
	                y2=y1+v;
	                count2 +=1;
					jx1[count2]=x1;
					jy1[count2]=y1;
					jx2[count2]=x2;
					jy2[count2]=y2;
					type[count2]=ctype;
					x1=x2;
					y1=y2;
                              
				}
				    
                    x2=x3;
	                y2=y3;
                    count2 +=1;
					jx1[count2]=x1;
					jy1[count2]=y1;
					jx2[count2]=x2;
					jy2[count2]=y2;
					type[count2]=ctype;
		  }
                 if(strcmp(str[i],"MTEXT")==0)
				 {
					 if(strcmp(str[i+9],"62")==0)
					  {
			             dec=2;
					  }
		              else
					  {
                  	     dec=0;
					  }
					 if(strcmp(str[i+26+dec],"F")==0)
					 {
                      fixn +=1;
                      fx[fixn]=atof(str[i+12+dec]);
			          fy[fixn]=atof(str[i+14+dec]);
					 }
				  	 if(strcmp(str[i+26+dec],"L")==0)
					 {
                      loadn +=1;
                      lx[loadn]=atof(str[i+12+dec]);
			          ly[loadn]=atof(str[i+14+dec]);
					 }
				  	 if(strcmp(str[i+26+dec],"M")==0)
					 {
                      measn +=1;
                      mx[measn]=atof(str[i+12+dec]);
			          my[measn]=atof(str[i+14+dec]);
					 }
					 if(strcmp(str[i+26+dec],"H")==0)
					 {
                      holen +=1;
                      hx[holen]=atof(str[i+12+dec]);
			          hy[holen]=atof(str[i+14+dec]);
					 }
				 }
    }
	  fclose(fp1);
      // From this line the data that extracted from dxf file will be sorted 
	  // and will be wrote into geo file.
      strcpy (filepath.gfile, strcat(filepath.rootname, ".geo"));  
      fp2 = fopen(filepath.gfile,"w");

   fprintf(fp2,"<?xml version=\"1.0\" standalone=\"no\"?>\n");
   fprintf(fp2,"<!DOCTYPE DDA SYSTEM \"geometry.dtd\">\n");
   fprintf(fp2,"<Berkeley:DDA xmlns:Berkeley=\"http://www.tsoft.com/~bdoolin/dda\">\n");
   fprintf(fp2,"<!-- Bogus comment to keep ddaml tree-stripping\n");
   fprintf(fp2,"from seg faulting on bad child node. -->\n\n");
   fprintf(fp2,"<Geometry>\n");
   fprintf(fp2,"   <Edgenodedist distance=\"%lf\"/>\n",0.001);
   fprintf(fp2,"   <Jointlist>\n");
   for(i=1;i<=count2;i++) 
   {
   fprintf(fp2,"      <Joint type=\"%d\">",type[i]);
   fprintf(fp2,"  %lf   %lf   %lf   %lf",jx1[i],jy1[i],jx2[i],jy2[i]);
   fprintf(fp2,"  </Joint>\n");
   }
   fprintf(fp2,"</Jointlist>\n");

   fprintf(fp2,"<Fixedpointlist>\n");
   for(i=1;i<=fixn;i++)fprintf(fp2,"<Line> %lf   %lf   %lf   %lf </Line>\n",fx[i],fy[i],fx[i],fy[i]);
   fprintf(fp2,"</Fixedpointlist>\n");

   fprintf(fp2,"<Loadpointlist>\n");
   for(i=1;i<=loadn;i++)fprintf(fp2,"<Point> %lf   %lf  </Point>\n",lx[i],ly[i]);
   fprintf(fp2,"</Loadpointlist>\n");
   
   fprintf(fp2,"<Measuredpointlist>\n");
   for(i=1;i<=measn;i++)fprintf(fp2,"<Point> %lf   %lf  </Point>\n",mx[i],my[i]);
   fprintf(fp2,"</Measuredpointlist>\n");
   
   fprintf(fp2,"<Holepointlist>\n");
   for(i=1;i<=holen;i++)fprintf(fp2,"<Point> %lf   %lf  </Point>\n",hx[i],hy[i]);
   fprintf(fp2,"</Holepointlist>\n");

   fprintf(fp2,"</Geometry>\n");
   fprintf(fp2,"</Berkeley:DDA>\n");

   fclose(fp2);



   free(jx1);
   free(jy1);
   free(jx2);
   free(jy2);
   free(type);

   return;

}


// This subroutine is wrote by Roozbeh to read the .dxf 
// format and to create the .geo format
static int 
handleDxfBrowse(HWND hwMain, LPARAM lParam) //Added by Roozbeh
{
   FILE *fp1;//,*fp2;
   //int count1=0,count2=0,i,j;
   //char str[5000][50];
   //long *type,pn;
   //int cabcou=0,dec,ii,pltype,ctype,n1;
   int fixn=0,measn=0,holen=0,loadn=0;
   //double r0,se,u,v,x1,x2,x3,y1,y2,y3,x0,y0,deg;
   //double fx[25],fy[25],lx[25],ly[25];
   //double mx[25],my[25],hx[25],hy[25];
   int nline=0,npoly=0,narc=0,ncir=0,ntext=0;
   OPENFILENAME ofn;
   char temp[200];
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);

   LPCTSTR szFilter[] = {"Autocad Dxf files (*.dxf)\0*.dxf\0All files (*.*)\0*.*\0\0"};
   fileBrowse(hwMain, &ofn, szFilter, filepath.gpath, filepath.gfile, "dxf");

   if( !GetOpenFileName(&ofn) ) {

      strcpy(filepath.gpath, filepath.oldpath);
      return 0;  /* user pressed cancel */

   } else {


      sprintf(mainWinTitle, "%s for Windows 95/NT  ---  Geometry = %s", (LPSTR) szAppName, (LPSTR) filepath.gfile);

      SetWindowText(hwMain, (LPCTSTR) mainWinTitle);

      dda_set_menu_state(dda,GEOM_STATE);

     /* Initialize the path. Note that the .dxf file is already loaded into 
      * the struct, so it doesn't have to be reloaded.
      */
      strcpy(temp, filepath.gfile);
      strcpy (filepath.rootname, strtok(temp, "."));

      dda_set_output_directory("output",sizeof("output"));
	  // The dxf file will be read from this line and lines, polylines
	  // and arcs will be recognized from dxf file
      fp1 = fopen(filepath.gfile,"r");

      // Pass everything in to readDXF.
      readDXF(fp1);
      
   }

   return 1;

} // End of handleDxfBrowse()


int
handleWMCommand(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   static DLGPROC lpfnDlgProc;
   static OPENFILENAME ofn;
   //static HINSTANCE hInst;
   static PARAMBLOCK pb;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);
   Analysisdata * ad = dda_get_analysisdata(dda);

   switch (wParam)
 	{

      case FILE_METAFILE:
      case EXPORT_EMF:
         handleMetafile(hwMain, wParam, lParam);
         break;

      case EXPORT_EPS:
         break;

      case IMPORT_DXF:   // Added by Roozbeh
         handleDxfBrowse(hwMain, lParam); // Added by Roozbeh
         break; // Added by Roozbeh

      case VIEW_TBAR:
      case VIEW_SBAR:
      case VIEW_POPUP:
      case VIEW_TOOLTIPS:
         handleViewToggles(hwMain,wParam,lParam);
         break;

      case MENU_HELPINDEX:
      case GEOM_BROWSE:
         handleGeomBrowse(hwMain, lParam);
         break;  

		case GEOM_APPLY:
         handleGeomApply(hwMain,geomdata->scale);
         break;

		case GEOM_NEW:
         handleGeomDraw(hwMain);
			break;

	  	case GEOM_EDITTXTD:
         handleGeometryDialog(hwMain, lParam);
			break;

		case GEOM_EDITTXTN:
		   sprintf(mess, " %s", filepath.gfile);
	    	loadNotepad(&pb, mess);
		 	break;

      case GEOMDRAW:
         handleGeomDraw(hwMain);
			break;

      case GEOM_EDIT_VE:
         handleVisualEditor(hwMain, lParam);
         break;

      case GEOM_VE:
         handleVisualEditor(hwMain, lParam);
         break;

      case GEOM_VIEWGRF:
		   InvalidateRect(hwMain, NULL, TRUE);
			UpdateWindow(hwMain);
			break;

     /* Print the geometry. */
      case GEOM_PGRF:
         handleGeomPrintGraphics(hwMain);
  			break;

  		case GEOM_CANCEL:
         handleGeomCancel(hwMain, wParam, lParam);
		 	break;

      case  ANAL_BROWSE :
         handleAnalBrowse(hwMain, lParam);
         break; 

		case ANAL_RUN:
         handleAnalRun(hwMain);
         break;

 		case ANAL_ABORT:  /* Doesn't seem to work.  Gross. */
			handleAbortAnalysis(hwMain, wParam, lParam);
         break;

      case ANAL_PAUSE:
         handlePauseAnalysis(hwMain, wParam, lParam);
         break;

     /* FIXME:  This is broken.  Not sure why.  Probably has do to with 
      * new file formats or new data structs.
      */
    	case ANAL_NEW:
         handleAnalNew(hwMain, lParam);
			break;

      case ANAL_EDITBLX:  
		   sprintf(mess, " block.in");
			loadNotepad(&pb, mess);
		  	break;

      case ANAL_EDITPARAMD:
         handleAnalEditDialog(hwMain, lParam);
	  		break;

		case ANAL_EDITPARAMN:
			sprintf(mess, " %s", filepath.afile);
			loadNotepad(&pb, mess);
			break;

      case AD_TESTPROP:   
         CreateTestPropSheet(hwMain, ad);
         break;

		case RES_VTXT:
        /* TODO: Check and see whether the return value
         * needs to be trapped. 
         */
         handleResultViewText(hwMain);
  			break;

     /*  This is just a bit kludgy, but it ought to work. */
      case RES_VGRF:
        /* TODO: Check and see whether the return value
         * needs to be trapped. 
         */
         handleResultsViewReplay(hwMain, lParam,g);
         break;

	  	case RES_REPLAY:
         replay_analysis(hwMain,g,filepath.replayfile);
	  	  	break;
                
     /* Most of this is going to get passed to the printing routine
      * that takes care of printing the geometry.  It will need to pass
      * a reference to the window and a title string szDocName which can 
		* be set here.  
      */
		case RES_PGRF:
        /* TODO: Check and see whether the return value
         * needs to be trapped. 
         */
         handleResultsPrintGraphics(hwMain, lParam);
	  		break;
 
      case DEMO_TSLOPE:
         break;

      case DEMO_ARS:   
         //CreateTestPropSheet(hwMain, hInst);
         //handleARS(hwMain);
         break;

      case APWEDGE:
         handleAPWEDGE(hwMain);
         break;

	  	case DDAHELP_OVERVIEW:
      case DDAHELP_INDEX:
      case DDAHELP_GEOMETRY:
      case DDAHELP_ANALYSIS:
      case DDAHELP_SEARCH:
         handleHtmlHelp(hwMain, wParam, lParam);
         break;

		case DDAHELP_ABOUTDDA:
         handleMainAbout(hwMain);
	  		break;

      case OPTIONS_ORIGGEOM:
      case OPTIONS_SPYPLOTS:
      case OPTIONS_FLAGSTATES:
      case OPTIONS_PARAMETERS:
      case OPTIONS_TIMING:
      case OPTIONS_FIXEDPOINTS:
      case OPTIONS_BLOCKAREAS:
      case OPTIONS_MEASUREDPOINTS:
         handleOptionsMenu(hwMain,wParam,&options);
         break;

      case POPUP_GEOMETRY:
         handleGeomBrowse(hwMain, lParam);
         break;

      case POPUP_ANALYSIS:
         handleAnalBrowse(hwMain, lParam);
         break;

      case POPUP_REPLAY:
         /* FIXME: popup menus are segfaulting on this. */
         //if(!ad->isRunning)
         replay_analysis(hwMain,g,filepath.replayfile);
         break;

      case POPUP_ABORT:
	   	handleAbortAnalysis(hwMain, wParam, lParam);
         break;

      case POPUP_EXIT:
			whatToDraw = NOTHING;
		   DestroyWindow( hwMain );
         break;

		case MAIN_EXIT:
			whatToDraw = NOTHING;
         dda_delete(dda);
		   DestroyWindow( hwMain );
		   break;

   } // end switch wparam

   return 0;

}  /* close handleWMCOMMAND() */


/* WndProc handles messages from the top level window
 * only, sending everything to handler functions.
 */
LRESULT CALLBACK 
WndProc (HWND hwMain, UINT message, 
                          WPARAM wParam, LPARAM lParam)
{
   static DLGPROC lpfnDlgProc;
   static HINSTANCE hInst;
   static menu_item_number;  // Set by WM_MENUSELECT
   RECT winSize;
   int width, height;
   UINT menuflags;
   DDA * dda;

   GetClientRect(hwMain, &winSize);
  /* Size of the main window*/
   width= -(winSize.left-winSize.right); 
   height= -(winSize.top-winSize.bottom);
 
   dda = (DDA *)GetWindowLong(hwMain, GWL_USERDATA);


   switch (message)
   {

      case WM_SYSCHAR:
         return(handleSysChar(hwMain, wParam, lParam));

      case WM_KEYDOWN:
         return(handleKeydown(hwMain, wParam, lParam));

      case WM_CHAR:
         return(handleChar(hwMain, wParam, lParam));

     /* There are some control of flow problems with the DDA struct
      * segfaulting before being initialized.
      */
      case WM_CREATE:
         initializeDDAForWindows(hwMain, wParam, lParam);
         break;

      case WM_PAINT:
         handleWinPaint(hwMain, wParam, lParam, width, height);
         break;

	  case WM_COMMAND:
         return(handleWMCommand(hwMain, wParam, lParam));
   		  
      case WM_DROPFILES:
         handleDropFiles(hwMain, wParam, lParam, &filepath);
         break;

      case WM_INITMENUPOPUP:
        /* At some point the information contained in 
         * Rector and Newcomer, page 888, will be handled
         * here to set the state of all the menus.
         * Also, change name to handleWMInitMenuPopup()
         */
         updateMainMenuA(hwMain, dda_get_menu_state(dda));
         break;



      case WM_MENUSELECT:
        /* There needs to be a state variable here to track 
         * callbacks for handling dynamically loaded menu
         * items.  This is pretty nasty stuff.  See page 
         * 892 in Rector and Newcomer.
         */
         menuflags = HIWORD(wParam);
         if ( (menuflags & MF_POPUP) && (menuflags != 0xFFFF)) {
            menu_item_number = LOWORD(wParam);
         } 
         break; 

      case WM_MOUSEMOVE:
         handleMouseMove(hwMain, wParam, lParam);
         break;

	   case WM_NOTIFY:
		   handleWMNotify(hwMain, wParam, lParam);
         break;

      case WM_LBUTTONDOWN:
         handleLButtonDown(hwMain, wParam, lParam);
         break;

      case WM_RBUTTONDOWN: 
         handleRButtonDown(hwMain, wParam, lParam);
         break;

      case WM_RBUTTONUP:
         handleRButtonUp(hwMain, wParam, lParam);
         break;

      case WM_LBUTTONUP:
         handleLButtonUp(hwMain, wParam, lParam);
         break;

      case WM_SIZE:
         handleWMSize(hwMain, wParam, lParam);
         break;

      case WM_QUIT:
         HtmlHelp(NULL,NULL,HH_UNINITIALIZE,(DWORD)&dwHTMLCookie); 
         break;

      case WM_DESTROY:
	     PostQuitMessage (0) ;
	     return 0 ;
   }  // end switch (message)

   return DefWindowProc (hwMain, message, wParam, lParam) ;

} // end WndProc




HWND 
createDDAMainWindow(HINSTANCE hInstance)
{
   HWND hddawin;
   RECT desktoprect;

   GetWindowRect(GetDesktopWindow(), &desktoprect);

   hddawin = CreateWindowEx(WS_EX_ACCEPTFILES, 
                           szAppName, 
                           "Berkeley DDA for Windows NT", 
                           WS_OVERLAPPEDWINDOW    | 
                           WS_CLIPCHILDREN,
	 	                     (desktoprect.right  / 2) - (800  / 2), 
                           (desktoprect.bottom / 2) - (600 / 2), 
                           800, 600,  // use these values for centering window
	 	                     NULL, NULL, hInstance, NULL);

   ShowWindow (hddawin, SW_SHOWNORMAL);
   UpdateWindow (hddawin) ;

   return hddawin;

}  /* close createDDAMainWindow() */


/* FIXME: This needs to change to a RegisterClassEx
 * in the near future.
 */
registerDDAMainWindowClass(HINSTANCE hInstance)
{
   WNDCLASS    wndclass;

  /* Since we don't specify the device context (DC),
   * the DDA window class uses a "common" DC by default.
   * See page 221 of Rector and Newcomer for DC info. 
   */
   wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
   wndclass.lpfnWndProc   = (WNDPROC)WndProc ;
   wndclass.cbClsExtra    = 0;
   wndclass.cbWndExtra    = 4; //sizeof(DDA *);
   wndclass.hInstance     = hInstance ;
   wndclass.hIcon         = LoadIcon( hInstance, MAKEINTRESOURCE(ICON_1));
   wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
   wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wndclass.lpszMenuName  = MAKEINTRESOURCE( DDAAPMENU );
   wndclass.lpszClassName = szAppName ;

   RegisterClass (&wndclass) ;

}  /* close registerDDAMainWindowClass() */


int WINAPI 
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, 
					               LPSTR lpszCmd, int nCmdShow)
{
   HWND        hwMain;
   MSG         msg;
   HANDLE      hAccel;	
   int checkval;

   /* Move this code to the WM_CREATE or initializeDDA
    * functions to maintain symmetry in memory handling.
    */
   DDA * dda = dda_new();

  /* FIXME: Add code to uninitialize */
   HtmlHelp(NULL,NULL,HH_INITIALIZE,(DWORD)&dwHTMLCookie); 

   //memset(&dda,0xDA,sizeof(DDA));

   filepath.rootname[0] = '\0';

   registerDDAMainWindowClass(hInstance);

   hwMain = createDDAMainWindow(hInstance);

   /* This dissappears when I get the callback figured out. */
   mainwindow = hwMain;

  /* Move this stuff into the dda.ini file,which 
   * should be read in the initialization function.
   */
   dda_set_toolbarvis(dda,TRUE);
   dda_set_statusbarvis(dda,TRUE);
   dda_set_popupvis(dda,TRUE);
   dda_set_tooltipvis(dda,TRUE);
   dda_set_menu_state(dda,READY_STATE);   
   checkval = SetWindowLong(hwMain,GWL_USERDATA,(LONG)dda);



   hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE( DDAAPACCEL ));

  	while (GetMessage (&msg, NULL, 0, 0)) {
      if (!TranslateAccelerator(msg.hwnd,hAccel,&msg)) {
         TranslateMessage (&msg);
         DispatchMessage (&msg);
      } 

   }  

   return msg.wParam;

}  /* close WinMain()  */

