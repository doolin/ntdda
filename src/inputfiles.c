
/* 
 * inputfiles.c
 *
 * Handle different file formats, etc.
 *
 * $Author: doolin $
 * $Date: 2001/06/15 20:11:20 $
 * $Source: /cvsroot/dda/ntdda/src/inputfiles.c,v $
 * $Revision: 1.2 $
 *
 */

#include <assert.h>
#include <string.h>
#include "dda.h"

/* Figure out the file type by seeing if there is 
 * a magic number on the first line.  This function fails 
 * if there is nothing on the first line to parse.
 */
int
getFileType(char *infile)
{
   char buf[128];
   char * magicnum;

  /* geomInFile is the geometry data input file.  
   */
   FILE *InFile;

   assert(infile != NULL);

   InFile = fopen (infile, "r");
  /* If this assertion fires, its probably because bad path? */
   assert (InFile != NULL);

   fgets(buf, 128, InFile);
   fclose(InFile);


   /* FIXME: Check the size of magicnum tomake sure 
    * there are enough characters there so that 
    * strncmp doesn't seg fault.  See next comment.
    */
   magicnum = strtok(buf," \n\r");

   /* FIXME:  If there is only 1 char (e.g., \n) 
    * on the first line, this code segfaults.
    */
   if (!strncmp(magicnum,"<?",2))
   {  
      return ddaml;
   }

   if (!strncmp(magicnum,"#!0xDDA-v1.5",12))
      return extended;

   return original;


}  /* Close getFileType()  */




