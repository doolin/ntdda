/* 
 * winmain.c
 * 
 * DDA for Windows NT
 * 
 * This is the main gui message handling code to run the 
 * dda gui interface.
 * 
 * $Author: doolin $
 * $Date: 2002/05/19 00:08:42 $
 * $Source: /cvsroot/dda/ntdda/src/win32gui/winmain.c,v $
 * $Revision: 1.12 $
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

#include "win32utils.h"
#include "resource.h"
#include "interface.h"
#include "wingraph.h"
#include "graphics.h"
#include "replay.h"


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

char aText[9][256] = {ABOUT};
int whatToDraw = TITLE;

int showOrig = FALSE;

/* Used by mouse handler to scroll around the screen. */
int xoff = 2500;
int yoff = 2500;
long zoom = 25000;

/* These set the cursor location on a mouse button down */
int xcursor;
int ycursor;


/* FIXME: Move to local if possible */
/* These won't work until version msvc 6 */
// User global variables
HICON g_hIcon0 = NULL;		// User icon 0, handle return by parser when 'Icon0' keyword is used
HICON g_hIcon1 = NULL;		// User icon 1, handle return by parser when 'Icon1' keyword is used


/* Unfortunately, several variables are better 
 * handled as externs for the time being.  Later
 * these can be handled in hash tables or something
 * to provide the ability for more than one analysis
 * at a time.
 */
OPTIONS options;
FILEPATHS filepath;
FILEPOINTERS fp;

//Geometrydata * geomdata = NULL;
//Geometrydata * geometry2draw;


GRAPHICS * g = NULL;


/* For HTML Help */
DWORD dwHTMLCookie;

/* FIXME: Get rid of the external Analysisdata * ad */
//Analysisdata * ad = NULL;



/* FIXME: Make these into just one status bar. */
HWND readystatus, geomstatus, anastatus;
HWND hprogbar;
HWND hToolBar;

/* This is a terrible kludge.  What needs to happen is for the 
 * appropriate code to ask the control whether it is visible or not,
 * and then take the appropriate action, either show or hide. 
 * Basically, these variables can all go local once I figure
 * out how to check the visibility of these items by SendMessage().
 */
/* FIXME: Just delete these and handle with messages. */


/* Used by display.c to update the analysis status bar fields. */
HDC anahdc;




/* handleWMCommand has to be declared here to 
 * avoid namespace collision with the analysis
 * dialog box.  handleMouseMove collides with 
 * same in drawdialog().
 */
/* FIXME: Orthogonalize header files. */
//static int handleWMCommand(HWND, WPARAM wParam, LPARAM lParam);
static void displayBlockNumber(HWND hwMain, WPARAM wParam, LPARAM lParam);
static void deleteBlockNumber(HWND hwMain, WPARAM wParam, LPARAM lParam);
static void addBitMaptoToolbar(HWND hwMain);
static void insertNewMainMenuItem(HWND hwMain);

void initMainToolbar(HWND hwMain);



void displayPhysicalCoordinates(HWND hwMain, WPARAM wParam, LPARAM lParam);
DPoint DPtoPP(HWND hwMain, int xpos, int ypos);

static void updateGeometryStatusBar(HWND);
   
/* Move this to its own header file. */
int CreateTestPropSheet(HWND hwMain, Analysisdata *);


/* LAST LONELY GLOBAL, PASSED AROUND WHEREVER
 * NEEDED!
 */
InterFace * iface;


Print_Func
handle_error(void * stream, const char * title, const char * message) {

   MessageBox((HWND)stream,title,message,MB_OK);

   return 0;

}

/* TODO: Flesh out a preferences API.  The 
 * default location for the .ini files are
 * the Windows directory, which is reasonable.
 * It would take quite a bit of work to implement
 * all the ini reading and writing into some 
 * specified directory.
 */
static void
writeDDAProfile() {

   /*
   //char message[240];
  
   char winbuf[240];
   char sysbuf[240];
   char retbuf[8192*4];

   WritePrivateProfileString("DDA_Top_Level_Section",  // section name
                             "Foo",       // key
                             "Bar",       // value
                             "dda.ini\0");  // filename
   WritePrivateProfileString("DDA_Top_Level_Section",  // section name
                             "Bar",       // key
                             "Baz",       // value
                             "dda.ini\0");  // filename   
   WritePrivateProfileString("DDA_Output_Options_Section",  // section name
                             "mpoints",       // key
                             "1",       // value
                             "dda.ini\0");  // filename
   
   GetWindowsDirectory(winbuf,240);
   MessageBox(NULL,winbuf,NULL,MB_OK);
   GetSystemDirectory(sysbuf,240);
   MessageBox(NULL,sysbuf,NULL,MB_OK);

   GetPrivateProfileSection("DDA_Top_Level_Section",
                             retbuf,
                             8192*4,
                             "dda.ini");
   MessageBox(NULL,retbuf,NULL,MB_OK);

   showlasterror(GetLastError());
*/

}  /* close writeDDAProfile() */


