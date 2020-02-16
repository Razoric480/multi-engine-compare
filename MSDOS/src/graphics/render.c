#include <alloc.h>
#include <mem.h>
#include <DOS.h>

#define RENDER_IMPORT
#include "graphics\render.h"

#define SET_MODE        0x00
#define MODE_13         0x13
#define MODE_TEXT       0x03
#define VIDEO_INTERRUPT 0x10
#define VIDEO_SIZE      64000 /* 320 * 200 */
#define VIDEO_HSIZE     VIDEO_SIZE/2
#define INPUT_STATUS    0x3DA
#define VRETRACE        0x08
#define PALETTE_GET     0x3C7
#define PALETTE_SET     0x3C8
#define PALETTE_COLOR   0x3C9

#define SC_INDEX        0x3c4
#define CRTC_INDEX      0x3d4

#define MEMORY_MODE     0x04
#define UNDERLINE_LOC   0x14
#define MODE_CONTROL    0x17

typedef struct BufferBlock_t {
    int x, y, width, height;
} BufferBlock;

static BufferBlock spriteBlocks[64];
static unsigned char drawnBlocks = 0;

static unsigned char* videoMemory = (unsigned char*)0xA0000000L;
static unsigned char* videoBuffer[2];
static unsigned char notTracing = 0;
static Layer layer = BG;

ErrorState Renderer_initialize(void) {
    videoBuffer[0] = (unsigned char*)calloc(VIDEO_SIZE, 1);
    if(!videoBuffer[0]) {
        return ERR_OUT_OF_MEMORY;
    }

    videoBuffer[1] = (unsigned char*)calloc(VIDEO_SIZE, 1);
    if(!videoBuffer[1]) {
        free(videoBuffer[0]);
        return ERR_OUT_OF_MEMORY;
    }
    
    /* union REGS regs;
        regs.h.ah = SET_MODE;
        regs.h.al = MODE_13;
        int86(VIDEO_INTERRUPT, &regs, &regs);*/
    _asm {
        mov ah,SET_MODE;
        mov al,MODE_13;
        int VIDEO_INTERRUPT;
    }

    return ERR_SUCCESS;
}

void Renderer_terminate(void) {
    if(videoBuffer[0]) {
        free(videoBuffer[0]);
        videoBuffer[0] = 0;
    }
    if(videoBuffer[1]) {
        free(videoBuffer[1]);
        videoBuffer[1] = 0;
    }

    /* union REGS regs;
        regs.h.ah = SET_MODE;
        regs.h.al = MODE_TEXT;
        int86(VIDEO_INTERRUPT, &regs, &regs);*/
    _asm {
        mov ah,SET_MODE;
        mov al,MODE_TEXT;
        int VIDEO_INTERRUPT;
    }
}

void Renderer_clear(unsigned char colorHandle) {
    /*memset(videoMemory, colorHandle, VIDEO_SIZE)*/
    _asm {
        mov AX, 0A000h;
        mov ES, AX;
        xor di, di;
        mov CX, VIDEO_HSIZE;
        mov AL, BYTE PTR colorHandle;
        mov AH, AL;
        rep stosw;
    }
}

void Renderer_clearBuffer(unsigned char colorHandle, Layer layer) {
    /*memset(videoBuffer, colorHandle, VIDEO_SIZE)*/
    unsigned char* buffer = videoBuffer[layer];
    _asm {
        push ds;
        les di, buffer;
        mov cx,VIDEO_HSIZE;
        mov al,BYTE PTR colorHandle;
        mov AH, AL;
        rep stosw;
        pop ds;
    }
}

void Renderer_buildColor(unsigned char r, unsigned char b, unsigned char g, Color *color) {
    if(color) {
        color->r = r > 63 ? 63 : r;
        color->g = g > 63 ? 63 : g;
        color->b = b > 63 ? 63 : b;
    }
}

void Renderer_getPaletteAt(unsigned char handle, Color *color) {
    if(color) {
        unsigned char r, g, b;

        /*
        outportb(PALETTE_GET, handle);
        color.r = inportb(PALETTE_COLOR);
        color.g = inportb(PALETTE_COLOR);
        color.b = inportb(PALETTE_COLOR);*/
        _asm {
            mov al,handle
            mov dx,PALETTE_GET;
            out dx,al;
            in al,dx;
            mov r,al;
            in al,dx;
            mov g,al;
            in al,dx;
            mov b,al;
        }

        color->r = r;
        color->g = g;
        color->b = b;
    }
}

