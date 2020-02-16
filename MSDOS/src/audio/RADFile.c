#include <alloc.h>
#include <string.h>
#include <stdio.h>

#include "Engine.h"

#define RADFILE_IMPORT
#include "audio\RADFile.h"

#define DESCRIPTION_START   0x12

ErrorState RADFile_loadFile(const char* path, RADFile *outFile) {
    FILE *file;
    unsigned long length, readLength;
    int i;

    file = fopen(path, "rb");
    if(!file) {
        return ERR_FILE_NOT_FOUND;
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    *outFile = (RADFile)calloc(length, sizeof(unsigned char));
    if(!*outFile) {
        fclose(file);
        return ERR_OUT_OF_MEMORY;
    }

    readLength = fread(*outFile, sizeof(unsigned char), length, file);
    fclose(file);
    if(length != readLength) {
        free(*outFile);
        *outFile = 0;
        return ERR_FILE_FAILURE;
    }

    Engine_registerResource(outFile, RAD);

    return ERR_SUCCESS;
}

void RADFile_free(RADFile *file) {
    if(file) {
        free(*file);
        *file = 0;
    }
}

const char* RADFile_readDescription(RADFile file) {
    static char buffer[128];
    unsigned char *s = file+DESCRIPTION_START;
    int i = 0;

    while(*s && i<128) {
        buffer[i++] = *s++;
    }

    return buffer;
}