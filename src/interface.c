/* This interface module will end up taking lots and lots 
 * of header files.  This should take the burden off of
 * both the front end and the kernel for header dependencies.
 */

#define WINDOWS 1

#include "interface.h"
#include "resource.h"
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include "winmain.h"


/* Who gets to initialize this interface, and where
 * should the initialization call be executed from?
 */

/* Fix these with get/sets */

extern HWND anastatus;


/* We might have to display message boxes to windows 
 * other than the main window.  So this should be set
 * explicitly when necessary.
 */
static HWND currentHWND;


/* Private methods */
static void initializeIFace(void);
static void displayMessage(char * message);
static void setDisplay(unsigned int handle);
static void setAnalysisFText(char * text);
static void updateDisplay(void);
static void updateStatusBar(int bar);
static void displayWarning(char * warning);
static void setOCCount(int oc_count);


void setCurrentHWND(HWND);
HWND getCurrentHWND(void);


/* Public Methods */
InterFace *
getNewIFace()
{
    InterFace * iface;
    iface = (InterFace*)calloc(1,sizeof(InterFace));
    if (iface)
    {  
       iface->rungeometry = ddacut;
       //iface->runanalysis = ddanalysis;
       iface->displaymessage = displayMessage;
       iface->setdisplay = setDisplay;
       iface->setafetext = setAnalysisFText;
	    iface->updatedisplay = updateDisplay;
       iface->updatestatusbar = updateStatusBar;
       iface->displaywarning = displayWarning;
       iface->setoc_count = setOCCount;

       iface->this = iface;

       return iface;
    }
    else
       return NULL;  // FIXME: throw an error

    
}  /* close getNewIFace() */


static void
setDisplay(unsigned int handle)
{

  /* for windows */
   setCurrentHWND((HWND)handle);

}  /* close setDisplay() */

static void 
setCurrentHWND(HWND hwnd)
{

   currentHWND = hwnd;

} /* close setCurrentHWND() */


/* Private Methods */

static void 
displayMessage(char * message)
{
   //va_list ap;
   char * str = NULL;
   //char temp[10];
   //char * str2;
#if WINDOWS
   HWND currentwindow;
  // int i = 1;
   currentwindow = getCurrentHWND();
   //va_start(ap, message);
   //while (ap != NULL)
   //{
   //   va_arg(ap, void *);
   //   i++;
   //}
   //sprintf(temp,"%d",i);
   //MessageBox(currentwindow,temp,temp,MB_OK);
//   str = va_arg(ap, char *);
//   va_arg(ap, char *);
//   if (ap == NULL)
      MessageBox(currentwindow,message,"FIXME",MB_OK);
//   else 
//      MessageBox(currentwindow,message,str,MB_OK);
#else 
     fprintf(stdout,message);
#endif
   //va_end(ap);
}


HWND 
getCurrentHWND(void)
{

   return currentHWND;

}  /* close getCurrentHWND */



static void
setAnalysisFText(char * text)
{
   //SendMessage(anastatus,SB_SETICON,9,(LPARAM)0);

   SendMessage(anastatus,SB_SETTEXT,(WPARAM)(9|SBT_NOBORDERS),
               (LPARAM)text);

}  /* close setAnalysisFText() */

static void
setOCCount(int oc_count)
{

   char count[8];

   sprintf(count,"OC: %d",oc_count);
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)3,(LPARAM)count);

}  /* close setOCCount() */



static void
updateStatusBar(int bar)
{

   switch(bar)
   {
      case 0:  // ready bar
         break;

      case 1:  // geometry bar
         break;

      case 2:  // analysis bar
        /*
         if (AData->currTimeStep == 1 && AData->gravityflag == 0)
         {
            SendMessage(hprogbar,PBM_SETRANGE,(WPARAM)0,MAKELPARAM(0,AData->nTimeSteps));
         }
         if (AData->currTimeStep > 1 && AData->gravityflag == 0)
         {
            SendMessage(hprogbar,PBM_SETPOS,(WPARAM)AData->currTimeStep,(LPARAM)0);
         }
         */
         break;
  
      default:
         break;
   }



}  /* close updateStatusBar() */


/* For now, this will just warn of loss of diagonal dominance
 * in the status bar.
 */
static void
displayWarning(char * message)
{
   HICON warning_icon;
   HINSTANCE hInst;
   HWND hwMain;

  /* Recipe for bugs.  Need to get a current global hinst for 
   * this module.
   */
   hwMain = getCurrentHWND(); 
   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   warning_icon = LoadImage(hInst,MAKEINTRESOURCE(ICON_REDLEDON),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

   SendMessage(anastatus,SB_SETICON,9,(LPARAM)warning_icon);
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)(9|SBT_NOBORDERS),
               (LPARAM)message);

}  /* close displayWarning() */

void
updateDisplay(void)
{
   HWND hwMain;

   hwMain = getCurrentHWND();

  /* redraw window with new configuration  */
#if OPENGLDRAW /* OpenGL */
   InvalidateRect(hwMain, NULL, FALSE);
   SwapBuffers(hdc);
#else
   InvalidateRect(hwMain, NULL, TRUE);
#endif /* WIN32GRAPHICS */
   UpdateWindow(hwMain);

  /* FIXME: Move this into the interface file because this is a 
   * win32 feature only. 
   * Update the status bar for the next time step. 
   */
   updateAnalysisStatusBar(hwMain);


}  /* close updateDisplay() */
