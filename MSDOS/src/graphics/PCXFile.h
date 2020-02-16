#ifndef PCXFILE_H
#define PCXFILE_H

#include "Error.h"
#include "types\Types.h"
#include "graphics\render.h"

#ifdef PCXFILE_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN ErrorState PCXFile_loadFile(const char* path, PCXFile *outFile);
EXTERN void PCXFile_free(PCXFile *file);
EXTERN void PCXFile_setPalette(PCXFile file);

#undef EXTERN
#undef PCXFILE_IMPORT

#endif