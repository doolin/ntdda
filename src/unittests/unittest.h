
#ifndef __UNITTEST_H__
#define __UNITTEST_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


/** @todo Put these into a unittest.h header file.
 */
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE 
#define TRUE (!FALSE)
#endif




/** Each unit test builds a table that can be 
 * traversed to exercise each function in the 
 * component.
 */
typedef struct _testfunc {
  int (*test)(void);
  const char * testname;
} TestFunc;



/**  Perform all of the unit testing specified in a 
 *  table.
 *
 *  @param TestFunc * an array of test functions that 
 *  are invoked sequentially from the calling function.
 *
 *  @return int TRUE if everything passes the unit test,
 *  false if any function fails its unit test.
 */
int  unittest              (TestFunc * testfunc);


/**
 * Utility functions.
 */
//Get rid of the FILE *
void unittest_print_header (FILE * ofp, 
                            const char * tag);



/** Tests for individual components. */
int   fixedpoint_test   (void);

int   polar_test        (void);



#ifdef __cplusplus
}
#endif

#endif  /* __UNITTEST_H__ */
