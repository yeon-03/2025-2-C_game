// src/game.h
#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "board.h"
#include "logic.h"
#include "render.h"
#include "audio.h"
#include "io.h"
#include "util.h"
#include <SDL.h>

typedef struct {
    Board ai_board;
    Board player_board;

    Difficulty difficulty;
    Config   *config;

    float time_left;

    int player_combo;
    int ai_combo;

    int  sel_r, sel_c;
    bool has_first;

    // 틀린 카드 잠깐 보여주기용
    bool  mismatch_active;
    float mismatch_timer;
    int   mismatch_r1, mismatch_c1;
    int   mismatch_r2, mismatch_c2;

    bool running;
    bool show_result;
    bool player_win;
} GameState;



void game_run(SDL_Window *win, SDL_Renderer *renderer, Config *config);

#endif // GAME_H
