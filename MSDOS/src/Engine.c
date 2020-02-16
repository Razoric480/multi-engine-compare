#include <string.h>
#include <stdio.h>

#include "audio\AudCtrl.h"

#include "input\Key.h"

#include "Timer.h"

#include "graphics\render.h"

#include "types\Types.h"
#include "graphics\PCXFile.h"
#include "graphics\BMPFile.h"
#include "audio\RADFile.h"
#include "audio\WAVFile.h"

#include "types\LinkList.h"

#define ENGINE_IMPORT
#include "Engine.h"

static unsigned char audio;
static unsigned char running = 0;
static void (*gameUpdate)(void);
static void (*gameDraw)(void);
static void (*gameEnd)(void);
static unsigned long next = 2, rate = 2;

typedef struct Resource_t {
    void* resource;
    ResourceType type;
} Resource, *ResourcePtr;

static LinkList resources;

void timeCritical() {
    if(audio) {
        AudioController_update();
    }
}

ErrorState Engine_initialize(void (*start)(void), void(*update)(void), void(*draw)(void), void (*end)(void)) {
    ErrorState error;

    if(running) {
        return ERR_SUCCESS;
    }

    error = LinkList_alloc(&resources);
    if(error != ERR_SUCCESS) {
        printf("LinkList resources: %s\n", Error_toString(error));
        return error;
    }

    error = Renderer_initialize();
    if(error != ERR_SUCCESS) {
        printf("Renderer: %s\n", Error_toString(error));
        return error;
    }

    error = AudioController_init();
    if(error != ERR_SUCCESS) {
        printf("AudioController: %s\n", Error_toString(error));
        audio = 0;
    }
    else {
        audio = 1;
    }

    Timer_setTime();
    Timer_setTimeCritical(&timeCritical);
    Key_initialize();

    running = 1;
    gameUpdate = update;
    gameDraw = draw;
    gameEnd = end;

    start();

    return ERR_SUCCESS;
}

void Engine_stop(void) {
    gameEnd();
    running = 0;
}

void Engine_registerResource(void* resource, ResourceType resourceType) {
    ResourcePtr resourceNode = (ResourcePtr)calloc(1, sizeof(Resource));
    resourceNode->resource = resource;
    resourceNode->type = resourceType;
    LinkList_addLast(resources, resourceNode);
}

void Engine_freeResources() {
     while(LinkList_listSize(resources) > 0) {
        ResourcePtr resource = ((ResourcePtr)LinkList_getFirst(resources));
        ResourceType type = resource->type;
        switch(type) {
            case PCX:
                PCXFile_free(&((PCXFile)resource->resource));
                break;
            case BMP:
                BMPFile_free(&((BMPFile)resource->resource));
                break;
            case RAD:
                RADFile_free(&((RADFile)resource->resource));
                break;
            case WAV:
                WAVFile_free(&((WAVFile)resource->resource));
                break;
            case SPRITE:
                Types_freeSprite(&((SpritePtr)resource->resource));
                break;
            case ANIMSPRITE:
                Types_freeAnimatedSprite(&((AnimatedSpritePtr)resource->resource));
                break;
            case ANIMDATA:
                Types_freeAnimationData(&((AnimationDataPtr)resource->resource));
                break;
            
        }
        LinkList_removeFirst(resources);
    }
}

void Engine_terminate(void) {
    if(audio) {
        Timer_setTimeCritical(0);
        AudioController_terminate();
        audio = 0;
    }

    Key_terminate();
    Timer_resetTime();
    Renderer_terminate();

    Engine_freeResources();
    LinkList_free(&resources);
    resources = 0;
}

void Engine_run(void) {
    while(running) {        
        if(Timer_getFastTick() > next) {
            next += rate;

            gameUpdate();
        }

        gameDraw();
        Renderer_waitForVSync();
        Renderer_swapBuffers(1);
    };
}