#ifndef WAVFILE_H
#define WAVFILE_H

#include "Error.h"

#ifdef WAVFILE_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

typedef struct WAV_File {
    unsigned char* data;
    unsigned long dataLength;
    unsigned int sampleRate;
    float duration;
    unsigned char channelCount;
} WAVFile_t;

typedef WAVFile_t* WAVFile;

EXTERN ErrorState WAVFile_loadFile(const char *path, WAVFile* outFile);
EXTERN void WAVFile_free(WAVFile *file);

#undef WAVFILE_IMPORT
#undef EXTERN

#endif /* WAVFILE_H */