
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



//extern HWND readystatus;
//extern HWND geomstatus;
extern HWND anastatus;

extern HWND statusbar;

/* FIXME: Move to local if possible */
/* These won't work until version msvc 6 */
// User global variables
HICON g_hIcon0 = NULL;		// User icon 0, handle return by parser when 'Icon0' keyword is used
HICON g_hIcon1 = NULL;		// User icon 1, handle return by parser when 'Icon1' keyword is used


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


static int visibility = 1;

static int
statusbar_toggle_visibility() {

   if (visibility == 1) {
      visibility = 0;
   } else {
      visibility = 1;
   }
}

static int
statusbar_get_visibility() {
   return visibility;
}

static void 
statusbar_set_visibility(int v) {
   visibility = v;
}

void
statusbar_show(void) {

   if (visibility == 1) {
      ShowWindow(statusbar, SW_MAXIMIZE);
   }
}



void
statusbar_set_analysis_parts() {


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



/*
static void
statusbar_send_message(HWND hToolBar, unsigned int button, unsigned short state) {

   SendMessage(hToolBar, TB_SETSTATE, button, MAKELPARAM(state,0));
}
*/



void
statusbar_set_state(HWND sbar, unsigned int state) {

   switch (state) {
   
      case (GEOM_STATE | FINISHED):
         //statusbar_send_message(hToolBar,GEOM_APPLY, TBSTATE_ENABLED);
      break;

      case (ANA_STATE | RUNNING):
         //statusbar_send_message(hToolBar,ANAL_ABORT, TBSTATE_ENABLED);
      break;

      case (ANA_STATE | FINISHED):
         //statusbar_send_message(hToolBar,GEOM_APPLY, TBSTATE_ENABLED);
      break;

      default:
      break;
   }

}


void 
updateGeometryStatusBar(int numblocks) {


  /* This is ill-advised, and is only a kludge until 
   * status text size can be computed.
   */
   char numblocktext[STATUSBAR_TEXT_WIDTH];
   char numcontactext[STATUSBAR_TEXT_WIDTH];
   char maxareatext[STATUSBAR_TEXT_WIDTH];
   char minareatext[STATUSBAR_TEXT_WIDTH];

   //char blocknumtext[STATUSBAR_TEXT_WIDTH];
   //char xycoordstext[STATUSBAR_TEXT_WIDTH];

   statusbar_set_geometry_parts();


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

}  /* close updateGeometryStatusBar() */





void
updateAnalysisStatusBar(void) {

  /* This is ill-advised, and is only a kludge until 
   * status text size can be computed.
   */
   char numblocktext[STATUSBAR_TEXT_WIDTH];
   char elapsedtimetext[STATUSBAR_TEXT_WIDTH];
   char currtimesteptext[STATUSBAR_TEXT_WIDTH];
   char oc_contactext[STATUSBAR_TEXT_WIDTH];
   char rcondtext[STATUSBAR_TEXT_WIDTH];


   //gg->numtimesteps = AData->nTimeSteps;
  /* FIXME: Change gg->timestep to gg->delta_t */
   //gg->timestep = AData->currTimeStep;
   //g->currenttime = ad->currentTime;

   //sprintf(numblocktext,"NB: %d", geomdata->nBlocks);
   //sprintf(numblocktext,"NB: %d", numblocks);

   //sprintf(elapsedtimetext,"ET: %.4f", ad->currentTime); // /*g->currenttime*/);
   //sprintf(elapsedtimetext,"ET: %.4f", ad->elapsedTime); // /*g->currenttime*/);
   //sprintf(currtimesteptext,"TS: %d/%d",ad->currTimeStep,ad->nTimeSteps);
   

   //sprintf(oc_contactext,"OC: %d", g->openclosecount);
   sprintf(rcondtext,"|1|: %.4f", 0.0);

   SendMessage(anastatus,SB_SETTEXT,(WPARAM)0,(LPARAM)numblocktext);
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)1,(LPARAM)elapsedtimetext);
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)2,(LPARAM)currtimesteptext);
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)3,(LPARAM)oc_contactext);
  /* Progress bar in slot 4 */
   SendMessage(anastatus,SB_SETTEXT,(WPARAM)5,(LPARAM)rcondtext);

   //redoff = LoadImage(hInst,MAKEINTRESOURCE(ICON_REDLEDOFF),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
   //SendMessage(anastatus,SB_SETICON,9,(LPARAM)redoff);

   //ShowWindow(readystatus, SW_HIDE);
   //ShowWindow(geomstatus, SW_HIDE);

   ShowWindow(statusbar, SW_HIDE);

  /*
   if(dda_get_statusbarvis(dda))
   */
      ShowWindow(anastatus, SW_MAXIMIZE);

}  /* close updateAnalysisStatusBar() */



void
statusbar_init(HWND hwMain) {

  /* FIXME: This needs to be just one control ID for 
   * one status bar, and it should be set from the 
   * appropriate value in the resource.h file.
   */
   UINT DDA_R_STATUSBAR = 0;
   UINT DDA_G_STATUSBAR = 1;
   UINT DDA_A_STATUSBAR = 2;
   UINT DDA_STATUSBAR   = 3;

   HINSTANCE hInst;
   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);


   g_hIcon0 = LoadImage(hInst,MAKEINTRESOURCE(ICON_GREENLEDON),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
   g_hIcon1 = LoadImage(hInst,MAKEINTRESOURCE(ICON_GREENLEDOFF),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);


   statusbar   = CreateStatusWindow(WS_CHILD | WS_MINIMIZE | WS_CLIPCHILDREN,
                                    "", hwMain, DDA_STATUSBAR);

  /* Create this initially invisible, the use the view menu to 
   * toggle whether or not it shows itself.
   */
   /*
   readystatus = CreateStatusWindow(WS_CHILD | WS_MINIMIZE | WS_CLIPCHILDREN,
                                    "", hwMain, DDA_R_STATUSBAR);
*/


  /* These field stops need to be handled with a finite state
   * machine after font metric handling is implemented.  The 
   * (x,y) coordinates field can be computed using domain 
   * scaling width g0.
   */
  /*
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
   */

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


   statusbar_set_ready_parts();
   SendMessage(statusbar, SB_SETTEXT,(WPARAM)(SBT_NOBORDERS),(LPARAM)"Ready");

}


#ifdef __cplusplus
}
#endif