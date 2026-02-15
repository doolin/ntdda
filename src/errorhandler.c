#include<stdio.h>
#include"errorhandler.h"
#include "dda.h"


extern FILEPOINTERS fp;

/*
void error(char * errorstring)
{

   fprintf(fp.errorfile, errorstring);
   fprintf(fp.errorfile,"\n");

   return;
}
*/

#ifdef HJHGHHHHHH
/*  print out a matrix.
 */

void matrixprint(double ** matrix, int n, int m)
{
   FILE *ofp;
   int i, j;

   ofp = fopen("matrix.log", "w");

   for (i = 1; i <= n; i++)
   {
      for (j = 1; j <= m; j++);
      {
         fprintf(ofp, "%f  ", matrix[i][j]);
      }
      fprintf(ofp, "\n");
   }
   fclose(ofp);
}

#endif