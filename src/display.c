
/* FIXME: Everything in this file needs to be platform independent.
 */
/* FIXME: Get rid of these somehow by fixing this display 
 * function to not depend on windows at all.
 */
#define WINGRAPHICS 1

//#include "compilecontrol.h"

#include "analysis.h"
#include "graphics.h"
#include "interface.h"
#include "postprocess.h"
#include <stdio.h>




#if DDA_FOR_WINDOWS
#ifndef STRICT 
#define STRICT
#endif
#include <windows.h>
#include <commctrl.h>
#endif



/* This should be the only extern global variable in this file.
 */
extern InterFace * iface;

#if DDA_FOR_WINDOWS
extern HWND anahwnd;
extern HDC anahdc;
extern HWND anastatus;
#endif


/* FIXME: Get rid of this function prototype by moving the 
 * function call into interface file.
 */
//void updateAnalysisStatusBar();

void
//display(HWND hwMain, HDC hdc, Geometrydata * GData, Analysisdata * AData, 
//         GRAPHICS *gg)
display(Geometrydata * GData, Analysisdata * AData, GRAPHICS *gg)
{

#if DDA_FOR_WINDOWS
   MSG winMess;
   extern HWND hprogbar;  // yuck
   //HWND hwMain = anahwnd;
   HDC hdc = anahdc;
#endif

  /* FIXME: This is a kludge.  Need to get a proper header file for this. */
   //extern void writeReplayFile(Geometrydata * gd, Analysisdata * ad);


   if (!(AData->currTimeStep%AData->tsSaveInterval))
      writeReplayFile(GData, AData);  

   //computeStresses(GData, AData, e0, moments, gg);


#if DDA_FOR_WINDOWS     

  /* FIXME: Move all of this stuff into the analysis status bar 
   * updating function.
   */
   if (AData->currTimeStep == 1 && AData->gravityflag == 0)
   {
      SendMessage(hprogbar,PBM_SETRANGE,(WPARAM)0,MAKELPARAM(0,AData->nTimeSteps));
   }
   if (AData->currTimeStep > 1 && AData->gravityflag == 0)
   {
      SendMessage(hprogbar,PBM_SETPOS,(WPARAM)AData->currTimeStep,(LPARAM)0);

   }


   gg->numcontacts = AData->nCurrentContacts;
  /* FIXME: This only has to be set once, it is set here 
   * on every time step because this is a kludge.
   */
   gg->numtimesteps = AData->nTimeSteps;
  /* FIXME: Change gg->timestep to gg->delta_t */
   gg->timestep = AData->currTimeStep;
   //gg->currenttime = AData->currentTime;
   gg->elapsedtime = AData->elapsedTime;
   gg->openclosecount = AData->m9;

   iface->updatedisplay();

     /* check for other messages (eg, abort)
      * FIXME: This code does not work very well right
      * now.  It would be really nice to be able to suspend
      * or cleanly abort a running analysis.  Currently,
      * the only menu item working is File->Exit
      */
      while (PeekMessage(&winMess, NULL, 0, 0, PM_REMOVE)) 
      {
         if (winMess.message == WM_QUIT)
            exit (0);
          //if (winMess.message == WM_COMMAND && GET_WM_COMMAND_ID(wParam, lParam)== ANAL_ABORT)
          	//return 0;
         TranslateMessage(&winMess);
         DispatchMessage(&winMess);
      } /* end while  */
#else
   fprintf(stderr,".");
#endif
}  /* close display() */