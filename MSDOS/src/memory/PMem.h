#ifndef PMEM_H
#define PMEM_H

#include <mem.h>

#ifdef PMEM_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define LO(x)           (x & 0xFF)
#define HI(x)           ((x >> 8 ) & 0xFF)

#undef PMEM_IMPORT
#undef EXTERN

#endif