/*
 * This file contains helper functions useful for debugging
 * and code investigation on the windows side.
 */

#include <windows.h>
#include <stdio.h>

void
checkRect(HWND parent, RECT * rect)
{
   char rectstring[100];

   sprintf(rectstring,"Top: %d\nBottom: %d\nLeft: %d\nRight: %d",
       rect->top, rect->bottom, rect->left, rect->right);

   MessageBox(parent, rectstring, "RECT Checker", MB_OK);


}  /* close checkRect() */