// ===============================================
//  render.c — FINAL
//  - card_1~8, card_back, win, lose 이미지 로딩 & 렌더링
//  - UI 레이아웃/색상 유지 (타이머 바 + 흰 배경)
//  - Win/Lose 오버레이 표시
//  - Combo 3회 이상부터 텍스트 표시 (간단 블럭 텍스트)
//  - 심플 영어 랭킹 화면 (SDL_ttf 사용 안 함)
// ===============================================

#include "render.h"
#include <SDL_image.h>
#include <SDL_ttf.h>

// 전역 렌더러
static SDL_Renderer *rnd_global = NULL;

// 카드/결과 이미지 텍스처
static SDL_Texture *img_back = NULL;
static SDL_Texture *img_cards[9] = { NULL }; // 1~8 사용
static SDL_Texture *img_win  = NULL;
static SDL_Texture *img_lose = NULL;

// 폰트
// 폰트
static TTF_Font *g_font = NULL;
static TTF_Font *g_font_header = NULL;

// -----------------------------------------------
//  내부 유틸: 카드 위치 계산
// -----------------------------------------------
static void get_card_rect(int r, int c,
                          int offset_x, int offset_y,
                          SDL_Rect *rc)
{
    const int CARD_W = 95;
    const int CARD_H = 125;
    const int GAP    = 12;

    rc->x = offset_x + c * (CARD_W + GAP);
    rc->y = offset_y + r * (CARD_H + GAP);
    rc->w = CARD_W;
    rc->h = CARD_H;
}

