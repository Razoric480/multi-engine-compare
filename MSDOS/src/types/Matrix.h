#ifndef MATRIX_H
#define MATRIX_H

#ifdef MATRIX_IMPORT
#define EXTERN
#else
#define EXTERN extern
#endif

typedef struct Matrix_t {
    float m[3][3];
} Matrix;

EXTERN void Matrix_addTo(Matrix *dest, Matrix *add, Matrix *result);
EXTERN void Matrix_subFrom(Matrix *dest, Matrix *sub, Matrix *result);
EXTERN void Matrix_mulWith(Matrix *dest, Matrix *mul, Matrix *result);
EXTERN void Matrix_scaleWith(Matrix *dest, float scale, Matrix *result);

EXTERN void identity(Matrix *toIdentity);

#undef EXTERN
#undef MATRIX_IMPORT

#endif