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

BOOL CALLBACK
BlockDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  double xl, yl, xu, yu;

  extern POINT ptBegin, ptNew, ptOld;
  extern int type;
  extern double ScaleX, ScaleY;
  extern int maxSize;
  extern JOINTLIST *jointlist;

  switch (message) {
  case WM_INITDIALOG:

    SetDlgItemInt (hDlg, IDC_XL, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_YL, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_XU, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_YU, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_TYPE, 1, FALSE);

    return TRUE;

  case WM_COMMAND:
    switch (LOWORD (wParam)) {
    case IDOK:

      xl = GetDlgItemInt (hDlg, IDC_XL, NULL, TRUE);
      yl = GetDlgItemInt (hDlg, IDC_YL, NULL, TRUE);
      xu = GetDlgItemInt (hDlg, IDC_XU, NULL, TRUE);
      yu = GetDlgItemInt (hDlg, IDC_YU, NULL, TRUE);
      type = GetDlgItemInt (hDlg, IDC_TYPE, NULL, TRUE);

      ptBegin.x = (long) floor (xl / (ScaleX / maxSize));
      ptBegin.y = (long) floor (yl / (ScaleY / maxSize));
      ptNew.x = (long) floor (xu / (ScaleX / maxSize));
      ptNew.y = (long) floor (yl / (ScaleY / maxSize));
      addJoint (hDlg);

      ptBegin.x = (long) floor (xu / (ScaleX / maxSize));
      ptBegin.y = (long) floor (yl / (ScaleY / maxSize));
      ptNew.x = (long) floor (xu / (ScaleX / maxSize));
      ptNew.y = (long) floor (yu / (ScaleY / maxSize));
      addJoint (hDlg);

      ptBegin.x = (long) floor (xu / (ScaleX / maxSize));
      ptBegin.y = (long) floor (yu / (ScaleY / maxSize));
      ptNew.x = (long) floor (xl / (ScaleX / maxSize));
      ptNew.y = (long) floor (yu / (ScaleY / maxSize));
      addJoint (hDlg);

      ptBegin.x = (long) floor (xl / (ScaleX / maxSize));
      ptBegin.y = (long) floor (yu / (ScaleY / maxSize));
      ptNew.x = (long) floor (xl / (ScaleX / maxSize));
      ptNew.y = (long) floor (yl / (ScaleY / maxSize));
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