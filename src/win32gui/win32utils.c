/*
 * win32utils.c
 * 
 */

#include "win32utils.h"
#include <stdio.h>



// Comment for illustrating cvs email.
int
findMenuItemPosition(HMENU hsubmenu, UINT ID)
{
   int i;
   MENUITEMINFO mii;
   //char mess[80];

   mii.cbSize = sizeof(MENUITEMINFO);
   mii.fMask = MIIM_ID;

   for (i=0; i<GetMenuItemCount(hsubmenu); i++)
   {
      GetMenuItemInfo(hsubmenu, i,TRUE,&mii);
      //sprintf(mess,"mii.wID:  %d, ID: %d", mii.wID, ID);
      //MessageBox(hmain, mess,NULL,MB_OK);
      if (mii.wID == ID)
         return i;
   }

   return -1;  // oops

}  /* close findMenuItemPosition()  */


/* Loop over the menus looking for the popup
 * that handles the current ID.  From Rector
 * and Newcomer, page 879.
 */
int 
findSubMenuPosition(HMENU hmenu, UINT ID)
{
   int i;
   HMENU hsubmenu;

   for (i=0; i<GetMenuItemCount(hmenu); i++)
   {  
      hsubmenu = GetSubMenu(hmenu, i);
      if (hsubmenu == NULL)
         continue;  // No popup
      if (GetMenuState(hsubmenu,ID,MF_BYCOMMAND) == 0xFFFFFFFF)
         continue;  // Not in this menu
      return i;
   }
   return -1;  // oops

}  /* close findSubMenuPosition() */


void
displayWindowRect(HWND hwMain, WPARAM wParam, LPARAM lParam)
{
  /* Temporary, prints into messagebox */
   char mess[180];
   RECT r;

   GetClientRect(hwMain,&r);

   sprintf(mess,"top: %d, right %d, bottom: %d, left: %d",
                r.top, r.right, r.bottom, r.left);
   MessageBox(hwMain, mess, "Window Rect",MB_OK);
   

}  /* close displayWindowRect() */



void
displayRect(HWND hwnd, RECT r)
{
  /* Temporary, prints into messagebox */
   char mess[180];

   sprintf(mess,"top: %d, right %d, bottom: %d, left: %d",
                r.top, r.right, r.bottom, r.left);
   MessageBox(hwnd, mess, "Window Rect",MB_OK);
   

}  /* close displayRect() */



void
showlasterror(DWORD lasterror)
{
   LPVOID lpMsgBuf;
   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                 FORMAT_MESSAGE_FROM_SYSTEM | 
                 FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL,
                 lasterror,
                 //GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                 (LPTSTR) &lpMsgBuf,
                 0,
                 NULL);

   // Process any inserts in lpMsgBuf.
   // ...
   // Display the string.
   MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
   // Free the buffer.
   LocalFree( lpMsgBuf );
 
}  /* close showlasterror() */


#if  WIN32_SOCKETS_PROGRAM
void
showWin32Error(char * calledfrom)
{
   LPVOID lpMsgBuf;
   int lasterror;

   lasterror = WSAGetLastError();

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                 FORMAT_MESSAGE_FROM_SYSTEM | 
                 FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL,
                 //GetLastError(),
                 lasterror,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                 (LPTSTR) &lpMsgBuf,
                 0,
                 NULL);  
   MessageBox(NULL,lpMsgBuf,NULL,MB_OK);

}  /* showWin32Error() */
#endif
