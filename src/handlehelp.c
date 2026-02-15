

/* FIXME: There is a bug in here.  If the HTML Help is not
 * used initially, then it will not launch subsequently.
 * This may be related to improper callback returns in
 * wmcommand switchyard.
 * Another possibility is that it can't find the help
 * directory.  Since I am not tracking the return from
 * HTMLHelp(), hard to say...
 */
#include <windows.h>
#include "resource.h"
#include <htmlhelp.h>
#include <winmain.h>
#include "win32utils.h"


void
handleHtmlHelp(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
   HH_FTS_QUERY q ;

   //exit(0);  // always seems to fire.

   switch(wParam)
   {
      case DDAHELP_INDEX:
		   HtmlHelp(hwMain, "ddahelp\\ddahelp.chm", HH_DISPLAY_INDEX, (DWORD)"geometry");
	  		break;

      case DDAHELP_TUTORIAL:
		   HtmlHelp(hwMain, "ddahelp\\ddahelp.chm::/ddaintro.html", HH_DISPLAY_TOPIC, (DWORD)"geometry");
	  		break;

      case DDAHELP_SEARCH:
		   //HtmlHelp(hwMain, "ddahelp\\ddahelp.chm", HH_DISPLAY_SEARCH, 0L );
         if(HtmlHelp(hwMain,"ddahelp\\ddahelp.chm",HH_DISPLAY_SEARCH,(DWORD)&q))
	  		   break;
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
            break;
            //return 0;
         }
#endif

      case DDAHELP_GEOMETRY:
         HtmlHelp(NULL, "ddahelp\\ddahelp.chm::/ddageometry.html", HH_DISPLAY_TOPIC, 0);
         break;

      case DDAHELP_ANALYSIS:
         HtmlHelp(NULL, "ddahelp\\ddahelp.chm::/analysis.html#analysismenu", HH_DISPLAY_TOPIC, 0);
         break;

	  	case DDAHELP_OVERVIEW:
        /* If this call doesn't work, make the path to the compiled
         * help file is correct.
         */
         if (HtmlHelp(NULL, "ddahelp\\ddahelp.chm", HH_DISPLAY_TOPIC, 0))
            break;
#if _DEBUG
         else
         {
            showlasterror(GetLastError());
            break;
            //return 0;
         }
#endif


		case MAIN_ABOUT:
         handleMainAbout(hwMain);
	  		break;
   }

} /* close handleHtmlHelp() */



