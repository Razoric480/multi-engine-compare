#define ANIMATOR_IMPORT
#include "graphics\Animator.h"

void Animator_playAnimation(AnimatedSpritePtr sprite, unsigned int animation) {
    if(!sprite || animation >= sprite->animationData->animationCount) {
        return;
    }

    sprite->animationData->currentAnimation = animation;
    sprite->animationData->currentFrame = 0;
    sprite->animationData->animating = 1;
}

void Animator_updateAnimation(AnimatedSpritePtr sprite) {
    if(sprite->animationData->animating) {
        unsigned int currentAnimation = sprite->animationData->currentAnimation;
        sprite->animationData->updateTick = (sprite->animationData->updateTick+1) % sprite->animationData->updateRate[currentAnimation];
        if(sprite->animationData->updateTick == 0) {
            unsigned int frameCount = sprite->animationData->frameCount[currentAnimation];
            sprite->animationData->currentFrame = (sprite->animationData->currentFrame + 1) % frameCount;
        }
    }
}

void Animator_stop(AnimatedSpritePtr sprite) {
    sprite->animationData->animating = 0;
}