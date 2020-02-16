#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "types\Types.h"

#ifdef ANIMATOR_IMPORT
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN void Animator_playAnimation(AnimatedSpritePtr sprite, unsigned int animation);
EXTERN void Animator_updateAnimation(AnimatedSpritePtr sprite);
EXTERN void Animator_stop(AnimatedSpritePtr sprite);

#undef ANIMATOR_IMPORT
#undef EXTERN

#endif