/*#include "Error.h"
#include "Engine.h"
#include "Game.h"

int main() {
    ErrorState error = Engine_initialize(&Game_start, &Game_update, &Game_draw, &Game_end);
    if(error != ERR_SUCCESS) {
        return 0;
    }

    Engine_run();

    Engine_terminate();

    return 0;
}*/
#include <time.h>
#include <stdlib.h>
#include <dos.h>
#include <mem.h>
#include <stdio.h>

#include "xlib\xlib.h"
#include "graphics\PCXFile.h"
#include "xlib\XVBITMAP.H"
#include "xlib\XPAL.H"
#include "xlib\XPBITMAP.H"
#include "xlib\XBMTOOLS.H"
#include "xlib\XPBMCLIP.H"
#include "Error.h"
#include "xlib\XCBITMAP.H"
#include "xlib\XVBITMAP.H"
#include "tiles\Tilemap.h"

int main() {
    // PCXFile sheet;
    // unsigned char* linearData;
    // unsigned char* planarData;
    // unsigned char* compiledData;
    // unsigned char* videoData;
    // unsigned char* row;
    // unsigned int compiledSize;
    // int i, j;
    // x_set_mode(X_MODE_320x240, 328);
    // x_set_cliprect(1,0, 80, 240);
    // x_set_start_addr(4, 0);


    // PCXFile_loadFile("assets\\ssheet.pcx", &sheet);
    // linearData = (unsigned char*)calloc(44*52+2, 1);
    // linearData[0] = 40;
    // linearData[1] = 52;
    // for(i=0; i<52; ++i) {
    //     memcpy(linearData+2+(i*40), sheet->data+(i*sheet->width), 40);
    // }
    // for(i=1; i<256; ++i) {
    //     x_set_rgb(i, sheet->palette[i].r, sheet->palette[i].g, sheet->palette[i].b);
    // }
    
    // planarData = (unsigned char*)malloc(linearData[0]*linearData[1]*4+2);
    // x_bm_to_pbm(linearData, planarData);
    // compiledSize = x_sizeof_cbitmap(ScrnLogicalByteWidth, linearData);
    // compiledData = (unsigned char*)malloc(compiledSize);
    // x_compile_bitmap(ScrnLogicalByteWidth, linearData, compiledData);
    // videoData = x_make_vbm(linearData, (&NonVisual_Offs)+ScrnLogicalByteWidth+240);

    // for(i=4; i<320; i += 40) {
    //     for(j=0; j<480-52; j += 52) {
    //         /*x_put_cbitmap(i, j, 0, compiledData);*/
    //         /*x_put_pbm(i, j, 0, planarData);*/
    //         x_put_masked_vbm(i, j, 0, videoData);
    //     }
    // }
    
    // while(!kbhit()) {
        
    // }

    // x_text_mode();

    // free(linearData);
    // free(compiledData);
    // free(planarData);
    // free(videoData);
    // PCXFile_free(&sheet);

    TileMapPtr tilemap;
    TilesetPtr tileset;
    PCXFile palette;
    ErrorState error;
    int l, i, j, col, x, y;
    char dir = 0;

    error = Tile_loadTilemap("assets\\outdoor.tmx", &tilemap);
    if(error != ERR_SUCCESS) {
        printf("%s\n", Error_toString(error));
        return -1;
    }
    error = Tile_loadTileset("assets\\outdoor.pcx", 16, 16, &tileset);
    if(error != ERR_SUCCESS) {
        Tile_freeTilemap(&tilemap);
        printf("%s\n", Error_toString(error));
        return -1;
    }
    
    x_set_mode(X_MODE_320x240, 352);
    x_set_start_addr(16, 16);

    error = PCXFile_loadFile("assets\\outdoor.pcx", &palette);
    if(error == ERR_SUCCESS) {
        for(i=0; i<256; ++i) {
            x_set_rgb(i, palette->palette[i].r, palette->palette[i].g, palette->palette[i].b);
        }
        PCXFile_free(&palette);
    }

    for(l=0; l<tilemap->layerCount; ++l) {
        for(i=0; i<17; ++i) {
            for(j=0; j<22; ++j) {
                int tile = tilemap->tiles[l][i][j];
                char* bitmap = tileset->bitmaps[tile];
                x_put_cbitmap(j*tilemap->tileWidth, i*tilemap->tileHeight, 0, bitmap);
            }
        }
    }

    x = 16;
    while(!kbhit()) {
        if(dir == 0) {
            x++;
            if(x > 32) {
                x = 32;
                dir = 1;
            }
        }
        else {
            x--;
            if(x < 0) {
                x = 0;
                dir = 0;
            }
        }
        x_set_start_addr(x, 16);
    }

    x_text_mode();

    Tile_freeTileset(&tileset);
    Tile_freeTilemap(&tilemap);
    
    return 0;
}