void Renderer_setPaletteAt(unsigned char handle, Color *color) {
    if(color) {
        unsigned char r = color->r, g = color->g, b = color->b;

        /*
        outportb(PALETTE_SET, handle);
        outportb(PALETTE_COLOR, color.r);
        outportb(PALETTE_COLOR, color.g);
        outportb(PALETTE_COLOR, color.b);*/
        _asm {
            mov dx, PALETTE_SET;
            mov al, handle;
            out dx,al;
            mov dx, PALETTE_COLOR;
            mov al, r;
            out dx, al;
            mov al, g;
            out dx, al;
            mov al, b;
            out dx, al;
        }
    }
}

void Renderer_swapBuffers(unsigned char toMemory) {
    if(toMemory) {
        unsigned char* buffer = videoBuffer[FG];
        _asm {
            push ds;
            les di, videoMemory;
            lds si, buffer;
            mov cx,320*200/2;
            rep movsw;
            pop ds;
        }
    }
    else {
        unsigned char* bg = videoBuffer[BG];
        unsigned char* fg = videoBuffer[FG];
        _asm {
            push ds;
            les di, fg;
            lds si, bg;
            mov cx,320*200/2;
            rep movsw;
            pop ds;
        }
    }
}

void Renderer_getBufferAt(int x, int y, int width, int height, unsigned char *dest, Layer layer) {
    unsigned int offset = (y << 8) + (y << 6) + x, idy;
    unsigned int workOffset = 0;
    for(idy=0; idy<height; ++idy) {
        memcpy(&dest[workOffset], &videoBuffer[layer][offset], width);
        offset += 320;
        workOffset += width;
    }
}

void Renderer_renderBufferAt(int x, int y, int width, int height, unsigned char *src, Layer layer) {
    unsigned int offset = (y << 8) + (y << 6) + x, idy;
    unsigned int workOffset = 0;
    for(idy=0; idy<height; ++idy) {
        memcpy(&videoBuffer[layer][offset], &src[workOffset], width);
        offset += 320;
        workOffset += width;
    }
}

void renderBufferPageAt(int x, int y, int width, int height, int pageWidth, unsigned char *src, Layer layer) {
    unsigned int offset = (y << 8) + (y << 6) + x, idy;
    unsigned int workOffset = 0;
    for(idy=0; idy<height; ++idy) {
        memcpy(&videoBuffer[layer][offset], &src[workOffset], width);
        offset += 320;
        workOffset += pageWidth;
    }
}

void renderBufferPageAtTransparent(int x, int y, int width, int height, int pageWidth, unsigned char *src, Layer layer) {
    unsigned int offset = (y << 8) + (y << 6) + x, idy, idx;
    unsigned int workOffset = 0;
    unsigned char data;
    for(idy=0; idy<height; ++idy) {
        for(idx=0; idx<width; ++idx) {
            data = src[workOffset+idx];
            if(data) {
                videoBuffer[layer][offset+idx] = data;
            }
        }

        offset += 320;
        workOffset += pageWidth;
    }
}

void Renderer_renderBufferPageAt(int x, int y, int width, int height, int pageWidth, unsigned char transparent, unsigned char *src, Layer layer) {
    if(transparent) {
        renderBufferPageAtTransparent(x,y,width, height, pageWidth, src, layer);
    }
    else {
        renderBufferPageAt(x,y,width, height, pageWidth, src, layer);
    }
}

void Renderer_drawPCX(int x, int y, PCXFile file, Layer layer) {
    unsigned char *buffer = file->data;
    unsigned char data;
    unsigned int offset = (y << 8) + (y << 6) + x, idy, idx, workOffset = 0;
    unsigned int width = file->width, height = file->height;
    for(idy=0; idy<height; ++idy) {
        for(idx=0; idx<width; ++idx) {
            data = buffer[workOffset+idx];
            if(data) {
                videoBuffer[layer][offset+idx] = data;
            }
        }

        offset += 320;
        workOffset += width;
    }
}

