#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Engine.h"

#define WAVFILE_IMPORT
#include "audio\WAVFile.h"

/*
    Wave file breakdown:
        1x4 bytes   "RIFF"
        32 bytes    Length of file-8
        1x4 bytes   WAVE"
        1x4 bytes   "fmt "
        32 bytes    Length of chunk
        16 bytes    WAV file format
        16 bytes    # of channels
        32 bytes    Sample rate in Hz
        32 bytes    Average bytes per second
        16 bytes    Block align
        16 bytes    Bits per sample
        1x4 bytes   "data"
        32 bytes    Length of data
        # bytes     Actual data
*/
typedef struct Wave_Header {
    unsigned char groupID[4];
    unsigned long fileLength;
    unsigned char riffType[4];
    unsigned char chunkHeaderID[4];
    unsigned long chunkSize;
    unsigned int formatTag;
    unsigned int channelCount;
    unsigned long samplesPerSecond;
    unsigned long averageBytesPerSecond;
    unsigned int blockAlign;
    unsigned int bitsPerSample;
    unsigned char dataID[4];
    unsigned long dataLength;
} WAVHeader;

/** Public **/

ErrorState WAVFile_loadFile(const char *path, WAVFile* outFile) {
    WAVHeader fileHeader;
    FILE* file = fopen(path, "rb");

    if(!file) {
        return ERR_FILE_NOT_FOUND;
    }

    fread(&fileHeader, sizeof(WAVHeader), 1, file);

    if(fileHeader.groupID[0] != 'R' || fileHeader.groupID[1] != 'I' || fileHeader.groupID[2] != 'F' || fileHeader.groupID[3] != 'F'
    || fileHeader.riffType[0] != 'W' || fileHeader.riffType[1] != 'A' || fileHeader.riffType[2] != 'V' || fileHeader.riffType[3] != 'E') {
        fclose(file);
        return ERR_WRONG_FILE_FORMAT;
    }

    if(fileHeader.chunkHeaderID[0] != 'f' || fileHeader.chunkHeaderID[1] != 'm' || fileHeader.chunkHeaderID[2] != 't' || fileHeader.chunkHeaderID[3] != ' ') {
        fclose(file);
        return ERR_WRONG_FILE_FORMAT;
    }

    if(fileHeader.bitsPerSample != 8) {
        fclose(file);
        return ERR_WRONG_FILE_FORMAT;
    }

    if(fileHeader.dataID[0] != 'd' || fileHeader.dataID[1] != 'a' || fileHeader.dataID[2] != 't' || fileHeader.dataID[3] != 'a') {
        fclose(file);
        return ERR_WRONG_FILE_FORMAT;
    }

    (*outFile) = (WAVFile)calloc(1, sizeof(WAVFile_t));
    if(!(*outFile)) {
        fclose(file);
        return ERR_OUT_OF_MEMORY;
    }
    (*outFile)->sampleRate = fileHeader.samplesPerSecond;

    (*outFile)->dataLength = fileHeader.dataLength;
    (*outFile)->duration = (float)(*outFile)->dataLength / (float)(*outFile)->sampleRate;
    (*outFile)->channelCount = fileHeader.channelCount;
    (*outFile)->data = (char*)calloc((*outFile)->dataLength, sizeof(char));
    if(!(*outFile)->data) {
        fclose(file);
        free(outFile);
        return ERR_OUT_OF_MEMORY;
    }
    fread((*outFile)->data, (*outFile)->dataLength, sizeof(char), file);

    fclose(file);

    Engine_registerResource(outFile, WAV);

    return ERR_SUCCESS;
}

void WAVFile_free(WAVFile *file) {
    if(*file) {
        free((*file)->data);
        free(*file);
        *file = 0;
    }
}