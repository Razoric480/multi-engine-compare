#include <alloc.h>
#include <stdio.h>

#include "Engine.h"

#define BMPFILE_IMPORT
#include "graphics\BMPFile.h"

static void fskip(FILE *fp, int numBytes) {
	int i;
	for(i=0; i<numBytes; ++i) {
		fgetc(fp);
	}
}

ErrorState BMPFile_loadFile(const char* path, BMPFile* outFile) {
	FILE *file;
	long index;
	unsigned int numColors;
	int x;

	if((file = fopen(path, "rb")) == 0) {
		printf("Error opening file %s.\n", file);
		return ERR_FILE_NOT_FOUND;
	}

	if(fgetc(file) != 'B' || fgetc(file) != 'M') {
		fclose(file);
		printf("%s is not a bitmap file.\n", file);
		return ERR_WRONG_FILE_FORMAT;
	}

    *outFile = (BMPFile)calloc(1, sizeof(BMPFile));
    if(!*outFile) {
        fclose(file);
        return ERR_OUT_OF_MEMORY;
    }

	fskip(file, 16);
	fread(&(*outFile)->width, sizeof(unsigned int), 1, file);
	fskip(file, 2);
	fread(&(*outFile)->height, sizeof(unsigned int), 1, file);
	fskip(file,22);
	fread(&numColors, sizeof(unsigned int), 1, file);
	fskip(file,6);

	if(numColors == 0) {
		numColors = 256;
	}

    (*outFile)->data = (unsigned char*)malloc((unsigned int)((*outFile)->width*(*outFile)->height));
	if(!(*outFile)->data) {
		fclose(file);
        free(*outFile);
        *outFile = 0;
		return ERR_OUT_OF_MEMORY;
	}

	for(index=0; index<numColors; index++) {
		(*outFile)->palette[index].b = fgetc(file) >> 2;
		(*outFile)->palette[index].g = fgetc(file) >> 2;
		(*outFile)->palette[index].r = fgetc(file) >> 2;
		x = fgetc(file);
	}

	for(index=((*outFile)->height-1)*(*outFile)->width; index >= 0; index -= (*outFile)->width) {
		for(x = 0; x < (*outFile)->width; x++) {
			(*outFile)->data[(unsigned int)index+x] = (unsigned char)fgetc(file);
		}
	}

	fclose(file);

	Engine_registerResource(outFile, BMP);

    return ERR_SUCCESS;
}

void BMPFile_free(BMPFile* outfile) {
    if(*outfile) {
        free((*outfile)->data);
        free(*outfile);
        *outfile = 0;
    }
}

void BMPFile_setPalette(BMPFile file) {
    int index;
    for(index=0; index<256; ++index) {
        Renderer_setPaletteAt(index, &file->palette[index]);
    }
}