static void 
initializeDDAForWindows(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   HINSTANCE hInst;
   INITCOMMONCONTROLSEX icex;

  /* FIXME: This needs to be just one control ID for 
   * one status bar, and it should be set from the 
   * appropriate value in the resource.h file.
   */
   UINT DDA_R_STATUSBAR = 0;
   UINT DDA_G_STATUSBAR = 1;
   UINT DDA_A_STATUSBAR = 2;

   int readyparts[1] = { -1 };
#define SB_GEOM_PARTS 7
   int geomparts[SB_GEOM_PARTS];
#define SB_ANAL_PARTS 10
   int anaparts[SB_ANAL_PARTS];

   icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
   InitCommonControlsEx(&icex);


//sprintf(mess,"WM_KEYDOWN: %p",WM_KEYDOWN);
//MessageBox(NULL,mess,mess,MB_OK);


  /* This might be a problem too... */
   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   initializePens();
   initializeBrushes();
   
  /* FIXME: g is handling lots of things it ought not 
   * be handling.  The win32 gui control parts need to 
   * be abstracted out.
   */ 
   g = initGraphicStruct();

  /* Interface functions */
   iface = getNewIFace();
   iface->setdisplay((unsigned int)hwMain);

  /* Create this initially invisible, the use the view menu to 
   * toggle whether or not it shows itself.
   */
   readystatus = CreateStatusWindow(WS_CHILD | WS_MINIMIZE | WS_CLIPCHILDREN,
      "", hwMain, DDA_R_STATUSBAR);

  /* These field stops need to be handled with a finite state
   * machine after font metric handling is implemented.  The 
   * (x,y) coordinates field can be computed using domain 
   * scaling width g0.
   */
   geomparts[0] = 45+45; // gfs[0] field stop 0
   geomparts[1] = 90+45;
   geomparts[2] = 140+45;
   geomparts[3] = 200+45;
   geomparts[4] = 250+45;
   geomparts[5] = 350+85;  // This is the (x,y) field
   geomparts[6] = -1;

   geomstatus = CreateStatusWindow(WS_CHILD | WS_MINIMIZE | WS_CLIPCHILDREN,
      "", hwMain, DDA_G_STATUSBAR);
   SendMessage(geomstatus,SB_SETPARTS,(WPARAM)SB_GEOM_PARTS,(LPARAM)geomparts);
   

  /* These field stops need to be handled with a finite state
   * machine after font metric handling is implemented.
   */
   anaparts[0] = 45;  // afs[0] = field stop 0
   anaparts[1] = 110;
   anaparts[2] = 180;
   anaparts[3] = 225;
   anaparts[4] = 350;
   anaparts[5] = 400;
   anaparts[6] = 450;
   anaparts[7] = 490;
   anaparts[8] = 560;
   anaparts[9] = -1;

   anastatus = CreateStatusWindow(WS_CHILD | WS_MINIMIZE | WS_CLIPCHILDREN,
      "", hwMain, DDA_A_STATUSBAR);
   SendMessage(anastatus,SB_SETPARTS,(WPARAM)SB_ANAL_PARTS,(LPARAM)anaparts);

  /* FIXME: Explain the structure of the parameter cast by (WPARAM) 
   * or (LPARAM).  Where are the relevant bits?
   */
   SendMessage(readystatus, SB_SETTEXT,(WPARAM)(SBT_NOBORDERS),(LPARAM)"Ready");

   //if(dda->statusbarvis)
   //   ShowWindow(readystatus, SW_MAXIMIZE);

  /* FIXME: Move this to its own initialization function. 
   * Initialization needs to take location parameters computed
   * according to size of status bar slot instead of hardwired 
   * as it is here.
   */
   hprogbar = CreateWindow(PROGRESS_CLASS, "Prog bar", WS_CHILD|PBS_SMOOTH,
      230,5,117,12,  // ulx, uly, xwidth, ywidth
      anastatus,NULL,hInst,NULL);

   ShowWindow(hprogbar, SW_SHOWNORMAL);

   initMainToolbar(hwMain);
   ShowWindow(hToolBar, SW_SHOWNORMAL);
   ShowWindow(readystatus, SW_SHOWNORMAL);

   writeDDAProfile();

  /* Handle a possible command line argument from 
   * a file drag and drop.
   */
   if (__argc > 1)
      handleCommandLine(hwMain, __argc, __argv, &filepath);

}  /* close initializeDDAForWindows() */



