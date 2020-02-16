#include <mem.h>
#include <alloc.h>
#include <stdio.h>

#include "graphics\render.h"
#include "Engine.h"

#define PCXFILE_IMPORT
#include "graphics\PCXFile.h"

typedef struct PCXHeader_t {
    unsigned char manufacturer;
    unsigned char version;
    unsigned char encoding;
    unsigned char bitsPerPixel;
    unsigned int xMin, yMin;
    unsigned int xMax, yMax;
    unsigned int horiRes, vertRes;
    unsigned char egaPalette[48];
    unsigned char reserved;
    unsigned char numColorPlanes;
    unsigned int bytesPerLine;
    unsigned int paletteType;
    unsigned char padding[58];
} PCXHeader;

ErrorState PCXFile_loadFile(const char* path, PCXFile *outFile) {
    FILE *file;
    unsigned int numBytes, index;
    unsigned long count;
    unsigned char data;
    unsigned int size, width, height;
    PCXHeader header;

    file = fopen(path, "rb");
    if(!file) {
        return ERR_FILE_NOT_FOUND;
    }

    fread(&header, 128, 1, file);
    if(header.manufacturer != 0x0A) {
        fclose(file);
        return ERR_WRONG_FILE_FORMAT;
    }
    width = header.xMax - header.xMin + 1;
    height = header.yMax - header.yMin + 1;
    
    size = width * height;
    (*outFile) = (PCXFile)calloc(1, sizeof(PCXFile_t));
    if(!(*outFile)) {
        fclose(file);
        return ERR_OUT_OF_MEMORY;
    }

    (*outFile)->data = (unsigned char*)calloc(size, 1);
    if(!(*outFile)->data) {
        free(*outFile);
        fclose(file);
        return ERR_OUT_OF_MEMORY;
    }

    (*outFile)->width = width;
    (*outFile)->height = height;

    count = 0;

    while(count <= size) {
        data = getc(file);
        if(data >= 192) {
            numBytes = data-192;
            data = getc(file);
            memset(&(*outFile)->data[count], data, numBytes);
            count += numBytes;
        }
        else {
            (*outFile)->data[count++] = data;
        }
    }

    fseek(file, -768L, SEEK_END);
    for(index = 0; index < 256; ++index) {
        (*outFile)->palette[index].r = (getc(file) >> 2);
        (*outFile)->palette[index].g = (getc(file) >> 2);
        (*outFile)->palette[index].b = (getc(file) >> 2);
    }

    fclose(file);

    Engine_registerResource(outFile, PCX);

    return ERR_SUCCESS;
}

void PCXFile_free(PCXFile *file) {
    if(*file) {
        if((*file)->data) {
            free((*file)->data);
        }
        free(*file);
        *file = 0;
    }
}

void PCXFile_setPalette(PCXFile file) {
    int index;
    for(index=0; index<256; ++index) {
        Renderer_setPaletteAt(index, &file->palette[index]);
    }
}