/*
 * ddaparse.c
 * 
 * Handles strings of values and attributes returned by
 * libxml parser.
 *
 * $Author: doolin $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <gnome-xml/parser.h>
#include "dda.h"
#include "ddamemory.h"

#define DEBUG(x) printf(x)

/*
 * A person record
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

/*
 * a Description for a Job
 */
typedef struct job {
    char *projectID;
    char *application;
    char *category;
    personPtr contact;
    int nbDevelopers;
    personPtr developers[100]; /* using dynamic alloc is left as an exercise */
} job, *jobPtr;

/*
 * A pool of Gnome Jobs
 */
typedef struct gjob {
    int nbJobs;
    jobPtr jobs[500]; /* using dynamic alloc is left as an exercise */
} gJob, *gJobPtr;



gJobPtr parseGjobFile(char *filename);
void handleGjob(gJobPtr cur);

/*  An example main */
  /*
int 
main(int argc, char **argv) 
{
    int i;
    gJobPtr cur;

    for (i = 1; i < argc ; i++) {
        cur = parseGjobFile(argv[i]);
        handleGjob(cur);
    }
    return(0);
}
*/

/*
 * And the code needed to parse it
 */
personPtr 
parsePerson(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
    personPtr ret = NULL;

DEBUG("parsePerson\n");
    /*
     * allocate the struct
     */
    ret = (personPtr) malloc(sizeof(person));
    if (ret == NULL) {
        fprintf(stderr,"out of memory\n");
	return(NULL);
    }
    memset(ret, 0, sizeof(person));

    /* We don't care what the top level element name is */
    cur = cur->childs;
    while (cur != NULL) {
        if ((!strcmp(cur->name, "Person")) && (cur->ns == ns))
	    ret->name = xmlNodeListGetString(doc, cur->childs, 1);
        if ((!strcmp(cur->name, "Email")) && (cur->ns == ns))
	    ret->email = xmlNodeListGetString(doc, cur->childs, 1);
	cur = cur->next;
    }

    return(ret);
}



/*
 * and to print it
 */
void 
printPerson(personPtr cur) 
{
    if (cur == NULL) return;
    printf("------ Person\n");
    if (cur->name) printf("	name: %s\n", cur->name);
    if (cur->email) printf("	email: %s\n", cur->email);
    if (cur->company) printf("	company: %s\n", cur->company);
    if (cur->organisation) printf("	organisation: %s\n", cur->organisation);
    if (cur->smail) printf("	smail: %s\n", cur->smail);
    if (cur->webPage) printf("	Web: %s\n", cur->webPage);
    if (cur->phone) printf("	phone: %s\n", cur->phone);
    printf("------\n");
}



/*
 * And the code needed to parse it
 */
jobPtr 
parseJob(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
    jobPtr ret = NULL;

DEBUG("parseJob\n");
    /*
     * allocate the struct
     */
    ret = (jobPtr) malloc(sizeof(job));
    if (ret == NULL) {
        fprintf(stderr,"out of memory\n");
	return(NULL);
    }
    memset(ret, 0, sizeof(job));

    /* We don't care what the top level element name is */
    cur = cur->childs;
    while (cur != NULL) {
        
        if ((!strcmp(cur->name, "Project")) && (cur->ns == ns)) {
	    ret->projectID = xmlGetProp(cur, "ID");
	    if (ret->projectID == NULL) {
		fprintf(stderr, "Project has no ID\n");
	    }
	}
        if ((!strcmp(cur->name, "Application")) && (cur->ns == ns))
	    ret->application = xmlNodeListGetString(doc, cur->childs, 1);
        if ((!strcmp(cur->name, "Category")) && (cur->ns == ns))
	    ret->category = xmlNodeListGetString(doc, cur->childs, 1);
        if ((!strcmp(cur->name, "Contact")) && (cur->ns == ns))
	    ret->contact = parsePerson(doc, ns, cur);
	cur = cur->next;
    }

    return(ret);
}

/*
 * and to print it
 */
void 
printJob(jobPtr cur) 
{
    int i;

    if (cur == NULL) return;
    printf("=======  Job\n");
    if (cur->projectID != NULL) printf("projectID: %s\n", cur->projectID);
    if (cur->application != NULL) printf("application: %s\n", cur->application);
    if (cur->category != NULL) printf("category: %s\n", cur->category);
    if (cur->contact != NULL) printPerson(cur->contact);
    printf("%d developers\n", cur->nbDevelopers);

    for (i = 0;i < cur->nbDevelopers;i++) printPerson(cur->developers[i]);
    printf("======= \n");
}




gJobPtr parseGjobFile(char *filename) 
{
    xmlDocPtr doc;
    gJobPtr ret;
    jobPtr job;
    xmlNsPtr ns;
    xmlNodePtr cur;

    /*
     * build an XML tree from a the file;
     */
    doc = xmlParseFile(filename);
    if (doc == NULL) return(NULL);

    /*
     * Check the document is of the right kind
     */
    cur = doc->root;
    if (cur == NULL) {
        fprintf(stderr,"empty document\n");
	xmlFreeDoc(doc);
	return(NULL);
    }
    ns = xmlSearchNsByHref(doc, cur, "http://www.gnome.org/some-location");
    if (ns == NULL) {
        fprintf(stderr,
	        "document of the wrong type, GJob Namespace not found\n");
	xmlFreeDoc(doc);
	return(NULL);
    }
    if (strcmp(cur->name, "Helping")) {
        fprintf(stderr,"document of the wrong type, root node != Helping");
	xmlFreeDoc(doc);
	return(NULL);
    }

    /*
     * Allocate the structure to be returned.
     */
    ret = (gJobPtr) malloc(sizeof(gJob));
    if (ret == NULL) {
        fprintf(stderr,"out of memory\n");
	xmlFreeDoc(doc);
	return(NULL);
    }
    memset(ret, 0, sizeof(gJob));

    /*
     * Now, walk the tree.
     */
    /* First level we expect just Jobs */
    cur = cur->childs;
    if ((strcmp(cur->name, "Jobs")) || (cur->ns != ns)) {
        fprintf(stderr,"document of the wrong type, Jobs expected");
	xmlFreeDoc(doc);
	free(ret);
	return(NULL);
    }

    /* Second level is a list of Job, but be laxist */
    cur = cur->childs;
    while (cur != NULL) {
        if ((!strcmp(cur->name, "Job")) && (cur->ns == ns)) {
	    job = parseJob(doc, ns, cur);
	    if (job != NULL)
	        ret->jobs[ret->nbJobs++] = job;
            if (ret->nbJobs >= 500) break;
	}
	cur = cur->next;
    }

    return(ret);
}

void 
handleGjob(gJobPtr cur) 
{
    int i;

    /*
     * Do whatever you want and free the structure.
     */
    printf("%d Jobs registered\n", cur->nbJobs);
    for (i = 0; i < cur->nbJobs; i++) printJob(cur->jobs[i]);
}

