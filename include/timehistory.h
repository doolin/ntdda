#ifndef __TIMEHISTORY_H__
#define __TIMEHISTORY_H__

#include "functions.h"
#include "ddadlist.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



typedef struct _timehistory TimeHistory;
typedef enum _format{ shake, matlab } format;

#ifndef DISPLAYFUNC
#define DISPLAYFUNC
typedef void (*DisplayFunc)(const char * message);
#endif

TimeHistory *   th_new                       (void);
TimeHistory *   th_clone                     (TimeHistory * th);
void            th_delete                    (TimeHistory * th);


/**
 * @todo Explain what a valid time history is. 
 *
 * @return 1 pass 0 fail
 */
int             th_validate                  (TimeHistory * th);

void            th_read_data_file            (TimeHistory * th,
                                              char * filename, 
                                              format);

int             th_get_number_of_datapoints  (TimeHistory * th);

double          th_get_delta_t               (TimeHistory * th);

double          th_get_data_value            (TimeHistory * th, 
                                              int timestep);


/** At some point in the future, this will be moved into a 
 * seismic module, and the timehistory will be hidden in that.
 */
void            seismic_update_points        (DList * seispoints, 
                                              double ** moments,  
                                              double ** F,
                                              int * k1,
                                              TransMap transmap, 
                                              TransApply transapply);


#ifdef __cplusplus
}
#endif


#endif /* __TIMEHISTORY_H__ */

