

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

//HWND hprogbar;

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

typedef struct _progbar {
   HWND hwnd;
   u_int ulx;   //230,5,117,12,  // ulx, uly, xwidth, ywidth
   u_int uly;// = 5;
   u_int xwidth;// = 117;
   u_int ywidth;// = 12;

} Progbar;

Progbar progbar;

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
progressbar_move(Progbar * pb, u_int ulx, u_int xwidth) {

   /*
BOOL MoveWindow(
  HWND hWnd,      // handle to window
  int X,          // horizontal position
  int Y,          // vertical position
  int nWidth,     // width
  int nHeight,    // height
  BOOL bRepaint   // repaint flag
);
*/

   MoveWindow(pb->hwnd,
              ulx,
              pb->uly,
              xwidth,
              pb->ywidth,
              TRUE);
 
}


void
statusbar_set_analysis_parts() {

   u_int field0,field1,field2,field3,field4,field5,field6;
   u_int field7,field8,field9;
   int offset = 10;

  /* These field stops need to be handled with a finite state
   * machine after font metric handling is implemented.
   */   
   
   field0 = 60;
   field1 = field0 + 90;
   field2 = field1 + 90;
   field3 = field2 + 45;
   field4 = field3 + 145;
   field5 = field4 + 60;
   field6 = field5 + 60;
   field7 = field6 + 60;
   field8 = field7 + 60;
   field9 = -1;


   progressbar_move(&progbar,
                    field3 + offset,
                    field4- (field3 + 2*offset));

   anaparts[0] = field0; 
   anaparts[1] = field1;
   anaparts[2] = field2;
   anaparts[3] = field3;
   anaparts[4] = field4;
   anaparts[5] = field5;
   anaparts[6] = field6;
   anaparts[7] = field7;
   anaparts[8] = field8;
   anaparts[9] = field9;

   SendMessage(statusbar,SB_SETPARTS,(WPARAM)SB_ANAL_PARTS,(LPARAM)anaparts);
}


void
statusbar_set_geometry_parts() {

   u_int field0,field1,field2,field3,field4,field5,field6;

  /* These field stops need to be handled with a finite state
   * machine after font metric handling is implemented.  The 
   * (x,y) coordinates field can be computed using domain 
   * scaling width g0.
   */
   field0 = 90;
   field1 = field0 + 90;
   field2 = field1 + 60;
   field3 = field2 + 90;
   field4 = field3 + 90;
   field5 = field4 + 120;
   field6 = -1;

   geomparts[0] = field0;
   geomparts[1] = field1;
   geomparts[2] = field2;
   geomparts[3] = field3;
   geomparts[4] = field4;
   geomparts[5] = field5;  // This is the (x,y) field
   geomparts[6] = field6;

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
         //ShowWindow(hprogbar, SW_HIDE);
         ShowWindow(progbar.hwnd, SW_HIDE);
      break;

      case (ANA_STATE | RUNNING):
         statusbar_set_analysis_parts();
         //ShowWindow(hprogbar, SW_SHOWNORMAL);
         ShowWindow(progbar.hwnd, SW_SHOWNORMAL);
      break;

      case (ANA_STATE | FINISHED):
      break;

      default:
      break;
   }

}

void 
statusbar_set_progbar_range(unsigned short value) {

   SendMessage(progbar.hwnd,PBM_SETRANGE,(WPARAM)0,MAKELPARAM(0,value));
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
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)4,(LPARAM)NULL);
   SendMessage(statusbar,SB_SETTEXT,(WPARAM)5,(LPARAM)rcondtext);

   //redoff = LoadImage(hInst,MAKEINTRESOURCE(ICON_REDLEDOFF),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

}  


void
statusbar_update_progbar(unsigned int wparam) {

   SendMessage(progbar.hwnd,PBM_SETPOS,(WPARAM)wparam,(LPARAM)0);
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
   //u_int ulx;   //230,5,117,12,  // ulx, uly, xwidth, ywidth
   progbar.uly = 5;
   progbar.xwidth = 117;
   progbar.ywidth = 12;
   progbar.hwnd = CreateWindow(PROGRESS_CLASS, 
                               "Prog bar", 
                               WS_CHILD|PBS_SMOOTH,
                               230,
                               progbar.uly,
                               progbar.xwidth,
                               progbar.ywidth,
                               statusbar,
                               NULL,
                               hInst,
                               NULL);
   ShowWindow(progbar.hwnd, SW_HIDE);

   statusbar_set_visibility(1);
   statusbar_set_ready_parts();
   SendMessage(statusbar, SB_SETTEXT,(WPARAM)(SBT_NOBORDERS),(LPARAM)"Ready");
}


#ifdef __cplusplus
}
#endif