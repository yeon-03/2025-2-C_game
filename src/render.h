// ===============================================
//  render.h — FINAL
// ===============================================

#ifndef RENDER_H
#define RENDER_H

#include "common.h"
#include "board.h"
#include <SDL.h>

bool render_init(SDL_Renderer *renderer);
void render_cleanup();

void render_clear();
void render_present();

void render_board(Board *board, int offset_x, int offset_y);

bool render_get_card_click(int mx, int my,
                           int offset_x, int offset_y,
                           int *out_r, int *out_c);

void render_ui(SDL_Renderer *renderer,
               float time_left,
               int ai_combo, int player_combo,
               bool show_result, bool player_win);

// ★ New: Ranking screen (SDL UI, English)
void render_ranking_screen(SDL_Renderer *renderer,
                           Difficulty diff,
                           RankingEntry *entries,
                           int count);

// ★ New: Nickname input screen
void render_nickname_input(SDL_Renderer *renderer, const char *text);

#endif