/* This is ill-advised, and is only a kludge until 
 * status text size can be computed.
 */
#define STATUSBAR_TEXT_WIDTH 25
static 
void handleMouseMove(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
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

   dda->xcur = mousepos.x;
   dda->ycur = mousepos.y;

  /* Horrid kludge */
  /* FIXME: Get rid of this kludge, redesign, anything...
   * The following code is extremely unstable and in 
   * general not maintainable.
   */
   //if (geomdata != NULL && ad == NULL)
  /* There is a serious bug somewhere that allows a null vertex and 
   * vindex arrays to pass into this block.  Not sure where or why this
   * is happening. in the meantime, try this instead... 
   */
   if (dda->menustate == (GEOM_STATE | FINISHED) )
   {
      p = DPtoPP(hwMain, xnew,ynew);
      blocknumber = geomdata->getblocknumber(geomdata->this,p.x,p.y);
      sprintf(blocknumtext,"BN: %d",blocknumber);
      sprintf(xycoordstext,"\t(%.3f,%.3f)\t",p.x,p.y);
      SendMessage(geomstatus,SB_SETTEXT,(WPARAM)2,(LPARAM)blocknumtext);
      SendMessage(geomstatus,SB_SETTEXT,(WPARAM)5,(LPARAM)xycoordstext);

   }

   if (dda->menustate == (ANA_STATE | FINISHED) )
   {
      p = DPtoPP(hwMain, xnew,ynew);
      //blocknumber = geomdata->getblocknumber(geomdata->this,p.x,p.y);
      //sprintf(blocknumtext,"BN: %d",blocknumber);
      sprintf(xycoordstext,"\t(%.3f,%.3f)\t",p.x,p.y);
      //SendMessage(geomstatus,SB_SETTEXT,(WPARAM)2,(LPARAM)blocknumtext);
      SendMessage(anastatus,SB_SETTEXT,(WPARAM)9,(LPARAM)xycoordstext);
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

   char mess[80];
   int vkcode;
   int xnew, ynew;
   int blocknumber;
   DPoint p;
   DDA * dda = (DDA *)GetWindowLong(hwMain, GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);

   xnew = dda->xcur;
   ynew = dda->ycur;

   /* FIXME:  This is a kludgy way to handle this. */
   if (whatToDraw == LINES || whatToDraw == BLOCKS) {
      vkcode = wParam;

      switch (vkcode) {
      case 'c': {
            double centroid[2] = {0};
            //displayPhysicalCoordinates(hwMain,wParam,lParam);
            p = DPtoPP(hwMain, xnew,ynew);
            blocknumber = geomdata->getblocknumber(geomdata->this,p.x,p.y);
            gd_get_block_centroid(geomdata,blocknumber,centroid);
            sprintf(mess,"Block %d centroid: (%lf,%lf)",blocknumber,centroid[0],centroid[1]);
            MessageBox(NULL,mess,"Block centroid coordinates",MB_OK);
                }
            break;

      case VK_F1:
         MessageBox(NULL,NULL,NULL,MB_OK);
         break;

         default:
            break;
      }
   }

   return 0;

}  /* close handleChar() */




/* Handlers follow, in approximate order of menu entry for 
 * convenience.
 */


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
			   drawBolts(hdc, screenPen, geomdata, g, geomdata->origbolts);
         }
         drawPoints(hdc, g, geomdata, geomdata->points);
         drawBolts(hdc, screenPen, geomdata, g, geomdata->rockbolts);
         if (0/* g->stresses */)
            drawStresses(hdc, screenPen, geomdata, g);
         break;

      case TITLE:	 	
         handleMainAbout(hwMain);
         //drawTitle(draw_wnd, hdc);
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
handleGeomApply(HWND hwMain, double scale_params[])
{

   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);


   //updateMainMenu(hwMain, geomstate*runningstate);

   dda->menustate = GEOM_STATE | RUNNING;

   whatToDraw = LINES;

  /* These calls need to be changed to function pointers 
   * to get headers under control.
   */
  /* Don't worry about freeing the geometry data for now. */
   /*
   if (geomdata != NULL)
   {
      freeGeometrydata(geomdata);
   }
   */

   //if (g != NULL)
   //{
   freeGraphicStruct(g);
   //g = initGraphicStruct(geomdata, ad, g);
   g = initGraphicStruct();
   //}

   //geomhwnd = hwMain;

   geomdata = ddacut(&filepath, g);
   dda_set_geometrydata(dda,geomdata);

   //geometry2draw = geomdata;

  /* This handles having a bad geometry file.  An alternative 
   * would be to have it come back to the geometry dialog,
   * but we would then need to handle the `cancel' button 
   * event from the dialog.  Smarter than your average bear...
   */
   if (!geomdata) 
   { 
      whatToDraw = NOTHING;
	   InvalidateRect(hwMain, NULL, TRUE);
		UpdateWindow(hwMain);
      return 0;
   }

   if(geomdata->nBlocks>0) 
   {
      whatToDraw = BLOCKS;
      //updateMainMenu(hwMain, geomstate*finishedstate);
      dda->menustate = GEOM_STATE | FINISHED;
      updateGeometryStatusBar(hwMain);
	} 
   else 
   {
      whatToDraw = NOTHING;
		filepath.gfile[0] = '\0';
		sprintf(mainWinTitle, "%s for Windows 95/NT", (LPSTR) szAppName);
		SetWindowText(hwMain, (LPCTSTR) mainWinTitle);
      //updateMainMenu(hwMain, geomstate*abortedstate);
      dda->menustate = GEOM_STATE | ABORTED;
   }

   InvalidateRect(hwMain, NULL, TRUE);
   UpdateWindow(hwMain);

  /* These functions were previously handled in the menu code,
   * which is the wrong place.  This is not the right place
   * either, so FIXME: Find a home for the following toolbar code.
   */
   SendMessage(hToolBar, TB_SETSTATE, GEOM_APPLY, MAKELPARAM(TBSTATE_ENABLED,0));
   SendMessage(hToolBar, TB_SETSTATE, ANAL_BROWSE, MAKELPARAM(TBSTATE_ENABLED,0));
   SendMessage(hToolBar, TB_SETSTATE, ANAL_NEW, MAKELPARAM(TBSTATE_ENABLED,0));

   //dda->menustate = GEOM_STATE | FINISHED;

	return 0; //end case geom_apply
}  /* close handleGeomApply() */



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
   iface->setdisplay((unsigned int)hwMain);

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
      //updateMainMenu(hwMain, geomstate);
      dda->menustate = GEOM_STATE;
      //updateMainMenu(hwMain, GEOM_STATE);

     /* Initialize the path. Note that the .geo file is already loaded into 
      * the struct, so it doesn't have to be reloaded.
      */
      strcpy(temp, filepath.gfile);
      strcpy (filepath.rootname, strtok(temp, "."));

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
      dda->menustate = ANA_STATE | READY_STATE;

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
handleAnalRun(HWND hwMain)
{
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   int retval;

   dda->menustate = ANA_STATE | RUNNING;
   /* FIXME: Move the toolbar code to a handler
    */
    SendMessage(hToolBar, TB_SETSTATE,ANAL_ABORT,MAKELPARAM(TBSTATE_ENABLED,0));
    SendMessage(hToolBar, TB_SETSTATE, GEOM_APPLY, MAKELPARAM(0,0));
    SendMessage(hToolBar, TB_SETSTATE, GEOM_NEW, MAKELPARAM(0,0));
    SendMessage(hToolBar, TB_SETSTATE, GEOM_BROWSE, MAKELPARAM(0,0));
    SendMessage(hToolBar, TB_SETSTATE, ANAL_BROWSE, MAKELPARAM(0,0));
    SendMessage(hToolBar, TB_SETSTATE, ANAL_NEW, MAKELPARAM(0,0));

   whatToDraw = BLOCKS;

#if WINGRAPHICS
   InvalidateRect(hwMain, NULL, TRUE);
   UpdateWindow(hwMain);
#else
   SwapBuffers(hdc);
#endif /* WINGRAPHICS */

   /* So I really need to grab the analysis struct before calling
    * so that all the initialization can be done...  
    */
   retval = ddanalysis(dda, &filepath, g);

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

   dda->menustate = ANA_STATE | FINISHED;

  /* FIXME: Move the following code to a handler.
   */
   SendMessage(hToolBar, TB_SETSTATE, GEOM_APPLY, MAKELPARAM(TBSTATE_ENABLED,0));
   SendMessage(hToolBar, TB_SETSTATE, ANAL_BROWSE, MAKELPARAM(TBSTATE_ENABLED,0));
   SendMessage(hToolBar, TB_SETSTATE, GEOM_NEW, MAKELPARAM(TBSTATE_ENABLED,0));
   SendMessage(hToolBar, TB_SETSTATE, GEOM_BROWSE, MAKELPARAM(TBSTATE_ENABLED,0));
   SendMessage(hToolBar, TB_SETSTATE, ANAL_NEW, MAKELPARAM(TBSTATE_ENABLED,0));
   SendMessage(hToolBar,TB_SETSTATE,ANAL_ABORT,MAKELPARAM(0,0));

   showOrig = TRUE;  /*  showOrig is a global int masquerading as a boolean. */
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

   iface->setdisplay((unsigned int)hwMain);

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
handleMainAbout(HWND hwMain)
{
   char about[560];
			    		
   sprintf(about, "%s\n%s%s%s\n%s\n%s%s%s", aText[0], aText[1], aText[2], 
   aText[3], aText[4], aText[5],  aText[6], aText[7]);
  	MessageBox( hwMain, about, "DDA for Windows", MB_OK );

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
   

   ad->abort(ad->this);


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
 */
static void 
handleRButtonUp(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   HMENU hmenu, hpopupmenu;
   POINT pt;
   HINSTANCE hInst;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);
   Analysisdata * ad = dda_get_analysisdata(dda);

   if(dda->popupvis == 0)
      return;

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);
   hmenu = LoadMenu(hInst, "IDR_CHILDCONTEXT");
   hpopupmenu = GetSubMenu(hmenu,0);
   if (geomdata == NULL)
   {
      EnableMenuItem(hpopupmenu, POPUP_ANALYSIS, MF_BYCOMMAND | MF_GRAYED);
     /* FIXME: Replay handling does not work very well here. */
    	 EnableMenuItem(hpopupmenu, POPUP_REPLAY, MF_BYCOMMAND | MF_GRAYED);
   }

  /* FIXME: Get rid of the external Analysisdata * ad */
   if(ad == NULL)
   {
  	   EnableMenuItem(hpopupmenu, POPUP_ABORT, MF_BYCOMMAND | MF_GRAYED);
     /* FIXME: Replay handling does not work very well here. */
    	 //EnableMenuItem(hpopupmenu, POPUP_REPLAY, MF_BYCOMMAND | MF_GRAYED);
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

   blocknum = geomdata->getblocknumber(geomdata->this, p.x, p.y);

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

   blocknum = geomdata->getblocknumber(geomdata->this, p.x, p.y);

   /* FIXME: Confirmation code does not work. */
   sprintf(mess,"Confirm delete block number %d", blocknum);
   MessageBox(hwMain, mess, "Delete block", MB_OK);

   geomdata->deleteblock(geomdata->this, blocknum);


   updateGeometryStatusBar(hwMain);

			InvalidateRect(hwMain, NULL, TRUE);
			UpdateWindow(hwMain);

}  /* deleteBlockNumber() */




/* FIXME: The status bar should be initialized elsewhere.
 * this function should really not do much at all, other
 * than reset fields as geometry, analysis states change.
 * The actual field entry updates need to be handled directly
 * using function pointers.
 */
void 
updateStatusBar(HWND hwMain) //, WPARAM wParam, LPARAM lParam)
{

   char nblock[10];
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);

   switch(whatToDraw)
   {
      case TITLE:
         break;

      case BLOCKS:
         sprintf(nblock,"NB: %d", geomdata->nBlocks);
  			SendMessage(readystatus,SB_SETTEXT,0,(LPARAM)nblock);
	   	SendMessage(readystatus,SB_SETTEXT,2,(LPARAM)"Status Bar");
         break;

      default:
         break;
   }

}  /* close updateStatusBar() */



