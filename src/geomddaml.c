
/*
 * parseGeometry.c 
 *
 * An example program for DDA input file  XML format
 *
 * Copyright: http://www.gnu.org/lesser
 *
 * David M. Doolin  doolin@ce.berkeley.edu
 *
 * $Author: doolin $
 * $Date: 2001/05/20 20:59:44 $
 * $Source: /cvsroot/dda/ntdda/src/geomddaml.c,v $
 * $Revision: 1.1 $
 */


/* FIXME: Point handling.  If there are no fixed points, the  
 * code fails.  HAs to do with how the structs are laid out.
 * It would have been easier to understand if it had segfaulted.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "ddaxmlparse.h"
#include "ddamemory.h"
#include "geometrydata.h"
#include "interface.h"

extern InterFace * iface;



typedef DList JOINTLIST;
typedef DList POINTLIST; 
typedef DList FPOINTLIST; 
typedef DList LPOINTLIST;
typedef DList SPOINTLIST;
typedef DList BOLTLIST; 
typedef DList MATLINELIST; 
typedef DList BOUNDLIST; 

static JOINTLIST * jointlist = NULL;
static POINTLIST * pointlist = NULL;
static FPOINTLIST * fpointlist = NULL;
static LPOINTLIST * lpointlist = NULL;
static SPOINTLIST * spointlist = NULL;
static BOLTLIST * boltlist = NULL;
static MATLINELIST * matlinelist = NULL;
static BOUNDLIST * boundlist = NULL;

static xmlNsPtr nspace;

//static enum pointtype {fixed=0,measured,load,hole} pointtype;

static Geometrydata * parseGeometry(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);

static void * parseEdgenodedist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseJointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseFixedpointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseMeasuredpointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseLoadpointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseHolepointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseSeismicpointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseBoltlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseMatlinelist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseBoundlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseJoints(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseJointset(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);

static Joint * getNewJoint(void);
static DDAPoint * getNewPoint(void);
static DDALine * getNewLine(void);
static BOLT * getNewBolt(void);

static void transferGData(void);
static void transferJointlistToGeomStruct(Geometrydata *, JOINTLIST *);
static void transferPointlistToGeomStruct(Geometrydata *, POINTLIST *);
static void transferBoltlistToGeomStruct(Geometrydata *, BOLTLIST *);
static void transferMatlinelistToGeomStruct(Geometrydata *, MATLINELIST *);
static double ** DoubMat2DGetMem(int n, int m);


static void * checkGeometryDoc(xmlDocPtr doc);
static void initializeGLists(void);

typedef struct _gkwdtab
{
   char *kwd;			/* text of the keyword        */
  /* Token codes can be used to point at the relevant 
   * function pointer in the union.
   */
   enum  tokentype  {node = 0, string, prop} ktok;
   union {
      void *(*nodeparse)(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);		/* function */
      void *(*stringparse)(xmlDocPtr doc, xmlNodePtr cur, int );		/* function */
      void *(*propparse)(xmlDocPtr doc, xmlNodePtr cur, int );		/* function */
   } pparse;

} GKWDTAB;

GKWDTAB gtab_type[] = {
   {"Edgenodedist", 0, *parseEdgenodedist},
   {"Jointlist", 0, *parseJointlist},
   {"Fixedpointlist", 0, *parseFixedpointlist},
   {"Measuredpointlist", 0, *parseMeasuredpointlist},
   {"Loadpointlist", 0, *parseLoadpointlist},
   {"Holepointlist", 0, *parseHolepointlist},
   {"Seismicpointlist", 0, *parseSeismicpointlist},
   {"Boltlist", 0, *parseBoltlist},
   {"Matlinelist", 0, *parseMatlinelist},
   {"Boundlist", 0, *parseBoundlist},
//   {"Jointset",0,*parseJointset},  /* Don't need this yet.  */
   {NULL, 0, 0}			/* Ends a scanning loop.  See comment above. */
};


Geometrydata * gdata;

#ifdef STANDALONE
int 
main(int argc, char **argv) 
{
   int i;
   fprintf(stderr,"Parsing geometry data from geometry main\n");

   for (i = 1; i < argc ; i++) 
   {
	     gdata = XMLparseDDA_Geometry_File(argv[i]);
   }

   return(0);
}  /* close main() */

#endif /* STANDALONE */


