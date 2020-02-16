#include <alloc.h>

#include "Engine.h"

#define TYPES_IMPORT
#include "types\Types.h"

ErrorState Types_makeSprite(unsigned char* data, unsigned int width, unsigned int height, unsigned int pageWidth, unsigned int transparent, SpritePtr *outSprite) {
    if(!data) {
        return ERR_NULL_PTR;
    }

    *outSprite = (SpritePtr)calloc(1, sizeof(Sprite));
    if(!(*outSprite)) {
        return ERR_OUT_OF_MEMORY;
    }

    (*outSprite)->width = width;
    (*outSprite)->height = height;
    (*outSprite)->data = data;
    (*outSprite)->pageWidth = pageWidth;
    (*outSprite)->transparent = transparent;
    (*outSprite)->firstRender = 1;

    Engine_registerResource(outSprite, SPRITE);

    return ERR_SUCCESS;
}

ErrorState Types_addOrCreateAnimation(unsigned char** data, unsigned int frameCount, unsigned int pageWidth, unsigned int updateRate, AnimationDataPtr *animationData) {
    unsigned int lastAnim;
    if(!data) {
        return ERR_NULL_PTR;
    }
    if(!(*animationData)) {
        *animationData = (AnimationDataPtr)calloc(1, sizeof(AnimationData));
        if(!(*animationData)) {
            return ERR_OUT_OF_MEMORY;
        }

        Engine_registerResource(animationData, ANIMDATA);
    }

    if((*animationData)->animationCount >= MAX_ANIMATIONS || frameCount >= MAX_SPRITE_FRAMES) {
        return ERR_ILLEGAL_ARG;
    }

    lastAnim = (*animationData)->animationCount++;
    (*animationData)->data[lastAnim] = data;
    (*animationData)->frameCount[lastAnim] = frameCount;
    (*animationData)->pageWidth[lastAnim] = pageWidth;
    (*animationData)->updateRate[lastAnim] = updateRate;

    return ERR_SUCCESS;
}

ErrorState Types_makeAnimatedSprite(AnimationDataPtr animationData, unsigned int width, unsigned int height, unsigned int transparent, AnimatedSpritePtr *outSprite) {
    if(!animationData) {
        return ERR_NULL_PTR;
    }

    *outSprite = (AnimatedSpritePtr)calloc(1, sizeof(AnimatedSprite));
    if(!(*outSprite)) {
        return ERR_OUT_OF_MEMORY;
    }

    (*outSprite)->width = width;
    (*outSprite)->height = height;
    (*outSprite)->animationData = animationData;
    (*outSprite)->transparent = transparent;
    (*outSprite)->firstRender = 1;

    Engine_registerResource(outSprite, ANIMSPRITE);
    
    return ERR_SUCCESS;
}

void Types_freeSprite(SpritePtr* sprite) {
    if(sprite) {
        free(*sprite);
        *sprite = 0;
    }
}

void Types_freeAnimatedSprite(AnimatedSpritePtr* sprite) {
    if(sprite) {
        free(*sprite);
        *sprite = 0;
    }
}

void Types_freeAnimationData(AnimationDataPtr* animationData) {
    if(*animationData) {
        free(*animationData);
        *animationData = 0;
    }
}