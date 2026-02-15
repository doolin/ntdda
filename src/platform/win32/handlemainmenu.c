/*
 * handlemainmenu.c
 *
 * All of the main menu code gets handled here in
 * response to a WM_INITMENU message.
 */

#include "winmain.h"
#include "win32utils.h"
#include "resource.h"
#include "menu.h"
#include "options.h"
#include "runstates.h"



/* FIXME: Get rid of the "A" suffix later
 */
void
updateMainMenuA(HWND hwMain, int state)
{
   HMENU hMainMenu, hSubMenu;
   int submenuposition;

   hMainMenu = GetMenu(hwMain);

   switch(state) {

      case READY_STATE:
         //MessageBox(NULL,"Ready menu state",NULL,MB_OK);
         break;

      case GEOM_STATE:
         //exit(0);  // This case never reached.
         submenuposition = findSubMenuPosition(hMainMenu, GEOM_APPLY);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
			EnableMenuItem(hSubMenu, GEOM_EDITTXTD, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, GEOM_EDITTXTN, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, GEOM_EDIT_VE, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, GEOM_APPLY, MF_BYCOMMAND | MF_ENABLED);
         break;

      case (GEOM_STATE | RUNNING):
         submenuposition = findSubMenuPosition(hMainMenu, GEOM_CANCEL);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
			EnableMenuItem(hSubMenu, GEOM_CANCEL, MF_BYCOMMAND | MF_ENABLED);

         submenuposition = findSubMenuPosition(hMainMenu, ANAL_RUN);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
		   EnableMenuItem(hSubMenu, ANAL_RUN, MF_BYCOMMAND | MF_GRAYED);
		   EnableMenuItem(hSubMenu, ANAL_NEW, MF_BYCOMMAND | MF_GRAYED);
	      EnableMenuItem(hSubMenu, ANAL_BROWSE, MF_BYCOMMAND | MF_GRAYED);

         submenuposition = findSubMenuPosition(hMainMenu, RES_REPLAY);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
			EnableMenuItem(hSubMenu, RES_REPLAY, MF_BYCOMMAND | MF_GRAYED);
         break;

      case (GEOM_STATE | ABORTED):
         submenuposition = findSubMenuPosition(hMainMenu, GEOM_APPLY);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
			EnableMenuItem(hSubMenu, GEOM_APPLY, MF_BYCOMMAND | MF_GRAYED);
			//EnableMenuItem(hMainMenu, analysismenu, MF_BYPOSITION | MF_GRAYED);
         break;

      case (GEOM_STATE | FINISHED):
         submenuposition = findSubMenuPosition(hMainMenu, GEOM_NEW);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
		   EnableMenuItem(hSubMenu, GEOM_CANCEL, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hSubMenu, GEOM_VIEWGRF, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, GEOM_PGRF, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, GEOM_EDITTXTD, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, GEOM_EDITTXTN, MF_BYCOMMAND | MF_ENABLED);

         submenuposition = findSubMenuPosition(hMainMenu, ANAL_NEW);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
			EnableMenuItem(hSubMenu, ANAL_NEW, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, ANAL_BROWSE, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, ANAL_EDITBLX, MF_BYCOMMAND | MF_ENABLED);
         break;

      case ANA_STATE:
         break;

      case (ANA_STATE | READY_STATE):
         submenuposition = findSubMenuPosition(hMainMenu, ANAL_RUN);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
			EnableMenuItem(hSubMenu, ANAL_EDITPARAMD, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, ANAL_EDITPARAMN, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, ANAL_RUN, MF_BYCOMMAND | MF_ENABLED);
         break;

      case (ANA_STATE | FINISHED):
         submenuposition = findSubMenuPosition(hMainMenu, RES_REPLAY);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
			EnableMenuItem(hSubMenu, RES_REPLAY, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, RES_VGRF, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, RES_VTXT, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, RES_PGRF, MF_BYCOMMAND | MF_ENABLED);

         submenuposition = findSubMenuPosition(hMainMenu, ANAL_PAUSE);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
         EnableMenuItem(hSubMenu, ANAL_PAUSE, MF_BYCOMMAND | MF_GRAYED);
	 	   EnableMenuItem(hSubMenu, ANAL_ABORT, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hSubMenu, ANAL_EDITPARAMD, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hSubMenu, ANAL_EDITPARAMN, MF_BYCOMMAND | MF_ENABLED);
         break;

      case (ANA_STATE | RUNNING):
         submenuposition = findSubMenuPosition(hMainMenu, ANAL_ABORT);
         hSubMenu = GetSubMenu(hMainMenu, submenuposition);
  	      EnableMenuItem(hSubMenu, ANAL_PAUSE, MF_BYCOMMAND | MF_ENABLED);
 	      EnableMenuItem(hSubMenu, ANAL_ABORT, MF_BYCOMMAND | MF_ENABLED);
         break;

      default:
         //MessageBox(NULL,"Bad menu state",NULL,MB_OK);
         break;
   }

}  /* close updateMainMenu() */


