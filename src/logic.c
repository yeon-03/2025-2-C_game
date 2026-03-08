// ===============================================
//  logic.c — OMNISCIENT PROBABILISTIC AI
//  - "God Mode" with Handicap
//  - Accuracy %: Chance to instantly find a pair.
//  - (100 - Accuracy) %: Intentionally picks unique cards (mismatch).
// ===============================================

#include "logic.h"
#include "board.h"
#include "util.h"
#include <stdlib.h>
#include <stdbool.h>

// AI 행동 타이머
static float ai_action_timer = 0.0f;

// AI 전용 mismatch 상태
static bool  ai_mismatch_active = false;
static float ai_mismatch_timer  = 0.0f;
static int   ai_mr1 = -1, ai_mc1 = -1;
static int   ai_mr2 = -1, ai_mc2 = -1;

// 새 게임 감지용
static bool  ai_inited      = false;
static Board *last_board_ptr = NULL;


// -------------------------------------------
//  AI 상태 초기화 (새 게임마다)
// -------------------------------------------
static void reset_ai_state(Board *board)
{
    ai_action_timer   = 0.0f;
    ai_mismatch_active = false;
    ai_mismatch_timer  = 0.0f;
    ai_mr1 = ai_mc1 = ai_mr2 = ai_mc2 = -1;

    last_board_ptr = board;
    ai_inited      = true;
}


// -------------------------------------------
//  남은(매칭 안 된) 카드 개수
// -------------------------------------------
static int count_unmatched(Board *board)
{
    int cnt = 0;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (!board->cards[r][c].is_matched)
                cnt++;
        }
    }
    return cnt;
}


// -------------------------------------------
//  [God Mode] 정답(짝) 찾기
//  - 매칭 안 된 카드 중, 짝이 있는 것을 찾아냄.
// -------------------------------------------
static bool find_cheat_pair(Board *board, int *r1, int *c1, int *r2, int *c2)
{
    // 보드 전체를 뒤져서 짝을 찾음
    // 랜덤성을 위해 시작 위치를 섞거나 할 수도 있지만,
    // 간단히 순차 탐색 후, 매칭되는 목록에서 랜덤 선택하는 것이 자연스러움.

    // 1. 매칭 안 된 모든 카드 위치 수집
    int idxs[BOARD_ROWS * BOARD_COLS];
    int cnt = 0;
    for (int r=0; r<BOARD_ROWS; ++r) {
        for (int c=0; c<BOARD_COLS; ++c) {
            if (!board->cards[r][c].is_matched) {
                idxs[cnt++] = r * BOARD_COLS + c;
            }
        }
    }
    if (cnt < 2) return false;

    // 2. 셔플 (항상 같은 짝만 먼저 찾지 않게)
    for (int i=0; i<cnt; ++i) {
        int j = util_random_int(0, cnt-1);
        int tmp = idxs[i];
        idxs[i] = idxs[j];
        idxs[j] = tmp;
    }

    // 3. 짝 찾기
    for (int i=0; i<cnt; ++i) {
        int idxA = idxs[i];
        int ra = idxA / BOARD_COLS;
        int ca = idxA % BOARD_COLS;
        int valA = board->cards[ra][ca].value;

        for (int j=i+1; j<cnt; ++j) {
            int idxB = idxs[j];
            int rb = idxB / BOARD_COLS;
            int cb = idxB % BOARD_COLS;
            int valB = board->cards[rb][cb].value;

            if (valA == valB) {
                // Found pair!
                *r1 = ra; *c1 = ca;
                *r2 = rb; *c2 = cb;
                return true;
            }
        }
    }

    return false;
}


// -------------------------------------------
//  [God Mode] 오답(Mismatch) 일부러 만들기
//  - 매칭 안 된 카드 중, 서로 다른 값인 두 장 선택
// -------------------------------------------
static bool pick_intentional_mismatch(Board *board, int *r1, int *c1, int *r2, int *c2)
{
    // 1. 매칭 안 된 카드 수집
    int idxs[BOARD_ROWS * BOARD_COLS];
    int cnt = 0;
    for (int r=0; r<BOARD_ROWS; ++r) {
        for (int c=0; c<BOARD_COLS; ++c) {
            if (!board->cards[r][c].is_matched) {
                idxs[cnt++] = r * BOARD_COLS + c;
            }
        }
    }
    if (cnt < 2) return false;

    // 2. 셔플
    for (int i=0; i<cnt; ++i) {
        int j = util_random_int(0, cnt-1);
        int tmp = idxs[i];
        idxs[i] = idxs[j];
        idxs[j] = tmp;
    }

    // 3. 서로 다른 값인 쌍 찾기
    for (int i=0; i<cnt; ++i) {
        int idxA = idxs[i];
        int ra = idxA / BOARD_COLS;
        int ca = idxA % BOARD_COLS; 
        
        for (int j=i+1; j<cnt; ++j) {
            int idxB = idxs[j];
            int rb = idxB / BOARD_COLS;
            int cb = idxB % BOARD_COLS;
            
            if (board->cards[ra][ca].value != board->cards[rb][cb].value) {
                *r1 = ra; *c1 = ca;
                *r2 = rb; *c2 = cb;
                return true;
            }
        }
    }

    // 만약 남은게 전부 같은 값(짝) 뿐이라면? (이론상 2장 남으면 짝이어야 함)
    // 그래도 그냥 첫 두장 리턴 (어쩔 수 없이 정답이 됨)
    int a = idxs[0];
    int b = idxs[1];
    *r1 = a / BOARD_COLS; *c1 = a % BOARD_COLS;
    *r2 = b / BOARD_COLS; *c2 = b % BOARD_COLS;
    return true;
}


