
/**
 * Functions pulled from utils.c into ddafile.c to
 * match ddafile.h header.  Then this stuff can be
 * abstracted into something a lot more useful.
 */

/* ================================================================
 * ADR: ddafile.c — File Utility Layer
 * Status: documenting current state and required changes
 * ================================================================
 *
 * WHAT THIS FILE IS SUPPOSED TO DO
 *
 * Provide all file I/O management for DDA: identify input file
 * formats, construct output paths from a rootname, and open/close
 * the ~25 output streams that an analysis run produces.
 *
 * There are two independent APIs here that were never reconciled:
 *
 *   1. DDAFile — An opaque per-file handle (struct _ddafile).
 *      Intended to encapsulate filename + FILE* as a pair.
 *      Never completed; never called from production code.
 *
 *   2. Filepaths + FILEPOINTERS — Parallel structs defined in
 *      ddafile.h.  Filepaths carries 37 char[256] path buffers;
 *      FILEPOINTERS carries the corresponding FILE* handles.
 *      Both are accessed as extern globals.  This is the API that
 *      the analysis engine actually uses.
 *
 * The original intent (line 176, commented-out call) was to replace
 * the repetitive strcpy/strcat/fopen blocks in openAnalysisFiles()
 * with calls to ddafile_open_file().  This was never finished.
 *
 *
 * KNOWN BUGS (confirmed by ddafile_test.c)
 *
 * Bug 1 — ddafile_new() ignores the filename parameter.
 *   malloc allocates the struct but never copies the filename in.
 *   The filename field contains uninitialized heap memory (visible
 *   as 0xDA when pre-poisoned in tests).
 *
 * Bug 2 — ddafile_new_open() calls fopen() on the garbage filename
 *   from Bug 1.  The intended file is never created.
 *
 * Bug 3 — ddafile_open_file() uses sizeof(rootname) which evaluates
 *   to sizeof(char*) = 8, not the string length.  Only 8 bytes are
 *   copied.  The second strncpy overwrites the first (should be
 *   strncat).  The initializer {'0'} sets byte 0 to ASCII 0x30, not
 *   the intended '\0'.
 *
 * Bug 4 — ddafile_get_type() calls dda_display_error() when the
 *   magic number is NULL but does not return.  Execution falls
 *   through to strncmp(NULL, ...) which is a NULL dereference.
 *
 * Bug 5 — openAnalysisFiles() does not check any fopen() return
 *   values.  If the output directory doesn't exist or isn't
 *   writable, every FILE* in FILEPOINTERS is NULL.  Subsequent
 *   fprintf/fclose calls on NULL FILE* are undefined behavior.
 *
 * Bug 6 — The `rootname` field is documented as "the entire path"
 *   (line 78 of ddafile.h), making the name misleading.  On Win32
 *   it is set via strtok(filename, ".") which also breaks on
 *   filenames with multiple dots (e.g., "model.v2.ana").
 *
 *
 * WHAT NEEDS TO CHANGE
 *
 * 1. Finish or remove DDAFile.
 *
 *   The opaque-handle design is sound.  If kept, the constructor
 *   must copy the filename (strncpy + null-terminate), the
 *   destructor should poison with 0xDD then free, and the open
 *   function should check fopen's return value.  If not kept,
 *   delete the dead code (ddafile_new, ddafile_new_open,
 *   ddafile_open_file) so it doesn't mislead future readers.
 *
 * 2. Separate path construction from file opening.
 *
 *   openAnalysisFiles() currently does three things in one pass:
 *   constructs paths, creates the output directory, and opens every
 *   file.  The existing FIXME at line 127 calls this out.  Split
 *   into:
 *     a) initOutputPaths(Filepaths *fp, const char *outdir)
 *        Pure computation.  Constructs all output paths from outdir
 *        + rootname + extension.  Easily testable in isolation.
 *     b) openOutputFiles(Filepaths *fp, FILEPOINTERS *fptrs)
 *        Opens the files.  Checks each fopen; returns error count.
 *
 * 3. Use snprintf for all path construction.
 *
 *   Replace the strcpy/strcat chains with:
 *     snprintf(fp->replayfile, FNAME_BUFSIZE, "%s/%s.replay",
 *              outdir, fp->rootname);
 *   This provides bounds checking in a single call and eliminates
 *   the temp[256] intermediate buffer (which itself is a truncation
 *   risk when outdir + rootname + extension exceeds 255 bytes).
 *
 * 4. Add a portable path_join helper.
 *
 *   A small function that appends a path separator between
 *   components, choosing '/' or '\\' via #ifdef.  Replaces the
 *   repeated #ifdef WIN32 / strncat blocks with:
 *     path_join(outdir, BUFSIZE, cwdbuf, "output");
 *   This is what dda_set_output_directory() in dda.c already does
 *   inline; factor it out so every caller benefits.
 *
 * 5. Eliminate extern globals for file handles.
 *
 *   FILEPOINTERS is declared extern in openAnalysisFiles() and
 *   closeAnalysisFiles(), coupling this module to a global defined
 *   elsewhere (combineddf.c).  Instead, pass FILEPOINTERS* as a
 *   parameter.  This makes the code testable without global state,
 *   and makes the data flow explicit in the call chain.
 *
 * 6. Fix ddafile_get_type error handling.
 *
 *   Replace the assert() calls with error returns so the function
 *   is usable in non-debug builds.  After the NULL magicnum check,
 *   return an error value (or a new enum member like `filetype_error`)
 *   instead of falling through.  Let the caller decide how to report
 *   the error.
 *
 * 7. Consider table-driven file registration.
 *
 *   The 25 repetitive strcpy/strcat/fopen blocks could be replaced
 *   by iterating over a descriptor table:
 *
 *     struct output_desc {
 *         const char *suffix;
 *         size_t      path_offset;   // offsetof into Filepaths
 *         size_t      fp_offset;     // offsetof into FILEPOINTERS
 *         const char *header;        // e.g. "contactforces = [\n"
 *         const char *trailer;       // e.g. "];\n"
 *     };
 *
 *   This consolidates the open, close, and path construction logic
 *   into a single loop, making it trivial to add or remove output
 *   files.
 *
 * 8. Increase FNAME_BUFSIZE.
 *
 *   256 bytes is too small for modern paths.  PATH_MAX is 1024 on
 *   macOS and 4096 on Linux.  FNAME_BUFSIZE should be at least 1024,
 *   or dynamically allocated.  The temp[256] local in
 *   openAnalysisFiles() should match or be eliminated in favor of
 *   snprintf directly into the Filepaths field.
 *
 * ================================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

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
   // output files into "Output" folder
   // output folder is created when programe is ran.
   #define BUFSIZE 1024
   char cwdbuf[BUFSIZE];
   char * outdir;

   outdir = malloc(BUFSIZE);
   getcwd(cwdbuf,BUFSIZE);
   dda_set_output_directory("output",sizeof("output"));
   strncpy(outdir,cwdbuf,sizeof(cwdbuf));
#ifdef WIN32
   strncat(outdir,"\\",sizeof("\\"));
#else
   strncat(outdir,"/",sizeof("/"));
#endif
   strncat(outdir,"output",sizeof("output"));
#ifdef WIN32
   strncat(outdir,"\\",sizeof("\\"));
#else
   strncat(outdir,"/",sizeof("/"));
#endif
   strcat(outdir, filepath->rootname);
   // This part is added by Roozbeh to copy all
   //output files into "Output" folder


  /* Note that the rootname contains the entire path.  This
   * is probably not real good, but is messy to handle
   * otherwise on win32.
   */
   strcpy(temp, outdir);
   strcat(temp, ".replay");
   strcpy(filepath->replayfile, temp);
   fp.replayfile = fopen(filepath->replayfile, "w");

