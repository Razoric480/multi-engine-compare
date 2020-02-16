#ifndef RENDER_H
#define RENDER_H

#include "Error.h"
#include "types\Types.h"

#ifdef RENDER_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

typedef enum Layer_t {
    BG, FG
} Layer;

EXTERN ErrorState Renderer_initialize(void);
EXTERN void Renderer_terminate(void);
EXTERN void Renderer_clear(unsigned char colorHandle);
EXTERN void Renderer_buildColor(unsigned char r, unsigned char b, unsigned char g, Color *color);
EXTERN void Renderer_getPaletteAt(unsigned char handle, Color *color);
EXTERN void Renderer_setPaletteAt(unsigned char handle, Color *color);
EXTERN void Renderer_swapBuffers(unsigned char toMemory);
EXTERN void Renderer_drawPCX(int x, int y, PCXFile file, Layer layer);
EXTERN void Renderer_drawBMP(int x, int y, BMPFile file, Layer layer);
EXTERN void Renderer_drawPCXOpaque(int x, int y, PCXFile file, Layer layer);
EXTERN void Renderer_drawBMPOpaque(int x, int y, BMPFile file, Layer layer);
EXTERN void Renderer_plotPixel(int x, int y, unsigned char colorHandle, Layer layer);
EXTERN void Renderer_waitForVSync(void);
EXTERN void Renderer_clearBuffer(unsigned char colorHandle, Layer layer);
EXTERN void Renderer_getBufferAt(int x, int y, int width, int height, unsigned char *dest, Layer layer);
EXTERN void Renderer_renderBufferAt(int x, int y, int width, int height, unsigned char *src, Layer layer);
EXTERN void Renderer_renderBufferPageAt(int x, int y, int width, int height, int pageWidth, unsigned char transparent, unsigned char *src, Layer layer);
EXTERN void Renderer_renderSprite(SpritePtr sprite);
EXTERN void Renderer_renderAnimatedSprite(AnimatedSpritePtr sprite);
EXTERN void Renderer_repairBuffer(void);

#undef RENDER_IMPORT
#undef EXTERN

#endif