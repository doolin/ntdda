

//#include "compilecontrol.h"
#include "dda.h"
#include "resource.h"
#include "winmain.h"



#ifdef WIN32
#ifndef STRICT
#define STRICT
#endif
#include <windows.h>
/* Take the pragma out if not using VC++
 */
#pragma hdrstop
#include <windowsx.h>
#else
typedef long HWND;
typedef long HDC;
typedef long MSG;
#endif

FILEPATHS filepath;

void
handleVisualEditor(HWND hwMain, LPARAM lParam)
{

char mess[80];

/* The following defed out code is from the `New Geometry'
 * case, which is a bit different from handling existing 
 * geometries.  TODO: Integrate this later.
 */
#if NEWVISEDGEOM          
   STARTUPINFO ves; 
	PROCESS_INFORMATION pi;
   int doDraw=0;
			  		
   strcpy(filepath.oldfile, filepath.gfile);
	filepath.gfile[0] = '\0';
	sprintf(mainWinTitle, "%s for Windows 95/NT", (LPSTR) szAppName);
	SetWindowText(hwMain, (LPCTSTR) mainWinTitle);
 	ves.lpReserved=NULL;
 	ves.lpDesktop=NULL;
 	ves.lpTitle=NULL;
 	ves.cbReserved2=0;
   ves.lpReserved2=NULL;
   ves.cb=sizeof(STARTUPINFO);
   CreateProcess(NULL,"VisualEditor",NULL,NULL,FALSE,CREATE_NEW_PROCESS_GROUP|HIGH_PRIORITY_CLASS,NULL,NULL,&ves,&pi);
#endif  /* Code for new geometry with visual editor */

       STARTUPINFO ves; 
       BOOL  flag;
	      PROCESS_INFORMATION pi;
	      ves.lpReserved=NULL;
	      ves.lpDesktop=NULL;
      	ves.lpTitle=NULL;
	      ves.cbReserved2=0;
	      ves.lpReserved2=NULL;
	      ves.cb=sizeof(STARTUPINFO);
	      strcpy(mess,"VisualEditor ");
	      strcat (mess, filepath.gfile);
       flag = CreateProcess(NULL,(LPTSTR)mess,NULL,NULL,FALSE,
          /* CREATE_NEW_PROCESS_GROUP|*/NORMAL_PRIORITY_CLASS,NULL,NULL,&ves,&pi);
       flag = WaitForSingleObject(pi.hProcess,INFINITE);

       if (flag != WAIT_OBJECT_0)
       { 
          MessageBox(hwMain, "Visual Editor not found in path", "Not found", MB_OK);
       }         
       else
          ShowWindow(hwMain, SW_MINIMIZE);

       SendMessage(hwMain, WM_COMMAND, GEOM_APPLY, lParam);
       ShowWindow(hwMain, SW_RESTORE);
    

}  /* close handleVisualEditor() */