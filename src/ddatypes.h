#ifndef __DDATYPES_H__
#define __DDATYPES_H__

/* The following will need to be relocated into
 * a "ddatypes.h" file for convenience.  The reason
 * for explicitly declaring a type boolean is to
 * provide for platform independence.  That is,
 * `boolean' is not a defined type in the c language
 * and counting on the compiler to do the right thing
 * is an invitation to disaster.
 */
typedef int ddaboolean;

/* TODO: collect all the enums for the program to right here
 * for the time being.  Later, a more rational plan can be
 * established for restricting scope.
 */
//typedef enum _FrictionLaw {coulomb, druckerprager, voight} FrictionLaw;
typedef enum _pointtype {fixed=0,measured,load,hole} PointType;


typedef struct {
	double x;
	double y;
 int type;
} DPoint;


/* TODO: Get rid of all this stuff.  Put it into a common
 * header file for the dialogs boxes and I/O functions.
 */
typedef struct _jointmat JointMat;
typedef struct _blockmat BlockMat;
typedef struct _boltmat BoltMat;
typedef struct _pstress PStress;
typedef struct _joint Joint;


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


typedef struct _ddapoint DDAPoint;
typedef struct _ddaline DDALine;


struct _ddapoint{
	double x;
	double y;
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

struct _blockmat{
  	double dens;
  	double wt;
  	double ymod;
  	double pois;
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

#endif /* __DDATYPES_H__ */