//ddafile_open_file(&fp.replayfile, filepath->rootname,".replay");

   strcpy(temp, outdir);
   strcat(temp, ".html");
   strcpy(filepath->htmlfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_data.m");
   strcpy(filepath->datafile, temp);

   strcpy(temp, outdir);
   strcat(temp, ".gnu");
   strcpy(filepath->gnuplotfile, temp);

   strcpy(temp, outdir);
   strcat(temp, ".log");
   strcpy(filepath->logfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_meas.m");
   strcpy(filepath->measfile, temp);

   strcpy(temp, outdir);
   strcat(temp, ".por");
   strcpy(filepath->porefile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_time.m");
   strcpy(filepath->timefile, temp);

   strcpy(temp, outdir);
   strcat(temp, ".par");
   strcpy(filepath->parfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_moments.m");
   strcpy(filepath->momentfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_mass.m");
   strcpy(filepath->massfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_defs.m");
   strcpy(filepath->dfile, temp);

   strcpy(temp, outdir);
   strcat(temp, ".grav");
   strcpy(filepath->gravfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_fpoint.m");
   strcpy(filepath->fpointfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_cforce.m");
   strcpy(filepath->cforce, temp);

   strcpy(temp, outdir);
   strcat(temp, "_fforce.m");
   strcpy(filepath->fforce, temp);

   strcpy(temp, outdir);
   strcat(temp, "_bolt.m");
   strcpy(filepath->boltfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_bolt.log");
   strcpy(filepath->boltlogfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_vertices.m");
   strcpy(filepath->vertexfile, temp);

   strcpy(temp, outdir);
   strcat(temp, "_vertices.log");
   strcpy(filepath->vertexlogfile, temp);

  /* Copied in from geometry driver. */
   strcpy(temp, outdir);
   strcat(temp, ".blk");
   strcpy(filepath->blockfile, temp);

   strcpy(temp, outdir);
   strcat(temp, ".err");
   strcpy(filepath->errorfile, temp);

   strcpy(temp, outdir);
   strcat(temp, ".spy1");
   strcpy(filepath->spyfile1, temp);


   strcpy(temp, outdir);
   strcat(temp, ".spy2");
   strcpy(filepath->spyfile2, temp);

   strcpy(temp, outdir);
   strcat(temp, ".m");
   strcpy(filepath->mfile, temp);

   strcpy(temp, outdir);
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
free(outdir); //Added By Roozbeh
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