static void 
handleWMSize(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   SendMessage(readystatus,WM_SIZE,0,0);
   SendMessage(geomstatus,WM_SIZE,0,0);
   SendMessage(anastatus,WM_SIZE,0,0);
   SendMessage(hToolBar,WM_SIZE,0,0);

   return;
}  /* close handleWMSize() */



static void 
updateGeometryStatusBar(HWND hwMain)
{
   HINSTANCE hInst;
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);

  /* This is ill-advised, and is only a kludge until 
   * status text size can be computed.
   */
   char numblocktext[STATUSBAR_TEXT_WIDTH];
   char numcontactext[STATUSBAR_TEXT_WIDTH];
   char maxareatext[STATUSBAR_TEXT_WIDTH];
   char minareatext[STATUSBAR_TEXT_WIDTH];

   //char blocknumtext[STATUSBAR_TEXT_WIDTH];
   //char xycoordstext[STATUSBAR_TEXT_WIDTH];

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

  /* This should never happen because this function is only called 
   * after a geometry is successfully loaded.
   */
   assert(geomdata != NULL);

   sprintf(numblocktext,"NB: %d", geomdata->nBlocks);
   sprintf(numcontactext,"NC: %s", "000");
   sprintf(maxareatext,"Max: %.3f", 0.0);
   sprintf(minareatext,"Min: %.3f", 0.0);

   g_hIcon0 = LoadImage(hInst,MAKEINTRESOURCE(ICON_GREENLEDON),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
   g_hIcon1 = LoadImage(hInst,MAKEINTRESOURCE(ICON_GREENLEDOFF),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
   // Setup icons
   SendMessage(geomstatus,SB_SETICON,0,(LPARAM)g_hIcon0);
   SendMessage(geomstatus,SB_SETICON,1,(LPARAM)g_hIcon1);

   SendMessage(geomstatus,SB_SETTEXT,(WPARAM)0,(LPARAM)numblocktext);
   SendMessage(geomstatus,SB_SETTEXT,(WPARAM)1,(LPARAM)numcontactext);
   SendMessage(geomstatus,SB_SETTEXT,(WPARAM)3,(LPARAM)maxareatext);
   SendMessage(geomstatus,SB_SETTEXT,(WPARAM)4,(LPARAM)minareatext);
   SendMessage(geomstatus,SB_SETTEXT,(WPARAM)(6|SBT_NOBORDERS),(LPARAM)"");


   ShowWindow(readystatus, SW_HIDE);
   ShowWindow(anastatus, SW_HIDE);
   if(dda->statusbarvis == 1)
      ShowWindow(geomstatus, SW_MAXIMIZE);

}  /* close updateGeometryStatusBar() */


void
updateAnalysisStatusBar(HWND hwMain)
{
   //HICON redoff;
   HINSTANCE hInst;

   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   Geometrydata * geomdata = dda_get_geometrydata(dda);
   Analysisdata * ad = dda_get_analysisdata(dda);

  /* This is ill-advised, and is only a kludge until 
   * status text size can be computed.
   */
   char numblocktext[STATUSBAR_TEXT_WIDTH];
   char elapsedtimetext[STATUSBAR_TEXT_WIDTH];
   char currtimesteptext[STATUSBAR_TEXT_WIDTH];
   char oc_contactext[STATUSBAR_TEXT_WIDTH];
   char rcondtext[STATUSBAR_TEXT_WIDTH];

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   //gg->numtimesteps = AData->nTimeSteps;
  /* FIXME: Change gg->timestep to gg->delta_t */
   //gg->timestep = AData->currTimeStep;
   //g->currenttime = ad->currentTime;

   sprintf(numblocktext,"NB: %d", geomdata->nBlocks);
   //sprintf(elapsedtimetext,"ET: %.4f", ad->currentTime); // /*g->currenttime*/);
   sprintf(elapsedtimetext,"ET: %.4f", ad->elapsedTime); // /*g->currenttime*/);
   sprintf(currtimesteptext,"TS: %d/%d",ad->currTimeStep,ad->nTimeSteps);//g->timestep,g->numtimesteps);
   

   sprintf(oc_contactext,"OC: %d", g->openclosecount);
   sprintf(rcondtext,"|1|: %.4f", 0.0);

   SendMessage(anastatus,SB_SETTEXT,(WPARAM)0,(LPARAM)numblocktext);
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)1,(LPARAM)elapsedtimetext);
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)2,(LPARAM)currtimesteptext);
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)3,(LPARAM)oc_contactext);
  /* Progress bar in slot 4 */
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)5,(LPARAM)rcondtext);

   //redoff = LoadImage(hInst,MAKEINTRESOURCE(ICON_REDLEDOFF),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
   //SendMessage(anastatus,SB_SETICON,9,(LPARAM)redoff);

   ShowWindow(readystatus, SW_HIDE);
   ShowWindow(geomstatus, SW_HIDE);
   if(dda->statusbarvis)
      ShowWindow(anastatus, SW_MAXIMIZE);

}  /* close updateAnalysisStatusBar() */

		  
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
 
   if(lpToolTipText->hdr.code == TTN_NEEDTEXT  && dda->tooltipvis == 1)
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



