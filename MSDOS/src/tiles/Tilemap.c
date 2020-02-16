#include <alloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>

#include "graphics\PCXFile.h"
#include "xlib\XLIB.H"
#include "xlib\XCBITMAP.H"

#define TILEMAP_IMPORT
#include "tiles\Tilemap.h"

void freeProgress(int currentl, int currenti, TileMapPtr tilemap) {
    int l, i;
    for(l=0; l<currentl; ++l) {
        for(i=0; i<currenti; ++i) {
            free(tilemap->tiles[l][i]);
        }
        free(tilemap->tiles[l]);
    }
    free(tilemap->tiles);
}

ErrorState Tile_loadTilemap(const char* path, TileMapPtr *outTilemap) {
    FILE *file;
    int i, j;
    char buffer[512];
    int width, height, tileWidth, tileHeight, layerCount = 0, ***tiles, currentLayer, id = 0;
    file = fopen(path, "r");
    if(!file) {
        return ERR_FILE_NOT_FOUND;
    }

    while((fgets(buffer, sizeof buffer, file) != 0)) {
        if(strncmp(buffer, "<map ", 5) == 0) {
            unsigned char* string = strstr(buffer, "width=")+7;
            width = atoi(string);
            string = strstr(buffer, "height=")+8;
            height =  atoi(string);
            string = strstr(buffer, "tilewidth=")+11;
            tileWidth = atoi(string);
            string = strstr(buffer, "tileheight=")+12;
            tileHeight = atoi(string);
        }
        else if(strncmp(buffer, " <layer ", 8) == 0) {
            layerCount++;
        }
    }
    fseek(file, SEEK_SET, 0);

    tiles = (int***)malloc(sizeof(int**)*layerCount);
    if(!tiles) {
        return ERR_OUT_OF_MEMORY;
    }
    for(i=0; i<layerCount; ++i) {
        tiles[i] = (int**)malloc(sizeof(int*)*height);
        if(!tiles[i]) {
            freeProgress(i, 0, *outTilemap);
            return ERR_OUT_OF_MEMORY;
        }
        for(j=0; j<height; ++j) {
            tiles[i][j] = (int*)malloc(sizeof(int)*width);
            if(!tiles[i][j]) {
                freeProgress(i, j, *outTilemap);
                return ERR_OUT_OF_MEMORY;
            }
        }
    }

    currentLayer = 0;
    while((fgets(buffer, sizeof buffer, file) != 0)) {
        if(strncmp(buffer, "  <data ", 8) == 0) {
            for(i=0; i<height; ++i) {
                unsigned char* bufferPtr;
                fgets(buffer, sizeof buffer, file);
                bufferPtr = buffer;
                for(j=0; j<width; ++j) {
                    int tile = atoi(bufferPtr);
                    if(tile > 99) {
                        bufferPtr += 4;
                    }
                    else if(tile > 9) {
                        bufferPtr += 3;
                    }
                    else {
                        bufferPtr += 2;
                    }
                    tiles[currentLayer][i][j] = tile-1;
                    if(tile-1 > id) {
                        id = tile-1;
                    }
                }
            }
            currentLayer++;
        }
    }

    fclose(file);

    *outTilemap = (TileMapPtr)malloc(sizeof(TileMap));
    if(!outTilemap) {
        freeProgress(layerCount, height, *outTilemap);
        return ERR_OUT_OF_MEMORY;
    }
    (*outTilemap)->layerCount = layerCount;
    (*outTilemap)->tiles = tiles;
    (*outTilemap)->tilesHeightCount = height;
    (*outTilemap)->tilesWidthCount = width;
    (*outTilemap)->tileWidth = tileWidth;
    (*outTilemap)->tileHeight = tileHeight;

    return ERR_SUCCESS;
}

void Tile_freeTilemap(TileMapPtr *tilemap) {
    if(*tilemap) {
        freeProgress((*tilemap)->layerCount, (*tilemap)->tilesHeightCount, *tilemap);
        free(*tilemap);
        *tilemap = 0;
    }
}

ErrorState Tile_loadTileset(const char* path, int tileWidth, int tileHeight, TilesetPtr *outTileset) {
    PCXFile file;
    char* subBitmap;
    ErrorState error;
    int i, j, row;
    int width, height, cbitmapSize = 0;
    error = PCXFile_loadFile(path, &file);
    if(error != ERR_SUCCESS) {
        return error;
    }

    *outTileset = (TilesetPtr)malloc(sizeof(Tileset));
    if(!outTileset) {
        PCXFile_free(&file);
        return ERR_OUT_OF_MEMORY;
    }

    width = file->width/tileWidth;
    height = file->height/tileHeight;

    (*outTileset)->bitmaps = (char**)malloc(width*height*sizeof(char*));
    if(!(*outTileset)->bitmaps) {
        PCXFile_free(&file);
        free(*outTileset);
        return ERR_OUT_OF_MEMORY;
    }

    (*outTileset)->bitmapTypes = (unsigned char*)calloc(width*height, 1);
    if(!(*outTileset)->bitmapTypes) {
        PCXFile_free(&file);
        free((*outTileset)->bitmaps);
        free(*outTileset);
        return ERR_OUT_OF_MEMORY;
    }

    subBitmap = (char*)malloc(2+tileWidth*tileHeight);
    if(!subBitmap) {
        PCXFile_free(&file);
        free((*outTileset)->bitmapTypes);
        free((*outTileset)->bitmaps);
        free(*outTileset);
        return ERR_OUT_OF_MEMORY;
    }
    subBitmap[0] = tileWidth;
    subBitmap[1] = tileHeight;

    (*outTileset)->tileCount = width*height;

    for(i=0; i<height; ++i) {
        int heightStartingPoint = file->width*i*tileHeight;
        for(j=0; j<width; ++j) {
            int startingPoint = heightStartingPoint + j*tileWidth;
            
            for(row=0; row<tileHeight; ++row) {
                memcpy(subBitmap+2+(row*tileWidth), file->data+startingPoint+(row*file->width), tileWidth);
            }
                
            cbitmapSize = x_sizeof_cbitmap(88, subBitmap);
            (*outTileset)->bitmaps[i*width+j] = (char*)malloc(cbitmapSize);
            x_compile_bitmap(88, subBitmap, (*outTileset)->bitmaps[i*width+j]);
        }
    }

    free(subBitmap);
    PCXFile_free(&file);

    return ERR_SUCCESS;
}

void Tile_freeTileset(TilesetPtr *tileset) {
    TilesetPtr ts = *tileset;
    int i;
    free(ts->bitmapTypes);
    for(i=0; i<ts->tileCount; ++i) {
        free(ts->bitmaps[i]);
    }
    free(ts->bitmaps);
    free(*tileset);
    *tileset = 0;
}