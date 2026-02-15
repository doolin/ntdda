
#define STRICT

#include <windows.h>
#include "winmain.h"
#include "resource.h"
#include "dda.h"



void
handleCommandLine(HWND hwMain, int argc, char ** argv, Filepaths * fpath)
{
   char * fnamesuffix;

   if (argc > 2)
      MessageBox(hwMain, "Only 1 file may be dropped","File drop message",
                 MB_OK | MB_ICONSTOP);

  /* str just points, do not free str */
   fnamesuffix = strrchr(argv[1],'.');

  /* Only handle geometry files for now. */
   if (!strcmp(fnamesuffix,".geo")) // || !strcmp(str,".ana") || !strcmp(str,".replay")
   {
     /* copy the rootfilename etc over */
      strcpy (fpath->gfile, argv[1]);
      strcpy (fpath->rootname, strtok(argv[1], "."));
      SendMessage(hwMain, WM_COMMAND, GEOM_APPLY, 0);//lParam);
   }
   else
      MessageBox(hwMain, "Wrong file type","File drop message", MB_OK);

}  /* close handleCommandLine() */


void
handleDropFiles(HWND hwMain, WPARAM wParam, LPARAM lParam,Filepaths * fpath)
{
   char * str;
   HDROP hDrop;
  /* number of files dropped */
   int numfiles;
   LPTSTR dropfilebuf[200];
   hDrop = (HANDLE)wParam;

   numfiles = DragQueryFile(hDrop, 0xFFFFFFFF, (LPTSTR)dropfilebuf, 200);

   if (numfiles > 1)
      MessageBox(hwMain, "Only 1 file may be dropped","File drop message",
                 MB_OK | MB_ICONSTOP);

   DragQueryFile(hDrop, 0, (LPTSTR)dropfilebuf, 200);

   str = strrchr((char *)dropfilebuf, '.');

  /* Only handle geometry files for now. */
   if (!strcmp(str,".geo")) // || !strcmp(str,".ana") || !strcmp(str,".replay")
   {
     /* copy the rootfilename etc over */
      strcpy (fpath->gfile, (char *)dropfilebuf);
      strcpy (fpath->rootname, strtok((char *)dropfilebuf, "."));
      SendMessage(hwMain, WM_COMMAND, GEOM_APPLY, lParam);
   }
   else
      MessageBox(hwMain, "Wrong file type","File drop message", MB_OK);


}  /* close handleDropFiles() */
