
/*
 *
 */

#ifndef __DDAML_H__
#define __DDAML_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


#define LIBXML_STATIC

void ddaml_read_geometry_file  (void * userdata,
                                char * filename);

void ddaml_read_analysis_file  (void * userdata,
                                char * filename);

void ddaml_check_document      (void * userdata, //xmlDocPtr doc,
                                const char * name_space,
                                const char * rootname);

void  (*ddaml_display_warning) (const char * message);

void  (*ddaml_display_error)   (const char * message);


#ifdef __cplusplus
}
#endif

#endif  /* __DDAML_H__ */





