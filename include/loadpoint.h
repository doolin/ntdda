

#ifndef __LOADPOINT_H__
#define __LOADPOINT_H__

#include "ddatypes.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#ifndef PRINTFUNC
#define PRINTFUNC
typedef int (*PrintFunc)(void * stream, const char * format, ...);
#endif

typedef struct _loadpoint Loadpoint;


struct _loadpoint {

   double xload,yload;
   double time;

   DisplayFunc display_warning;
   DisplayFunc display_error;

   int loadpointsize1;
   int loadpointsize2;  /* = 3 */
   double ** vals;
};



Loadpoint * loadpoint_new       (void);

Loadpoint * loadpoint_new_array (int umloadpoints);

Loadpoint * loadpoint_clone     (Loadpoint * lp,
                                 int numloadpoints);

void        loadpoint_delete    (Loadpoint * lp);


int         loadpoint_test      (void);

void        loadpoint_print     (Loadpoint *,
                                 int numloadpoints,
                                 PrintFunc printer,
                                 void * stream);


void        loadpoint_print_current (Loadpoint * lp,
                                     PrintFunc printer,
                                     void * stream);


void        df09                    (Loadpoint * loadpoints,
          double ** points,
          double ** globalTime,
          double ** timeDeps,
          int ** tindex,
          int nfp,
          int numloadpoints,
          int cts,
          double delta_t);

#ifdef __cplusplus
}
#endif

#endif  /* __LOADPOINT_H__ */
