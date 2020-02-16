#ifndef GAME_H
#define GAME_H

#ifdef GAME_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN void Game_start(void);
EXTERN void Game_update(void);
EXTERN void Game_end(void);
EXTERN void Game_draw(void);

#endif