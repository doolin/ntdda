
/*  
 *
 */

#ifndef __DDAML_H__
#define __DDAML_H__

#include <gnome-xml/parser.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

extern int xmlDoValidityCheckingDefaultValue;


void ddaml_read_geometry_file (void * userdata, char * filename);
void ddaml_read_analysis_file (void * userdata, char * filename);


void  (*ddaml_display_warning) (const char * message);
void  (*ddaml_display_error)   (const char * message);

#ifdef __cplusplus
}
#endif

#endif  /* __DDAML_H__ */





