
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

/**
 * @todo Move this struct definition into the 
 * ddaml header file and combine with the struct
 * definition in the analysisddaml parsing file.
 */
typedef struct _gkwdtab {

  /* text of the keyword        */   
   char *kwd;		
  /* Token codes can be used to point at the relevant 
   * function pointer in the union.
   */
   enum  tokentype  {node = 0, string, prop} ktok;
   union {
      void (*nodeparse)  (xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
      void (*stringparse)(xmlDocPtr doc, xmlNodePtr cur, int );	
      void (*propparse)  (xmlDocPtr doc, xmlNodePtr cur, int );
   } parsefn;
} KWDTAB;


void ddaml_read_geometry_file  (void * userdata, 
                                char * filename);

void ddaml_read_analysis_file  (void * userdata, 
                                char * filename);

void ddaml_check_document      (xmlDocPtr doc, 
                                const char * name_space, 
                                const char * rootname);

void  (*ddaml_display_warning) (const char * message);

void  (*ddaml_display_error)   (const char * message);


#ifdef __cplusplus
}
#endif

#endif  /* __DDAML_H__ */





