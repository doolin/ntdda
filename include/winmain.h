

#ifndef __WINMAIN_H__
#define __WINMAIN_H__


#include <windows.h>
#include <windowsx.h>


#include "dda.h"
#include "options.h"


/* These were moved in from dda.h */
#define WINGRAPHICS 1
#define OPENGL 0



/* This should really go into the header file, but it needs 
 * windows.h to work properly.  Change it later.  
 */
typedef struct tagParamBlock {
   WORD 		wEnvSeg;		  	// usually NULL
	  LPSTR		lpCmdLine;	  	// command line string
	  LPVOID	    lpCmdShow;		// WORD w[2]
	  DWORD		dwReserved;		// always NULL
} PARAMBLOCK;


BOOL CALLBACK  GeomDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK  AnalDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK  UnitDlgProc (HWND, UINT, WPARAM, LPARAM);



void printGeom(HWND, PSTR, Geometrydata *, double [], GRAPHICS *);
void fileBrowse(HWND, OPENFILENAME *, LPCTSTR *, char *, char *, char *);
void loadNotepad(PARAMBLOCK *, char *);  /* Should return an int for error checking.  */
void stub();

void initializePens(void);
void initializeBrushes();
void drawTitle(HWND, HDC);
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


void handleDropFiles(HWND hwMain, WPARAM wParam, LPARAM lParam, FILEPATHS *);
void handleCommandLine(HWND, int argc, char ** argv, FILEPATHS *);


void handleViewToggles(HWND hwMain, WPARAM wParam, LPARAM lParam);

void handleMainAbout(HWND hwMain);

void handleHtmlHelp(HWND hwMain, WPARAM wParam, LPARAM lParam);
void updateMainMenuA(HWND hwMain, int state);
void handleOptionsMenu(HWND hwMain, WPARAM wParam, Options * o);


/* FIXME: Definitely move this out of this 
 * header file.
 */
BOOL CALLBACK DrawDlgProc (HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK APWEDGEDlgProc (HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

int handleAPWEDGE(HWND);


#endif /*  __WINMAIN_H__  */
