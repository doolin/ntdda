

#include <windows.h>
#include <windowsx.h> 
#include "resource.h"
#include <commctrl.h>
#include "toolbar.h"
#include "runstates.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



HWND hToolBar;

static int visibility;

struct _toolbar {
   int state;
};

#if 0
static int
toolbar_toggle_visibility() {

   if (visibility == 1) {
      visibility = 0;
      ShowWindow(hToolBar, SW_HIDE);
   } else {
      visibility = 1;
      ShowWindow(hToolBar, SW_SHOWNORMAL);
   }
}
#endif


int
toolbar_get_visibility() {
   return visibility;
}


void 
toolbar_set_visibility(int v) {
   visibility = v;
   toolbar_show();
}


void
toolbar_show(void) {

   if (visibility == 1) {
      ShowWindow(hToolBar, SW_SHOWNORMAL);
   } else {
      ShowWindow(hToolBar, SW_HIDE);
   }
}


static void
toolbar_send_message(HWND hToolBar, unsigned int button, unsigned short state) {

   SendMessage(hToolBar, TB_SETSTATE, button, MAKELPARAM(state,0));
}


void 
toolbar_resize(void){

   SendMessage(hToolBar,WM_SIZE,0,0);
}

void
toolbar_set_state(unsigned int state) {


   switch (state) {
   
      case (GEOM_STATE | FINISHED):
         toolbar_send_message(hToolBar,GEOM_APPLY, TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_BROWSE,TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_NEW,   TBSTATE_ENABLED);
		 toolbar_send_message(hToolBar,TOOLBAR_ZOOMOUT,   TBSTATE_ENABLED); //Added by Roozbeh
		 toolbar_send_message(hToolBar,TOOLBAR_ZOOMIN,   TBSTATE_ENABLED); //Added by Roozbeh
		 toolbar_send_message(hToolBar,TOOLBAR_ZOOMBACK,   TBSTATE_ENABLED); //Added by Roozbeh
      break;

      case (ANA_STATE | RUNNING):
         toolbar_send_message(hToolBar,ANAL_ABORT, TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,GEOM_APPLY, 0);
         toolbar_send_message(hToolBar,GEOM_NEW,   0);
         toolbar_send_message(hToolBar,GEOM_BROWSE,0);
         toolbar_send_message(hToolBar,ANAL_BROWSE,0);
         toolbar_send_message(hToolBar,ANAL_NEW,   0);
		 toolbar_send_message(hToolBar,TOOLBAR_ZOOMOUT,   0); //Added by Roozbeh
		 toolbar_send_message(hToolBar,TOOLBAR_ZOOMIN,   0); //Added by Roozbeh
		 toolbar_send_message(hToolBar,TOOLBAR_ZOOMBACK,   0); //Added by Roozbeh
      break;

      case (ANA_STATE | FINISHED):
         toolbar_send_message(hToolBar,GEOM_APPLY, TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_BROWSE,TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,GEOM_NEW,   TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,GEOM_BROWSE,TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_NEW,   TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_ABORT, 0);
		 toolbar_send_message(hToolBar,TOOLBAR_ZOOMOUT,   TBSTATE_ENABLED); //Added by Roozbeh
		 toolbar_send_message(hToolBar,TOOLBAR_ZOOMIN,   TBSTATE_ENABLED); //Added by Roozbeh
		 toolbar_send_message(hToolBar,TOOLBAR_ZOOMBACK,   TBSTATE_ENABLED); //Added by Roozbeh

      break;

      default:
      break;
   }



}




void 
toolbar_init(HWND hwMain) {

   HINSTANCE hInst;

   TBBUTTON tbbutton[] = {  
	  /* FIXME:  Second member of array is actually the command. */
	   {0, GEOM_NEW,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,   0},
	   {1, ANAL_NEW,        0,               TBSTYLE_BUTTON, 0,   0},
       {0, 0,               TBSTATE_ENABLED, TBSTYLE_SEP,    0L, -1},
	   {2, GEOM_BROWSE,     TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,   0},
	   {3, ANAL_BROWSE,     0,               TBSTYLE_BUTTON, 0,   0},
	   {4, ANAL_ABORT,      0,               TBSTYLE_BUTTON, 0,   0},
	 //{4, GEOM_NEW,        TBSTATE_ENABLED, TBSTYLE_CHECK,  0,   0},
	   {5, GEOM_APPLY,      0,               TBSTYLE_BUTTON, 0,   0},
       {0, 0,               TBSTATE_ENABLED, TBSTYLE_SEP,    0L, -1},
	   {7, TOOLBAR_ZOOMOUT, 0, TBSTYLE_BUTTON,  0,   0}, //Enabled by Roozbeh
	   {8, TOOLBAR_ZOOMIN,  0, TBSTYLE_BUTTON,  0,   0}, //Enabled by Roozbeh
	   {9, TOOLBAR_ZOOMBACK,  0, TBSTYLE_BUTTON,  0,   0}, //Enabled by Roozbeh
       //{6, TOOLBAR_PRINT,   0,               TBSTYLE_BUTTON, 0,   0}
    //{0, 0,               TBSTATE_ENABLED, TBSTYLE_SEP,    0L, -1},
    //{9, TOOLBAR_PRINT,   0,               TBSTYLE_BUTTON, 0,   0}

   };

#define NUMIMAGES 18
#define IMAGEWIDTH 16
#define IMAGEHEIGHT 16
#define BUTTONWIDTH 0
#define BUTTONHEIGHT 0

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   hToolBar = CreateToolbarEx(hwMain, 
	                           WS_CHILD | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE, 
	                           TOOLBAR_DDA_MAIN,
                              NUMIMAGES,
                              hInst, 
                              TOOLBAR_DDA_MAIN,
                              tbbutton,
                              sizeof(tbbutton)/sizeof(TBBUTTON),
                              BUTTONWIDTH,
                              BUTTONHEIGHT, 
                              IMAGEWIDTH,
                              IMAGEHEIGHT, 
                              sizeof(TBBUTTON));

   toolbar_set_visibility(1);
}  



#ifdef __cplusplus
}
#endif