Geometrydata *
XMLparseDDA_Geometry_File(char *filename) 
{
   xmlDocPtr doc;
   xmlNsPtr ns;
   xmlNodePtr cur;
   Geometrydata * gd;

   xmlDoValidityCheckingDefaultValue = 1;

  /*
   * build an XML tree from a the file;
   */
   doc = xmlParseFile(filename);

   checkGeometryDoc(doc);

  /* This is where we need an exception */
   if (doc == NULL)
     return NULL;

  /* FIXME: Handle a null document, which means that there
   * is a messed up tag or something.
   */
   cur = doc->root;
   ns = nspace;

  /*
   * Now, walk the tree.  All the following code will
   * be replaced with keyscanning.  Initially, do not
   * check for keyscanning failure.  Let the dtd validator
   * handle that.  Later, a few simple checks can be added,
   * just to handle exceptional cases.
   */
  /* Here, we not only do not want the parent, we want to 
   * bypass the top level node, in this case `Geometry',
   * completely to start with the first element in the 
   * geometry part of the document.
   */
   cur = cur->childs->next;

   gd = parseGeometry(doc, ns, cur); 

   return(gd);

}  /* close XMLparseDDA_Analysis_File() */


void *
checkGeometryDoc(xmlDocPtr doc)
{
   xmlNodePtr cur;
    
   if (doc == NULL) 
   {
     /* Log this as an error somewhere, then */
      fprintf(stderr,"Null document, really bad error somewhere\n");
      return(NULL);
   }

  /*
   * Check the document is of the right kind
   */
   cur = doc->root;
    
   if (cur == NULL) 
   {
      fprintf(stderr,"empty geometry document\n");
	     xmlFreeDoc(doc);
	     return(NULL);
   }

  /*
   * Allocate the structure to be returned.
   */
   //gdata = (Geometrydata *) malloc(sizeof(Geometrydata));
   //memset(gdata, 0, sizeof(DDA));

   //if (gdata == NULL) 
   //{
   //   fprintf(stderr,"out of memory for geometry data\n");
	  //   xmlFreeDoc(doc);
   //  	return(NULL);
   // }
   
   //gdata->namespace = xmlSearchNsByHref(doc, cur, "http://www.tsoft.com/~bdoolin/dda");
   nspace = xmlSearchNsByHref(doc, cur, "http://www.tsoft.com/~bdoolin/dda");
 
   if (nspace == NULL) 
   {
      fprintf(stderr,"Namespace error, check URL\n");
	     xmlFreeDoc(doc);
	     return(NULL);
   }
    
   if (strcmp(cur->name, "DDA")) 
   {
      fprintf(stderr,"document of the wrong type, root node != DDA");
	     xmlFreeDoc(doc);
	     return(NULL);
   }

   return NULL;

}  /* close checkGeometryDoc() */


static Geometrydata *
parseGeometry(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   int i = 0;
   //FILE * outtestfile;

   //outtestfile = fopen("testgeomxml.geo","w");

  /* Easiest to parse into a list, then count 
   * and grab array memory, then copy data into 
   * arrays.  This will be good, because I will 
   * now have lists of elements that can eventually
   * be used in the main code.
   */
   initializeGLists(); 

  /*
   * allocate the struct
   */
   //gdata = (Geometrydata *) malloc(sizeof(Geometrydata));

   gdata = initGeometrydata();

   if (gdata == NULL) 
   {
      fprintf(stderr,"out of memory\n");
     	return(NULL);
   }
   
   //memset(gdata, 0, sizeof(Geometrydata));

  /* FIXME: This is bogus.  It is only in here because 
   * of a single comment in front of the the <Geometry>
   * and <Analysis> tags in the input files.
   */
   cur = cur->childs;

   while (cur != NULL) 
   {
      i = 0;
      while (gtab_type[i].kwd)
      {
         if ( ( !strcmp(gtab_type[i].kwd,cur->name) ) )//&&  (cur->ns == ns)  ) 
         {
            fprintf(stderr,"Keyscan loop: Current name %s:\n", cur->name);
            switch(gtab_type[i].ktok)
            {
               case node:
	                 gtab_type[i].pparse.nodeparse(doc,ns,cur);
               break;

               case string:
	                 gtab_type[i].pparse.stringparse(doc,cur,1);
               break;

               case prop:
	                 gtab_type[i].pparse.propparse(doc,cur,1);
               break;

               default:;
               break;
            }  /* end switch on node type */
           /* Should probably have a continue or break in here */
            break;
         }  /* end if strcmp */
         i++;
      }   //close key scan loop 

	     cur = cur->next;
   }  /* end while cur != NULL  */
   
  /* Somewhere here there has to be a function for transferring
   * data from the dlists to the arrays.  Might find something
   * in the draw dialog box that does this.
   */
   transferGData();
  /* Write it out and see if we can load it... */
   //dumpGeometrydata(gdata, outtestfile);
   //fclose(outtestfile);
  /* Now clean everything up. */
   //freeDlistsAndStuff();

   return(gdata);

}  /* close parseGeometry() */


