

#ifndef __LOADPOINT_H__
#define __LOADPOINT_H__


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

#ifdef __cplusplus
}
#endif

#endif  /* __LOADPOINT_H__ */
