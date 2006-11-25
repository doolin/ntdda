#define STRICT
#include<windows.h>
#include"resource.h"
#include "drawdialog.h"
//#include "joint.h"
#include"math.h"

//#include "winmain.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


extern addJoint (hDlg);

// Added by Roozbeh to Draw a Joint using dialog box
BOOL CALLBACK
JointDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  extern POINT ptBegin, ptNew, ptOld;
  extern int type;
  extern double ScaleX, ScaleY;
  extern int maxSize;
  extern JOINTLIST *jointlist;

  switch (message) {
  case WM_INITDIALOG:

    SetDlgItemInt (hDlg, IDC_X1, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_Y1, 0, FALSE);
    SetDlgItemInt (hDlg, IDC_X2, 10, FALSE);
    SetDlgItemInt (hDlg, IDC_Y2, 10, FALSE);
    SetDlgItemInt (hDlg, IDC_TYPE, 1, FALSE);

    return TRUE;

  case WM_COMMAND:
    switch (LOWORD (wParam)) {

    case IDOK:

      // Make this a function call with a descriptive name.
      ptBegin.x = GetDlgItemInt (hDlg, IDC_X1, NULL, TRUE);
      ptBegin.y = GetDlgItemInt (hDlg, IDC_Y1, NULL, TRUE);
      ptNew.x = GetDlgItemInt (hDlg, IDC_X2, NULL, TRUE);
      ptNew.y = GetDlgItemInt (hDlg, IDC_Y2, NULL, TRUE);
      type = GetDlgItemInt (hDlg, IDC_TYPE, NULL, TRUE);
      ptBegin.x = (long) floor (ptBegin.x / (ScaleX / maxSize));
      ptBegin.y = (long) floor (ptBegin.y / (ScaleY / maxSize));
      ptNew.x = (long) floor (ptNew.x / (ScaleX / maxSize));
      ptNew.y = (long) floor (ptNew.y / (ScaleY / maxSize));

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