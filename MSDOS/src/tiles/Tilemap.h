#ifndef TILEMAP_H
#define TILEMAP_H

#include "Error.h"

#ifdef TILEMAP_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define TILE_COMPILED_BITMAP    0
#define TILE_VIDEO_BITMAP       1

typedef struct TileMap_t {
    int*** tiles;
    int layerCount;
    int tilesWidthCount;
    int tilesHeightCount;
    int tileWidth;
    int tileHeight;
} TileMap, *TileMapPtr;

typedef struct Tileset_t {
    unsigned char* bitmapTypes;
    char **bitmaps;
    int tileWidth;
    int tileHeight;
    int tileCount;
} Tileset, *TilesetPtr;

EXTERN ErrorState Tile_loadTilemap(const char* path, TileMapPtr *outTilemap);
EXTERN void Tile_freeTilemap(TileMapPtr *tilemap);
EXTERN ErrorState Tile_loadTileset(const char* path, int tileWidth, int tileHeight, TilesetPtr *outTileset);
EXTERN void Tile_freeTileset(TilesetPtr *tileset);

#undef EXTERN
#undef TILEMAP_IMPORT

#endif