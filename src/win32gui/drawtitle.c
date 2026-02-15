/*
 * drawtitle.c
 *
 * Removes functionality from the message handling
 * switch statement.  Helps to clean up the code a
 * bit.
 */

#define STRICT
#include <windows.h>




/**
 * FIXME: Change the name of this function to
 * "displayImage" or something.  Then it can be invoked
 * from which ever functions need it.
 */
void
drawTitle(HWND hwMain, HDC hdc, HINSTANCE ghInstance)
{
   HDC hdcMem;
   int nWidth;
   int nHeight;
   HBITMAP hBitmap;
   BITMAP bm;
   RECT rectClient;
   HINSTANCE hInst;

   hInst = (HINSTANCE) GetWindowLong(hwMain, GWL_HINSTANCE);

   GetClientRect( hwMain , &rectClient );
			nWidth = rectClient.right;    // rectClient.left is 0
			nHeight = rectClient.bottom;  // rectClient.top is 0

			rectClient.left += (nWidth / 10); // 10% margin left/right
			rectClient.top  += (nHeight / 5);  // 20% margin top/bottom
			rectClient.right -= (nWidth / 5);
			rectClient.bottom -= (long)(nHeight / 2.5);

	//if(hBitmap = LoadBitmap(ghInstance, "TITLE")) {
	if(hBitmap = LoadBitmap(hInst, "TITLE"))
   {
	   hdcMem = CreateCompatibleDC(hdc);
		SelectObject(hdcMem, hBitmap);
		GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
      //sprintf(mess,"bitmap size: %d", bm.bmWidth*bm.bmHeight);
      //MessageBox( hwMain, mess, "Rect size", MB_ICONINFORMATION );
		nWidth = bm.bmWidth;
		nHeight = bm.bmHeight;
		StretchBlt(hdc, rectClient.left, rectClient.top, rectClient.right, rectClient.bottom, hdcMem, 0, 0, nWidth, nHeight, SRCCOPY);
		ReleaseDC(hwMain, hdc);
	   DeleteDC(hdcMem);
		DeleteObject(hBitmap);
   } /* end if */

}  /* Close drawTitle()  */



