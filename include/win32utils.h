
/*
 * win32utils.h
 *
 * This should eventually be built as a dll for using with 
 * debugging windows apps, etc.
 */


#ifndef _WIN32UTILS_H_
#define _WIN32UTILS_H_


#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <windowsx.h>


/* Public, exportable utility functions. */
void showlasterror(DWORD lasterror);
int findMenuItemPosition(HMENU hsubmenu, UINT ID);
int findSubMenuPosition(HMENU hmenu, UINT ID);
void displayWindowRect(HWND hwMain, WPARAM wParam, LPARAM lParam);
void displayRect(HWND hwnd, RECT r);
void showWin32Error(char * calledfrom);


#endif /* STRICT */