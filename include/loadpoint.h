

#ifndef __LOADPOINT_H__
#define __LOADPOINT_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
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

#ifdef __cplusplus
}
#endif

#endif  /* __LOADPOINT_H__ */
