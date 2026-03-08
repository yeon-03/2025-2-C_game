#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

// 환경에 따라 <SDL.h> 일 수도 있습니다.
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

// SDL 리소스 묶음
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;

    // 카드 앞면 (1~8)
    SDL_Texture *card_front[9];    // [1]~[8] 사용
    SDL_Texture *card_back;

    SDL_Texture *img_combo;
    SDL_Texture *img_lose;
    SDL_Texture *img_win;

    // BGM/효과음
    Mix_Music *bgm_countdown;      // 게임 시작 카운트 다운

    Mix_Chunk *s_flip_ok;          // 뒤집기 성공
    Mix_Chunk *s_flip_fail;        // 뒤집기 실패
    Mix_Chunk *s_rank_up;          // 랭킹 갱신
    Mix_Chunk *s_interfere;        // 방해공작
    Mix_Chunk *s_button;           // 버튼클릭
    Mix_Chunk *s_bonus;            // 보상이벤트 발생
    Mix_Chunk *s_win;              // 승리
    Mix_Chunk *s_fast_timer;       // 시간 제한 타이머 빠른
    Mix_Chunk *s_lose;             // 패배

    int screen_w;
    int screen_h;
} SDLResources;

// 초기화/정리
bool sdl_init(SDLResources *res, int width, int height);
void sdl_shutdown(SDLResources *res);

// 효과음 재생 래퍼
void sdl_play_flip_ok(SDLResources *res);
void sdl_play_flip_fail(SDLResources *res);
void sdl_play_button(SDLResources *res);
void sdl_play_bonus(SDLResources *res);
void sdl_play_interfere(SDLResources *res);
void sdl_play_rank_up(SDLResources *res);
void sdl_play_win(SDLResources *res);
void sdl_play_lose(SDLResources *res);
void sdl_play_fast_timer(SDLResources *res);

// 나중에 여기다 렌더링 함수 추가하면 됩니다.
// 예) void sdl_draw_board(SDLResources *res, const Board *player, const Board *ai);

#endif // GRAPHICS_H
