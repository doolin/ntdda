/*
 * header for a testing list view. 
 */

#ifndef STRICT 
#define STRICT
#endif

#include <windows.h>
#include <windowsx.h>

void createListView(HWND hwMain);
HWND createHeaderWindow(HWND hwMain);
BOOL insertItem(HWND hdrwnd, LPSTR lpsz);
