/*
 * eq.lex.c 
 * 
 * lexer/parser for eq data.  This should work with the same files
 * used with SHAKE.
 *
 * $Author: doolin $
 * $Date: 2002/05/25 14:49:41 $
 * $Source: /cvsroot/dda/ntdda/src/timehistory.c,v $
 * $Revision: 1.2 $
 */

#include<stdio.h>

#include<string.h>
#include<stdlib.h>

#include "timehistory.h"

#ifndef STANDALONE
#include "interface.h"
extern InterFace * iface;
#endif

struct _TimeHistory {
  int  number_of_datapoints;
  int block_number;
  double delta_t;
  double * data;
};



static TimeHistory * getNewTimeHistory();
static double * getTHData(int);

int numseisdatapoints;
double * seisdatarray;

#define BUFSIZE 2000



/* I should separate the "lexing" stuff from the 
 * driver, i.e. just have this main() handle the 
 * file io and call the lexer/parser as a function 
 * call that returns a struct of the data and headers.
 * Will do it later.
 */

#ifdef STANDALONE
typedef struct  _iface {
  int (*displaymessage)(const char *,...);
}Iface;

Iface * iface;

int
main(int argc, char *argv[])
{
  TimeHistory * th;

   if (argc != 2)
   { 
      printf("Usage: eqlex <motion file>\n");
      return 0;
   }

   iface = (Iface *)malloc(sizeof(Iface));
   iface->displaymessage = printf;


   th = getTimeHistory(argv[1], matlab);

   destroyTimeHistory(th);

   free(iface);

   return 0;

}  /* close main() */
#endif


static void 
writeATHistory(TimeHistory *th)
{
   int i;
   double dt = 0;
   FILE * seisfile;

   seisfile = fopen("athistory.m","w");

   fprintf(seisfile,"timehistory = [\n");
   for (i=0;i<th->number_of_datapoints;i++)
   {
      fprintf(seisfile,"%f %f\n",dt+=0.02,th->data[i]);
   }
   fprintf(seisfile,"];\n");

   fflush(seisfile);
   fclose(seisfile);

}  /* close writeATHistory() */

      
int 
th_get_number_of_datapoints(TimeHistory * th)
{
   return th->number_of_datapoints;

}  /* close th_get_number_of_datapoints() */


double 
th_get_delta_t(TimeHistory * th)
{
   return th->delta_t;

}  /* close th_get_delta_t() */


double
th_get_data_value(TimeHistory * th, int timestep)
{
   return th->data[timestep];

}  /* close th_get_data_value() */


static void
handleBadFileName(char * filename) {

   char mess[80];
   sprintf(mess,"Time history file %s does not exist\n",filename);
   dda_display_error(mess);
   //edit the file
   exit(-1);
}  /* close handleBadFileName(0 */



TimeHistory * 
parseShakeFormat(FILE * motionfile)
{
   TimeHistory * th;
   char  firstline[256];
   char  secondline[256];
   char linebuf[BUFSIZE];
   char * temp;

   //FILE * motionfile;
   double * data; 
   int i, j;

   th = getNewTimeHistory();

  /* Get the first 2 lines and parse them.
   */
   fgets(firstline, 256, motionfile); 
   fgets(secondline, 256, motionfile); 
   //iface->displaymessage(firstline);
   //iface->displaymessage(secondline);
 
   temp = strtok(secondline, " ");
   //iface->displaymessage(temp);

   th->number_of_datapoints = atoi(temp);

  /* FIXME: The value of the time step is at the 9th
   * token parsing on whitespace.  For now, just set it
   * at 0.02 seconds.
   */
   th->delta_t = 0.02;

   data = getTHData(th->number_of_datapoints);

   i = 0;
   while (fgets (linebuf, BUFSIZE, motionfile) != NULL)
   {
      data[i] = atof(strtok(linebuf, " "));
      i++;
      for (j = 1; j < 8; j++) 
      { 
         data[i] = atof(strtok((char *)NULL, " "));
         i++;
      }
   }

   th->data = data;
   return th;

}  /* close parseShakeFormat() */


TimeHistory *
parseMatlabFormat(FILE * motionfile)
{
   TimeHistory * th;
   char linebuf[BUFSIZE];
   char * token;
   double * data;
   int i;

   th = getNewTimeHistory();

   fgets(linebuf, BUFSIZE, motionfile);

   token = strtok(linebuf," ");
   //iface->displaymessage(token);
   th->delta_t = atof(token);
   token = strtok(NULL," ");
   th->number_of_datapoints = atoi(token);

   //iface->displaymessage(token);
   data = getTHData(th->number_of_datapoints);

   i = 0;
   while (fgets (linebuf, BUFSIZE, motionfile) != NULL)
   {
      data[i] = atof(strtok(linebuf, " "));
      i++;
   }
 
   th->data = data;
   return th;

}  /* close parseMatlabMotion() */



TimeHistory * 
getTimeHistory(char * filename, format fileformat)
{
   TimeHistory * th;
   //char  firstline[256];
   //char  secondline[256];
   //char linebuf[BUFSIZE];

   FILE * motionfile;
   //double * data; 
   //int i, j;

   if (filename == NULL)
      handleBadFileName((char*)filename);

   motionfile = fopen(filename,"r");

   if (motionfile == NULL)
      handleBadFileName((char*)filename);


   switch (fileformat)
   {
      case shake:
         //iface->displaymessage("shake\n");
         th = parseShakeFormat(motionfile);
         break;

       case matlab:
         th = parseMatlabFormat(motionfile);
         //iface->displaymessage("matlab\n");
         break;
       
       default:
          dda_display_warning("Warning: need a file format to parse\n");
          break;       
   }

   fclose(motionfile);
   writeATHistory(th);
   return th;

}  /* close getTimeHistory() */



static double *
getTHData(int n)
{
   double * data;
   size_t size;
  /* This may need to be n + 1 */
   size = (n)*sizeof(double);
   data = (double *)malloc(size);
   memset(data,0xDA,size);
   return data;
}  /* close getTHData() */


static TimeHistory * 
getNewTimeHistory()
{
   TimeHistory * th;
   th = (TimeHistory *)malloc(sizeof(TimeHistory));
   memset(th,0xDA,sizeof(TimeHistory));
   return th;
}  /* close getNewTimeHistory() */


void * 
freeTHData(double * data, int n)
{
   memset(data, 0xDD, n*sizeof(double));
   free(data);
   return NULL;
}  /* close freeTHData() */


void * 
destroyTimeHistory(TimeHistory * th)
{
  /* FIXME: Shred the data first!! */
   th->data = freeTHData(th->data, th->number_of_datapoints);
   memset(th,0xDD,sizeof(TimeHistory));
   free(th);
   return NULL;
}  /* close destroyTimeHistory() */

