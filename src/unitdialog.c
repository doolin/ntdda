


#define STRICT
#include<windows.h>
#include"resource.h"


BOOL CALLBACK
UnitDlgProc (HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
   char scaletext[10];

  	switch (iMessage)
	  {
	    	case WM_INITDIALOG:
		      	CheckRadioButton(hDlg, IDC_ENGLISH, IDC_METRIC, IDC_ENGLISH);
			      strcpy (scaletext,"ft");
	   		return TRUE;

		    case WM_COMMAND:
		   	switch (LOWORD (wParam))
	   		{
			     	case IDC_ENGLISH:
	           //MessageBox(hDlg, "English", "Add", MB_OK);
				        strcpy (scaletext,"ft");
	           //EndDialog (hDlg, 1);
		      	return TRUE;

				     case IDC_METRIC:
	           //MessageBox(hDlg, "Metric", "Add", MB_OK);
		        		strcpy (scaletext,"m");
	           //EndDialog (hDlg, 1);
   		     		return TRUE;

			     	case IDC_OK:
				        EndDialog (hDlg,1);
			     	return TRUE;

				     case IDCANCEL:
				        EndDialog (hDlg,0);
				     return FALSE;
			   }

		    default:
		   	return FALSE;
	  }
} // end UnitDlgProc