void 
initializeGLists()
{

   jointlist = make_dl();
   pointlist = make_dl();
   fpointlist = make_dl();
   spointlist = make_dl();
   boltlist = make_dl();
   matlinelist = make_dl();

}  /* close initializeLists() */



void *
parseEdgenodedist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur)
{

   fprintf(stdout, "Parsing Edge-node distance\n");
   
   gdata->e00 = atof(xmlGetProp(cur, "distance"));

   fprintf(stdout,"Edge-node distance: %f\n\n",gdata->e00);

   return NULL;

}  /* close parseEdgenodedist() */



void *
parseJointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
  /* This has to be initialized so that the mickeysoft
   * compiler understands that floating point means 
   * floating point.  Piece of crap.
   */
   double temp[4] = {0.0};
   char * tempstring;
   Joint * joint;
   //xmlNodePtr jscur;

   fprintf(stdout, "Parsing Joint list\n");

  /* Don't care about parent. */
   cur = cur->childs;


/* Added Jointset tag */
//while (cur != NULL)
//{
   
   //jscur = cur->childs;

  /* Now there should be a linked list of <Joint> 
   * elements to traverse over.  The validator (DTD
   * checker) should make some of the following code 
   * irrelevant.
   */
   while (cur != NULL)
   {
      if ((!strcmp(cur->name, "Joint")) )//&& (cur->ns == ns)) 
      {

        /* Here I will have to load up a joint structure that 
         * goes into a dlist. So I have to grab another joint
         * structure, and another dlist node
         */
         joint = getNewJoint();
        /* `type' will have to be converted to an integer,
         * and checked to see if it is range.
         * FIXME: xmlGetProp(cur, "type") returning NULL causes a segfault here.
         */
         joint->type = atoi(xmlGetProp(cur, "type"));
  	
        /* WARNING!!!  The MickeySoft compiler will not load the 
         * floating point library without an explicit reference
         * to a double.  It WILL NOT scan the format strings to 
         * pick this up.  Piece of crap.
         */
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         //fprintf(stdout,"%s\n", tempstring);
         sscanf(tempstring,"%lf%lf%lf%lf",&temp[0],&temp[1],&temp[2],&temp[3]);  
         //fprintf(stdout,"%f %f %f %f\n",temp[0],temp[1],temp[2],temp[3]);  
         joint->epx1 = temp[0];
         joint->epy1 = temp[1];
         joint->epx2 = temp[2];
         joint->epy2 = temp[3];

        /* Here, add the joint struct to the dlidt node, then 
         * add the node to the end of the dlist.
         */
         dl_insert_b(jointlist, (void*)joint);
      }
      else
      {
        /* Big problems.  Validator failed to find a wrong 
         * markup element.  Fix this by writing to log file.
         * FIXME: An XML comment statement (<!--) fires this warning.
         * This needs to be trapped nd ignored.
         */
         //iface->displaymessage("Validation failure in joint parsing."  
         //                " Contact author.\n");
         ;  //  null statement
      }
       
      gdata->nJoints++;
      //jscur = jscur->next;
      cur = cur->next;
   }
//cur = cur->next;
//}  /* end while loop over Jointset */

 

   fprintf(stdout, "\n");

   return NULL;

}  /*  close parseJointlist() */

/* FIXME: Move this to a fpointlist instead of a pointlist.
 */
