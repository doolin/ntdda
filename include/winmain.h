

#ifndef _WINMAIN
#define _WINMAIN

//#define WINDOWS 1

#include <windows.h>
#include <windowsx.h>
//#include <winuser.h>


/*
 * Add stuff so that the dda.h file is not so full
 */
#include "dda.h"


/* These were moved in from dda.h */
#define WINGRAPHICS 1
#define OPENGL 0

/* Use these to set top level menu items */
#define READY_STATE 0x000001
#define GEOM_STATE  0x000002
#define ANA_STATE   0x000004
#define RUNNING     0x000008
#define FINISHED    0x000010
#define ABORTED     0x000020



#define ABOUT "UC Berkeley DDA for Windows 95/NT(unstable),\n", \
              "$Id: winmain.h,v 1.8 2002/05/25 14:49:39 doolin Exp $\n", \
				  "by Mary M. MacLaughlin (Montana Tech), and Nicholas Sitar & David Doolin\n", \
              "Department of Civil Engineering, Geotechnical Group\n", \
              "University of California, Berkeley, CA 94720\n", \
              "This program is based on the original code by Dr. Gen-hua Shi.\n", \
              "Original development was supported in part by funding from the\n", \
              "U.S. Army Engineer Waterways Experiment Station, Vicksburg, MS.\n", \
              "This executable was compiled for internal use only."


/* This is to collect up a bunch of global 
 * variables and clean up the interface.  These
 * structs should only be shared between the windows
 * files.  None of the numerical back-end should have
 * any need of any information here.
 */
typedef struct _win_options_tag   {
	int showOrig;
  /* Booleans for controlling output.  These probably need to 
   * into a different struct.  These need to go into a project
   * output struct.
   */
   int spyplots;
   int flagstates;
   int maxdisp;
   int maxtstep;
   int timing;
   int parameters;
   int fixedpoints;

} WINOPTIONS;


/* This should really go into the header file, but it needs 
 * windows.h to work properly.  Change it later.  
 */

typedef struct tagParamBlock {
   WORD 		wEnvSeg;		  	// usually NULL
	  LPSTR		lpCmdLine;	  	// command line string
	  LPVOID	    lpCmdShow;		// WORD w[2]
	  DWORD		dwReserved;		// always NULL
} PARAMBLOCK;


//LRESULT CALLBACK WndProc ( HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK  GeomDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK  AnalDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK  UnitDlgProc (HWND, UINT, WPARAM, LPARAM);

/* Open GL Function Declaration */
//LRESULT CALLBACK WndProc (HWND hwMain, UINT message, WPARAM wParam, LPARAM lParam);
//VOID EnableOpenGL (HWND hwMain, HDC * hDC, HGLRC * hRC);
//VOID DisableOpenGL (HWND hwMain, HDC hDC, HGLRC hRC);
//VOID GLlines (HDC * hdc);

void printGeom(HWND, PSTR, Geometrydata *, double [], GRAPHICS *);
void fileBrowse(HWND, OPENFILENAME *, LPCTSTR *, char *, char *, char *);
void loadNotepad(PARAMBLOCK *, char *);  /* Should return an int for error checking.  */
void stub();
//void DrawOrStore(HWND, HDC, GRAPHICS *, double [], Block *, Block *,
//                 DPoint *, DPoint *, int *, int *);
void initializePens(void);
void initializeBrushes();
void drawTitle(HWND, HDC);
//void replay(HWND, Block *, DPoint *, int *, int *);
void replay(HWND, Geometrydata *, GRAPHICS *, char * replayfilename);
void handleTSlope(HWND);
void handleGnuplot(HWND);
void handleARS(HWND);
int  handleGeomApply(HWND, double []);
void handleVisualEditor(HWND, LPARAM);

void handleGLPaint(HWND,int ,int);
void handleWinPaint(HWND,WPARAM,LPARAM,int ,int);
void handleGeomDraw(HWND);
void handleVisualEditor(HWND hwMain, LPARAM);
void handleGeometryDialog(HWND hwMain, LPARAM);
/* This function is actually in winmain.c */
void updateAnalysisStatusBar(HWND);
void handleDropFiles(HWND hwMain, WPARAM wParam, LPARAM lParam, FILEPATHS *);
void handleCommandLine(HWND, int argc, char ** argv, FILEPATHS *);


void handleViewToggles(HWND hwMain, WPARAM wParam, LPARAM lParam);
//void showlasterror(DWORD);

void handleMainAbout(HWND hwMain);

void handleHtmlHelp(HWND hwMain, WPARAM wParam, LPARAM lParam);
void updateMainMenuA(HWND hwMain, int state);
void handleOptionsMenu(HWND hwMain, WPARAM wParam, OPTIONS * o);


/* FIXME: Definitely move this out of this 
 * header file.
 */
BOOL CALLBACK DrawDlgProc (HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK APWEDGEDlgProc (HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

int handleAPWEDGE(HWND);

/*** function declarations ***/

#endif /*  _WINMAIN  */
