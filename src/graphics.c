// src/graphics.c
#include <stdio.h>
#include <string.h>
#include "graphics.h"

// ------------------ 경로 상수 ------------------
static const char *CARD_IMG_PATHS[9] = {
    NULL,
    "assets/images/카드 1.png",
    "assets/images/카드 2.png",
    "assets/images/카드 3.png",
    "assets/images/카드 4.png",
    "assets/images/카드 5.png",
    "assets/images/카드 6.png",
    "assets/images/카드 7.png",
    "assets/images/카드 8.png"
};

#define IMG_CARD_BACK   "assets/images/카드 뒷면.png"
#define IMG_COMBO       "assets/images/combo.png"
#define IMG_LOSE        "assets/images/lose.png"
#define IMG_WIN         "assets/images/win.png"

#define S_BGM_COUNTDOWN "assets/sounds/게임 시작 카운트 다운.mp3"
#define S_FLIP_OK       "assets/sounds/뒤집기 성공.mp3"
#define S_FLIP_FAIL     "assets/sounds/뒤집기 실패.mp3"
#define S_RANK_UP       "assets/sounds/랭킹 갱신.mp3"
#define S_INTERFERE     "assets/sounds/방해공작.mp3"
#define S_BUTTON        "assets/sounds/버튼클릭.mp3"            // 실제 파일명에 맞게 변경
#define S_BONUS         "assets/sounds/보상이벤트 발생.mp3"
#define S_WIN           "assets/sounds/승리.mp3"
#define S_FAST_TIMER    "assets/sounds/시간 제한 타이머 빠른.mp3"
#define S_LOSE          "assets/sounds/패배.mp3"

// ------------------ 내부 로더 ------------------
static SDL_Texture *load_texture(SDL_Renderer *r, const char *path) {
    SDL_Texture *tex = IMG_LoadTexture(r, path);
    if (!tex) {
        fprintf(stderr, "[SDL_IMAGE] %s: %s\n", path, IMG_GetError());
    }
    return tex;
}

static Mix_Chunk *load_chunk(const char *path) {
    Mix_Chunk *c = Mix_LoadWAV(path);
    if (!c) {
        fprintf(stderr, "[SDL_MIXER] %s: %s\n", path, Mix_GetError());
    }
    return c;
}

// ------------------ 초기화/정리 ------------------
bool sdl_init(SDLResources *res, int width, int height) {
    if (!res) return false;
    memset(res, 0, sizeof(*res));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init 실패: %s\n", SDL_GetError());
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        fprintf(stderr, "IMG_Init 실패: %s\n", IMG_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Mix_OpenAudio 실패: %s\n", Mix_GetError());
        return false;
    }

    res->screen_w = width;
    res->screen_h = height;

    res->window = SDL_CreateWindow(
        "Speed Flip - 카드전쟁",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );
    if (!res->window) {
        fprintf(stderr, "SDL_CreateWindow 실패: %s\n", SDL_GetError());
        return false;
    }

    res->renderer = SDL_CreateRenderer(res->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!res->renderer) {
        fprintf(stderr, "SDL_CreateRenderer 실패: %s\n", SDL_GetError());
        return false;
    }

    // 카드 이미지
    for (int i = 1; i <= 8; i++) {
        res->card_front[i] = load_texture(res->renderer, CARD_IMG_PATHS[i]);
        if (!res->card_front[i]) return false;
    }
    res->card_back = load_texture(res->renderer, IMG_CARD_BACK);
    res->img_combo = load_texture(res->renderer, IMG_COMBO);
    res->img_lose  = load_texture(res->renderer, IMG_LOSE);
    res->img_win   = load_texture(res->renderer, IMG_WIN);

    if (!res->card_back || !res->img_combo || !res->img_lose || !res->img_win)
        return false;

    // 사운드
    res->bgm_countdown = Mix_LoadMUS(S_BGM_COUNTDOWN);
    if (!res->bgm_countdown) {
        fprintf(stderr, "Mix_LoadMUS 실패(%s): %s\n", S_BGM_COUNTDOWN, Mix_GetError());
        // BGM 없이 진행해도 되면 여기서 return 안 해도 됩니다.
    }

    res->s_flip_ok    = load_chunk(S_FLIP_OK);
    res->s_flip_fail  = load_chunk(S_FLIP_FAIL);
    res->s_rank_up    = load_chunk(S_RANK_UP);
    res->s_interfere  = load_chunk(S_INTERFERE);
    res->s_button     = load_chunk(S_BUTTON);
    res->s_bonus      = load_chunk(S_BONUS);
    res->s_win        = load_chunk(S_WIN);
    res->s_fast_timer = load_chunk(S_FAST_TIMER);
    res->s_lose       = load_chunk(S_LOSE);

    return true;
}

void sdl_shutdown(SDLResources *res) {
    if (!res) return;

    for (int i = 1; i <= 8; i++) {
        if (res->card_front[i]) SDL_DestroyTexture(res->card_front[i]);
    }
    if (res->card_back) SDL_DestroyTexture(res->card_back);
    if (res->img_combo) SDL_DestroyTexture(res->img_combo);
    if (res->img_lose)  SDL_DestroyTexture(res->img_lose);
    if (res->img_win)   SDL_DestroyTexture(res->img_win);

    if (res->bgm_countdown) Mix_FreeMusic(res->bgm_countdown);

    if (res->s_flip_ok)    Mix_FreeChunk(res->s_flip_ok);
    if (res->s_flip_fail)  Mix_FreeChunk(res->s_flip_fail);
    if (res->s_rank_up)    Mix_FreeChunk(res->s_rank_up);
    if (res->s_interfere)  Mix_FreeChunk(res->s_interfere);
    if (res->s_button)     Mix_FreeChunk(res->s_button);
    if (res->s_bonus)      Mix_FreeChunk(res->s_bonus);
    if (res->s_win)        Mix_FreeChunk(res->s_win);
    if (res->s_fast_timer) Mix_FreeChunk(res->s_fast_timer);
    if (res->s_lose)       Mix_FreeChunk(res->s_lose);

    if (res->renderer) SDL_DestroyRenderer(res->renderer);
    if (res->window)   SDL_DestroyWindow(res->window);

    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}

// ------------------ 효과음 래퍼 ------------------
static void play_chunk(Mix_Chunk *c) {
    if (c) Mix_PlayChannel(-1, c, 0);
}

void sdl_play_flip_ok(SDLResources *res)    { play_chunk(res->s_flip_ok); }
void sdl_play_flip_fail(SDLResources *res)  { play_chunk(res->s_flip_fail); }
void sdl_play_button(SDLResources *res)     { play_chunk(res->s_button); }
void sdl_play_bonus(SDLResources *res)      { play_chunk(res->s_bonus); }
void sdl_play_interfere(SDLResources *res)  { play_chunk(res->s_interfere); }
void sdl_play_rank_up(SDLResources *res)    { play_chunk(res->s_rank_up); }
void sdl_play_win(SDLResources *res)        { play_chunk(res->s_win); }
void sdl_play_lose(SDLResources *res)       { play_chunk(res->s_lose); }
void sdl_play_fast_timer(SDLResources *res) { play_chunk(res->s_fast_timer); }
