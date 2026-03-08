// ===============================================
//  src/audio.c — MP3 완전 안정 버전 (★ 필수 수정 적용)
// ===============================================

#include "audio.h"
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>

// ------------------------------------------------------
//   효과음 버퍼
// ------------------------------------------------------
static Mix_Chunk *s_flip_success = NULL;
static Mix_Chunk *s_flip_fail    = NULL;
static Mix_Chunk *s_combo        = NULL;
static Mix_Chunk *s_win          = NULL;
static Mix_Chunk *s_lose         = NULL;
static Mix_Chunk *s_click        = NULL;

// ★ 타이머 전용 채널
static Mix_Chunk *s_timer = NULL;
static int timer_channel = -1;

// ★ 시작 카운트 (1회 재생)
static Mix_Chunk *s_start_count = NULL;
static bool start_count_played = false;

// BGM
static Mix_Music *bgm = NULL;


// ------------------------------------------------------
//  초기화
// ------------------------------------------------------
bool audio_init(Config *config)
{
    // ★ 반드시 필요 (MP3 디코더 활성화)
    int flags = MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags) {
        printf("Mix_Init Error (MP3): %s\n", Mix_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer Init Error: %s\n", Mix_GetError());
        return false;
    }

    Mix_AllocateChannels(32);

    // ★ MP3 로딩
    s_flip_success = Mix_LoadWAV("assets/sounds/flip_success.mp3");
    s_flip_fail    = Mix_LoadWAV("assets/sounds/flip_fail.mp3");
    s_combo        = Mix_LoadWAV("assets/sounds/reward_event.mp3");
    s_win          = Mix_LoadWAV("assets/sounds/win.mp3");
    s_lose         = Mix_LoadWAV("assets/sounds/lose.mp3");
    s_click        = Mix_LoadWAV("assets/sounds/button_click.mp3");

    s_timer        = Mix_LoadWAV("assets/sounds/timer.mp3");

    // ★ 시작 카운트다운 (1회 재생용)
    s_start_count  = Mix_LoadWAV("assets/sounds/start_count.mp3");

    // BGM (선택)
    bgm = Mix_LoadMUS("assets/sounds/bgm.mp3");

    return true;
}


// ------------------------------------------------------
//   Cleanup
// ------------------------------------------------------
void audio_cleanup(void)
{
    if (s_flip_success) Mix_FreeChunk(s_flip_success);
    if (s_flip_fail)    Mix_FreeChunk(s_flip_fail);
    if (s_combo)        Mix_FreeChunk(s_combo);
    if (s_win)          Mix_FreeChunk(s_win);
    if (s_lose)         Mix_FreeChunk(s_lose);
    if (s_click)        Mix_FreeChunk(s_click);

    if (s_timer)        Mix_FreeChunk(s_timer);
    if (s_start_count)  Mix_FreeChunk(s_start_count);

    if (bgm) Mix_FreeMusic(bgm);

    Mix_CloseAudio();
    Mix_Quit();
}


// ------------------------------------------------------
//   기본 효과음
// ------------------------------------------------------
void audio_play_flip_success(void) { if (s_flip_success) Mix_PlayChannel(-1, s_flip_success, 0); }
void audio_play_flip_fail(void)    { if (s_flip_fail)    Mix_PlayChannel(-1, s_flip_fail,    0); }
void audio_play_combo(void)        { if (s_combo)        Mix_PlayChannel(-1, s_combo,         0); }
void audio_play_win(void)          { if (s_win)          Mix_PlayChannel(-1, s_win,           0); }
void audio_play_lose(void)         { if (s_lose)         Mix_PlayChannel(-1, s_lose,          0); }
void audio_play_click(void)        { if (s_click)        Mix_PlayChannel(-1, s_click,         0); }


// ------------------------------------------------------
//   타이머 효과음 (★ 전용 채널)
// ------------------------------------------------------
void audio_play_timer(void)
{
    if (!s_timer) return;

    // 타이머가 이미 재생 중이면 무시
    if (timer_channel != -1 && Mix_Playing(timer_channel))
        return;

    // 반복재생
    timer_channel = Mix_PlayChannel(-1, s_timer, -1);
}

void audio_stop_timer(void)
{
    if (timer_channel != -1)
        Mix_HaltChannel(timer_channel);

    timer_channel = -1;
}


// ------------------------------------------------------
//   start_count — 딱 1회만 재생
// ------------------------------------------------------
void audio_reset_start_count(void)
{
    start_count_played = false;
}

void audio_play_start_count_once(void)
{
    if (!s_start_count || start_count_played)
        return;

    Mix_PlayChannel(-1, s_start_count, 0);
    start_count_played = true;
}


// ------------------------------------------------------
//   BGM
// ------------------------------------------------------
void audio_play_bgm(void)
{
    if (bgm)
        Mix_PlayMusic(bgm, -1);
}

void audio_stop_bgm(void)
{
    Mix_HaltMusic();
}
