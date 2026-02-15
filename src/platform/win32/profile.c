
/** read/write .ini files. Could be used for registry also.
 */

#include "profile.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



/* TODO: Flesh out a preferences API.  The
 * default location for the .ini files are
 * the Windows directory, which is reasonable.
 * It would take quite a bit of work to implement
 * all the ini reading and writing into some
 * specified directory.
 */
 /* void for now... not using it yet. */
void
profile_write(void) {

   /*
   //char message[240];

   char winbuf[240];
   char sysbuf[240];
   char retbuf[8192*4];

   WritePrivateProfileString("DDA_Top_Level_Section",  // section name
                             "Foo",       // key
                             "Bar",       // value
                             "dda.ini\0");  // filename
   WritePrivateProfileString("DDA_Top_Level_Section",  // section name
                             "Bar",       // key
                             "Baz",       // value
                             "dda.ini\0");  // filename
   WritePrivateProfileString("DDA_Output_Options_Section",  // section name
                             "mpoints",       // key
                             "1",       // value
                             "dda.ini\0");  // filename

   GetWindowsDirectory(winbuf,240);
   MessageBox(NULL,winbuf,NULL,MB_OK);
   GetSystemDirectory(sysbuf,240);
   MessageBox(NULL,sysbuf,NULL,MB_OK);

   GetPrivateProfileSection("DDA_Top_Level_Section",
                             retbuf,
                             8192*4,
                             "dda.ini");
   MessageBox(NULL,retbuf,NULL,MB_OK);

   showlasterror(GetLastError());
*/

}





#ifdef __cplusplus
}
#endif