
//#include <stdio.h>
#include "geometrydata.h"
#include "ddafile.h"

typedef struct iface_tag InterFace;

struct iface_tag {

   InterFace * this;

   char message[80];
  
  /* a generic display message.  The interface.c file has to 
   * implement this appropriately.
   */
   Geometrydata * (*rungeometry)(FILEPATHS *, 
                                 GRAPHICS *);
   //void (*displaymessage)(char *);
   void (*setdisplay)(unsigned int);  // In windows, HWND is a long...
   void (*setafetext)(char *);
   void (*updatedisplay)(void);
   void (*updatestatusbar)(int);
   void (*displaywarning)(char * warning);
   void (*setoc_count)(int);
};


/* Public method, any one including the header can call it.
 * Set the initialization function pointer here.
 */
InterFace * getNewIFace();
