#include <stdio.h>
#include <assert.h>
#include "logic.h"
#include "board.h" // access to Board definition

// Mock function to simulate board_all_matched behavior
bool force_p_matched = false;
bool force_a_matched = false;

// Override board_all_matched for testing without needing actual board state?
// Since we are linking against board.c, we should use the real board functions but manipulate the board state.
// board_all_matched checks if all cards have is_matched == true.

void set_board_matched(Board *b, bool all) {
    for(int r=0; r<BOARD_ROWS; r++) {
        for(int c=0; c<BOARD_COLS; c++) {
            b->cards[r][c].is_matched = all;
        }
    }
}

int main() {
    Board p_board, a_board;
    int p_combo, a_combo;
    int result;

    printf("Running Win Condition Tests...\n");

    // Case 0: No one finished
    set_board_matched(&p_board, false);
    set_board_matched(&a_board, false);
    result = check_win_status(&p_board, &a_board, 0, 0);
    assert(result == WIN_NONE);
    printf("[PASS] Case 0: No one finished -> NONE\n");

    // Case 1: Player wins normally
    set_board_matched(&p_board, true);
    set_board_matched(&a_board, false);
    result = check_win_status(&p_board, &a_board, 5, 2);
    assert(result == WIN_PLAYER);
    printf("[PASS] Case 1: Player finished only -> PLAYER\n");

    // Case 2: AI wins normally
    set_board_matched(&p_board, false);
    set_board_matched(&a_board, true);
    result = check_win_status(&p_board, &a_board, 5, 2);
    assert(result == WIN_AI);
    printf("[PASS] Case 2: AI finished only -> AI\n");

    // Case 3: Both finished, P_Combo (5) > A_Combo (3)
    set_board_matched(&p_board, true);
    set_board_matched(&a_board, true);
    p_combo = 5;
    a_combo = 3;
    result = check_win_status(&p_board, &a_board, p_combo, a_combo);
    assert(result == WIN_PLAYER);
    printf("[PASS] Case 3: Both finished, Player Combo > AI Combo -> PLAYER\n");

    // Case 4: Both finished, P_Combo (3) <= A_Combo (5) (AI wins)
    set_board_matched(&p_board, true);
    set_board_matched(&a_board, true);
    p_combo = 3;
    a_combo = 5;
    result = check_win_status(&p_board, &a_board, p_combo, a_combo);
    assert(result == WIN_AI);
    printf("[PASS] Case 4: Both finished, Player Combo < AI Combo -> AI\n");

    // Case 5: Both finished, P_Combo (5) == A_Combo (5) (AI wins by default/tie-break)
    set_board_matched(&p_board, true);
    set_board_matched(&a_board, true);
    p_combo = 5;
    a_combo = 5;
    result = check_win_status(&p_board, &a_board, p_combo, a_combo);
    assert(result == WIN_AI);
    printf("[PASS] Case 5: Both finished, Equal Combo -> AI (Default tie-break)\n");

    printf("All matching logic tests passed!\n");
    return 0;
}
