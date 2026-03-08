// src/board.c
#include "board.h"

// 카드 뒤집기 (애니메이션 플래그 제거)
bool board_flip_card(Board *board, int r, int c)
{
    if (!board) return false;
    if (r < 0 || r >= BOARD_ROWS) return false;
    if (c < 0 || c >= BOARD_COLS) return false;

    Card *card = &board->cards[r][c];

    if (card->is_matched) return false;  // 이미 맞춘 카드
    if (card->is_flipped) return false;  // 이미 열린 카드

    // ★ 애니메이션 관련 플래그 사용 안 함
    card->is_flipped = true;
    card->animating  = false;
    card->anim_t     = 0.0f;
    return true;
}

bool board_check_match(Board *board, int r1, int c1, int r2, int c2)
{
    if (!board) return false;
    if (r1 < 0 || r1 >= BOARD_ROWS) return false;
    if (c1 < 0 || c1 >= BOARD_COLS) return false;
    if (r2 < 0 || r2 >= BOARD_ROWS) return false;
    if (c2 < 0 || c2 >= BOARD_COLS) return false;

    Card *a = &board->cards[r1][c1];
    Card *b = &board->cards[r2][c2];

    if (a->is_matched || b->is_matched) return false;

    return (a->value == b->value);
}

void board_mark_matched(Board *board, int r1, int c1, int r2, int c2)
{
    if (!board) return;
    if (r1 < 0 || r1 >= BOARD_ROWS) return;
    if (c1 < 0 || c1 >= BOARD_COLS) return;
    if (r2 < 0 || r2 >= BOARD_ROWS) return;
    if (c2 < 0 || c2 >= BOARD_COLS) return;

    Card *a = &board->cards[r1][c1];
    Card *b = &board->cards[r2][c2];

    a->is_matched = true;
    b->is_matched = true;

    // 애니메이션 안 쓰니까 false로 고정
    a->animating = false;
    b->animating = false;
    a->anim_t = 0.0f;
    b->anim_t = 0.0f;
}

bool board_all_matched(Board *board)
{
    if (!board) return false;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (!board->cards[r][c].is_matched)
                return false;
        }
    }
    return true;
}