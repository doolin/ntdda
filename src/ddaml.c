/**
 * Functions common to all ddaml documents.
 */

#include <stdlib.h>
#include <string.h>

#include "ddaml.h"


void
ddaml_check_document(xmlDocPtr doc, const char * name_space, const char * rootname) {

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

}  