// -------------------------------------------
//  메인 AI 로직
// -------------------------------------------
void logic_ai_move(Board *board, Difficulty diff,
                   float dt, Config *cfg)
{
    if (!board) return;

    // 새 게임 감지 (board 포인터 변경 or 처음 진입)
    if (!ai_inited || board != last_board_ptr) {
        reset_ai_state(board);
    }

    // ---------------------------------------
    // 0) AI가 틀린 두 장을 보여주는 중이면
    //    타이머만 줄이고 다 되면 다시 뒤집음
    // ---------------------------------------
    if (ai_mismatch_active) {
        ai_mismatch_timer -= dt;
        if (ai_mismatch_timer <= 0.0f) {
            ai_mismatch_active = false;

            // 카드 1
            if (ai_mr1 >= 0 && ai_mr1 < BOARD_ROWS &&
                ai_mc1 >= 0 && ai_mc1 < BOARD_COLS)
            {
                Card *a = &board->cards[ai_mr1][ai_mc1];
                if (!a->is_matched) {
                    a->is_flipped = false;
                    a->animating  = false;
                    a->anim_t     = 0.0f;
                }
            }

            // 카드 2
            if (ai_mr2 >= 0 && ai_mr2 < BOARD_ROWS &&
                ai_mc2 >= 0 && ai_mc2 < BOARD_COLS)
            {
                Card *b = &board->cards[ai_mr2][ai_mc2];
                if (!b->is_matched) {
                    b->is_flipped = false;
                    b->animating  = false;
                    b->anim_t     = 0.0f;
                }
            }
            ai_mr1 = ai_mc1 = ai_mr2 = ai_mc2 = -1;
        }
        return; 
    }

    // 매칭 가능한 카드가 2장 미만이면 행동할 필요 없음
    if (count_unmatched(board) < 2)
        return;


    // ---------------------------------------
    // 1) 난이도 파라미터 세팅
    // ---------------------------------------
    float action_speed;   // 행동 간격 (초)
    float accuracy;       // 정답 확률 (Omniscient)

    if (cfg) {
        action_speed = cfg->ai[diff].reaction_ms / 1000.0f;
        accuracy     = cfg->ai[diff].accuracy; 
    } else {
        // Fallback defaults
        switch (diff) {
        case DIFF_EASY:
            action_speed = 1.0f;
            accuracy     = 0.20f;
            break;
        case DIFF_NORMAL:
            action_speed = 1.0f;
            accuracy     = 0.60f;
            break;
        case DIFF_HARD:
        default:
            action_speed = 1.0f;
            accuracy     = 0.95f;
            break;
        }
    }

    // 행동 간격 타이머
    ai_action_timer += dt;
    if (ai_action_timer < action_speed)
        return;
    ai_action_timer = 0.0f;


    // ---------------------------------------
    // 2) Omniscient Decision (God Mode Check)
    // ---------------------------------------
    int r1 = -1, c1 = -1, r2 = -1, c2 = -1;
    bool success = false;

    // 운명의 주사위 굴리기
    if (util_random_float() < accuracy) {
        // 성공: 무조건 정답 찾기
        if (find_cheat_pair(board, &r1, &c1, &r2, &c2)) {
            success = true;
        }
    }

    // 실패했거나(주사위 꽝), 정답이 없으면(이론상 희박) -> 무조건 오답 찾기
    if (!success) {
        pick_intentional_mismatch(board, &r1, &c1, &r2, &c2);
    }

    // ---------------------------------------
    // 3) 선택된 두 장 뒤집기
    // ---------------------------------------
    if (!board->cards[r1][c1].is_matched)
        board_flip_card(board, r1, c1);
    if (!board->cards[r2][c2].is_matched)
        board_flip_card(board, r2, c2);


    // ---------------------------------------
    // 4) 매칭 여부 판정 (실제 결과 처리)
    // ---------------------------------------
    if (board->cards[r1][c1].value ==
        board->cards[r2][c2].value)
    {
        // 맞춘 경우
        board_mark_matched(board, r1, c1, r2, c2);
    }
    else {
        // 틀린 경우: 0.7초 동안 보여줬다가 다시 뒤집기
        ai_mismatch_active = true;
        ai_mismatch_timer  = 0.7f;
        ai_mr1 = r1; ai_mc1 = c1;
        ai_mr2 = r2; ai_mc2 = c2;
    }
}

// -------------------------------------------
//  승리 조건 판별 (Refactored for testing)
// -------------------------------------------
int check_win_status(Board *player_board, Board *ai_board, int player_combo, int ai_combo)
{
    bool p_finish = board_all_matched(player_board);
    bool a_finish = board_all_matched(ai_board);

    if (!p_finish && !a_finish) return WIN_NONE;

    if (p_finish && a_finish) {
        // 동시 종료 시 콤보가 더 많은 쪽이 승리
        if (player_combo > ai_combo) {
            return WIN_PLAYER;
        } else {
            return WIN_AI;
        }
    }

    if (p_finish) return WIN_PLAYER;
    if (a_finish) return WIN_AI;

    return WIN_NONE;
}

