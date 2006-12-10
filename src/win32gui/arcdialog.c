// Added by Roozbeh
#define STRICT
#include<windows.h>
#include"resource.h"
#include "drawdialog.h"
#include"math.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


extern addJoint (hDlg);

// Added by Roozbeh to Draw a Arc using dialog box

// Refactor all these messages to call functions.
// Feel free to put them into new files.  It will 
// make it easier to maintain in the future.
BOOL CALLBACK
ArcDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  double r0, u, v, x1, x2, x3, y1, y2, y3, x0, y0, deg;
  double xc, yc, xs, ys, xe, ye;
  int i, se = 20;

  extern POINT ptBegin, ptNew, ptOld;
  extern int type;
  extern double ScaleX, ScaleY;
  extern int maxSize;
  extern JOINTLIST *jointlist;

  switch (message) {
  case WM_INITDIALOG:

    SetDlgItemInt (hDlg, IDC_XC, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_YC, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_XS, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_YS, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_XE, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_YE, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_SEG, 20, FALSE);
    SetDlgItemInt (hDlg, IDC_DEG, 180, FALSE);
    SetDlgItemInt (hDlg, IDC_TYPE, 1, FALSE);

    return TRUE;

  case WM_COMMAND:
    switch (LOWORD (wParam)) {
    case IDOK:

      xc = GetDlgItemInt (hDlg, IDC_XC, NULL, TRUE);
      yc = GetDlgItemInt (hDlg, IDC_YC, NULL, TRUE);
      xs = GetDlgItemInt (hDlg, IDC_XS, NULL, TRUE);
      ys = GetDlgItemInt (hDlg, IDC_YS, NULL, TRUE);
      xe = GetDlgItemInt (hDlg, IDC_XE, NULL, TRUE);
      ye = GetDlgItemInt (hDlg, IDC_YE, NULL, TRUE);
      se = GetDlgItemInt (hDlg, IDC_SEG, NULL, TRUE);
      deg = GetDlgItemInt (hDlg, IDC_DEG, NULL, TRUE);
      type = GetDlgItemInt (hDlg, IDC_TYPE, NULL, TRUE);
      r0 = ((3.1415926535) * deg) / (se * 180);
      x1 = xs;
      y1 = ys;
      x2 = xc;
      y2 = yc;
      x0 = x2;
      y0 = y2;
      x3 = xe;
      y3 = ye;
      for (i = 1; i < se; i++) {
        u = ((x1 - x0) * (cos (r0) - 1)) - ((y1 - y0) * sin (r0));
        v = ((x1 - x0) * sin (r0)) + ((y1 - y0) * (cos (r0) - 1));

        x2 = x1 + u;
        y2 = y1 + v;
        ptBegin.x = (long) floor (x1 / (ScaleX / maxSize));
        ptBegin.y = (long) floor (y1 / (ScaleY / maxSize));
        ptNew.x = (long) floor (x2 / (ScaleX / maxSize));
        ptNew.y = (long) floor (y2 / (ScaleY / maxSize));
        addJoint (hDlg);
        x1 = x2;
        y1 = y2;
      }
      x2 = x3;
      y2 = y3;
      ptBegin.x = (long) floor (x1 / (ScaleX / maxSize));
      ptBegin.y = (long) floor (y1 / (ScaleY / maxSize));
      ptNew.x = (long) floor (x2 / (ScaleX / maxSize));
      ptNew.y = (long) floor (y2 / (ScaleY / maxSize));
      addJoint (hDlg);

      return FALSE;
    case IDCANCEL:
      EndDialog (hDlg, 0);
      return TRUE;
    }
    break;
  }
  return FALSE;
}