/* Toggles the view state of tool bar, status bar, etc. 
 * FIXME: All this stuff needs to go into the menu handling 
 * function for WM_INITMENUPOPUP.
 */
static void
handleViewToggles(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   HMENU hMainMenu, hSubMenu;
   int submenuposition;
   //enum mmenu { mainmenu=0,viewmenu,geometrymenu, analysismenu, resultsmenu};
   DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
            
   hMainMenu = GetMenu(hwMain);

   submenuposition = findSubMenuPosition(hMainMenu, VIEW_TBAR);
   hSubMenu = GetSubMenu(hMainMenu, submenuposition);

   switch (wParam)
   {
      case VIEW_TBAR:
         //hSubMenu = GetSubMenu(hMainMenu, viewmenu);
         if (dda->toolbarvis == TRUE)
         {
            ShowWindow(hToolBar, SW_HIDE);
            dda->toolbarvis = FALSE;
           /* Very kludgy.  This should send a message to get 
            * the current text of the menu item.
            */
            ModifyMenu(hSubMenu, VIEW_TBAR, MF_BYCOMMAND | MF_UNCHECKED,VIEW_TBAR,"&Tool Bar");
         }
         else
         {
            ShowWindow(hToolBar, SW_SHOWNORMAL);
            dda->toolbarvis = TRUE;
            ModifyMenu(hSubMenu, VIEW_TBAR, MF_BYCOMMAND | MF_CHECKED,VIEW_TBAR,"&Tool Bar");
         }
         break;

      case VIEW_SBAR:
         //hSubMenu = GetSubMenu(hMainMenu, viewmenu);
         if (dda->statusbarvis == TRUE)
         {
            ShowWindow(readystatus, SW_HIDE);
            dda->statusbarvis = FALSE;
           /* Very kludgy.  This should send a message to get 
            * the current text of the menu item.
            */
            ModifyMenu(hSubMenu, VIEW_SBAR, MF_BYCOMMAND | MF_UNCHECKED,VIEW_SBAR,"&Status Bar");
         }
         else
         {
            ShowWindow(readystatus, SW_SHOWNORMAL);
            dda->statusbarvis = TRUE;
            ModifyMenu(hSubMenu, VIEW_SBAR, MF_BYCOMMAND | MF_CHECKED,VIEW_SBAR,"&Status Bar");
         }
         break;

      case VIEW_POPUP:
         //hSubMenu = GetSubMenu(hMainMenu, viewmenu);
         if (dda->popupvis == TRUE)
         {
            dda->popupvis = FALSE;
           /* Very kludgy.  This should send a message to get 
            * the current text of the menu item.
            */
            ModifyMenu(hSubMenu, VIEW_POPUP, MF_BYCOMMAND | MF_UNCHECKED,VIEW_POPUP,"&Enable Pop-up");
         }
         else
         {
            dda->popupvis = TRUE;
            ModifyMenu(hSubMenu, VIEW_POPUP, MF_BYCOMMAND | MF_CHECKED,VIEW_POPUP,"&Enable Pop-up");
         }
         break;

      case VIEW_TOOLTIPS:
 		   //hSubMenu = GetSubMenu(hMainMenu, viewmenu);
         if (dda->tooltipvis == TRUE)
         {
            dda->tooltipvis = FALSE;
           /* Very kludgy.  This should send a message to get 
            * the current text of the menu item.
            */
            ModifyMenu(hSubMenu, VIEW_TOOLTIPS, MF_BYCOMMAND | MF_UNCHECKED,VIEW_TOOLTIPS,"T&ool Tips");
         }
         else
         {
            dda->tooltipvis = TRUE;
            ModifyMenu(hSubMenu, VIEW_TOOLTIPS, MF_BYCOMMAND | MF_CHECKED,VIEW_TOOLTIPS,"T&ool Tips");
         }
         break;      

      default:
         break;
   }

} /* close handleViewToggles() */