static void *
parseFixedpointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   DDALine * line;
  /* Make the compiler link the floating point libraries. */
   double temp[4] = {0.0};
   char * tempstring;

   fprintf(stdout, "Parsing Fixed point list\n");

  /* Don't care about parent */
   cur = cur->childs;

   while (cur != NULL)
   {

      if ((!strcmp(cur->name, "Line")) )// && (cur->ns == ns)) 
      {
         fprintf(stdout, "Fixed line\n");

        /* Here I will have to load up a ddapoint structure that 
         * goes into a dlist. So I have to grab another ddapoint
         * structure, and another dlist node
         */
         line = getNewLine();
         line->type = fixed; //  This will be an enum for fixed.
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         fprintf(stdout,"%s\n", tempstring);
         sscanf(tempstring,"%lf%lf%lf%lf",&temp[0],&temp[1],&temp[2],&temp[3]);  
         fprintf(stdout,"%f %f %f %f\n",temp[0],temp[1],temp[2],temp[3]);  
         line->x1 = temp[0];
         line->y1 = temp[1];
         line->x2 = temp[2];
         line->y2 = temp[3];

        /* Here, add the joint struct to the dlidt node, then 
         * add the node to the end of the dlist.
         */
         dl_insert_b(fpointlist,(void*)line);
         gdata->nFPoints++;
      } 
      else if ((!strcmp(cur->name, "Point")) )//&& (cur->ns == ns)) 
      {
         fprintf(stdout, "Fixed point\n");

         line = getNewLine();
         line->type = fixed; //  enum for fixed 
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         fprintf(stdout,"%s\n", tempstring);
         sscanf(tempstring,"%lf%lf",&temp[0],&temp[1]);  
         fprintf(stdout,"%f %f\n",temp[0],temp[1]);  
         line->x1 = temp[0];
         line->y1 = temp[1];
         line->x2 = temp[0];
         line->y2 = temp[1];
         dl_insert_b(fpointlist,(void*)line);
         gdata->nFPoints++;
      }
      else
      { 
        /* Big problems.  Something passed the validator that was
         * not supposed to pass.  Do Something About It!
         */         
         fprintf(stderr,"Validation failure in fixed point parsing."  
                        " Contact author.\n");

      }
      cur = cur->next;
   }

   fprintf(stdout,"Number fixed points %d\n\n", gdata->nFPoints);

   return NULL;

}  /*  close parseFixedpointlist() */


static void *
parseMeasuredpointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{   
   DDAPoint * point;
  /* Make the compiler link the floating point libraries. */
   double temp[2] = {0.0};
   char * tempstring;
   int checkval;

   fprintf(stdout, "Parsing Measured point list\n");

  /* Don't care about parent */
   cur = cur->childs;

   while (cur != NULL)
   {
      if ((!strcmp(cur->name, "Point")) )//&& (cur->ns == ns))
      {

         point = getNewPoint();
         point->type = measured; //  enum for measured 
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         if (tempstring == NULL)
         {  
            fprintf(stdout,"Warning: Empty measured point value\n");
         }
         else 
         {
            fprintf(stdout,"%s\n", tempstring);
            checkval = sscanf(tempstring,"%lf%lf",&temp[0],&temp[1]); 
            if (checkval == 2)
            { 
               fprintf(stdout,"%f %f\n",temp[0],temp[1]);  
               point->x = temp[0];
               point->y = temp[1];
               dl_insert_b(pointlist,point);
               gdata->nMPoints++;
            }
            else  /* Throw an error or warning */
            {
               fprintf(stdout,"Warning: Empty measured points tag.\n");
            }
         } 
      }
      else
      { 
        /* Big problems.  Something passed the validator that was
         * not supposed to pass.  Do Something About It!
         */         
         fprintf(stderr,"Validation failure in measured point parsing."  
                        " Contact author.\n");
      }
      cur = cur->next;
   }

   fprintf(stdout, "\n");

   return NULL;

}  /*  close parseMeasuredpointlist() */

static void *
parseSeismicpointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{   
   DDAPoint * point;
  /* Make the compiler link the floating point libraries. */
   double temp[2] = {0.0};
   char * tempstring;
   int checkval;

   fprintf(stdout, "Parsing Seismic point list\n");

  /* Don't care about parent */
   cur = cur->childs;

   while (cur != NULL)
   {
      if ((!strcmp(cur->name, "Point")) )//&& (cur->ns == ns))
      {

         point = getNewPoint();
         point->type = seismic; //  enum for measured 
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         if (tempstring == NULL)
         {  
            fprintf(stdout,"Warning: Empty seismic point value\n");
         }
         else 
         {
            fprintf(stdout,"%s\n", tempstring);
            checkval = sscanf(tempstring,"%lf%lf",&temp[0],&temp[1]); 
            if (checkval == 2)
            { 
               fprintf(stdout,"%f %f\n",temp[0],temp[1]);  
               point->x = temp[0];
               point->y = temp[1];
               dl_insert_b(spointlist,point);
               gdata->nSPoints++;
            }
            else  /* Throw an error or warning */
            {
               fprintf(stdout,"Warning: Empty measured points tag.\n");
            }
         } 
      }
      else
      { 
        /* Big problems.  Something passed the validator that was
         * not supposed to pass.  Do Something About It!
         */         
         fprintf(stderr,"Validation failure in measured point parsing."  
                        " Contact author.\n");
      }
      cur = cur->next;
   }

   fprintf(stdout, "\n");

   return NULL;

}  /*  close parseSeismicpointlist() */



