
/*
 * analysisddaml.c 
 *
 * An example program for DDA analysis input file  XML format.
 *
 * Copyright: http://www.gnu.org/copyleft/lesser.html
 *
 * David M. Doolin  doolin@ce.berkeley.edu
 *
 * $Author: doolin $
 * $Date: 2001/09/15 14:14:34 $
 * $Source: /cvsroot/dda/ntdda/src/analysisddaml.c,v $
 * $Revision: 1.2 $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "ddaxmlparse.h"
#include "ddamemory.h"
#include "analysisdata.h"
#include "interface.h"
#include "timehistory.h"
//#include "ddatypes.h"

/* Mostly for debugging windows. */
static char mess[180];

/*  GAAAKKK!!!  */
extern InterFace * iface;


typedef DList LOADPOINTLIST;
typedef DList JOINTMATLIST;
typedef DList BOLTMATLIST; 
typedef DList BLOCKMATLIST; 

static LOADPOINTLIST * loadpointlist;
static JOINTMATLIST * jointmatlist;
static BLOCKMATLIST * blockmatlist;
static BOLTMATLIST * boltmatlist;

static xmlNsPtr nspace;

//static JointMat *jmat, *jmatOld = NULL;
//static BlockMat *bmat, *bmatOld = NULL;

static Analysisdata * parseAnalysis(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);

