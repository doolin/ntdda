
/** Handle different file formats, etc.
 *
 * $Author: doolin $
 * $Date: 2003/12/17 23:36:36 $
 * $Source: /cvsroot/dda/ntdda/src/inputfiles.c,v $
 * $Revision: 1.6 $
 *
 */

#include <assert.h>
#include <string.h>

#include "dda.h"

#if 0

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
getFileType(char *infilename) {

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

#endif