void 
initMainToolbar(HWND hwMain)
{
   HINSTANCE hInst;
  /* Called from WM_CREATE before the dda struct is passed to 
   * the window handle.
   */
   //DDA * dda = (DDA *)GetWindowLong(hwMain,GWL_USERDATA);
   TBBUTTON tbbutton[] = 
   {  
	  /* FIXME:  Second member of array is actually the command. */
	   {0, GEOM_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON , 0, 0},
	   {1, ANAL_NEW, 0, TBSTYLE_BUTTON, 0, 0},
      {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, -1},
	   {2, GEOM_BROWSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	   {3, ANAL_BROWSE, 0, TBSTYLE_BUTTON, 0, 0},
	   {4, ANAL_ABORT, 0, TBSTYLE_BUTTON, 0, 0},
	   //{4, GEOM_NEW, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
	   {5, GEOM_APPLY, 0, TBSTYLE_BUTTON, 0, 0},
      {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, -1},
      //{6, TOOLBAR_ZOOMIN, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
      //{7, TOOLBAR_ZOOMOUT, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
      {6, TOOLBAR_PRINT, 0, TBSTYLE_BUTTON, 0, 0}
      //{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, -1},
      //{9, TOOLBAR_PRINT, 0, TBSTYLE_BUTTON, 0, 0}

   };

//#define NUMIMAGES 5 
#define NUMIMAGES 18
#define IMAGEWIDTH 16
#define IMAGEHEIGHT 16
#define BUTTONWIDTH 0
#define BUTTONHEIGHT 0

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   hToolBar = CreateToolbarEx(hwMain, 
	                           WS_CHILD | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE, 
	                           TOOLBAR_DDA_MAIN,
                              NUMIMAGES,
                              hInst, 
                              TOOLBAR_DDA_MAIN,
                              tbbutton,
                              sizeof(tbbutton)/sizeof(TBBUTTON),
                              BUTTONWIDTH,
                              BUTTONHEIGHT, 
                              IMAGEWIDTH,
                              IMAGEHEIGHT, 
                              sizeof(TBBUTTON));

  /* FIXME: Send a message to uncheck the appropriate menu item.
   * FIXME: Move all this code to a toolbar.c file.
   */
   //if (dda->toolbarvis)
   //   ShowWindow(hToolBar, SW_SHOWNORMAL);

}  /* close initMainToolbar() */



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
   mfdc = CreateEnhMetaFile(hdcmain,"C:\\mfile.emf",NULL,NULL);

#if _DEBUG
   if(!mfdc)
      showlasterror(GetLastError());
#endif
    
   g->scale = setScale(hwMain, mfdc, g, geomdata->scale);
   drawBlocks(mfdc, hBr, g, geomdata);
   if(showOrig)
      drawJoints(mfdc, screenPen, g, geomdata, geomdata->origvertices, TRUE);
   drawJoints(mfdc, screenPen, g, geomdata, geomdata->vertices, FALSE);
   drawPoints(mfdc, g, geomdata, geomdata->points);
   drawBolts(mfdc, screenPen, geomdata, g, geomdata->rockbolts);

   hmf = CloseEnhMetaFile(mfdc);
   DeleteEnhMetaFile(hmf);
   ReleaseDC(hwMain, hdcmain);

}  /* close handleMetafile() */




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
         readReplayFile(hwMain,g,filepath.replayfile);
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

		case MAIN_ABOUT:
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
         readReplayFile(hwMain,g,filepath.replayfile);
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

