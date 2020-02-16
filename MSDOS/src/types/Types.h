#ifndef TYPES_H
#define TYPES_H

#include "Error.h"

#ifdef TYPES_IMPORT
#define EXTERN
#else
#define EXTERN extern
#endif

typedef struct Color_t {
    unsigned char r;
    unsigned char b;
    unsigned char g;
} Color;

typedef struct PCXFile_t {
    unsigned int width, height;
    Color palette[256];
    unsigned char *data;
} PCXFile_t, *PCXFile;

typedef struct BMPFile_t {
	unsigned int width, height;
	Color palette[256];
	unsigned char *data;
} BMPFile_t, *BMPFile;

typedef struct Sprite_t {
    int x, y;
    int lastX, lastY;
    unsigned int width, height;
    unsigned int pageWidth;
    unsigned int transparent;
    unsigned char firstRender;
    unsigned char* data;
} Sprite, *SpritePtr;

#define MAX_ANIMATIONS      8
#define MAX_SPRITE_FRAMES   16

typedef struct AnimationData_t {
    unsigned char** data[MAX_ANIMATIONS];
    unsigned int currentAnimation;
    unsigned int animationCount;
    unsigned int currentFrame;
    unsigned int pageWidth[MAX_ANIMATIONS];
    unsigned int frameCount[MAX_ANIMATIONS];
    unsigned int updateRate[MAX_ANIMATIONS];
    unsigned int updateTick;
    unsigned char animating;
} AnimationData, *AnimationDataPtr;

typedef struct AnimatedSprite_t {
    int x, y;
    unsigned int width, height;
    unsigned int transparent;
    unsigned char firstRender;
    AnimationDataPtr animationData;
} AnimatedSprite, *AnimatedSpritePtr;

EXTERN ErrorState Types_makeSprite(unsigned char* data, unsigned int width, unsigned int height, unsigned int pageWidth, unsigned int transparent, SpritePtr *outSprite);
EXTERN ErrorState Types_addOrCreateAnimation(unsigned char** data, unsigned int frameCount, unsigned int pageWidth, unsigned int updateRate, AnimationDataPtr *animationData);
EXTERN ErrorState Types_makeAnimatedSprite(AnimationDataPtr animationData, unsigned int width, unsigned int height, unsigned int transparent, AnimatedSpritePtr *outSprite);
EXTERN void Types_freeSprite(SpritePtr* sprite);
EXTERN void Types_freeAnimatedSprite(AnimatedSpritePtr* sprite);
EXTERN void Types_freeAnimationData(AnimationDataPtr* animationData);

#undef EXTERN
#undef TYPES_IMPORT

#endif