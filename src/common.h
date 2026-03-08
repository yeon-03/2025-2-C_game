// src/common.h
#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdio.h>

#define BOARD_ROWS 4
#define BOARD_COLS 4

// 카드 구조체
typedef struct {
    int   value;       // 카드 값 (0~7 등)
    bool  is_flipped;  // 앞면이 보이는 상태인지
    bool  is_matched;  // 이미 맞춘 카드인지
    bool  animating;   // 애니메이션 중인지
    float anim_t;      // 애니메이션 진행도 (0.0 ~ 1.0)
} Card;

// 보드 구조체
typedef struct {
    Card  cards[BOARD_ROWS][BOARD_COLS];
    float freeze_timer; // AI용 잠시 멈춤 등에 사용 가능
    int   combo;        // 연속 맞추기용 콤보
} Board;

// 난이도
typedef enum {
    DIFF_EASY = 0,
    DIFF_NORMAL = 1,
    DIFF_HARD = 2
} Difficulty;

// 난이도별 AI 설정
typedef struct {
    float accuracy;    // 0.0 ~ 1.0 : 맞출 확률
    int   reaction_ms; // 한 번 움직이기까지 대기 시간(ms)
} AIDifficultyConfig;

// 전체 설정
typedef struct {
    AIDifficultyConfig ai[3]; // EASY, NORMAL, HARD
} Config;

// 랭킹용 (io.c에서 사용하는 간단한 구조체)
typedef struct {
    char  nickname[32];
    float time;
} RankingEntry;

#endif // COMMON_H
