
/** 
 * material.h --- Encapsulate the material properties for 
 * the blocks in DDA.  This has been done to help test 
 * a finite deformation implementation.  The variable in 
 * the original DDA code is e0;
 */

#ifndef __MATERIAL_H__
#define __MATERIAL_H__


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


typedef struct _material Material;


Material * material_new         (int numblocks);

Material * material_clone       (Material * m);

void       material_delete      (Material * m);

double **  material_get_props   (Material * m);


void       material_set_density (Material * m,
		                           double density);

double     material_get_density (Material * m);


/** Make sure that the block type is [1,nbmats].
 * If the block type is less than 1, it is set to 
 * 1.  If greater than nbmats, it is set to nbmats.
 * 
 * @warning Invalid material numbers are just fixed
 *  instead of failing.
 *
 * @todo Make this function fail for bad input instead
 *  of fixing it silently.
 *
 * @param vindex contains start and stop indexes for the 
 *  vertex array.  The block material type is stored in 
 * the 0th (!) slot.
 *
 * @param numblocks the number of blocks in the analysis
 *  used for looping over the vindex array.
 *
 * @param nbmats number of block materials.
 */
void       material_validate    (int ** vindex, 
                                 int numblocks,
                                 int nbmats);


#ifdef __cplusplus
}
#endif


#endif  /* __MATERIAL_H__ */
