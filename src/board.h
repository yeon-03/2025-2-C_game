// src/board.h
#ifndef BOARD_H
#define BOARD_H

#include "common.h"

bool board_flip_card(Board *board, int r, int c);
bool board_check_match(Board *board, int r1, int c1, int r2, int c2);
void board_mark_matched(Board *board, int r1, int c1, int r2, int c2);
bool board_all_matched(Board *board);

#endif
