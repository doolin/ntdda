


#ifndef __DDATYPES_H_
#define __DDATYPES_H_

#include <sys/types.h>

/* The reason 
 * for explicitly declaring a type boolean is to
 * provide for platform independence.  That is,
 * `boolean' is not a defined type in the c language
 * and counting on the compiler to do the right thing
 * is an invitation to disaster.
 */
typedef int ddaboolean;

#ifndef PI
#define PI 3.141592653
#endif


#ifndef FALSE
#define FALSE 0
#endif 
#ifndef TRUE
#define TRUE (!FALSE)
#endif

#define MACHEPS (3.*((4.0/3.0)-1.0)-1.0)


/** Standard typedefs for constructing callbacks. */
#ifndef PRINTFUNC
#define PRINTFUNC
typedef int  (*PrintFunc)(void *, const char *, ...);
#endif

#ifndef DISPLAYFUNC
#define DISPLAYFUNC
typedef int  (*DisplayFunc)(const char * message);
#endif

/* TODO: collect all the enums for the program to right here
 * for the time being.  Later, a more rational plan can be 
 * established for restricting scope.
 */
//typedef enum _FrictionLaw {coulomb, druckerprager, voight} FrictionLaw;

/* TODO: Get rid of all this stuff.  Put it into a common
 * header file for the dialogs boxes and I/O functions.
 */
typedef struct _jointmat JointMat;
typedef struct _blockmat BlockMat;
typedef struct _boltmat BoltMat;
typedef struct _pstress PStress;
typedef struct _joint Joint;
typedef struct _ddapoint DDAPoint;
typedef struct _ddaline DDALine;
typedef struct _ddarect DDARect;
typedef struct _ddarfile DDARFile;  // results file

/* Try to get rid of this enum. */
typedef enum _pointtype {fixed=0,measured,load,hole, seismic} PointType;

typedef struct {
	double x;
	double y;
 int type;
} DPoint;


/* Joint: pointer to 2 endpoints, material type, number of intersections,
 * and pointer to list of node addresses 
 */
struct _joint {
  	DPoint d1;
  	DPoint d2;
  	int	type;
  	DPoint *endpt1; 
  	DPoint *endpt2;
   double epx1, epy1, epx2, epy2;
  	//EqnCoeff *coeff;
  	int jmatype;
  	int numintersect;
  	//Nodelist *firstNode;
  	//Joint *next;
   double length;
   ddaboolean is_selected;
};

struct _jointmat {

  /* Need a union of structs and an 
   * enum in here to handle the various types
   * of friction laws.
   */
   double fric;
  	double coh;
	double tens;

  	int type;
	//StrengthParam *strength;
	//JointMat *next;
};



struct _ddapoint{
   double x,y;
   double u,v;
   double origx,origy;
  /* Use these instead of the enum typedefed above. */
   enum ptype {pfixed = 0, pload, pmeasured, phole, pseisic} ptype;
  /* Get rid of this */
   int type;
   int blocknum;
};


struct _ddaline{
	double x1;
	double y1;
   double x2;
   double y2;
   int type;
};


/* FIXME: Should have same member order as MS RECT type */
struct _ddarect{
   double left;
   double top;
   double right;
   double bottom;
};

struct _blockmat{
  	double dens;
  	double wt;
  	double ymod; 
  	double pois; 
   double damping;
  	double iss[3];
  	double ist[3];
  	double ivel[3];
};

typedef struct bolt_tag {
  	DPoint d1;
  	DPoint d2;
   double x1,y1,x2,y2;
   double epx1,epy1,epx2,epy2;
   double strength;
   double pretension;
} BOLT;

struct _boltmat{
   double e00;
   double t0;
   double f0;
};

/* This struct is not used yet. */
typedef struct time_tag {
	int currTimeStep;
	int tsSaveInterval;
	double currentTime;
} TIME;

struct _pstress {
  /* End points of lines showing magnitude
   * and direction of principle stress.
   */
   double majorx1,majory1,majorx2,majory2;
   double minorx1,minory1,minorx2,minory2;
};


/* Results file.  Hopefully, this abstraction will 
 * allow subclassing or whatever to get different 
 * file output formats.
 */
struct _ddarfile {

   char * headerstring;
   char * trailerstring;
   char path[256];
   //FILE * fp;
   void (*open)(DDARFile *);
   void (*appand)(DDARFile *);
   void (*close)(DDARFile *);

};


/* DDA Block. */
struct _ddablock {

  /* Integer tag for the block. Useful for 
   * searching maybe. 
   */
   int blocknumber;
   int numvertices;
  /* (x,y) pairs */
   double * vertices;

   double area, area0;
   double S0, S1, S2, S3;

   double E; // Young's modulus
   double density, density0;
   double x0,y0;  // centroid
   double ex,ey,exy;  // strains
   double sx,sy,sxy;  //stresses

   double KE;
   double EE;  // elastic energy

  /* List of all the points associated with a 
   * particular block.
   */
   // PointList * pointlist; 

};

#endif /* __DDATYPES_H__ */
