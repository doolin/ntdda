
#include <gnome-xml/parser.h>
#include "dda.h"

#include "ddadlist.h"

/*
 * A person record, which should go into project.h
 * at some point in the very near future.
 */
typedef struct person {
    char *name;
    char *email;
    char *company;
    char *organisation;
    char *smail;
    char *webPage;
    char *phone;
} person, *personPtr;

typedef struct _project {
  char * name;
  char * date;
  char * status;
  /* These next will become dlists */
  personPtr analyst;
  personPtr client;
  char * publication;
} Project,  * ProjectPtr;

/*
 * An analysis...
 */
//typedef struct _dda {
//  ProjectPtr project;/
//  xmlNsPtr namespace;
//} DDA, *DDAPtr;


personPtr parsePerson(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur); 

void printPerson(personPtr cur);

void * parseProject(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);


//ANALYSISDATA * parseAnalysis(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);


//void * parseJoints(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
//DDAPtr parseDDAFile(char *filename);
void printProject(ProjectPtr cur);
//DDAPtr checkdoc(xmlDocPtr doc);





extern int xmlDoValidityCheckingDefaultValue;