static void * parseAutotimestep(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseAnalysistype(xmlDocPtr doc, xmlNsPtr n2, xmlNodePtr cur);
static void * parseNumtimesteps(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parsePlanestrain(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseJointproperties(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseBlockmaterial(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseLoadpoints(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseBoltproperties(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);

static void * parseGravity(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseAutopenalty(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseRotation(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);

static void * parseAConstants(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseOpencloselimit(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseSaveinterval(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseMaxtimestep(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseMintimestep(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseMaxdisplacement(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseTimehistory(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
static void * parseContactDamping(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);


static JointMat * getNewJointMat(void);
static BlockMat * getNewBlockMat(void);
static BoltMat * getNewBoltMat(void);
static LOADPOINT * getNewLoadpoint(void);

void transferAData(void);
void transferJointMatlistToAStruct(Analysisdata * adata, JOINTMATLIST * jointmatlist);
void transferLoadpointlistToAStruct(Analysisdata * adata, LOADPOINTLIST * loadpointlist);
void transferBoltMatlistToAStruct(Analysisdata * adata, BOLTMATLIST * boltmatlist);
void transferBlockMatlistToAStruct(Analysisdata * adata, BLOCKMATLIST * blockmatlist);
static double ** DoubMat2DGetMem(int n, int m);


static ddaboolean checkAnalysisDoc(xmlDocPtr doc);
static void initializeALists(void);


typedef struct _akwdtab
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
   }parsefn;
}
AKWDTAB;

Analysisdata * adata;

AKWDTAB atab_type[] = {
   {"Rotation", node, *parseRotation},
   {"Gravity", 0, *parseGravity},
   {"Autotimestep", 0, *parseAutotimestep},
   {"Autopenalty", 0, *parseAutopenalty},
   {"Analysistype",0, *parseAnalysistype},
   {"Numtimesteps", 0, *parseNumtimesteps},
   {"Planestrain", 0, *parsePlanestrain},
   {"Analysis", 0, *parseAnalysis},
   {"Jointproperties",0, *parseJointproperties},
   {"Loadpointlist",0, *parseLoadpoints},
   {"Blockmaterial",0, *parseBlockmaterial},
   {"Boltproperties",0, *parseBoltproperties},
   {"AConstants",0, *parseAConstants},
   {"Opencloselimit",0, *parseOpencloselimit},
   {"Saveinterval",0, *parseSaveinterval},
   {"Maxtimestep",0, *parseMaxtimestep}, 
   {"Mintimestep",0, *parseMintimestep}, 
   {"Maxdisplacement",0,*parseMaxdisplacement},
   {"Timehistory",0,*parseTimehistory},
   {"ContactDamping",0,*parseContactDamping},
   {NULL, 0, 0}			/* Ends a scanning loop.  See comment above. */
};



#ifdef STANDALONE
int 
main(int argc, char **argv) 
{
   int i;
   fprintf(stderr,"Parsing analysis data...\n");

   for (i = 1; i < argc ; i++) 
   {
	     adata = XMLparseDDA_Analysis_File(argv[i]);
   }

   return(0);
}  /* close main() */

#endif /* STANDALONE */



Analysisdata *
XMLparseDDA_Analysis_File(char *filename) 
{
   xmlDocPtr doc;
   xmlNsPtr ns;
   xmlNodePtr cur;
   Analysisdata * ad;

   xmlDoValidityCheckingDefaultValue = 1;

  /*
   * build an XML tree from the file;
   */
   doc = xmlParseFile(filename);

   if(!checkAnalysisDoc(doc))
   {
      iface->displaymessage("Bad DDAML document");
      return NULL;
   }

  /* This is where we need an exception */
   //if (adata == NULL)
   //return NULL;

   cur = doc->root;
   ns = nspace;

  /*
   * Now, walk the tree.  All the following code will
   * be replaced with keyscanning.  Initially, do not
   * check for keyscanning failure.  Let the dtd validator
   * handle that.  Later, a few simple checks can be added,
   * just to handle exceptional cases.
   */
  /* FIXME:  This needs to be handled as an exception to
   * protect against dereferencing a null pointer.
   */
   cur = cur->childs->next;

   ad = parseAnalysis(doc, ns, cur);

   assert(ad != NULL);
   
   return(ad);

}  /* close XMLparseDDA_Analysis_File() */


ddaboolean
checkAnalysisDoc(xmlDocPtr doc)
{
   xmlNodePtr cur;
    
   if (doc == NULL) 
   {
     /* Log an error, then */
      iface->displaymessage("NULL DDAML document, probably malformed XML.  Check tags");
      return FALSE;
   }

  /*
   * Check the document is of the right kind
   */
   cur = doc->root;
    
   if (cur == NULL) 
   {
      fprintf(stderr,"empty analysis document\n");
      iface->displaymessage("Empty DDAML document");
	   xmlFreeDoc(doc);
	   return FALSE;
   }

  /*
   * FIXME: Rewrite this call to the analysis data structure 
   * intilializer.
   */
   //adata = (Analysisdata *) malloc(sizeof(Analysisdata));
   //memset(adata, 0, sizeof(Analysisdata));

   //if (adata == NULL) 
   //{
   //   fprintf(stderr,"out of memory for analysis data\n");
	  //   xmlFreeDoc(doc);
   //  	return(NULL);
   //}
   
   //adata->namespace = xmlSearchNsByHref(doc, cur, "http://www.tsoft.com/~bdoolin/dda");
   nspace = xmlSearchNsByHref(doc, cur, "http://www.tsoft.com/~bdoolin/dda");

   if (nspace == NULL) 
   {
      fprintf(stderr,"Namespace error, check URL\n");
      iface->displaymessage("DDAML namespace error, check URL");
	   xmlFreeDoc(doc);
	   return FALSE;
   }
    
   if (strcmp(cur->name, "DDA")) 
   {
      fprintf(stderr,"document of the wrong type, root node != DDA");
      iface->displaymessage("Bad root node, file is not a DDAML.");
	   xmlFreeDoc(doc);
	   return FALSE;
   }

   return TRUE;

}  /* close checkAnalysisDoc() */


Analysisdata *
parseAnalysis(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   int i = 0;
   //FILE * outtestfile;

   //outtestfile = fopen("testemxml.ana","w");

   fprintf (stderr,"From parseAnalysis...\n");
  /*
   * allocate the struct
   */
   adata = initAnalysisData(); //(Analysisdata *) malloc(sizeof(Analysisdata));

   if (adata == NULL) 
   {
      fprintf(stderr,"out of memory for analysis data\n");
     	return(NULL);
   }
   
   //memset(adata, 0, sizeof(Analysisdata));

  /* Easiest to parse into a list, then count 
   * and grab array memory, then copy data into 
   * arrays.  This will be good, because I will 
   * now have lists of elements that can eventually
   * be used in the main code.
   */
   initializeALists(); 

   

  /* We don't care what the top level element name is */
   cur = cur->childs;

   while (cur != NULL) 
   {
      i = 0;
      while (atab_type[i].kwd)
      {
         if ((!strcmp(atab_type[i].kwd,cur->name)) ) //&& (cur->ns == ns)) 
         {
            fprintf(stderr,"A Keyscan loop: Current name %s:\n", cur->name);
            switch(atab_type[i].ktok)
            {
               case node:
	                 atab_type[i].parsefn.nodeparse(doc,ns,cur);
               break;

               case string:
	                 atab_type[i].parsefn.stringparse(doc,cur,1);
               break;

               case prop:
	                 atab_type[i].parsefn.propparse(doc,cur,1);
               break;

               default:;
               break;
            }  /* end switch on node type */
         }
         i++;
      }   //close key scan loop 

      cur = cur->next;

   }  /* end loop over current pointer */

  /* Somewhere here there has to be a function for transferring
   * data from the dlists to the arrays.  Might find something
   * in the draw dialog box that does this.
   */
   transferAData();
  /* Pushing this into a separate function allows very clean
   * and robust implementation.  What this function does is
   * ensure that the data is "reasonable".  Example: negative
   * time steps would at the very least trigger a warning, if not
   * out right fail.
   */
   //rangecheckAnalysisdata(adata);
  /* Write it out and see if we can load it... */
   //dumpAnalysisData1(adata, outtestfile);
   //fclose(outtestfile);
  /* Now clean everything up. */
   //freeDlistsAndStuff();

   //assert(adata != NULL);

  /* kludge this in here for the moment... */
   adata->gravaccel = 9.81;

   return adata;

}  /* close parseAnalysis() */


void 
initializeALists()
{
   jointmatlist = make_dl();
   blockmatlist = make_dl();
   boltmatlist = make_dl();
   loadpointlist = make_dl();

}  /* close initializeLists() */


void 
transferAData(void)
{

   transferJointMatlistToAStruct(adata,jointmatlist);
   transferLoadpointlistToAStruct(adata, loadpointlist);
   transferBoltMatlistToAStruct(adata,boltmatlist);
   transferBlockMatlistToAStruct(adata, blockmatlist);
}  /* close transferAData() */


void 
transferJointMatlistToAStruct(Analysisdata * ad, JOINTMATLIST * jointmatlist)
{
   int i = 0;
   int njmat;
   JOINTMATLIST * ptr;
   JointMat * jmtmp;

  /* WARNING: Type attribute of xml tag is ignored for now.
   * Joint materials should be listed in order of occurrence
   * in the xml file.
   */
   njmat = dlist_length(jointmatlist);
   ad->nJointMats = njmat;
   ad->phicohesionsize1 = njmat + 1;
   ad->phicohesionsize2 = 3;
   ad->phiCohesion = DoubMat2DGetMem(adata->phicohesionsize1,adata->phicohesionsize2);

  /* WARNING: This assumes that joint types are listed in order 
   * of occurrence in xml file.  The type attribute is ignored
   * for now.
   */
   M_dl_traverse(ptr, jointmatlist)
   {
      jmtmp = ptr->val;
      ad->phiCohesion[i+1][0] = jmtmp->fric;
      ad->phiCohesion[i+1][1] = jmtmp->coh; 
      ad->phiCohesion[i+1][2] = jmtmp->tens;
      i++;
   }
    
}  /*  close transferJointMatListToAStruct() */


void 
transferLoadpointlistToAStruct(Analysisdata * ad, LOADPOINTLIST * loadpointlist)
{
   int i = 0;
   int s1,s2;
   double ** lp;
   
   LOADPOINT * lptmp;
   LOADPOINTLIST * ptr;
  
   ad->nLPoints = dlist_length(loadpointlist);

   if (ad->nLPoints == 0)
      return;

   ad->loadpoints = (LOADPOINT *) calloc (ad->nLPoints, sizeof (LOADPOINT));
  /* WARNING: This assumes that load points are listed in order 
   * of occurrence in xml file.  
   */
   M_dl_traverse(ptr, loadpointlist)
   {
      lptmp = ptr->val;
		s1 = lptmp->loadpointsize1;
		s2 = lptmp->loadpointsize2;
      lp = lptmp->vals;

      ad->loadpoints[i].loadpointsize1 = s1;
      ad->loadpoints[i].loadpointsize2 = s2;
      ad->loadpoints[i].vals = clone2DMatDoub(lp, s1, s2);
      i++;
   }
   
}  /*  close transferLoadPointlistToAStruct() */



void 
transferBoltMatlistToAStruct(Analysisdata * ad, BOLTMATLIST * boltmatlist)
{

   int i = 0;
   int nbmat;
   BOLTMATLIST * ptr;
   BoltMat * bmtmp;
  /* WARNING: Type attribute of xml tag is ignored for now.
   * Bolt materials should be listed in order of occurrence
   * in the xml file.
   */
   nbmat = dlist_length(boltmatlist);
   if (nbmat == 0)
      return;

   ad->nBoltMats = nbmat;
   ad->boltmatsize1 = nbmat;
   ad->boltmatsize2 = 3;
   ad->boltmats = DoubMat2DGetMem(adata->boltmatsize1,adata->boltmatsize2);
  /* WARNING: This assumes that joint types are listed in order 
   * of occurrence in xml file.  The type attribute is ignored
   * for now.
   */
   M_dl_traverse(ptr, boltmatlist)
   {
      bmtmp = ptr->val;
      ad->boltmats[i][0] = bmtmp->e00;
      ad->boltmats[i][1] = bmtmp->t0; 
      ad->boltmats[i][2] = bmtmp->f0;
      i++;
   }

}  /*  close transferBoltMatlistToAStruct() */


void 
transferBlockMatlistToAStruct(Analysisdata * ad, BLOCKMATLIST * blockmatlist)
{
   int i = 0;
   int nbmat;
   BLOCKMATLIST * ptr;
   BlockMat * bmtmp;

  /* WARNING: Type attribute of xml tag is ignored for now.
   * Joint materials should be listed in order of occurrence
   * in the xml file.
   */
   nbmat = dlist_length(blockmatlist);
   ad->nBlockMats = nbmat;
   ad->materialpropsize1 = nbmat + 1;
   ad->materialpropsize2 = 14;
   ad->materialProps = DoubMat2DGetMem(adata->materialpropsize1,adata->materialpropsize2);

  /* WARNING: This assumes that joint types are listed in order 
   * of occurrence in xml file.  The type attribute is ignored
   * for now.
   */
   M_dl_traverse(ptr, blockmatlist)
   {
      bmtmp = ptr->val;
      ad->materialProps[i+1][0] = bmtmp->dens;
      ad->materialProps[i+1][1] = bmtmp->wt; 
      ad->materialProps[i+1][2] = bmtmp->ymod;
      ad->materialProps[i+1][3] = bmtmp->pois;
      ad->materialProps[i+1][4] = bmtmp->iss[0];
      ad->materialProps[i+1][5] = bmtmp->iss[1];
      ad->materialProps[i+1][6] = bmtmp->iss[2];
      ad->materialProps[i+1][7] = bmtmp->ivel[0];
      ad->materialProps[i+1][8] = bmtmp->ivel[1];
      ad->materialProps[i+1][9] = bmtmp->ivel[2];
      ad->materialProps[i+1][10] = bmtmp->ist[0];
      ad->materialProps[i+1][11] = bmtmp->ist[1];
      ad->materialProps[i+1][12] = bmtmp->ist[2];
      ad->materialProps[i+1][13] = bmtmp->damping;
      i++;
   }
}  /*  close transferBlockMatlistToAStruct() */



static void *
parseRotation(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
  /* Need to grab a tempstring, then switch for different 
   * types of rotation,
   */
   char * temp;

   fprintf(stdout,"Parsing rotation...\n");
   
   temp = xmlGetProp(cur,"type");

   if (!strcmp(temp, "linear"))
      adata->rotationflag = 0;
   else if (!strcmp(temp, "secondorder"))
      adata->rotationflag = 1;
   else if (!strcmp(temp, "exact"))
      adata->rotationflag = 1;
   else
      // big problem
      ;  // do nothing for now


   return NULL;

}  /*close parseRotation() */


void *
parseGravity(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{

   fprintf(stdout,"Parsing gravity...\n");

  /* This will be a struct at some point */
   if (!strcmp("yes",xmlGetProp(cur,"flag")) )
   {
      Gravity * grav;
     /* Set the flag: */
      adata->gravityflag = 1;
     /* Collect the parameters controlling the 
      * the gravity time steps, convergence, etc. 
      */
      grav  = newGravity();  //(Gravity *)calloc(1,sizeof(Gravity));
     
      fprintf(stderr,"Inside gravity\n");

      cur = cur->childs;

      while (cur != NULL) 
      {
         if ((!strcmp(cur->name, "Gravitysteplimit")) )// && (cur->ns == ns))
            grav->gravTurnonLimit = atoi(xmlGetProp(cur,"val"));
         if ((!strcmp(cur->name, "Normaltolerance")) ) //&& (cur->ns == ns))
            grav->nforcetolerance = atof(xmlGetProp(cur,"val"));
         if ((!strcmp(cur->name, "Sheartolerance")) ) //&& (cur->ns == ns))
            grav->sforcetolerance = atof(xmlGetProp(cur,"val"));
         if ((!strcmp(cur->name, "Residualtolerance")) ) //&& (cur->ns == ns))
            grav->residual_tol = atof(xmlGetProp(cur,"val"));
         cur = cur->next;

      }  /* end loop over current pointer */

      adata->gravity = grav;
   }
   else
   {
      adata->gravityflag = 0;
   }

   fprintf(stdout,"gravity flag: %d\n\n", adata->gravityflag);

   return NULL;

}  /*close parseGravity() */


void *
parseAutotimestep(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   fprintf(stdout,"Parsing auto time step...\n");

  /* This will be a struct at some point */
   if (!strcmp("yes",xmlGetProp(cur,"flag")) )
   {
     /* Set the flag: */
      adata->autotimestepflag = 1;
   }
   else
   {
      adata->autotimestepflag = 0;
   }

   fprintf(stdout,"autotimestep flag: %d\n\n", adata->autotimestepflag);

   return NULL;

}  /*close parseAutoTimestep() */


void *
parseAnalysistype(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
  
  /* This will be a struct at some point */
   if (!strcmp("dynamic",xmlGetProp(cur,"type")) )
   {
     /* Set the flag: */
      adata->analysistype = 1;
   }
   else
   {
      adata->analysistype = 0;
   }

   return NULL;

}  /*close parseAnalysistype() */

void *
parseNumtimesteps(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{

   adata->nTimeSteps = atoi(xmlGetProp(cur,"timesteps"));

   return NULL;

}  /*close parseNumtimesteps() */


void *
parseAutopenalty(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   char * pfactor;
   fprintf(stdout,"Parsing auto penalty flag...\n");

  /* This will be a struct at some point */
   if (!strcmp("yes",xmlGetProp(cur,"flag")) )
   {
      adata->autopenaltyflag = 1;
   }
   else
   {
      adata->autopenaltyflag = 0;
   }

  /* Set this to zero in case the analysis file is not
   * updated for handling the pfactor attribute.  Then
   * the entire block system sinks.
   */
   adata->pfactor = 0.0;
   pfactor = xmlGetProp(cur,"pfactor");

   if (pfactor == NULL)
   {
      sprintf(mess,"Penalty factor missing from Autopenalty element");
      iface->displaymessage(mess);
   }
   else 
   {
      adata->pfactor = atof(pfactor);
   }

   fprintf(stdout,"autopenalty flag: %d\n\n", adata->autopenaltyflag);

   return NULL;

}  /*close parseAutopenalty() */



void *
parsePlanestrain(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{

   fprintf(stdout,"Parsing planestrain flag...\n");

  /* This will be a struct at some point */
   if (!strcmp("yes",xmlGetProp(cur,"flag")) )
   {
     /* Set the flag: */
      adata->planestrainflag = 1;
   }
   else
   {
      adata->planestrainflag = 0;
   }

   fprintf(stdout,"planestrain flag: %d\n\n", adata->planestrainflag);


   return NULL;

}  /*close parsePlanestrain() */


void *
parseAConstants(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
  /* constants needs to be changed into an instance 
   * variable.
   */
   CONSTANTS * constants;
   constants  = (CONSTANTS *)calloc(1,sizeof(CONSTANTS));
  
   fprintf(stderr,"Inside constants\n");

   cur = cur->childs;

   while (cur != NULL) 
   {
      //fprintf(stderr,"Inside constants loop\n");

      if (!strcmp(cur->name, "Openclose"))  // && (cur->ns == ns))
      { //char mess[100];
        constants->openclose = atof(xmlGetProp(cur,"value"));
        //sprintf(mess,"%.16f",constants->openclose);
        //iface->displaymessage(mess);
      }

      if (!strcmp(cur->name, "Opencriteria")) //&& (cur->ns == ns))
        constants->opencriteria = atof(xmlGetProp(cur,"value"));

     /* w0 gets reset during the analysis.  Probably need to turn this
      * into a factor of w0 to see if that helps scaling issues in 
      * problems with vastly different block sizes.
      */
      if (!strcmp(cur->name, "Domainscale")) //&& (cur->ns == ns))
        constants->w0 = atof(xmlGetProp(cur,"value"));

      /* The following three are derived from the other five */
      
      if (!strcmp(cur->name, "NormSpringPen")) //&& (cur->ns == ns))
      {
        constants->norm_spring_pen = atof(xmlGetProp(cur,"value"));
       /* if this assert fires, check the attribute value in the 
        * given tag.
        */
        assert(constants->norm_spring_pen > 0);
      }

     /*
      if (!strcmp(cur->name, "NormExternDist")) //&& (cur->ns == ns))
        constants->norm_extern_dist = atof(xmlGetProp(cur,"value"));

      if (!strcmp(cur->name, "NormPenDist")) //&& (cur->ns == ns))
        constants->norm_pen_dist = atof(xmlGetProp(cur,"value"));
      */

      if (!strcmp(cur->name, "AngleOverlap")) // && (cur->ns == ns))
        constants->angle_olap = atof(xmlGetProp(cur,"value"));

      if (!strcmp(cur->name, "ShearNormRatio")) // && (cur->ns == ns))
        constants->shear_norm_ratio = atof(xmlGetProp(cur,"value"));

     /* MinReflineFactor is scaled by the domain size. Probably do not
      * need this factor at all.
      */
      if (!strcmp(cur->name, "MinReflineFactor")) // && (cur->ns == ns))
        constants->min_refline_factor = atof(xmlGetProp(cur,"value"));

      //iface->displaymessage("In constants parsing code");
      cur = cur->next;

   }  /* end loop over current pointer */

   adata->constants = constants;   

   return NULL;

}  /*close parseAConstants() */



void *
parseOpencloselimit(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   adata->OCLimit = atoi(xmlGetProp(cur,"maxopenclose"));
   return NULL;
}  /*close parseOpencloselimit() */



void *
parseTimehistory(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   char * filename;
   char * fileformat;

   filename = xmlGetProp(cur,"filename");
   //iface->displaymessage(filename);

   fileformat = xmlGetProp(cur,"format");
   //iface->displaymessage(fileformat);

   if (!strcmp(fileformat,"shake"))
      adata->timehistory = getTimeHistory(filename,shake);
   else if (!strcmp(fileformat,"matlab"))
      adata->timehistory = getTimeHistory(filename,matlab);
   else // blow up
      iface->displaymessage("Bad motion file format");

   return NULL;
}  /*close parseTimehistory() */



void *
parseSaveinterval(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   char * step;

   fprintf(stdout,"Parsing save interval...\n");

  /* FIXME: Check to see if there is a null retrun value 
   * before running atoi.
   */
   step = xmlGetProp(cur,"step");
   
   if (step != NULL)
   {
      adata->tsSaveInterval = atoi(step);
   }
   else
   {
     /* Set default value of 10 */
      adata->tsSaveInterval = 10;
     /* And fire off a warning about bad attribute */
     /* FIXME: Handle warning. */
   }

   return NULL;

}  /*close parseSaveinterval() */


void *
parseMaxtimestep(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   fprintf(stdout,"Parsing max time step...\n");
   adata->maxtimestep = atof(xmlGetProp(cur,"maxtimestep"));
   return NULL;
}  /*close parseMaxtimestep() */


void *
parseMintimestep(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   fprintf(stdout,"Parsing max time step...\n");
   adata->min_delta_t = atof(xmlGetProp(cur,"mintimestep"));
   return NULL;
}  /*close parseMinimestep() */


void *
parseMaxdisplacement(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   fprintf(stdout,"Parsing max displacement...\n");
   adata->maxdisplacement = atof(xmlGetProp(cur,"maxdisplacement"));
   return NULL;
}  /*close parseAutoTimestep() */


void *
parseContactDamping(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   double cn = atof(xmlGetProp(cur,"value"));
   //fprintf(stdout,"Parsing max displacement...\n");
   adata_set_contact_damping(adata, cn);
   return NULL;

}  /*close parseAutoTimestep() */

void *
parseJointproperties(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   JointMat * jmat;

   cur = cur->childs;
  /* Grab a struct for joint properties */
   jmat = getNewJointMat();

  /* FIXME: Reimplement namespace checking.
   */
   while (cur != NULL) 
   {
      //fprintf(stderr,"In joint properties loop\n");
      if ((!strcmp(cur->name, "Friction")) ) // && (cur->ns == ns))
         jmat->fric = atof(xmlNodeListGetString(doc, cur->childs, 1));
      if ((!strcmp(cur->name, "Cohesion")) ) //&& (cur->ns == ns))
         jmat->coh = atof(xmlNodeListGetString(doc, cur->childs, 1));
      if ((!strcmp(cur->name, "Tensile")) ) //&& (cur->ns == ns))
         jmat->tens = atof(xmlNodeListGetString(doc, cur->childs, 1));
      cur = cur->next;
   }
  /* Append */
   dl_insert_b(jointmatlist,(void*)jmat);

  /* Grab a node for the block materials list, 
   * then add the parsed data to the list.
   */
   return NULL;
}  /*close parseJointproperties() */


void *
parseBlockmaterial(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   BlockMat * bmat;
   double temp[3] = {0.0};
   char * tempstring;
   int checkval;  //  Use this to make sure sscanf got enough numbers

   static int gotdamping = 0;

   cur = cur->childs;
  /* Grab a struct for block materials */
   bmat = getNewBlockMat(); 

  /* FIXME: Reimplement namespace checking.
   */
   while (cur != NULL) 
   {
      //fprintf(stderr,"In block materials loop\n");

      if ((!strcmp(cur->name, "Unitmass")) ) //&& (cur->ns == ns))
         bmat->dens = atof(xmlNodeListGetString(doc, cur->childs, 1));
         
      if ((!strcmp(cur->name, "Unitweight")) ) //&& (cur->ns == ns))
         bmat->wt = atof(xmlNodeListGetString(doc, cur->childs, 1));

      if ((!strcmp(cur->name, "Youngsmod")) ) //&& (cur->ns == ns))
         bmat->ymod = atof(xmlNodeListGetString(doc, cur->childs, 1));

      if ((!strcmp(cur->name, "Poissonratio")) ) //&& (cur->ns == ns))
         bmat->pois = atof(xmlNodeListGetString(doc, cur->childs, 1));

      if ((!strcmp(cur->name, "Damping")) ) //&& (cur->ns == ns))
      {
         bmat->damping = atof(xmlNodeListGetString(doc, cur->childs, 1));
         gotdamping = 1;
      }

     /* These will have three values each */
      if ((!strcmp(cur->name, "Istress")) ) //&& (cur->ns == ns))
      {
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         checkval = sscanf(tempstring,"%lf%lf%lf",&temp[0],&temp[1],&temp[2]);
         if (checkval == 3)
         {
            bmat->iss[0] = temp[0];
            bmat->iss[1] = temp[1];
            bmat->iss[2] = temp[2];
         }
         else
         { // Throw some kind of error.
         }  
      }

      if ((!strcmp(cur->name, "Istrain")) ) //&& (cur->ns == ns))
      {
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         checkval = sscanf(tempstring,"%lf%lf%lf",&temp[0],&temp[1],&temp[2]);
         if (checkval == 3)
         {
            bmat->ist[0] = temp[0];
            bmat->ist[1] = temp[1];
            bmat->ist[2] = temp[2];
         }
         else
         {  // Throw some kind of error
         }     
      }

      if ((!strcmp(cur->name, "Ivelocity")) ) //&& (cur->ns == ns))
      {
         tempstring = xmlNodeListGetString(doc, cur->childs, 1);
         checkval = sscanf(tempstring,"%lf%lf%lf",&temp[0],&temp[1],&temp[2]);
         if (checkval == 3)
         {
            bmat->ivel[0] = temp[0];
            bmat->ivel[1] = temp[1];
            bmat->ivel[2] = temp[2];
         }
         else
         {  // Throw some kind of error
         }
      }

      cur = cur->next;
   }

   if (!gotdamping)
   {
      iface->displaymessage("Damping tag not found in BlockMaterials");
      exit(0);
   }

   dl_insert_b(blockmatlist,(void*)bmat);

   return NULL;

}  /*close parseBlockmaterial() */

// Just for reference...
/*
typedef struct _loadpoint {
   double x, y;
   int loadpointsize1;
   int loadpointsize2;  // = 3 
   double ** vals;
}LOADPOINT;
*/
void *
parseLoadpoints(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   int i;
   int checkval;
   double temp[3] = {0.0};
   LOADPOINT * loadpoint;
   char * timehiststring;
   char * recordstring;

   cur = cur->childs;

   fprintf(stderr,"Parsing load points\n");

   while (cur != NULL) 
   {
      if ((!strcmp(cur->name, "Loadpoint")) ) //&& (cur->ns == ns))
      {
         loadpoint = getNewLoadpoint();
        /* Get the size of this thing from the attribute value: */
        /* FIXME: Segfaults if no attribute specified.  Either output
         * from the xml parser must be examined, or the call needs to 
         * checked for NULL return value.  To reproduce this error,
         * pick some analysis with loading points, and delete the 
         * `numpoints="n"' attribute.
         */
         loadpoint->loadpointsize1 = 1+atoi(xmlGetProp(cur,"numpoints"));
         loadpoint->loadpointsize2 = 3+1;
         loadpoint->vals = DoubMat2DGetMem(loadpoint->loadpointsize1,
                                           loadpoint->loadpointsize2);
        /* Slurp up the entire time history... */
         timehiststring = xmlNodeListGetString(doc, cur->childs, 1);
        /* Now, lots of heinous parsing... */
         recordstring = strtok(timehiststring, ";");
        /* FIXME: Check the behavior of this with respect to the 
         * start of indexing value 0 or 1.
         */
         for (i=0; i<loadpoint->loadpointsize1; i++)
         {
            checkval = sscanf(recordstring,"%lf%lf%lf",
                              &temp[0],&temp[1],&temp[2]);
            fprintf(stdout,"%f %f %f\n", temp[0],temp[1], temp[2]);
            if (checkval == 3)
            {
               loadpoint->vals[i][0] = temp[0];
               loadpoint->vals[i][1] = temp[1];
               loadpoint->vals[i][2] = temp[2];
               recordstring = strtok(NULL, ";");
            }
            else
            {
                fprintf(stdout,"Empty load point list.\n");
            }

         }  /* end for loop over time history.  */

         dl_insert_b(loadpointlist, loadpoint);

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

}  /*close parseLoadpoints() */


/* The values parsed here will correspond to the 
 * entries 7,8,9 in the geometry structure bolt array
 */ 
void *
parseBoltproperties(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{

   //adata->boltmatsize1 = 1;
   BoltMat * boltmat;
   char * tempstring;
   double temp[3] = {0.0};
   int checkval;  

   cur = cur->childs;
  /* Grab a struct for joint properties */
   boltmat = getNewBoltMat();
            
   tempstring = xmlNodeListGetString(doc, cur, 1);
   checkval = sscanf(tempstring,"%lf%lf%lf",&temp[0],&temp[1],&temp[2]);
   if (checkval == 3)
   {
      boltmat->e00 = temp[0];
      boltmat->t0 = temp[1];
      boltmat->f0 = temp[2];
   }
   else
   {  // Throw some kind of error
   }

  /* Append */
   dl_insert_b(boltmatlist,(void*)boltmat);
   return NULL;

}  /*close parseBoltproperties() */


LOADPOINT *
getNewLoadpoint(void)
{
   LOADPOINT * l;
   //fprintf(stdout,"Getting new load point\n");
   l = (LOADPOINT *)calloc(1,sizeof(LOADPOINT));
   return l;

}  /*  close getNewLoadpoint() */



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


static JointMat *
getNewJointMat(void)
{
   JointMat * jm;
   //fprintf(stdout,"Getting new joint\n");
   jm = (JointMat *)calloc(1,sizeof(JointMat));
   return jm;

}  /*  close getNewJointMat() */

static BlockMat *
getNewBlockMat(void)
{
   BlockMat * bm;
   //fprintf(stdout,"Getting new block mat\n");
   bm = (BlockMat *)malloc(sizeof(BlockMat));
   memset(bm,0xDA,sizeof(BlockMat));
   return bm;

}  /*  close getNewJointMat() */

static BoltMat *
getNewBoltMat(void)
{
   BoltMat * bm;
   //fprintf(stdout,"Getting new block mat\n");
   bm = (BoltMat *)malloc(sizeof(BoltMat));
   memset(bm,0xDA,sizeof(BoltMat));
   return bm;

}  /*  close getNewJointMat() */