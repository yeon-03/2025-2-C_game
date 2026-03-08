// src/logic.h
#ifndef LOGIC_H
#define LOGIC_H

#include "common.h"
#include "board.h"

// Win status codes
#define WIN_NONE   0
#define WIN_PLAYER 1
#define WIN_AI     2

// 승리 조건 판별 (동시 종료 시 콤보 비교 로직 포함)
int check_win_status(Board *player_board, Board *ai_board, int player_combo, int ai_combo);

// -------------------------------------------

void logic_ai_move(Board *board, Difficulty diff, float delta_time, Config *config);

#endif
