#ifndef ENGINE_H
#define ENGINE_H

#include "Error.h"

#ifdef ENGINE_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

typedef enum ResourceType_t {
    PCX, BMP, RAD, WAV, SPRITE, ANIMSPRITE, ANIMDATA
} ResourceType;

EXTERN ErrorState Engine_initialize(void (*initialize)(void), void(*update)(void), void (*draw)(void), void (*terminate)(void));
EXTERN void Engine_terminate(void);
EXTERN void Engine_stop(void);
EXTERN void Engine_run(void);
EXTERN void Engine_registerResource(void* resource, ResourceType resourceType);
EXTERN void Engine_freeResources();

#undef ENGINE_IMPORT
#undef EXTERN

#endif