// -----------------------------------------------
//  내부 유틸: 아주 단순한 블럭 텍스트 렌더링
//  (SDL_ttf 없이 텍스트 느낌만 내는 용도)
// -----------------------------------------------
static void render_simple_text(SDL_Renderer *renderer,
                               const char *text,
                               int x, int y,
                               int size)
{
    if (!renderer || !text) return;

    // 폰트가 없으면 기존 블럭 렌더링 (비상용)
    if (!g_font) {
        int w = size / 2;
        int h = size;
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        for (int i = 0; text[i]; i++) {
            SDL_Rect r = { x + i * (w + 4), y, w, h };
            SDL_RenderFillRect(renderer, &r);
        }
        return;
    }

    // 폰트 크기가 고정이라 size 파라미터는 무시하거나, 
    // 필요하다면 여러 크기의 폰트를 로드해서 골라 써야 함.
    // 여기서는 로드된 폰트 하나만 사용.
    
    SDL_Color color = { 20, 20, 20, 255 };
    SDL_Surface *surf = TTF_RenderText_Blended(g_font, text, color);
    if (surf) {
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (tex) {
            SDL_Rect dst = { x, y, surf->w, surf->h };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
        SDL_FreeSurface(surf);
    }
}

static void render_text_with_font(SDL_Renderer *renderer,
                                  TTF_Font *font,
                                  const char *text,
                                  int x, int y,
                                  SDL_Color color)
{
    if (!renderer || !font || !text) return;

    SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
    if (surf) {
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (tex) {
            SDL_Rect dst = { x, y, surf->w, surf->h };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
        SDL_FreeSurface(surf);
    }
}


// -----------------------------------------------
//  초기화 (텍스처 로드)
// -----------------------------------------------
bool render_init(SDL_Renderer *renderer)
{
    rnd_global = renderer;

    img_back = IMG_LoadTexture(renderer, "assets/images/card_back.png");
    if (!img_back) {
        printf("Failed to load card_back.png\n");
        return false;
    }

    char path[256];
    for (int i = 1; i <= 8; i++) {
        snprintf(path, sizeof(path), "assets/images/card_%d.png", i);
        img_cards[i] = IMG_LoadTexture(renderer, path);
        if (!img_cards[i]) {
            printf("Failed to load %s\n", path);
            return false;
        }
    }

    img_win  = IMG_LoadTexture(renderer, "assets/images/win.png");
    img_lose = IMG_LoadTexture(renderer, "assets/images/lose.png");

    if (!img_win)  printf("Warning: win.png not found.\n");
    if (!img_lose) printf("Warning: lose.png not found.\n");

    // 폰트 로드 (assets/font.ttf)
    // 크기는 32 정도로 고정 (필요시 여러 개 로드 가능)
    g_font = TTF_OpenFont("assets/OpenSans-VariableFont_wdth,wght.ttf", 32);
    if (!g_font) {
        printf("Failed to load font.ttf: %s\n", TTF_GetError());
        // 폰트 없어도 게임은 실행되게 (블럭 렌더링 폴백)
    }

    // 헤더용 폰트 (크게, 굵게)
    g_font_header = TTF_OpenFont("assets/OpenSans-VariableFont_wdth,wght.ttf", 48);
    if (g_font_header) {
        TTF_SetFontStyle(g_font_header, TTF_STYLE_BOLD);
    } else {
        printf("Failed to load header font: %s\n", TTF_GetError());
    }

    return true;
}


// -----------------------------------------------
//  클린업
// -----------------------------------------------
void render_cleanup()
{
    if (img_back) SDL_DestroyTexture(img_back);

    for (int i = 1; i <= 8; i++) {
        if (img_cards[i]) SDL_DestroyTexture(img_cards[i]);
    }

    if (img_win)  SDL_DestroyTexture(img_win);
    if (img_lose) SDL_DestroyTexture(img_lose);

    img_back = NULL;
    for (int i = 0; i < 9; i++) img_cards[i] = NULL;
    img_win  = NULL;
    img_lose = NULL;

    if (g_font) TTF_CloseFont(g_font);
    g_font = NULL;

    if (g_font_header) TTF_CloseFont(g_font_header);
    g_font_header = NULL;
}


// -----------------------------------------------
//  카드 클릭 판정
// -----------------------------------------------
bool render_get_card_click(int mx, int my,
                           int offset_x, int offset_y,
                           int *out_r, int *out_c)
{
    const int CARD_W = 95;
    const int CARD_H = 125;
    const int GAP    = 12;

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {

            int x = offset_x + c * (CARD_W + GAP);
            int y = offset_y + r * (CARD_H + GAP);

            if (mx >= x && mx <= x + CARD_W &&
                my >= y && my <= y + CARD_H)
            {
                *out_r = r;
                *out_c = c;
                return true;
            }
        }
    }
    return false;
}


// -----------------------------------------------
//  보드 렌더링
// -----------------------------------------------
void render_board(Board *board, int offset_x, int offset_y)
{
    if (!board) return;

    SDL_Rect rc;

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {

            get_card_rect(r, c, offset_x, offset_y, &rc);
            Card *cd = &board->cards[r][c];

            if (!cd->is_flipped) {
                SDL_RenderCopy(rnd_global, img_back, NULL, &rc);
            } else {
                int v = cd->value;
                if (v < 1 || v > 8) v = 1;
                SDL_RenderCopy(rnd_global, img_cards[v], NULL, &rc);
            }
        }
    }
}


// -----------------------------------------------
//  UI (Timer bar + Win/Lose + Combo>=3 표시)
//  ※ 기존 배치/색상 유지
// -----------------------------------------------
void render_ui(SDL_Renderer *renderer,
               float time_left,
               int ai_combo, int player_combo,
               bool show_result, bool player_win)
{
    // ----- Timer bar (기존 UI 유지) -----
    float ratio = time_left / 60.0f;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    SDL_Rect bar_bg = { 240, 30, 800, 22 };
    SDL_Rect bar_fg = { 240, 30, (int)(800 * ratio), 22 };

    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderFillRect(renderer, &bar_bg);

    SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
    SDL_RenderFillRect(renderer, &bar_fg);

    // ----- Combo 표시 (3 이상일 때만) -----
    // 기존 UI에 "추가"하는 형태라 레이아웃 무너뜨리지 않음
    if (player_combo >= 3) {
        char buf[32];
        sprintf(buf, "COMBO x%d", player_combo);
        // 오른쪽 상단 근처
        render_simple_text(renderer, buf, 900, 70, 28);
    }

    if (ai_combo >= 3) {
        char buf[32];
        sprintf(buf, "AI COMBO x%d", ai_combo);
        // 왼쪽 상단 근처
        render_simple_text(renderer, buf, 100, 70, 28);
    }

    // ----- Win / Lose 오버레이 -----
    if (show_result) {
        SDL_Texture *img = player_win ? img_win : img_lose;
        if (img) {
            SDL_Rect rc;
            rc.w = 600;
            rc.h = 300;
            rc.x = (1280 - rc.w) / 2;
            rc.y = (720  - rc.h) / 2;
            SDL_RenderCopy(renderer, img, NULL, &rc);
        }
    }
}


