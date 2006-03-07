
/**
 * Functions pulled from utils.c into ddafile.c to 
 * match ddafile.h header.  Then this stuff can be 
 * abstracted into something a lot more useful.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <direct.h>

#include "dda.h"
#include "ddafile.h"

#ifdef _cplusplus
extern "C" {
#endif
#if 0
}
#endif



struct _ddafile {

   char filename[1024];
   FILE * fp;
};


DDAFile *
ddafile_new (const char * filename) {

   DDAFile * df = (DDAFile*)malloc(sizeof(DDAFile));

   df->fp = NULL;
   return df;
}


DDAFile *
ddafile_new_open(const char * filename) {

   DDAFile * df = ddafile_new(filename);

   df->fp = fopen(df->filename,"w");
   return df;
}


void
ddafile_open_file(FILE ** fp, const char * rootname, 
                  const char * filename) {

   char temp[1024] = {'0'};

   strncpy(temp,rootname,sizeof(rootname));
   strncpy(temp,filename,sizeof(filename));
  *fp = fopen(temp,"w");
}



/** Figure out the file type by seeing if there is 
 *  a magic number on the first line.  This function fails 
 *  if there is nothing on the first line to parse.
 *
 *  @param char * filename of input, with enough path
 *         information to find the file.
 * 
 *  @return int enum of magic number indicating which 
 *          file type we need to parse.
 */
int 
ddafile_get_type (char * infilename) {

   char buf[128];
   char * magicnum;

   FILE *infile;

   assert(infilename != NULL);

   infile = fopen (infilename, "r");

  /* If this assertion fires, its probably because bad path? */
   assert (infile != NULL);

   fgets(buf, 128, infile);
   fclose(infile);

   magicnum = strtok(buf," \n\r");

  /* glibc manual states that strtok returns a null
   * pointer when the character buffer contains only 
   * delimiters.
   */
   if (magicnum == NULL) {
      dda_display_error("First line of input file is empty.");
   }

   if (!strncmp(magicnum,"<?",2)) {  
      return ddaml;
   }

   if (!strncmp(magicnum,"#!0xDDA-v1.5",12)) {
      return extended;
   }

   return original;
}  



/** Attempt to handle extern file pointer
 * declarations as a group.  That is, open them 
 * all at the same time, then later close all of the 
 * at the same time.
 * FIXME: replace all strcpy with strncpy for buffer
 * protection when the back end goes networked.
 * FIXME: The code for initializing file names should be split
 * from the code that actually opens the output streams.
 * This will make it easier to control which files should 
 * actually be open at any one time.
 *
 * @todo This is ugly as homemade sin and needs to be completely
 *       redesigned. 
 */