static void *
parseLoadpointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   DDAPoint * point;
  /* Make the compiler link the floating point libraries. */
   double temp[2] = {0.0};
   char * tempstring;
   int checkval;

   fprintf(stdout, "Parsing Load point list\n");

  /* Don't care about parent */
   cur = cur->childs;

   while (cur != NULL)
   {
      if ((!strcmp(cur->name, "Point")) )//&& (cur->ns == ns))
      {

         point = getNewPoint();
         point->type = load; //  enum for load 
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         if (tempstring == NULL)
         {  
            fprintf(stdout,"Warning: Empty load point value\n");
         }
         else 
         {
            fprintf(stdout,"%s\n", tempstring);
            checkval = sscanf(tempstring,"%lf%lf",&temp[0],&temp[1]); 
            if (checkval == 2)
            { 
               fprintf(stdout,"%f %f\n",temp[0],temp[1]);  
               point->x = temp[0];
               point->y = temp[1];
               dl_insert_b(pointlist,point);
               gdata->nLPoints++;
            }
            else  /* Throw an error or warning */
            {
               fprintf(stdout,"Warning: Empty load points tag.\n");
            }
         } 
      }
      else
      { 
        /* Big problems.  Something passed the validator that was
         * not supposed to pass.  Do Something About It!
         */         
         fprintf(stderr,"Validation failure in load point parsing."  
                        " Contact author.\n");

      }
      cur = cur->next;
   }

   fprintf(stdout,"\n");

   return NULL;

}  /*  close parseLoadpointlist() */



static void *
parseHolepointlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{

   DDAPoint * point;
  /* Make the compiler link the floating point libraries. */
   double temp[2] = {0.0};
   char * tempstring;
   int checkval;

   fprintf(stdout, "Parsing Hole point list\n");

  /* Don't care about parent */
   cur = cur->childs;

   while (cur != NULL)
   {
      if ((!strcmp(cur->name, "Point")) )//&& (cur->ns == ns))
      {

         point = getNewPoint();
         point->type = hole; //  enum for hole
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         if (tempstring == NULL)
         {  
            fprintf(stdout,"Warning: Empty hole point value\n");
         }
         else 
         {
            fprintf(stdout,"%s\n", tempstring);
            checkval = sscanf(tempstring,"%lf%lf",&temp[0],&temp[1]); 
            if (checkval == 2)
            { 
               fprintf(stdout,"%f %f\n",temp[0],temp[1]);  
               point->x = temp[0];
               point->y = temp[1];
               dl_insert_b(pointlist,point);
               gdata->nHPoints++;
            }
            else  /* Throw an error or warning */
            {
               fprintf(stdout,"Warning: Empty hole points tag.\n");
            }
         } 
      }
      else
      { 
        /* Big problems.  Something passed the validator that was
         * not supposed to pass.  Do Something About It!
         */         
         fprintf(stderr,"Validation failure in hole point parsing."  
                        " Contact author.\n");
      }
      cur = cur->next;
   }

   fprintf(stdout,"\n");

   return NULL;

}  /*  close parseHolepointlist() */


/* FIXME: The bolt properties used to be handled in the geometry 
 * file.  To make the file input system more uniform, bolt properties
 * are now handled in the analysis input file.  There needs to be
 * a function written that will transfer the bolt properties
 * from the analysis side to the geometry side.
 */ 
static void *
parseBoltlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   BOLT * bolt;
  /* Make the compiler link the floating point libraries. */
   double temp[4] = {0.0};
   char * tempstring;
   int checkval;

   gdata->nBolts = 0;

   fprintf(stdout, "Parsing Bolt list\n");

  /* Don't care about parent */
   cur = cur->childs;

   while (cur != NULL)
   {
      if ((!strcmp(cur->name, "Bolt")) )//&& (cur->ns == ns))
      {

         bolt = getNewBolt();
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         if (tempstring == NULL)
         {  
            fprintf(stdout,"Warning: Empty bolts tag\n");
         }
         else 
         {
            fprintf(stdout,"%s\n", tempstring);
            checkval = sscanf(tempstring,"%lf%lf%lf%lf",&temp[0],&temp[1],
                              &temp[2], &temp[3]); 
            if (checkval == 4)
            { 
               fprintf(stdout,"%f %f\n",temp[0],temp[1],temp[2],temp[3]);  
               bolt->epx1 = temp[0];
               bolt->epy1 = temp[1];
               bolt->epx2 = temp[2];
               bolt->epy2 = temp[3];
               dl_insert_b(boltlist,bolt);
               gdata->nBolts++;
            }
            else  /* Throw an error or warning */
            {
               fprintf(stdout,"Warning: Empty bolts tag.\n");
            }
         } 
      }
      else
      { 
        /* Big problems.  Something passed the validator that was
         * not supposed to pass.  Do Something About It!
         */         
         fprintf(stderr,"Validation failure in bolt parsing."  
                        " Contact author.\n");

      }

      cur = cur->next;
   }

   fprintf(stdout,"\n");


   return NULL;

}  /*  close parseBoltlist() */


