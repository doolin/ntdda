/*
 */

#define STRICT

#include <windows.h>
#include <prsht.h>      // includes the property sheet functionality
#include <assert.h>
#include "resource.h"
#include "analysisdata.h"




BOOL APIENTRY AFlagsProc(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY JointMatsProc(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY BlockMatsProc(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY LPointsProc(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY TimeStepProc(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY ContactProc(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY BoltMatsProc(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY GravityProc(HWND,UINT,WPARAM,LPARAM);
BOOL APIENTRY AnaSeismicProc(HWND,UINT,WPARAM,LPARAM);



void FillInPropertyPage( PROPSHEETPAGE* , int, LPSTR, DLGPROC, UINT, LPARAM, HINSTANCE);


int CreateTestPropSheet(HWND hwndOwner, Analysisdata * ad)
{
    
   PROPSHEETPAGE psp[9];
   PROPSHEETHEADER psh;
   LPARAM lparam;
   Analysisdata * adnew = NULL;// = cloneAnalysisData(ad->this);
   //int i;
   HINSTANCE hInst = (HINSTANCE)GetWindowLong(hwndOwner,GWL_HINSTANCE);
   //HWND handle;

   //for (i=0;i<sizeof(psp)/sizeof(PROPSHEETPAGE);i++)
   //   psp[i].hInstance = hInst;


  /* Right about in here I need to clone the analysisdata that 
   * is passed in so that "Cancel" will restore the original 
   * data.
   */
  /* Now, turn it into an LPARAM for each of the property sheet
   * pages to use:
   */
   lparam = (LPARAM)adnew;

	FillInPropertyPage( &psp[0], IDD_JOINTPROPS, TEXT("&Joints"), JointMatsProc, ICON_JOINTTYPE, lparam, hInst);
	FillInPropertyPage( &psp[1], IDD_BLOCKPROPS, TEXT("Blocks"), BlockMatsProc, ICON_BLOCKMATERIALS, lparam, hInst);
	FillInPropertyPage( &psp[2], IDD_AFLAGS, TEXT("Flags"), AFlagsProc, ICON_FLAGS, lparam, hInst);
	FillInPropertyPage( &psp[3], IDD_LOADPOINTS, TEXT("Load Points"), LPointsProc, ICON_LOADPOINT, lparam, hInst);
	FillInPropertyPage( &psp[4], IDD_TIMESTEP, TEXT("Time steps"), TimeStepProc, ICON_STOPWATCH, lparam, hInst);
	FillInPropertyPage( &psp[5], IDD_SEISMIC, TEXT("Contact"), ContactProc, ICON_CONTACT, lparam, hInst);
	FillInPropertyPage( &psp[6], IDD_BOLTPROPERTIES, TEXT("Bolts"), BoltMatsProc, ICON_ROCKBOLT, lparam, hInst);
	FillInPropertyPage( &psp[7], IDD_GRAVITY, TEXT("Gravity"), GravityProc, ICON_GRAVITY, lparam, hInst);
	FillInPropertyPage( &psp[8], IDD_SEISMIC, TEXT("Seismic"), AnaSeismicProc, ICON_EARTHQUAKE, lparam, hInst);
    
   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE   | 
                 PSH_HASHELP         |
                 PSH_USEHICON;
   psh.hwndParent = hwndOwner;
   psh.hInstance = hInst;
   psh.pszCaption = (LPSTR) TEXT("Analysis Data");
   psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
   psh.nStartPage = 2;
   psh.ppsp = (LPCPROPSHEETPAGE) &psp;
   psh.hIcon = LoadImage(hInst,  
                           MAKEINTRESOURCE(ICON_DDAMAIN), 
                           IMAGE_ICON, 
                           LR_DEFAULTSIZE,
                           LR_DEFAULTSIZE,
                           LR_DEFAULTCOLOR);


    PropertySheet(&psh);
    return TRUE;

} 



//
//  FUNCTION: FillInPropertyPage(PROPSHEETPAGE *, int, LPSTR, LPFN) 
//
//  PURPOSE: Fills in the given PROPSHEETPAGE structure 
//
//  COMMENTS:
//      This function fills in a PROPSHEETPAGE structure with the
//      information the system needs to create the page.
// 
void 
FillInPropertyPage( PROPSHEETPAGE* psp, 
                   int idDlg, 
                   LPSTR pszProc, 
                   DLGPROC pfnDlgProc,
                   UINT icon,
                   LPARAM lparam,
                   HINSTANCE hInst)
{
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->dwFlags = PSP_USETITLE | PSP_USEHICON;
    psp->hInstance = hInst;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);
    psp->pszIcon = NULL;
    psp->pfnDlgProc = pfnDlgProc;
    psp->pszTitle = pszProc;
    psp->lParam = lparam;
    psp->hIcon = LoadImage(hInst,  
                           MAKEINTRESOURCE(icon), 
                           IMAGE_ICON, 
                           LR_DEFAULTSIZE,
                           LR_DEFAULTSIZE,
                           LR_DEFAULTCOLOR);

}  /* close FillPropertyPage() */




void
handleNotify(HWND hparent, LPARAM lParam)
{
   
   switch (((NMHDR FAR *) lParam)->code) 
   {
   case PSN_SETACTIVE:
      //MessageBox(NULL,"SETACTIVE","SETACTIVE",MB_OK);
      break;

   case PSN_APPLY:
      {
         PostMessage(hparent,WM_DESTROY,0,0);
      }
      break;

   case PSN_KILLACTIVE:
      //(NULL,"KILLACTIVE","KILLACTIVE",MB_OK);
      //PostMessage(hparent,WM_DESTROY,0,0);
      break;

   case PSN_RESET:
      //MessageBox(NULL,"RESET","RESET",MB_OK);
      PostMessage(hparent,WM_DESTROY,0,0);
      break;
   }
}



BOOL APIENTRY
GravityProc(HWND hDlg, UINT messageID, WPARAM wParam, LPARAM lParam)
{

   switch(messageID)
   {
      case WM_INITDIALOG:
      {
         PROPSHEETPAGE * psp = (PROPSHEETPAGE *)lParam;
         //assert((Analysisdata *)psp->lParam == adglobalfortest);
      }  break;


      case WM_NOTIFY:
         handleNotify(hDlg, lParam);
         break;

      case WM_COMMAND:
      {
         int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
         int wNotifyCode = HIWORD(wParam);      // notification code 

         switch (idEditCtrl)
         {
            case IDC_LOADRESIDUAL:
               if (wNotifyCode == EN_KILLFOCUS)
               {
                  MessageBox(hDlg,"Residual edited",NULL,MB_OK);
               }
               break;
         }
      } break;
   }

return 0;
}  /* close BoltMatsProc() */



BOOL APIENTRY
BoltMatsProc(HWND hDlg, UINT messageID, WPARAM wParam, LPARAM lParam)
{
   switch(messageID)
   {
      case WM_INITDIALOG:
      {
         PROPSHEETPAGE * psp = (PROPSHEETPAGE *)lParam;
         //assert((Analysisdata *)psp->lParam == adglobalfortest);
      }  break;

      case WM_NOTIFY:
         handleNotify(hDlg, lParam);
         break;


      case WM_COMMAND:
      {
         int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
         int wNotifyCode = HIWORD(wParam);      // notification code 

         switch (idEditCtrl)
         {
            case IDC_LOADRESIDUAL:
               if (wNotifyCode == EN_KILLFOCUS)
               {
                  MessageBox(hDlg,"Residual edited",NULL,MB_OK);
               }
               break;
         }
      } break;
   }

return 0;
}  /* close BoltMatsProc() */


BOOL APIENTRY
JointMatsProc(HWND hDlg, UINT messageID, WPARAM wParam, LPARAM lParam)
{
   switch(messageID)
   {
      case WM_INITDIALOG:
      {
         PROPSHEETPAGE * psp = (PROPSHEETPAGE *)lParam;
         //assert((Analysisdata *)psp->lParam == adglobalfortest);
      }  break;

      case WM_NOTIFY:
         handleNotify(hDlg, lParam);
         break;


      case WM_COMMAND:
      {
         int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
         int wNotifyCode = HIWORD(wParam);      // notification code 

         switch (idEditCtrl)
         {
            case IDC_LOADRESIDUAL:
               if (wNotifyCode == EN_KILLFOCUS)
               {
                  MessageBox(hDlg,"Residual edited",NULL,MB_OK);
               }
               break;
         }
      } break;
   }

return 0;
}  /* close JointMatsProc() */



BOOL APIENTRY
BlockMatsProc(HWND hDlg, UINT messageID, WPARAM wParam, LPARAM lParam)
{
   switch(messageID)
   {
      case WM_INITDIALOG:
      {
         PROPSHEETPAGE * psp = (PROPSHEETPAGE *)lParam;
         //assert((Analysisdata *)psp->lParam == adglobalfortest);
      }  break;

      case WM_NOTIFY:
         handleNotify(hDlg, lParam);
         break;


      case WM_COMMAND:
      {
         int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
         int wNotifyCode = HIWORD(wParam);      // notification code 

         switch (idEditCtrl)
         {
            case IDC_LOADRESIDUAL:
               if (wNotifyCode == EN_KILLFOCUS)
               {
                  MessageBox(hDlg,"Residual edited",NULL,MB_OK);
               }
               break;
         }
      } break;
   }

return 0;
}  /* close BlockMatsProc() */



BOOL APIENTRY
AFlagsProc(HWND hDlg, UINT messageID, WPARAM wParam, LPARAM lParam)
{
   static Analysisdata * ad;
   //HWND hpage;

   switch(messageID)
   {
      case WM_INITDIALOG:
      {
         PROPSHEETPAGE * psp = (PROPSHEETPAGE *)lParam;
         ad = (Analysisdata *)psp->lParam;
         if (ad->gravityflag == 0)
            ;
         
         //hpage = psp->phpage[7];

         //MessageBox(hDlg,"Flags up",NULL,MB_OK);
         //return TRUE;
         //assert((Analysisdata *)psp->lParam == adglobalfortest);
      }  break;

      case WM_NOTIFY:
         handleNotify(hDlg, lParam);
         break;


      case WM_COMMAND:
      {
         int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
         int wNotifyCode = HIWORD(wParam);      // notification code
         HWND hcontrol = (HWND)lParam;

         switch (idEditCtrl)
         {
            case AD_GRAVITY:
               if (SendMessage(hcontrol,BM_GETCHECK,0,0) == BST_CHECKED)
               {
                  //ad->gravityflag = 1;
                  MessageBox(hDlg,"Gravity checked",NULL,MB_OK);
               }
               else 
               {
                  //ad->gravityflag = 0;
                  MessageBox(hDlg,"Gravity unchecked",NULL,MB_OK);
               }
               break;

            case AD_ROTATION:
               if (SendMessage(hcontrol,BM_GETCHECK,0,0) == BST_CHECKED)
               {
                  //ad->rotationflag = 1;
                  MessageBox(hDlg,"Rotation checked",NULL,MB_OK);
               }
               else 
               {
                  //ad->rotationflag = 0;
                  MessageBox(hDlg,"Rotation unchecked",NULL,MB_OK);
               }
               break;

            case AD_PSTRESS:
               //ad->planestrain = 0;
               MessageBox(hDlg,"Plane stress",NULL,MB_OK);
               break;

            case AD_PSTRAIN:
               //ad->planestrainflag = 1;
               MessageBox(hDlg,"Plane Strain",NULL,MB_OK);
               break;

            case AD_DYNAMIC:
               //ad->planestrain = 0;
               MessageBox(hDlg,"Dynamic analysis",NULL,MB_OK);
               break;

            case AD_STATIC:
               //ad->planestrainflag = 1;
               //MessageBox(hDlg,"Static analysis",NULL,MB_OK);
               return TRUE; //break;

            default:
               break;

         }
      } break;
   }

return 0;
}  /* close AFlagsProc() */



BOOL APIENTRY
ContactProc(HWND hDlg, UINT messageID, WPARAM wParam, LPARAM lParam)
{
   switch(messageID)
   {
      case WM_INITDIALOG:
      {
         PROPSHEETPAGE * psp = (PROPSHEETPAGE *)lParam;
         //assert((Analysisdata *)psp->lParam == adglobalfortest);
      }  break;

      case WM_NOTIFY:
         handleNotify(hDlg, lParam);
         break;


      case WM_COMMAND:
      {
         int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
         int wNotifyCode = HIWORD(wParam);      // notification code 

         switch (idEditCtrl)
         {
            case IDC_LOADRESIDUAL:
               if (wNotifyCode == EN_KILLFOCUS)
               {
                  MessageBox(hDlg,"Residual edited",NULL,MB_OK);
               }
               break;
         }
      } break;
   }

return 0;
}  /* close ContactProc() */



BOOL APIENTRY
TimeStepProc(HWND hDlg, UINT messageID, WPARAM wParam, LPARAM lParam)
{
   switch(messageID)
   {
      case WM_INITDIALOG:
      {
         PROPSHEETPAGE * psp = (PROPSHEETPAGE *)lParam;
         //assert((Analysisdata *)psp->lParam == adglobalfortest);
      }  break;

      case WM_NOTIFY:
         handleNotify(hDlg, lParam);
         break;


      case WM_COMMAND:
      {
         int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
         int wNotifyCode = HIWORD(wParam);      // notification code 

         switch (idEditCtrl)
         {
            case IDC_LOADRESIDUAL:
               if (wNotifyCode == EN_KILLFOCUS)
               {
                  MessageBox(hDlg,"Residual edited",NULL,MB_OK);
               }
               break;
         }
      } break;
   }

   return 0;
}  /* close TimeStepProc() */



BOOL APIENTRY
LPointsProc(HWND hDlg, UINT messageID, WPARAM wParam, LPARAM lParam)
{
   switch(messageID)
   {
      case WM_INITDIALOG:
      {
         PROPSHEETPAGE * psp = (PROPSHEETPAGE *)lParam;
         //assert((Analysisdata *)psp->lParam == adglobalfortest);
      }  break;

      case WM_NOTIFY:
         handleNotify(hDlg, lParam);
         break;


      case WM_COMMAND:
      {
         int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
         int wNotifyCode = HIWORD(wParam);      // notification code 

         switch (idEditCtrl)
         {
            case IDC_LOADRESIDUAL:
               if (wNotifyCode == EN_KILLFOCUS)
               {
                  MessageBox(hDlg,"Residual edited",NULL,MB_OK);
               }
               break;
         }
      } break;
   }

return 0;
}  /* close LPointsProc() */


BOOL APIENTRY
AnaSeismicProc(HWND hDlg, UINT messageID, WPARAM wParam, LPARAM lParam)
{
   switch(messageID)
   {
      case WM_INITDIALOG:
      {
         PROPSHEETPAGE * psp = (PROPSHEETPAGE *)lParam;
         //assert((Analysisdata *)psp->lParam == adglobalfortest);
      }  break;


      case WM_NOTIFY:
         handleNotify(hDlg, lParam);
         break;

      case WM_COMMAND:
      {
         int idEditCtrl = (int) LOWORD(wParam); // identifier of edit control 
         int wNotifyCode = HIWORD(wParam);      // notification code 

         switch (idEditCtrl)
         {
            case IDC_LOADRESIDUAL:
               if (wNotifyCode == EN_KILLFOCUS)
               {
                  MessageBox(hDlg,"Residual edited",NULL,MB_OK);
               }
               break;
         }
      } break;
   }

return 0;
}  /* close AnaSeismicProc() */

