#ifndef RADFILE_H
#define RADFILE_H

#include "Error.h"

#ifdef RADFILE_IMPORT
#define EXTERN
#else
#define EXTERN extern
#endif

typedef unsigned char* RADFile;

EXTERN ErrorState RADFile_loadFile(const char* path, RADFile *outFile);
EXTERN void RADFile_free(RADFile *file);
EXTERN const char* RADFile_readDescription(RADFile file);

#undef RADFILE_IMPORT
#undef EXTERN

#endif /* RADFILE_H */