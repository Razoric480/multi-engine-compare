#ifndef BMPFILE_H
#define BMPFILE_H

#include "Error.h"
#include "graphics\render.h"
#include "types\Types.h"

#ifdef BMPFILE_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN ErrorState BMPFile_loadFile(const char* path, BMPFile* outFile);
EXTERN void BMPFile_free(BMPFile* outfile);
EXTERN void BMPFile_setPalette(BMPFile file);

#undef BMPFILE_IMPORT
#undef EXTERN

#endif