/*
   if (message == 0x100) {
         MessageBox(hwMain,"keydown","keydown",MB_OK);
   }
*/

   switch (message)
   {

   case WM_SYSCHAR:
      MessageBox(NULL,"syschar",NULL,MB_OK);
      break;

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
         updateMainMenuA(hwMain, dda->menustate);
         break;



      case WM_MENUSELECT:
        /* There needs to be a state variable here to track 
         * callbacks for handling dynamically loaded menu
         * items.  This is pretty nasty stuff.  See page 
         * 892 in Rector and Newcomer.
         */
         menuflags = HIWORD(wParam);
         if ( (menuflags & MF_POPUP) && (menuflags != 0xFFFF))
         {
            menu_item_number = LOWORD(wParam);
            //sprintf(mess,"%d",menu_item_number);
            //MessageBox(hwMain,mess,mess,MB_OK);
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
   DDA dda;

  /* FIXME: Add code to uninitialize */
   HtmlHelp(NULL,NULL,HH_INITIALIZE,(DWORD)&dwHTMLCookie); 

   memset(&dda,0xDA,sizeof(DDA));

   filepath.rootname[0] = '\0';

   registerDDAMainWindowClass(hInstance);

   hwMain = createDDAMainWindow(hInstance);

   checkval = SetWindowLong(hwMain,GWL_USERDATA,(LONG)&dda);

  /* Move this stuff into the dda.ini file,which 
   * should be read in the initialization function.
   */
   dda.toolbarvis = 1;
   dda.statusbarvis = 1;
   dda.popupvis = 1;
   dda.tooltipvis = 1;
   dda.menustate = READY_STATE;

   hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE( DDAAPACCEL ));

  	while (GetMessage (&msg, NULL, 0, 0)) {
      if (!TranslateAccelerator(msg.hwnd,hAccel,&msg)) {
         TranslateMessage (&msg);
         DispatchMessage (&msg);
      } 

   }  

   return msg.wParam;

}  /* close WinMain()  */



