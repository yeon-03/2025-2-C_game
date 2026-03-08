// src/audio.h
#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#include <stdbool.h>

bool audio_init(Config *config);
void audio_cleanup(void);

void audio_reset_start_count();
void audio_play_start_count_once();

void audio_play_bgm(void);
void audio_stop_bgm(void);

void audio_play_flip_success(void);
void audio_play_flip_fail(void);
void audio_play_combo(void);
void audio_play_win(void);
void audio_play_lose(void);
void audio_play_click(void);

/* ★ 타이머 효과음 전용 */
void audio_play_timer(void);
void audio_stop_timer(void);

#endif
