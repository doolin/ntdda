/**
 * Functions common to all ddaml documents.
 */

#include <stdlib.h>
#include <string.h>

#include "ddaml_private.h"
#include "ddaml.h"

// only for MSDEV 6.0
// If using cpp, prefix with extern "C"
// See google for details of _ftol2
#if (_MSC_VER == 1200) && (WINVER < 0x0500)
long _ftol( double ); //defined by VC6 C libs
long _ftol2( double dblSource ) { 
   return _ftol( dblSource ); 
} 
#endif


void
ddaml_check_document(void * userdata, const char * name_space, const char * rootname) {

#if 0
   xmlDocPtr doc = (xmlDocPtr)userdata;
   xmlNsPtr nspace;

   if (doc == NULL) {
      ddaml_display_error("Invalid Geometry document, check geometry file syntax");
      exit (0);
   }

   if (doc->root == NULL) {
       ddaml_display_error("Empty geometry document.");
	    xmlFreeDoc(doc);
       exit (0);
   }

   //nspace = xmlSearchNsByHref(doc, cur, "http://www.tsoft.com/~bdoolin/dda");
   //nspace = xmlSearchNsByHref(doc, doc->root, "http://www.tsoft.com/~bdoolin/dda");
   nspace = xmlSearchNsByHref(doc, doc->root, name_space);
 
   if (nspace == NULL) {
        ddaml_display_error("Namespace error, check URL");
	     xmlFreeDoc(doc);
        exit (0);
   }
    
   //if (strcmp(doc->root->name, "DDA")) {
   if (strcmp(doc->root->name, rootname)) {
        char message[128];
        sprintf(message,"Document of the wrong type, root node != DDA");
	     ddaml_display_error(message);
	     xmlFreeDoc(doc);
        exit (0);
   }

#endif

}  
