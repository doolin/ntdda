//#if DEMO

#include "compilecontrol.h"
#include "dda.h"

#ifdef WIN32
#define STRICT
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


void
handleARS(HWND hwMain)
{

 char mess[80];

       STARTUPINFO ves;
       BOOL  flag;
	      PROCESS_INFORMATION pi;
	      ves.lpReserved=NULL;
	      ves.lpDesktop=NULL;
      	ves.lpTitle=NULL;
	      ves.cbReserved2=0;
	      ves.lpReserved2=NULL;
	      ves.cb=sizeof(STARTUPINFO);
	      strcpy(mess,"D:\\Program Files\\ars\\launcher.exe D:\\Program Files\\ars\\ars.ini");
       flag = CreateProcess(NULL,(LPTSTR)mess,NULL,NULL,FALSE,
          NORMAL_PRIORITY_CLASS,NULL,NULL,&ves,&pi);

       ShowWindow(hwMain, SW_MINIMIZE);
       flag = WaitForSingleObject(pi.hProcess,INFINITE);

       if (flag != WAIT_OBJECT_0)
       {
          MessageBox(hwMain, "TSlope not found in path",mess /* "Not found"*/, MB_OK);
       }
       else
          ShowWindow(hwMain, SW_MINIMIZE);

       ShowWindow(hwMain, SW_RESTORE);
}  /* close handleTSlope() */

void
handleTSlope(HWND hwMain)
{

char mess[80];

       STARTUPINFO ves;
       BOOL  flag;
	      PROCESS_INFORMATION pi;
	      ves.lpReserved=NULL;
	      ves.lpDesktop=NULL;
      	ves.lpTitle=NULL;
	      ves.cbReserved2=0;
	      ves.lpReserved2=NULL;
	      ves.cb=sizeof(STARTUPINFO);
	      strcpy(mess,"D:\\Program Files\\tslope\\launcher.exe D:\\Program Files\\tslope\\tslope.ini");
       flag = CreateProcess(NULL,(LPTSTR)mess,NULL,NULL,FALSE,
          NORMAL_PRIORITY_CLASS,NULL,NULL,&ves,&pi);

       ShowWindow(hwMain, SW_MINIMIZE);
       flag = WaitForSingleObject(pi.hProcess,INFINITE);

       if (flag != WAIT_OBJECT_0)
       {
          MessageBox(hwMain, "TSlope not found in path",mess /* "Not found"*/, MB_OK);
       }
       else
          ShowWindow(hwMain, SW_MINIMIZE);

       ShowWindow(hwMain, SW_RESTORE);
}  /* close handleTSlope() */
//#endif /* DEMO */