/* If the user wants to look at spy plot of the matrix,
 * then we have to handle the appropriate menu entry.
 * Most of this is fairly well documented in the win32
 * docs that shipped with msvc++ v6.
 */
void
handleOptionsMenu(HWND hwMain, WPARAM wParam, Options * o)
{
   int submenuposition;
   MENUITEMINFO mii;
   HMENU hMainMenu, hSubMenu;

   hMainMenu = GetMenu(hwMain);
   submenuposition = findSubMenuPosition(hMainMenu, OPTIONS_TIMING);
   hSubMenu = GetSubMenu(hMainMenu, submenuposition);

   mii.cbSize = sizeof(MENUITEMINFO);
   mii.fMask = MIIM_STATE;
   mii.hSubMenu = hSubMenu;
   mii.dwItemData = OPTIONS_SPYPLOTS;
   mii.hbmpChecked = NULL;  // Use default bitmap check mark
   mii.hbmpUnchecked = NULL; // Use no bit map at all


   switch (wParam)
   {
      case OPTIONS_SPYPLOTS:
        /* Check the state of the menu item.  If we can't find the
         * menu item for some reason, then throw an error in the
         * debug versin, do nothing in the release version.
         */
         if(GetMenuItemInfo(hSubMenu,OPTIONS_SPYPLOTS, FALSE, &mii))
         {
            if(mii.fState == MFS_CHECKED)
            {
               o->spyplots = 0;
               ModifyMenu(hSubMenu, OPTIONS_SPYPLOTS, MF_BYCOMMAND | MF_UNCHECKED,OPTIONS_SPYPLOTS,"&Spy plots");
            }
            else
            {
               o->spyplots = 1;
               ModifyMenu(hSubMenu, OPTIONS_SPYPLOTS, MF_BYCOMMAND | MF_CHECKED,OPTIONS_SPYPLOTS,"&Spy plots");
            }
         }
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
         }
#endif
         break;

      case OPTIONS_TIMING:
         mii.dwItemData = OPTIONS_TIMING;
         if(GetMenuItemInfo(hSubMenu,OPTIONS_TIMING, FALSE, &mii))
         {
            if(mii.fState == MFS_CHECKED)
            {
               o->timing = 0;
               ModifyMenu(hSubMenu, OPTIONS_TIMING, MF_BYCOMMAND | MF_UNCHECKED,OPTIONS_TIMING,"&Timing");
            }
            else
            {
               o->timing = 1;
               ModifyMenu(hSubMenu, OPTIONS_TIMING, MF_BYCOMMAND | MF_CHECKED,OPTIONS_TIMING,"&Timing");
            }
         }
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
         }
#endif
         break;

      case OPTIONS_PARAMETERS:
         mii.dwItemData = OPTIONS_PARAMETERS;
         if(GetMenuItemInfo(hSubMenu,OPTIONS_PARAMETERS, FALSE, &mii))
         {
            if(mii.fState == MFS_CHECKED)
            {
               o->parameters = 0;
               ModifyMenu(hSubMenu, OPTIONS_PARAMETERS, MF_BYCOMMAND | MF_UNCHECKED,OPTIONS_PARAMETERS,"&Parameters");
            }
            else
            {
               o->parameters = 1;
               ModifyMenu(hSubMenu, OPTIONS_PARAMETERS, MF_BYCOMMAND | MF_CHECKED,OPTIONS_PARAMETERS,"&Parameters");
            }
         }
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
         }
