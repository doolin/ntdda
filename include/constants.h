
/** Analysis constants are parameters that (attempt to) control
 * the simulation.  Most of these parameters are related to the
 * contact algorithm or open-close.
 *
 * @todo Split out contact parameters (about 3/4 of them) from the 
 *       open-close parameters, distribute the relevant functions
 *       appropriately, and ultimately make the Constants object 
 *       and these header and source files go away.
 *
 */

#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#ifndef PRINTFUNC
#define PRINTFUNC
typedef int  (*PrintFunc)(void *, const char *, ...);
#endif

typedef struct _constants Constants;


Constants * constants_new                   (void);
Constants * constants_clone                 (Constants * c);
void        constants_delete                (Constants * c);


void        constants_set_defaults          (Constants * c);

void        constants_init                  (Constants * c,
                                             double maxdisplacement);

void        constants_print_xml             (Constants * constants,
                                             PrintFunc printer,
                                             void * stream);

void        constants_print_matlab          (Constants * c,
                                             PrintFunc printer,
                                             void * stream);


double      constants_get_w0                (Constants * c);
void        constants_set_w0                (Constants * c, 
                                             double w0);


double      constants_get_shear_norm_ratio  (Constants * c);
void        constants_set_shear_norm_ratio  (Constants * c,
                                             double shear_norm_ratio);


// was s0, hardwired to 0.0002
double      constants_get_openclose         (Constants * c);  
void        constants_set_openclose         (Constants * c,
                                             double openclose);


// was f0, hardwired to 0.0000002
double      constants_get_opencriteria      (Constants * c);  
void        constants_set_opencriteria      (Constants * c,
                                             double opencriteria);


double      constants_get_norm_spring_pen   (Constants * c);
void        constants_set_norm_spring_pen   (Constants * c,
                                             double norm_spring_pen);


double      constants_get_angle_olap        (Constants * c);
void        constants_set_angle_olap        (Constants * c,
                                             double angle_olap);


double      constants_get_min_refline_factor(Constants * c);
void        constants_set_min_refline_factor(Constants * c,
                                             double min_refline_factor);

/** Derived constants */
double      constants_get_norm_extern_dist  (Constants * c);
double      constants_get_norm_pen_dist     (Constants * c);




#ifdef __cplusplus
}
#endif

#endif  /* __CONSTANTS_H__ */