// -----------------------------------------------
//  심플 영어 랭킹 화면 (글자 기반, SDL_ttf 미사용)
// -----------------------------------------------
void render_ranking_screen(SDL_Renderer *renderer,
                           Difficulty diff,
                           RankingEntry *entries,
                           int count)
{
    // 배경
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_RenderClear(renderer);

    // 헤더 박스
    SDL_Rect title_rect = { 200, 40, 880, 80 };
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &title_rect);

    // 헤더 텍스트
    const char *title =
        (diff == DIFF_EASY)   ? "RANKING - EASY" :
        (diff == DIFF_NORMAL) ? "RANKING - NORMAL" :
                                "RANKING - HARD";

    // 헤더: 흰색, 굵게, 크게
    if (g_font_header) {
        SDL_Color white = { 255, 255, 255, 255 };
        // 텍스트 중앙 정렬을 위해 사이즈 측정
        int w, h;
        TTF_SizeText(g_font_header, title, &w, &h);
        int tx = 200 + (880 - w) / 2; // title_rect center
        int ty = 40 + (80 - h) / 2;
        render_text_with_font(renderer, g_font_header, title, tx, ty, white);
    } else {
        render_simple_text(renderer, title, 240, 55, 48);
    }

    // 컬럼 헤더
    render_simple_text(renderer, "Rank",    260, 160, 32);
    render_simple_text(renderer, "Name",    420, 160, 32);
    render_simple_text(renderer, "Time(s)", 700, 160, 32);

    // 랭킹 데이터
    int max_show = (count < 10 ? count : 10);
    int y   = 210;
    int gap = 38;

    for (int i = 0; i < max_show; i++) {
        char rankbuf[32];
        char timebuf[32];

        sprintf(rankbuf, "%d", i + 1);
        sprintf(timebuf, "%.2f", entries[i].time);

        render_simple_text(renderer, rankbuf,             270, y, 25);
        render_simple_text(renderer, entries[i].nickname, 420, y, 25);
        render_simple_text(renderer, timebuf,             700, y, 25);

        y += gap;
    }

    // 하단 안내문
    render_simple_text(renderer,
        "Press any key or mouse button to continue",
        340, 660, 26);
}


// -----------------------------------------------
//  닉네임 입력 화면
// -----------------------------------------------
void render_nickname_input(SDL_Renderer *renderer, const char *text)
{
    // 배경
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_RenderClear(renderer);

    // 안내 텍스트
    if (g_font_header) {
        SDL_Color black = { 20, 20, 20, 255 };
        render_text_with_font(renderer, g_font_header, "ENTER NICKNAME", 450, 200, black);
    } else {
        render_simple_text(renderer, "ENTER NICKNAME", 450, 200, 48);
    }

    // 입력 박스 (밑줄)
    SDL_Rect line = { 440, 380, 400, 4 };
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &line);

    // 입력된 텍스트
    if (text && strlen(text) > 0) {
        if (g_font_header) {
            SDL_Color blue = { 0, 100, 200, 255 };
            render_text_with_font(renderer, g_font_header, text, 450, 320, blue);
        } else {
            render_simple_text(renderer, text, 450, 320, 48);
        }
    }
    
    render_present();
}


// -----------------------------------------------
//  화면 clear/present
// -----------------------------------------------
void render_clear()
{
    SDL_SetRenderDrawColor(rnd_global, 255, 255, 255, 255);
    SDL_RenderClear(rnd_global);
}

void render_present()
{
    SDL_RenderPresent(rnd_global);
}
