#include <mem.h>

#define MATRIX_IMPORT
#include "types\Matrix.h"

void Matrix_addTo(Matrix *a, Matrix *b, Matrix *result) {
    if(a && b && result) {
        int i, j;
        for(i=0; i<3; ++i) {
            for(j=0; j<3; ++j) {
                result->m[i][j] = a->m[i][j] + b->m[i][j];
            }
        }
    }
}

void Matrix_subFrom(Matrix *a, Matrix *b, Matrix *result) {
    if(a && b && result) {
        int i, j;
        for(i=0; i<3; ++i) {
            for(j=0; j<3; ++j) {
                result->m[i][j] = a->m[i][j] - b->m[i][j];
            }
        }
    }
}

void Matrix_mulWith(Matrix *a, Matrix *b, Matrix *result) {
    if(a && b && result) {
        int i, j, k;
        for(i=0; i<3; ++i) {
            for(j=0; j<3; ++j) {
                result->m[i][j] = 0;
                for(k=0; k<3; ++k) {
                    result->m[i][j] += a->m[i][k] * b->m[k][j];
                }
            }
        }
    }
}

void Matrix_scaleWith(Matrix *a, float scale, Matrix *result) {
    if(result && a) {
        int i, j;
        for(i=0; i<3; ++i) {
            for(j=0; j<3; ++j) {
                result->m[i][j] = a->m[i][j] * scale;
            }
        }
    }
}

void identity(Matrix *toIdentity) {
    if(toIdentity) {
        memset(toIdentity, 0, sizeof(Matrix));
        toIdentity->m[0][0] = toIdentity->m[1][1] = toIdentity->m[2][2] = 1;
    }
}