void *
parseMatlinelist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   DDALine * line;
  /* Make the compiler link the floating point libraries. */
   double temp[4] = {0.0};
   char * tempstring;
   int checkval;

#if _DEBUG
#if DDA_FOR_WINDOWS
   fprintf(stdout, "Parsing Matline list\n");
   //iface->displaymessage("Parsing Matline list");
#endif
#endif

  /* Don't care about parent */
   cur = cur->childs;

   while (cur != NULL)
   {
      if ((!strcmp(cur->name, "Line")) )//&& (cur->ns == ns))
      {

         line = getNewLine();
        /* Added this to handle material line types. */
         line->type = atoi(xmlGetProp(cur, "type"));

         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         if (tempstring == NULL)
         {  
            fprintf(stdout,"Warning: Empty material line tag\n");
         }
         else 
         {
            fprintf(stdout,"%s\n", tempstring);
            checkval = sscanf(tempstring,"%lf%lf%lf%lf",&temp[0],&temp[1],
                              &temp[2], &temp[3]); 
            if (checkval == 4)
            { 
               fprintf(stdout,"%f %f\n",temp[0],temp[1],temp[2],temp[3]);  
               line->x1 = temp[0];
               line->y1 = temp[1];
               line->x2 = temp[2];
               line->y2 = temp[3];
               dl_insert_b(matlinelist,line);
               gdata->nMatLines++;
            }
            else  /* Throw an error or warning */
            {
               iface->displaymessage("Warning: Malformed material lines tag.\n");
            }
         } 
      }
      else
      { 
        /* Big problems.  Something passed the validator that was
         * not supposed to pass.  Do Something About It!
         */         
         iface->displaymessage("Validation failure in material lines parsing."  
                                      " Contact author.\n");

      }

      cur = cur->next;
   }

   fprintf(stdout,"\n");



   return NULL;

}  /*  close parseMatlinelist() */

void * 
parseJointset(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur)
{


return NULL;

}  /* close parseJointset() */



void *
parseBoundlist(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   DDALine * line;
  /* Make the compiler link the floating point libraries. */
   double temp[4] = {0.0};
   char * tempstring;
   int checkval;
   fprintf(stdout, "Parsing Bounding list\n");

  /* Don't care about parent */
   cur = cur->childs;

   while (cur != NULL)
   {
      if ((!strcmp(cur->name, "Line")) )//&& (cur->ns == ns))
      {

         line = getNewLine();
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         if (tempstring == NULL)
         {  
            fprintf(stdout,"Warning: Empty boundary line tag\n");
         }
         else 
         {
            fprintf(stdout,"%s\n", tempstring);
            checkval = sscanf(tempstring,"%lf%lf%lf%lf",&temp[0],&temp[1],
                              &temp[2], &temp[3]); 
            if (checkval == 4)
            { 
               fprintf(stdout,"%f %f\n",temp[0],temp[1],temp[2],temp[3]);  
               line->x1 = temp[0];
               line->y1 = temp[1];
               line->x2 = temp[2];
               line->y2 = temp[3];
               dl_insert_b(boundlist,line);
               gdata->nBoundaryLines++;
            }
            else  /* Throw an error or warning */
            {
               fprintf(stdout,"Warning: Empty boundary lines tag.\n");
            }
         } 
      }
      else
      { 
        /* Big problems.  Something passed the validator that was
         * not supposed to pass.  Do Something About It!
         */         
         fprintf(stderr,"Validation failure in boundary lines parsing."  
                        " Contact author.\n");

      }

      cur = cur->next;
   }

   fprintf(stdout,"\n");
   return NULL;

}  /*  close parseBoundlist() */

Joint *
getNewJoint(void)
{
   Joint * j;
   //fprintf(stdout,"Getting new joint\n");
   j = (Joint *)calloc(1,sizeof(Joint));
   memset((void *)j,0xDA,sizeof(Joint));
   return j;

}  /*  close getNewJoint() */