void
openAnalysisFiles(Filepaths * filepath) {

   char temp[256];
   extern FILEPOINTERS fp;
   // This part is added by Roozbeh to copy all 
   //output files into "Output" folder
   #define BUFSIZE 1024
   const char * dirname;
   size_t dirnamesize;
   char wdbuf[BUFSIZE];
   char * outdir;

   outdir = malloc(BUFSIZE);
   getcwd(wdbuf,BUFSIZE);
   dirname="output";
   dirnamesize = sizeof(dirname);
   strncpy(outdir,wdbuf,sizeof(wdbuf));
   strncat(outdir,"\\",sizeof("\\"));
   strncat(outdir,dirname,dirnamesize);
   strncat(outdir,"\\",sizeof("\\"));
   strcat(outdir, filepath->rootname);
   strcpy(filepath->rootname, outdir);
   free(outdir);
   // This part is added by Roozbeh to copy all 
   //output files into "Output" folder


  /* Note that the rootname contains the entire path.  This 
   * is probably not real good, but is messy to handle 
   * otherwise on win32.
   */
   strcpy(temp, filepath->rootname);
   strcat(temp, ".replay");
   strcpy(filepath->replayfile, temp);
   fp.replayfile = fopen(filepath->replayfile, "w");

//ddafile_open_file(&fp.replayfile, filepath->rootname,".replay");

   strcpy(temp, filepath->rootname);
   strcat(temp, ".html");
   strcpy(filepath->htmlfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_data.m");
   strcpy(filepath->datafile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".gnu");
   strcpy(filepath->gnuplotfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".log");
   strcpy(filepath->logfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_meas.m");
   strcpy(filepath->measfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".por");
   strcpy(filepath->porefile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_time.m");
   strcpy(filepath->timefile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".par");
   strcpy(filepath->parfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_moments.m");
   strcpy(filepath->momentfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_mass.m");
   strcpy(filepath->massfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_defs.m");
   strcpy(filepath->dfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".grav");
   strcpy(filepath->gravfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_fpoint.m");
   strcpy(filepath->fpointfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_cforce.m");
   strcpy(filepath->cforce, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_fforce.m");
   strcpy(filepath->fforce, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_bolt.m");
   strcpy(filepath->boltfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_bolt.log");
   strcpy(filepath->boltlogfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_vertices.m");
   strcpy(filepath->vertexfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_vertices.log");
   strcpy(filepath->vertexlogfile, temp);

  /* Copied in from geometry driver. */
   strcpy(temp, filepath->rootname);
   strcat(temp, ".blk");
   strcpy(filepath->blockfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".err");
   strcpy(filepath->errorfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".spy1");
   strcpy(filepath->spyfile1, temp);


   strcpy(temp, filepath->rootname);
   strcat(temp, ".spy2");
   strcpy(filepath->spyfile2, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".m");
   strcpy(filepath->mfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_stress.m");
   strcpy(filepath->stressfile, temp);


 /* Do not get the file name of this yet. */
   //strcpy(temp, filepath->gpath);
   //strcat(temp, ".m");
   strcpy(filepath->eqfile, filepath->gpath);



   fp.measfile = fopen(filepath->measfile, "w");
   fp.logfile = fopen(filepath->logfile, "w");
   fp.errorfile = fopen(filepath->errorfile, "w");
  /* What is dispfile?
   */
   //fp.dispfile = fopen(filepath->dispfile, "w");
   //fp.grffile = fopen(filepath->grffile, "w");
  /* Cleft pressure file output. */
   fp.porefile = fopen(filepath->porefile, "w");
  /* time data */
   fp.timefile = fopen(filepath->timefile, "w");
  /* parameter file */
   fp.parfile = fopen(filepath->parfile, "w");
  /* block area file */
   fp.momentfile = fopen(filepath->momentfile, "w");
   fp.massfile = fopen(filepath->massfile, "w");
   fp.gravfile = fopen(filepath->gravfile, "w");
   fp.htmlfile = fopen(filepath->htmlfile, "w");
   fp.datafile = fopen(filepath->datafile, "w");
   fp.fpointfile = fopen(filepath->fpointfile, "w");

   fp.boltfile = fopen(filepath->boltfile, "w");
   fp.boltlogfile = fopen(filepath->boltlogfile, "w");
   //mmm: added force in bolt
   fprintf(fp.boltlogfile, "Elapsed Time: bolt1x1,bolt1y1 bolt1x2,bolt1y2 bolt1force; bolt2x1,bolt2y1 bolt2x2,bolt2y2 bolt2force; etc\n");
   fp.vertexfile = fopen(filepath->vertexfile, "w");
   fp.vertexlogfile = fopen(filepath->vertexlogfile, "w");

   fp.cforce = fopen(filepath->cforce, "w");
   fprintf(fp.cforce,"contactforces = [\n");

   fp.fforce = fopen(filepath->fforce, "w");
   fprintf(fp.fforce,"frictionforces = [\n");

   fp.spyfile1 = fopen(filepath->spyfile1, "w");
   fp.spyfile2 = fopen(filepath->spyfile2, "w");

   fp.gnuplotfile = fopen(filepath->gnuplotfile, "w");
   fp.mfile = fopen(filepath->mfile, "w");

   fp.dfile = fopen(filepath->dfile, "w");
   fprintf(fp.dfile,"deformations = [\n");

   fp.stressfile = fopen(filepath->stressfile, "w");
   fprintf(fp.stressfile,"stresses = [\n");

  /* Do not open this yet.  The function that parses this
   * data should handle all the io for the time history.
   */
  /*fp.eqfile = fopen(filepath->eqfile,"r");*/

}





void
closeAnalysisFiles() {

   extern FILEPOINTERS fp;

   fclose(fp.replayfile);
   //fprintf(fp.logfile, "Closed log file.\n");
   fclose(fp.logfile);
   //fprintf(fp.errorfile, "Closed error file.\n");
   fclose(fp.errorfile);
   //fclose(fp.dispfile);
   //fclose(fp.grffile);
   fclose(fp.measfile);
   fclose(fp.porefile);
   fclose(fp.timefile);
   fclose(fp.parfile);
   fclose(fp.momentfile);
   fclose(fp.massfile);
   fclose(fp.gravfile);
   fclose(fp.htmlfile);
   fclose(fp.datafile);
   fclose(fp.fpointfile);
   fclose(fp.boltfile);
   fclose(fp.boltlogfile);
   fclose(fp.vertexfile);
   fclose(fp.vertexlogfile);
   fprintf(fp.cforce,"];\n");
   fclose(fp.cforce);
   fprintf(fp.fforce,"];\n");
   fclose(fp.fforce);

   fclose(fp.spyfile1);
   fclose(fp.spyfile2);
   fclose(fp.gnuplotfile);
   fclose(fp.mfile);

   fprintf(fp.dfile,"];\n");
   fclose(fp.dfile);

  /* FIXME: This is non-portable. */
   fprintf(fp.stressfile,"];\n");
   fclose(fp.stressfile);

  /* Cosed in function handling the time history.
   * Noted for completeness and uniformity.
   */
   //fclose(fp.eqfile);
}  







#ifdef __cplusplus

}

#endif