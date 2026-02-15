

#ifndef __DDAMATRIX_H__
#define __DDAMATRIX_H__

typedef struct _ddamatrix_tag DDAMatrix;

struct _ddamatrix_tag {

   /* This is actually a boolean value, true if the
    * val pointer is initialized, false otherwise.
    */
    int initialized;

    int n;
    int m;
    DDAMatrix * this;

    double ** val;

    void (*alloc)(DDAMatrix *, int m, int n);
    void *(*free)(DDAMatrix *);
};


DDAMatrix * getNewDDAMatrix();

#endif __DDAMATRIX_H__