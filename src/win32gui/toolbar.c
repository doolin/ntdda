

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



struct _toolbar {
   int state;
};






static void
toolbar_send_message(HWND hToolBar, unsigned int button, unsigned short state) {

   SendMessage(hToolBar, TB_SETSTATE, button, MAKELPARAM(state,0));
}




void
toolbar_set_state(HWND hToolBar, unsigned int state) {


   switch (state) {
   
      case (GEOM_STATE | FINISHED):
         toolbar_send_message(hToolBar,GEOM_APPLY, TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_BROWSE,TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_NEW,   TBSTATE_ENABLED);
      break;

      case (ANA_STATE | RUNNING):
         toolbar_send_message(hToolBar,ANAL_ABORT, TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,GEOM_APPLY, 0);
         toolbar_send_message(hToolBar,GEOM_NEW,   0);
         toolbar_send_message(hToolBar,GEOM_BROWSE,0);
         toolbar_send_message(hToolBar,ANAL_BROWSE,0);
         toolbar_send_message(hToolBar,ANAL_NEW,   0);
      break;

      case (ANA_STATE | FINISHED):
         toolbar_send_message(hToolBar,GEOM_APPLY, TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_BROWSE,TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,GEOM_NEW,   TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,GEOM_BROWSE,TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_NEW,   TBSTATE_ENABLED);
         toolbar_send_message(hToolBar,ANAL_ABORT, 0);
      break;

      default:
      break;
   }



}




void 
toolbar_init(HWND hwMain,HWND * hToolBar) {

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
    //{6, TOOLBAR_ZOOMIN,  TBSTATE_ENABLED, TBSTYLE_CHECK,  0,   0},
    //{7, TOOLBAR_ZOOMOUT, TBSTATE_ENABLED, TBSTYLE_CHECK,  0,   0},
      {6, TOOLBAR_PRINT,   0,               TBSTYLE_BUTTON, 0,   0}
    //{0, 0,               TBSTATE_ENABLED, TBSTYLE_SEP,    0L, -1},
    //{9, TOOLBAR_PRINT,   0,               TBSTYLE_BUTTON, 0,   0}

   };

#define NUMIMAGES 18
#define IMAGEWIDTH 16
#define IMAGEHEIGHT 16
#define BUTTONWIDTH 0
#define BUTTONHEIGHT 0

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

  *hToolBar = CreateToolbarEx(hwMain, 
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

}  



#ifdef __cplusplus
}
#endif



