/*
 * export to fig format, v. 3.2
 *
 * http://epb1.lbl.gov/xfig/fig-format.html
 */

/*

*/
#define MAXVERTICES 25

//typedef struct _P {
//double x,y;
//int type;
//} POINT;


#if BLAH
int exportfig();

static int setScale(GRAPHICS * g, double scale_params[4]);
static double  setPrScale(GRAPHICS * g, double scale_params[4]); 
static int drawLines(double scale, POINT offset, 
          Joint *j, int numlines, int color); 
static int drawJoints(GRAPHICS *g, GEOMETRYDATA * gd, double ** d);
static int drawPoints(GRAPHICS * g, GEOMETRYDATA * gd, double ** points);
static int drawBlocks(GRAPHICS * g, GEOMETRYDATA * gd); 
static int drawBolts(GRAPHICS * g, double ** b);
static void printTime(GRAPHICS * g);
static int drawCentroids(GRAPHICS * g, DATALOG * dl);
static int drawStresses(GRAPHICS * g, DATALOG * dl);
#endif

