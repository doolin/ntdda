

#ifndef __TIMEHISTORY_H__
#define __TIMEHISTORY_H__

typedef struct _TimeHistory TimeHistory;
typedef enum _format{ shake, matlab } format;

TimeHistory * getTimeHistory(char * filename, format);

int th_get_number_of_datapoints(TimeHistory *);

double th_get_delta_t(TimeHistory *);

double th_get_data_value(TimeHistory *, int timestep);

void * destroyTimeHistory(TimeHistory *);  // returns NULL 

/* TODO: */
//cloneTimeHistory(TimeHistory *);

#endif /* __TIMEHISTORY_H__ */

