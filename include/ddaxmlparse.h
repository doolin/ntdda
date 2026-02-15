
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




personPtr parsePerson(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);

void printPerson(personPtr cur);

void * parseProject(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);


void printProject(ProjectPtr cur);


extern int xmlDoValidityCheckingDefaultValue;