static DDAPoint *
getNewPoint(void)
{
   DDAPoint * p;
   //fprintf(stdout,"Getting new point\n");
   p = (DDAPoint *)calloc(1,sizeof(DDAPoint));

  /* FIXME: Because DDA uses the value of "0" to designate 
   * a point type, in this case a fixed point, returning the 
   * calloced point ensures that the the fixed points code
   * will fire and run.  This is bad.  What needs to happen is 
   * that all the memory in the struct should be set to garbage
   * values to aid in debugging.  Lets try that now...
   */
   memset((void *)p,0xDA,sizeof(DDAPoint));

   return p;

}  /*  close getNewPoint() */

DDALine *
getNewLine(void)
{
   DDALine * l;
   //fprintf(stdout,"Getting new line\n");
   l = (DDALine *)calloc(1,sizeof(DDALine));
   memset((void *)l,0xDA,sizeof(DDALine));
   return l;

}  /*  close getNewLine() */


BOLT *
getNewBolt(void)
{
   BOLT * b;
   //fprintf(stdout,"Getting new bolt\n");
   b = (BOLT *)calloc(1,sizeof(BOLT));
   memset((void *)b,0xDA,sizeof(BOLT));
   return b;

}  /*  close getNewBolt() */

void
transferGData(void)
{

   transferJointlistToGeomStruct(gdata,jointlist);
   transferPointlistToGeomStruct(gdata,pointlist);
   transferBoltlistToGeomStruct(gdata,boltlist);
   transferMatlinelistToGeomStruct(gdata, matlinelist);

}  /* close transferGData() */

/* FIXME: Combine this with code in the geometry dialog.
 */
void
transferJointlistToGeomStruct(Geometrydata * gd, 
                              JOINTLIST * jointlist)
{
   int i = 0;
   int numjoints;
   JOINTLIST * ptr;
   Joint * jtmp;
  /* just in case... */
   numjoints = dlist_length(jointlist);
   gd->nJoints = numjoints;
   gd->jointsize1 = numjoints+1;
   gd->jointsize2 = 6;
   gd->joints = DoubMat2DGetMem(gd->jointsize1, gd->jointsize2);

  /* Draw joints stored in jointlist.
   */
   M_dl_traverse(ptr, jointlist)
   {
      jtmp = ptr->val;

      gd->joints[i+1][1] = jtmp->epx1;
      gd->joints[i+1][2] = jtmp->epy1; 
      gd->joints[i+1][3] = jtmp->epx2;
      gd->joints[i+1][4] = jtmp->epy2;
      gd->joints[i+1][5] = jtmp->type;
      i++;
   } 


}  /* close  transferJointlistToGeomStruct() */


void
transferPointlistToGeomStruct(Geometrydata * gd, 
                              POINTLIST * pointlist)
{
  /* WARNING!!!  The counter i is carried across four
   * loops for cumulative indexing.
   */
  /* WARNING!!! WARNING!!! WARNING!!!  The "points"
   * array has to be loaded in the correct order, namely
   * fixed, load, measure, hole.  Else extremely subtle
   * bugs will result from out of order points.
   */
   int i = 0;
   int n5;
   POINTLIST * ptr;
   DDAPoint * ptmp;
   DDALine * ltmp;  

  /* The edge-node distance probably ought to 
   * be set as a part of the CAD scaling.
   */
   //gd->e00 = 0.01;
   
   gd->pointCount = (gd->nFPoints) + (gd->nLPoints) + (gd->nMPoints);
   gd->nPoints = gd->nFPoints + gd->nMPoints + gd->nLPoints + gd->nHPoints;

  /* Horrible horrible kludge. */
	 	n5 =  (gd->nFPoints*(15+1))+gd->nLPoints+gd->nMPoints+gd->nHPoints+1;
	 	gd->pointsize1=n5+1;
	 	/* gid->pointsize2=5; */
   /* gid->pointsize2 = 7; */ /* Add x, y displacement values per ts. */
   //gd->pointsize2 = 9;  /* add cum disp in [][7] and [][8]  */
   //gid->points = DoubMat2DGetMem(gid->pointsize1, gid->pointsize2);

   //numpoints = dlist_length(pointlist);
   //gd->nPoints = numpoints;
   //gd->pointsize1 = numpoints+1;
   gd->pointsize2 = 9;
   gd->points = DoubMat2DGetMem(gd->pointsize1, gd->pointsize2);

  /* Massive kludgery here induced by the way that the 
   * points are ordered in the points array.  The order
   * and the type are not the same.  The technique is 
   * to traverse the list once for each type of point,
   * only storing one type at a time, and in the correct
   * order in the points array.  First up is FIXED:
   */

  /* This is really botched up.  The fixed point list has to 
   * be handled separately from the other lists because the
   * data structure is different.
   */
  /* FIXME: Move this to an fpointlist */
   M_dl_traverse(ptr, fpointlist)
   {
      ltmp = ptr->val;
      if (ltmp->type == fixed)
      {
         gd->points[i+1][0] = ltmp->type;
         gd->points[i+1][1] = ltmp->x1;
         gd->points[i+1][2] = ltmp->y1; 
         gd->points[i+1][3] = ltmp->x2;
         gd->points[i+1][4] = ltmp->y2;

        /* FIXME: This will conflict with other uses where the 
         * number of points is not known.
         */
         //gd->nFPoints++;
         i++;
      }
   } 

  /* Now store LOAD points, i carries forward */
   M_dl_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == load)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x;
         gd->points[i+1][2] = ptmp->y; 
     /* FIXME: This will conflict with other uses where the 
      * number of points is not known.
      */
         //gd->nLPoints++;
         i++;
      }
   }
   
  /* MEASURED points */
   M_dl_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == measured)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x;
         gd->points[i+1][2] = ptmp->y; 
     /* FIXME: This will conflict with other uses where the 
      * number of points is not known.
      */
         //gd->nMPoints++;
         i++;
      }
   }   

  /* HOLE points */
   M_dl_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == hole)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x;
         gd->points[i+1][2] = ptmp->y; 
     /* FIXME: This will conflict with other uses where the 
      * number of points is not known.
      */
         //gd->nHPoints++;
         i++;
      }
   } 

  /* The seismic points are independent, so we can try to 
   * do those correctly.
   */
   if (gd->nSPoints > 0)
      gd->seispoints = spointlist;

}  /* close transferPointlistToGeomStruct()  */


