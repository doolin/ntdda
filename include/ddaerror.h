/*
 * ddaerror.h
 *
 * Class for handling all kinds of that can occur during 
 * a DDA run.
 */

#ifndef __DDAERROR_H__
#define __DDAERROR_H__


#define __ZERO_MOMENT_ERROR__ 1

typedef struct dda_error_tag DDAError;

/*
char * errortable[] = {
{"Zero or negative block area"},
{"..."},
{NULL}
};
*/

struct dda_error_tag {

   int error;
  /* See the associated string table */
   char * errorstring;

};


#endif /* __DDAERROR_H__ */