void Renderer_drawBMP(int x, int y, BMPFile file, Layer layer) {
    unsigned char *buffer = file->data;
    unsigned char data;
    unsigned int offset = (y << 8) + (y << 6) + x, idy, idx, workOffset = 0;
    unsigned int width = file->width, height = file->height;
    for(idy=0; idy<height; ++idy) {
        for(idx=0; idx<width; ++idx) {
            data = buffer[workOffset+idx];
            if(data) {
                videoBuffer[layer][offset+idx] = data;
            }
        }

        offset += 320;
        workOffset += width;
    }
}

void Renderer_drawPCXOpaque(int x, int y, PCXFile file, Layer layer) {
    unsigned char *buffer = file->data;
    unsigned char data;
    unsigned int offset = (y << 8) + (y << 6) + x, idy, idx, workOffset = 0;
    unsigned int width = file->width, height = file->height;
    for(idy=0; idy<height; ++idy) {
        memcpy(&videoBuffer[layer][offset], &buffer[workOffset], width);

        offset += 320;
        workOffset += width;
    }
}

void Renderer_drawBMPOpaque(int x, int y, BMPFile file, Layer layer) {
    unsigned char *buffer = file->data;
    unsigned char data;
    unsigned int offset = (y << 8) + (y << 6) + x, idy, idx, workOffset = 0;
    unsigned int width = file->width, height = file->height;
    for(idy=0; idy<height; ++idy) {
        memcpy(&videoBuffer[layer][offset], &buffer[workOffset], width);

        offset += 320;
        workOffset += width;
    }
}

void Renderer_plotPixel(int x, int y, unsigned char colorHandle, Layer layer) {
    int offset = (y << 8) + (y << 6) + x;
    videoBuffer[layer][offset] = colorHandle;
}

void Renderer_waitForVSync(void) {
    while( (inportb(0x3da) & 0x08)) {
    }
    while(!(inportb(0x3da) & 0x08)) {

    }
}

void Renderer_renderSprite(SpritePtr sprite) {
    int x =sprite->x, y = sprite->y, width = sprite->width, height = sprite->height;
    /*if(!sprite->firstRender) {
        Renderer_renderBufferAt(sprite->lastX, sprite->lastY, width, height, bgBuffer, layer);    
    }
    else {
        sprite->firstRender = 0;
    }*/
    spriteBlocks[drawnBlocks].x = x;
    spriteBlocks[drawnBlocks].y = y;
    spriteBlocks[drawnBlocks].width = width;
    spriteBlocks[drawnBlocks].height = height;
    drawnBlocks++;
    Renderer_renderBufferPageAt(x, y, width, height, sprite->pageWidth, sprite->transparent, sprite->data, FG);
}

void Renderer_renderAnimatedSprite(AnimatedSpritePtr sprite) {
    AnimationDataPtr animationData = sprite->animationData;
    int x =sprite->x, y = sprite->y, width = sprite->width, height = sprite->height;
    unsigned char currentAnimation = animationData->currentAnimation;
    int pageWidth = animationData->pageWidth[currentAnimation];
    unsigned char* data = animationData->data[currentAnimation][animationData->currentFrame];
    /*if(!sprite->firstRender) {
        Renderer_renderBufferAt(sprite->lastX, sprite->lastY, width, height, bgBuffer, layer);    
    }
    else {
        sprite->firstRender = 0;
    }*/
    spriteBlocks[drawnBlocks].x = x;
    spriteBlocks[drawnBlocks].y = y;
    spriteBlocks[drawnBlocks].width = width;
    spriteBlocks[drawnBlocks].height = height;
    drawnBlocks++;
    Renderer_renderBufferPageAt(x, y, width, height, pageWidth, sprite->transparent, data, FG);
}

void Renderer_repairBuffer(void) {
    unsigned char i;
    unsigned int offset;
    for(i=0; i<drawnBlocks; ++i) {
        unsigned int x = spriteBlocks[i].x, y = spriteBlocks[i].y;
        offset = (y << 8) + (y << 6) + x;
        Renderer_renderBufferPageAt(x, y, spriteBlocks[i].width, spriteBlocks[i].height, 320, 0, videoBuffer[BG]+offset, FG);
    }
    drawnBlocks = 0;
}