void
transferBoltlistToGeomStruct(Geometrydata * gd, 
                             BOLTLIST * boltlist)
{
   int i = 0;
   int numbolts;
   BOLTLIST * ptr;
   BOLT * btmp;


   numbolts = dlist_length(boltlist);
   //if (numbolts == 0)
   //   return;

   fprintf(stdout,"Number of bolts: %d\n",numbolts);

   gd->nBolts = numbolts;
   gd->rockboltsize1 = numbolts+1;
   gd->rockboltsize2 = 14;
   gd->rockbolts = DoubMat2DGetMem(gd->rockboltsize1, gd->rockboltsize2);


   M_dl_traverse(ptr, boltlist)
   {
      btmp = ptr->val;

      gd->rockbolts[i][1] = btmp->epx1;
      gd->rockbolts[i][2] = btmp->epy1; 
      gd->rockbolts[i][3] = btmp->epx2;
      gd->rockbolts[i][4] = btmp->epy2;
      //gd->rockbolts[i+1][5] = btmp->type;
      i++;
   } 

   //if (gid->nBolts > 0)
   gd->origbolts = clone2DMatDoub(gd->rockbolts, gd->rockboltsize1, gd->rockboltsize2);

}  /* close transferBoltlistToGeomStruct()  */

//#if JKHJKHL
static double **
DoubMat2DGetMem(int n, int m)
{
   int i;
   double **x;

   assert ( (m!=0) && (n!=0) );

   //x = (double **)malloc(sizeof(double *)*n);
   x = (double **)calloc(n,sizeof(double *));
   if (x == NULL)
      return NULL;

   for ( i = 0; i < n; ++i)
   {
      //x[i] = (double *)malloc(sizeof(double)*m);
      x[i] = (double *)calloc(m,sizeof(double));
      if(x[i] == NULL)
         return NULL;
      //else 
         //memset(x[i], 0xDDA, m);
   }

   return x;

}  /* Close DoubMat2DGetMem().  */
//#endif

static void 
transferMatlinelistToGeomStruct(Geometrydata * gd, MATLINELIST * matlinelist)
{

   int i = 0;
   int nummatlines;
   MATLINELIST * ptr;
   DDALine * ltmp;
  /* just in case... */
   nummatlines = dlist_length(matlinelist);
   gd->nMatLines = nummatlines;
   
   gd->matlinesize1 = nummatlines+1;
   gd->matlinesize2 = 6;
   gd->matlines = DoubMat2DGetMem(gd->jointsize1, gd->jointsize2);

  /* Draw joints stored in jointlist.
   */
   M_dl_traverse(ptr, matlinelist)
   {
      ltmp = ptr->val;

      gd->matlines[i+1][1] = ltmp->x1;
      gd->matlines[i+1][2] = ltmp->y1; 
      gd->matlines[i+1][3] = ltmp->x2;
      gd->matlines[i+1][4] = ltmp->y2;
      gd->matlines[i+1][5] = ltmp->type;
      i++;
   } 

}  /* close transferMatlinelistToGeomStruct() */


