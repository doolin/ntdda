

#ifdef WIN32
#pragma warning( disable : 4201 4115 )        
#endif

#include <stdio.h>

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "statusbar.h"
#include "runstates.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


HWND statusbar;

HICON g_hIcon0 = NULL;		
HICON g_hIcon1 = NULL;		

HWND hprogbar;

/** 
 * @todo Move these into a statusbar struct that 
 * can be passed around by reference, which is a 
 * lot more portable.
 */
#define SB_READY_PARTS 1
static int readyparts[SB_READY_PARTS] = { -1 };
#define SB_GEOM_PARTS 7
static int geomparts[SB_GEOM_PARTS];
#define SB_ANAL_PARTS 10
static int anaparts[SB_ANAL_PARTS];


static int visibility;


#if 0
static int
statusbar_toggle_visibility() {

   if (visibility == 1) {
      visibility = 0;
      ShowWindow(statusbar, SW_HIDE);
   } else {
      visibility = 1;
      ShowWindow(statusbar, SW_SHOWNORMAL);
   }
}
#endif


int
statusbar_get_visibility() {
   return visibility;
}


void 
statusbar_set_visibility(int v) {
   visibility = v;
   statusbar_show();
}


void
statusbar_show(void) {

   if (visibility == 1) {
      ShowWindow(statusbar, SW_SHOWNORMAL);
   } else {
      ShowWindow(statusbar, SW_HIDE);
   }
}


void
statusbar_resize(void) {

   SendMessage(statusbar,WM_SIZE,0,0);
}


void
statusbar_set_text(WPARAM wParam, LPARAM lParam) {

   SendMessage(statusbar,SB_SETTEXT,wParam,lParam);
}



void
statusbar_set_analysis_parts() {

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

   SendMessage(statusbar,SB_SETPARTS,(WPARAM)SB_ANAL_PARTS,(LPARAM)anaparts);
}


void
statusbar_set_geometry_parts() {

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

   SendMessage(statusbar,SB_SETPARTS,(WPARAM)SB_GEOM_PARTS,(LPARAM)geomparts);

}

void
statusbar_set_ready_parts() {

   SendMessage(statusbar,SB_SETPARTS,(WPARAM)(SB_READY_PARTS),(LPARAM)readyparts);
   SendMessage(statusbar,SB_SETTEXT, (WPARAM)(SBT_NOBORDERS),(LPARAM)"Ready");
}




static void
statusbar_send_message(HWND sbar, unsigned int button, unsigned short state) {

   SendMessage(sbar, 0, button, MAKELPARAM(state,0));
}




void
statusbar_set_state(unsigned int state) {

   switch (state) {
   
      case (GEOM_STATE | FINISHED):
         statusbar_set_geometry_parts();
         ShowWindow(hprogbar, SW_HIDE);
      break;

      case (ANA_STATE | RUNNING):
         statusbar_set_analysis_parts();
         ShowWindow(hprogbar, SW_SHOWNORMAL);
      break;

      case (ANA_STATE | FINISHED):
      break;

      default:
      break;
   }

}


void 
statusbar_update_geometry(int numblocks) {


  /* This is ill-advised, and is only a kludge until 
   * status text size can be computed.
   */
   char numblocktext[STATUSBAR_TEXT_WIDTH];
   char numcontactext[STATUSBAR_TEXT_WIDTH];
   char maxareatext[STATUSBAR_TEXT_WIDTH];
   char minareatext[STATUSBAR_TEXT_WIDTH];

   //char blocknumtext[STATUSBAR_TEXT_WIDTH];
   //char xycoordstext[STATUSBAR_TEXT_WIDTH];



   sprintf(numblocktext,"NB: %d", numblocks);
   sprintf(numcontactext,"NC: %s", "000");
   sprintf(maxareatext,"Max: %.3f", 0.0);
   sprintf(minareatext,"Min: %.3f", 0.0);

   SendMessage(statusbar,SB_SETICON,0,(LPARAM)g_hIcon0);
   SendMessage(statusbar,SB_SETICON,1,(LPARAM)g_hIcon1);

   SendMessage(statusbar,SB_SETTEXT,(WPARAM)0,(LPARAM)numblocktext);
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)1,(LPARAM)numcontactext);
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)3,(LPARAM)maxareatext);
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)4,(LPARAM)minareatext);
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)(6|SBT_NOBORDERS),(LPARAM)"");

}  


void
statusbar_update_analysis(int numblocks, double elapsedtime,
                          int currtimestep, int numtimesteps,
                          int openclosecount) {

  /* This is ill-advised, and is only a kludge until 
   * status text size can be computed.
   */
   char numblocktext[STATUSBAR_TEXT_WIDTH];
   char elapsedtimetext[STATUSBAR_TEXT_WIDTH];
   char currtimesteptext[STATUSBAR_TEXT_WIDTH];
   char oc_contactext[STATUSBAR_TEXT_WIDTH];
   char rcondtext[STATUSBAR_TEXT_WIDTH];

   sprintf(numblocktext,"NB: %d", numblocks);

   sprintf(elapsedtimetext,"ET: %.4f",elapsedtime);
   sprintf(currtimesteptext,"TS: %d/%d",currtimestep,numtimesteps);

   sprintf(oc_contactext,"OC: %d",openclosecount);
   sprintf(rcondtext,"|1|: %.4f", 0.0);

   SendMessage(statusbar,SB_SETTEXT,(WPARAM)0,(LPARAM)numblocktext);
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)1,(LPARAM)elapsedtimetext);
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)2,(LPARAM)currtimesteptext);
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)3,(LPARAM)oc_contactext);
  /* Progress bar in slot 4 */
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)5,(LPARAM)rcondtext);

   //redoff = LoadImage(hInst,MAKEINTRESOURCE(ICON_REDLEDOFF),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

}  


void
statusbar_update_progbar(unsigned int wparam) {

   SendMessage(hprogbar,PBM_SETPOS,(WPARAM)wparam,(LPARAM)0);
}

void 
statusbar_set_progbar_range(unsigned short value) {

   SendMessage(hprogbar,PBM_SETRANGE,(WPARAM)0,MAKELPARAM(0,value));
}


void
statusbar_init(HWND hwMain) {

  /* FIXME: This needs to be just one control ID for 
   * one status bar, and it should be set from the 
   * appropriate value in the resource.h file.
   */
   UINT DDA_A_STATUSBAR = 2;
   UINT DDA_STATUSBAR   = 3;

   HINSTANCE hInst;
   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);


   g_hIcon0 = LoadImage(hInst,MAKEINTRESOURCE(ICON_GREENLEDON),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
   g_hIcon1 = LoadImage(hInst,MAKEINTRESOURCE(ICON_GREENLEDOFF),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);


   statusbar   = CreateStatusWindow(WS_CHILD | WS_MINIMIZE | WS_CLIPCHILDREN,
                                    "", hwMain, DDA_STATUSBAR);



  /* FIXME: Move this to its own initialization function. 
   * Initialization needs to take location parameters computed
   * according to size of status bar slot instead of hardwired 
   * as it is here.
   */
   hprogbar = CreateWindow(PROGRESS_CLASS, "Prog bar", WS_CHILD|PBS_SMOOTH,
      230,5,117,12,  // ulx, uly, xwidth, ywidth
      statusbar,NULL,hInst,NULL);
   ShowWindow(hprogbar, SW_HIDE);

   statusbar_set_visibility(1);
   statusbar_set_ready_parts();
   SendMessage(statusbar, SB_SETTEXT,(WPARAM)(SBT_NOBORDERS),(LPARAM)"Ready");
}


#ifdef __cplusplus
}
#endif