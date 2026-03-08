// main.c
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "game.h"
#include "io.h"
#include "util.h"
#include "audio.h"
#include "render.h"

int main(int argc, char *argv[])
{
    // SDL 초기화
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("SDL_image Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    // TTF 초기화
    if (TTF_Init() == -1) {
        printf("SDL_ttf Init Error: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // 윈도우 생성 (UI 해상도/배치 절대 변경 금지)
    SDL_Window *win = SDL_CreateWindow(
        "Speed Flip",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,                        // 기존 UI 레이아웃 유지
        SDL_WINDOW_SHOWN
    );

    if (!win) {
        printf("Window Error: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // 렌더러 생성
    SDL_Renderer *rnd = SDL_CreateRenderer(
        win,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!rnd) {
        printf("Renderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Config 로드
    Config cfg;
    io_load_config(&cfg);

    // 랭킹 파일 존재 여부 확인 및 초기화
    // (win/lose 표시, 랭킹 기능을 위해 CSV 헤더 보장)
    FILE *check = fopen("data/ranking.csv", "r");
    if (!check) {
        FILE *init = fopen("data/ranking.csv", "w");
        if (init) {
            fprintf(init, "Name,Time,Difficulty\n");
            fclose(init);
        } else {
            printf("Failed to create data/ranking.csv\n");
        }
    } else {
        fclose(check);
    }

    // 오디오 초기화 (모든 사운드는 이후 audio.c에서 mp3로 처리)
    if (!audio_init(&cfg)) {
        printf("Audio init failed\n");
        // 오디오 실패해도 게임 자체는 돌아가게 두고 싶으면 종료하지 않음
    }

    // 렌더 초기화 (card_1~8, card_back, win, lose 로딩/렌더링은 render.c에서 처리)
    if (!render_init(rnd)) {
        printf("Render init failed\n");
        audio_cleanup();
        SDL_DestroyRenderer(rnd);
        SDL_DestroyWindow(win);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // 랜덤 시드
    util_seed(SDL_GetTicks());

    // 실제 게임 실행
    game_run(win, rnd, &cfg);

    // 정리
    render_cleanup();
    audio_cleanup();

    SDL_DestroyRenderer(rnd);
    SDL_DestroyWindow(win);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
