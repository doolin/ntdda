#ifndef __TIMEHISTORY_H__
#define __TIMEHISTORY_H__


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
TimeHistory *   th_clone                     (TimeHistory *);
void            th_delete                    (TimeHistory *);

void            th_read_data_file            (TimeHistory *,
                                              char * filename, 
                                              format);

int             th_get_number_of_datapoints  (TimeHistory *);

double          th_get_delta_t               (TimeHistory *);

double          th_get_data_value            (TimeHistory *, 
                                              int timestep);

#ifdef __cplusplus
}
#endif


#endif /* __TIMEHISTORY_H__ */

