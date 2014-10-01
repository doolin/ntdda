
/* FIXME: Everything in this file needs to be platform independent.
 */

#include <stdio.h>

#include "analysis.h"
#include "postprocess.h"
#include "statusbar.h"


#ifdef WIN32
#pragma warning( disable : 4201 )        
#ifndef STRICT 
#define STRICT
#endif
#include <windows.h>
#include <commctrl.h>
#endif



/* This should be the only extern global variable in this file.
 */


void
display(Geometrydata * gd, Analysisdata * ad) {

#ifdef WIN32
   MSG winMess;
   extern HWND hprogbar;  // yuck
   extern HWND mainwindow;
#endif

  /* FIXME: This is a kludge.  Need to get a proper header file for this. */
   //extern void writeReplayFile(Geometrydata * gd, Analysisdata * ad);


   if (!(ad->cts%ad->tsSaveInterval)) {
      writeReplayFile(gd,ad);  
   }


#ifdef WIN32

   if (ad->cts == 1 && ad->gravityflag == 0) {
      //SendMessage(hprogbar,PBM_SETRANGE,(WPARAM)0,MAKELPARAM(0,ad->nTimeSteps));
      statusbar_set_progbar_range((unsigned short)ad->nTimeSteps);
   }
   
   if (ad->cts > 1 && ad->gravityflag == 0) {
      //SendMessage(hprogbar,PBM_SETPOS,(WPARAM)ad->currTimeStep,(LPARAM)0);
      statusbar_update_progbar((unsigned int)ad->cts);
   }


   statusbar_update_analysis(gd->nBlocks,
                             ad->elapsedTime,
                             ad->cts,
                             ad->nTimeSteps,
                             ad->m9);

   InvalidateRect(mainwindow, NULL, TRUE);
   UpdateWindow(mainwindow);

   //iface->updatedisplay();

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
}

