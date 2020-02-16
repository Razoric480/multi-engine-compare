#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "Engine.h"
#include "input\Key.h"
#include "audio\RADFile.h"
#include "audio\AudCtrl.h"
#include "graphics\render.h"
#include "graphics\PCXFile.h"
#include "types\Types.h"
#include "graphics\Animator.h"

#define GAME_IMPORT
#include "Game.h"

#define SPRITE_WIDTH    43
#define SPRITE_HEIGHT   52
#define ANI_WIDTH       56
#define ANI_HEIGHT      83

static RADFile file;
static Color color;
static PCXFile spriteSheet;
static PCXFile bg;
static SpritePtr jayzul;
static SpritePtr zorua;
static unsigned char *frameData[4];
static AnimationDataPtr animationData;
static AnimatedSpritePtr animatedSprite;

void Game_start(void) {
    
    ErrorState error = RADFile_loadFile("assets\\cybnet.rad", &file);
    if(error != ERR_SUCCESS) {
        printf("RADFile: %s\n", Error_toString(error));
    }

    error = PCXFile_loadFile("assets\\ssheet.PCX", &spriteSheet);
    if(error != ERR_SUCCESS) {
        printf("PCXFile: %s\n", Error_toString(error));
    }

    error = PCXFile_loadFile("assets\\bg.pcx", &bg);
    if(error != ERR_SUCCESS) {
        printf("PCXFile %s\n", Error_toString(error));
    }

    Types_makeSprite(spriteSheet->data, SPRITE_WIDTH, SPRITE_HEIGHT, 320, 1, &jayzul);
    Types_makeSprite(spriteSheet->data+SPRITE_WIDTH, SPRITE_WIDTH, SPRITE_HEIGHT, 320, 1, &zorua);
    jayzul->y = 50;
    zorua->y = 125;

    frameData[0] = spriteSheet->data+(SPRITE_WIDTH*2);
    frameData[1] = frameData[0]+56;
    frameData[2] = frameData[1]+56;
    frameData[3] = frameData[2]+56;
    Types_addOrCreateAnimation(frameData, 4, 320, 1, &animationData);
    Types_makeAnimatedSprite(animationData, ANI_WIDTH, ANI_HEIGHT, 1, &animatedSprite);
    animatedSprite->x = 320-ANI_WIDTH;
    animatedSprite->y = 200-ANI_HEIGHT;
    Animator_playAnimation(animatedSprite, 0);

    Renderer_clear(0);
    Renderer_clearBuffer(0, BG);
    Renderer_clearBuffer(0, FG);

    PCXFile_setPalette(bg);

    Renderer_buildColor(20, 63, 20, &color);
    Renderer_setPaletteAt(0, &color);

    Renderer_drawPCXOpaque(0, 0, bg, BG);
    Renderer_swapBuffers(0);

    if(file) {
        AudioController_playRAD(file);
    }

    srand(time(0));
}

void Game_update(void) {
    if(Key_pressed(K_SPACE)) {
        Engine_stop();
    }

    if(Key_down(K_D)) {
        zorua->x += 3;
    }
    if(Key_down(K_A)) {
        zorua->x -= 3;
    }
    if(Key_down(K_W)) {
        zorua->y -= 3;
    }
    if(Key_down(K_S)) {
        zorua->y += 3;
    }
    
    if(zorua->x < 0) {
        zorua->x = 320-zorua->width-1;
    }
    else if(zorua->x >= 320-zorua->width) {
        zorua->x = 0;
    }
    if(zorua->y < 0) {
        zorua->y = 200 - zorua->height-1;
    }
    else if(zorua->y >= 200 - zorua->height) {
        zorua->y = 0;
    }

    jayzul->x += rand()%5;

    if(jayzul->x >= 320 - jayzul->width) {
        jayzul->x = 0;
    }

    Animator_updateAnimation(animatedSprite);
}

void Game_end(void) {
}

void Game_draw(void) {
    if(spriteSheet) {
        Renderer_repairBuffer();

        Renderer_renderSprite(jayzul);
        Renderer_renderSprite(zorua);
        Renderer_renderAnimatedSprite(animatedSprite);
    }
}