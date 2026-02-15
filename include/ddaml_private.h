
/**
 *  Private include to help unit testing.
 */

#ifndef __DDAML_PRIVATE_H__
#define __DDAML_PRIVATE_H__

#include <libxml/parser.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



extern int xmlDoValidityCheckingDefaultValue;


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


#ifdef __cplusplus
}
#endif

#endif  /* __DDAML_PRIVATE_H__ */