#endif
         break;


      case OPTIONS_BLOCKAREAS:
         mii.dwItemData = OPTIONS_BLOCKAREAS;
         if(GetMenuItemInfo(hSubMenu,OPTIONS_BLOCKAREAS, FALSE, &mii))
         {
            if(mii.fState == MFS_CHECKED)
            {
               o->blockareas = 0;
               ModifyMenu(hSubMenu, OPTIONS_BLOCKAREAS, MF_BYCOMMAND | MF_UNCHECKED,OPTIONS_BLOCKAREAS,"&Block areas");
            }
            else
            {
               o->blockareas = 1;
               ModifyMenu(hSubMenu, OPTIONS_BLOCKAREAS, MF_BYCOMMAND | MF_CHECKED,OPTIONS_BLOCKAREAS,"&Block areas");
            }
         }
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
         }
#endif
         break;


      case OPTIONS_MEASUREDPOINTS:
         mii.dwItemData = OPTIONS_MEASUREDPOINTS;
         if(GetMenuItemInfo(hSubMenu,OPTIONS_MEASUREDPOINTS, FALSE, &mii))
         {
            if(mii.fState == MFS_CHECKED)
            {
               o->measuredpoints = 0;
               ModifyMenu(hSubMenu, OPTIONS_MEASUREDPOINTS, MF_BYCOMMAND | MF_UNCHECKED,OPTIONS_MEASUREDPOINTS,"&Measured points");
            }
            else
            {
               o->measuredpoints = 1;
               ModifyMenu(hSubMenu, OPTIONS_MEASUREDPOINTS, MF_BYCOMMAND | MF_CHECKED,OPTIONS_MEASUREDPOINTS,"&Measured points");
            }
         }
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
         }
#endif
         break;



      case OPTIONS_FLAGSTATES:
         mii.dwItemData = OPTIONS_FLAGSTATES;
         if(GetMenuItemInfo(hSubMenu,OPTIONS_FLAGSTATES, FALSE, &mii))
         {
            if(mii.fState == MFS_CHECKED)
            {
               o->flagstates = 0;
               ModifyMenu(hSubMenu, OPTIONS_FLAGSTATES, MF_BYCOMMAND | MF_UNCHECKED,OPTIONS_FLAGSTATES,"&Flag states");
            }
            else
            {
               o->flagstates = 1;
               ModifyMenu(hSubMenu, OPTIONS_FLAGSTATES, MF_BYCOMMAND | MF_CHECKED,OPTIONS_FLAGSTATES,"&Flag states");
            }
         }
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
         }
#endif
         break;

         /* Showing the original geometry is by default. Move that
          * option elsewhere, or move the analysis options from
          * their own main menu item to a subitem under analysis
          * menu item and make them a popup, and the values set either
          * in the registry or an ini file.
          */
         /*
      case OPTIONS_ORIGGEOM:
         mii.dwItemData = OPTIONS_ORIGGEOM;
         if(GetMenuItemInfo(hSubMenu,OPTIONS_ORIGGEOM, FALSE, &mii))
         {
            if(mii.fState == MFS_CHECKED)
            {
               o->showOrig = 0;
               ModifyMenu(hSubMenu, OPTIONS_ORIGGEOM, MF_BYCOMMAND | MF_UNCHECKED,OPTIONS_ORIGGEOM,"&Original geometry");
            }
            else
            {
               o->showOrig = 1;
               ModifyMenu(hSubMenu, OPTIONS_ORIGGEOM, MF_BYCOMMAND | MF_CHECKED,OPTIONS_ORIGGEOM,"&Original geometry");
            }
         }
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
         }
#endif
         break;
         */

      case OPTIONS_FIXEDPOINTS:
         mii.dwItemData = OPTIONS_FIXEDPOINTS;
         if(GetMenuItemInfo(hSubMenu,OPTIONS_FIXEDPOINTS, FALSE, &mii))
         {
            if(mii.fState == MFS_CHECKED)
            {
               o->fixedpoints = 0;
               ModifyMenu(hSubMenu, OPTIONS_FIXEDPOINTS, MF_BYCOMMAND | MF_UNCHECKED,OPTIONS_FIXEDPOINTS,"Fi&xed points");
            }
            else
            {
               o->fixedpoints = 1;
               ModifyMenu(hSubMenu, OPTIONS_FIXEDPOINTS, MF_BYCOMMAND | MF_CHECKED,OPTIONS_FIXEDPOINTS,"Fi&xed points");
            }
         }
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
         }
#endif
         break;

         default:
            return;
   }  /* close switch on option submenu items */

}  /* close handleOptionSpyplot() */
