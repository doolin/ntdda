
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


Material * material_new    (int numblocks);

Material * material_clone  (Material * m);

void       material_delete (Material * m);


#ifdef __cplusplus
}
#endif


#endif  /* __MATERIAL_H__ */