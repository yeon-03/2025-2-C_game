// ===============================================
//  src/game.c — FINAL CLEAN VERSION
//  - Opening 3 sec reveal
//  - Strong but fair AI (logic.c)
//  - No animation bug
//  - SDL Ranking screen (English)
// ===============================================

#include "game.h"
#include <SDL_image.h>
#include <time.h>

// Must match render.c layout
static const int CARD_W   = 95;
// static const int CARD_H   = 125; // unused
static const int CARD_GAP = 12;


// ------------------------------------------------
//   Reset board with random pairs
// ------------------------------------------------
static void board_reset_random(Board *board)
{
    int n = BOARD_ROWS * BOARD_COLS;
    int vals[n];

    int k = 0;
    for (int v = 0; v < n / 2; v++) {
        vals[k++] = v;
        vals[k++] = v;
    }

    // shuffle
    for (int i = 0; i < n; i++) {
        int j = util_random_int(0, n - 1);
        int tmp = vals[i];
        vals[i] = vals[j];
        vals[j] = tmp;
    }

    k = 0;
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {

            Card *cd = &board->cards[r][c];
            cd->value      = (vals[k++] % 8) + 1;  // 1~8
            cd->is_flipped = false;
            cd->is_matched = false;
            cd->animating  = false;
            cd->anim_t     = 0.f;
        }
    }

    board->combo        = 0;
    board->freeze_timer = 0.f;
}


// ------------------------------------------------
//   Initialize game state
// ------------------------------------------------
static void init_game(GameState *g, Config *cfg, Difficulty diff)
{
    g->config     = cfg;
    g->difficulty = diff;

    board_reset_random(&g->ai_board);
    board_reset_random(&g->player_board);

    g->time_left = 60.f;

    g->player_combo = 0;
    g->ai_combo     = 0;

    g->sel_r = -1;
    g->sel_c = -1;
    g->has_first = false;

    g->mismatch_active = false;
    g->mismatch_timer  = 0.f;

    g->running     = true;
    g->show_result = false;
    g->player_win  = false;
}


// ------------------------------------------------
//   Opening: reveal all cards for 3 seconds
// ------------------------------------------------
static void show_opening_reveal(GameState *g,
                                SDL_Renderer *renderer,
                                int ai_offset_x,
                                int player_offset_x,
                                int board_offset_y)
{
    // First frame (stabilize textures)
    render_clear();
    render_board(&g->ai_board,     ai_offset_x,     board_offset_y);
    render_board(&g->player_board, player_offset_x, board_offset_y);
    render_ui(renderer, g->time_left,
              g->ai_combo, g->player_combo,
              g->show_result, g->player_win);
    render_present();
    SDL_Delay(50);

    // Flip all cards
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            g->ai_board.cards[r][c].is_flipped     = true;
            g->player_board.cards[r][c].is_flipped = true;
        }
    }

    Uint32 start = SDL_GetTicks();

    while (SDL_GetTicks() - start < 3000) {  // 3 seconds
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                g->running = false;
                return;
            }
        }

        render_clear();
        render_board(&g->ai_board,     ai_offset_x,     board_offset_y);
        render_board(&g->player_board, player_offset_x, board_offset_y);
        render_ui(renderer, g->time_left,
                  g->ai_combo, g->player_combo,
                  g->show_result, g->player_win);
        render_present();
    }

    // Hide all cards again
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            g->ai_board.cards[r][c].is_flipped     = false;
            g->player_board.cards[r][c].is_flipped = false;
        }
    }
}


// ------------------------------------------------
//   Difficulty selection screen
// ------------------------------------------------
static Difficulty choose_difficulty(SDL_Window *win, SDL_Renderer *rnd)
{
    int w = 1280, h = 720;
    SDL_SetWindowSize(win, w, h);

    SDL_Texture *title  = IMG_LoadTexture(rnd, "assets/images/title.png");
    SDL_Texture *easy   = IMG_LoadTexture(rnd, "assets/images/easy.png");
    SDL_Texture *normal = IMG_LoadTexture(rnd, "assets/images/normal.png");
    SDL_Texture *hard   = IMG_LoadTexture(rnd, "assets/images/hard.png");

    SDL_Rect title_rect = { w/2 - 250, 50, 500, 450 };
    int btnW = 140, btnH = 70, Y = 520;

    SDL_Rect easy_rect  = { w/2 - 240, Y, btnW, btnH };
    SDL_Rect norm_rect  = { w/2 - 70,  Y, btnW, btnH };
    SDL_Rect hard_rect  = { w/2 + 100, Y, btnW, btnH };

    while (1) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_QUIT)
                return -1;

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                if (mx >= easy_rect.x && mx <= easy_rect.x + easy_rect.w &&
                    my >= easy_rect.y && my <= easy_rect.y + easy_rect.h)
                    return DIFF_EASY;

                if (mx >= norm_rect.x && mx <= norm_rect.x + norm_rect.w &&
                    my >= norm_rect.y && my <= norm_rect.y + norm_rect.h)
                    return DIFF_NORMAL;

                if (mx >= hard_rect.x && mx <= hard_rect.x + hard_rect.w &&
                    my >= hard_rect.y && my <= hard_rect.y + hard_rect.h)
                    return DIFF_HARD;
            }
        }

        render_clear();
        SDL_RenderCopy(rnd, title,  NULL, &title_rect);
        SDL_RenderCopy(rnd, easy,   NULL, &easy_rect);
        SDL_RenderCopy(rnd, normal, NULL, &norm_rect);
        SDL_RenderCopy(rnd, hard,   NULL, &hard_rect);
        render_present();



        SDL_Delay(16);
    }
}


// ------------------------------------------------
//   Nickname input loop
// ------------------------------------------------
static void get_nickname(SDL_Window *win, SDL_Renderer *renderer, char *buffer, int max_len)
{
    SDL_StartTextInput();

    // 초기화
    memset(buffer, 0, max_len);
    
    bool done = false;
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                done = true;
                break;
            }
            
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    done = true;
                } else if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    int len = strlen(buffer);
                    if (len > 0) {
                        buffer[len - 1] = '\0';
                    }
                }
            } else if (e.type == SDL_TEXTINPUT) {
                if (strlen(buffer) + strlen(e.text.text) < max_len) {
                    strcat(buffer, e.text.text);
                }
            }
        }

        render_nickname_input(renderer, buffer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    
    // 빈 입력이면 기본값
    if (strlen(buffer) == 0) {
        strcpy(buffer, "Player");
    }
}


// ------------------------------------------------
//   Show Win/Lose result for 3 seconds
// ------------------------------------------------
static void show_game_end_result(GameState *g,
                                 SDL_Renderer *renderer,
                                 int ai_offset_x,
                                 int player_offset_x,
                                 int board_offset_y)
{
    Uint32 start = SDL_GetTicks();
    
    // Wait for 3 seconds
    while (SDL_GetTicks() - start < 3000) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                g->running = false;
                SDL_PushEvent(&e); // Propagate quit
                return;
            }
        }

        render_clear();
        render_board(&g->ai_board,     ai_offset_x,     board_offset_y);
        render_board(&g->player_board, player_offset_x, board_offset_y);
        render_ui(renderer, g->time_left,
                  g->ai_combo, g->player_combo,
                  g->show_result, g->player_win);
        render_present();
        
        SDL_Delay(16);
    }
}


// ------------------------------------------------
//   Show ranking screen
// ------------------------------------------------
static void show_ranking_screen(SDL_Renderer *renderer, Difficulty diff)
{
    RankingEntry entries[10];
    int count = io_load_ranking(entries, 10, diff);

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_PushEvent(&e);
                return;
            }
            if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
                return;
            }
        }

        render_ranking_screen(renderer, diff, entries, count);
        render_present();
        SDL_Delay(16);
    }
}


// ------------------------------------------------
//   Main game loop
// ------------------------------------------------
void game_run(SDL_Window *win, SDL_Renderer *renderer, Config *config)
{
    // util_seed() is already called in main.c,
    // so we don't need an extra srand() here.

    const int SCREEN_W   = 1280;
    const int BOARD_W    = BOARD_COLS * (CARD_W + CARD_GAP) - CARD_GAP;
    const int AI_OFFSET_X     = (SCREEN_W / 4) * 1 - BOARD_W / 2;
    const int PLAYER_OFFSET_X = (SCREEN_W / 4) * 3 - BOARD_W / 2;
    const int BOARD_OFFSET_Y  = 180;

    bool app_running = true;

    while (app_running) {

        Difficulty diff = choose_difficulty(win, renderer);
        if (diff == -1)
            return;

        // 닉네임 입력
        char nickname[32];
        get_nickname(win, renderer, nickname, 32);

        // Opening sound: play once
        audio_reset_start_count();
        audio_play_start_count_once();

        GameState g;
        init_game(&g, config, diff);

        audio_play_bgm();

        // 3-sec full reveal
        show_opening_reveal(&g, renderer,
                            AI_OFFSET_X, PLAYER_OFFSET_X, BOARD_OFFSET_Y);

        audio_play_timer();

        Uint32 last = SDL_GetTicks();

        while (g.running) {

            SDL_Event e;
            while (SDL_PollEvent(&e)) {

                if (e.type == SDL_QUIT) {
                    g.running = false;
                    app_running = false;
                    break;
                }

                if (e.type == SDL_KEYDOWN &&
                    e.key.keysym.sym == SDLK_ESCAPE)
                {
                    audio_stop_timer();
                    audio_stop_bgm();
                    g.running = false;
                    break;
                }

                // Player input
                if (!g.show_result &&
                    e.type == SDL_MOUSEBUTTONDOWN &&
                    !g.mismatch_active)
                {
                    int mx = e.button.x;
                    int my = e.button.y;

                    int r, c;
                    if (render_get_card_click(mx, my,
                                              PLAYER_OFFSET_X, BOARD_OFFSET_Y,
                                              &r, &c))
                    {
                        Card *cd = &g.player_board.cards[r][c];

                        if (!cd->is_flipped && !cd->is_matched) {

                            if (!g.has_first) {

                                if (board_flip_card(&g.player_board, r, c)) {
                                    g.sel_r = r;
                                    g.sel_c = c;
                                    g.has_first = true;
                                    audio_play_flip_success();
                                }

                            } else {

                                if ((r != g.sel_r || c != g.sel_c) &&
                                    board_flip_card(&g.player_board, r, c))
                                {
                                    if (board_check_match(&g.player_board,
                                                          g.sel_r, g.sel_c,
                                                          r, c))
                                    {
                                        // correct match
                                        board_mark_matched(&g.player_board,
                                                           g.sel_r, g.sel_c,
                                                           r, c);
                                        g.player_combo++;
                                        audio_play_combo();
                                        g.has_first = false;

                                    } else {
                                        // mismatch: briefly show, then flip back
                                        g.mismatch_active = true;
                                        g.mismatch_timer  = 0.7f;

                                        g.mismatch_r1 = g.sel_r;
                                        g.mismatch_c1 = g.sel_c;
                                        g.mismatch_r2 = r;
                                        g.mismatch_c2 = c;

                                        g.player_combo = 0;
                                        audio_play_flip_fail();
                                        g.has_first = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (!g.running)
                break;

            Uint32 now = SDL_GetTicks();
            float dt = (now - last) / 1000.f;
            last = now;

            // -------------------------
            // Timer / Player / AI
            // -------------------------
            if (!g.show_result) {

                // timer
                g.time_left -= dt;
                if (g.time_left <= 0.f) {
                    g.time_left   = 0.f;
                    g.show_result = true;
                    g.player_win  = false;
                    audio_stop_timer();
                    audio_play_lose();

                    // treat as loss: show ranking (no save)
                    show_game_end_result(&g, renderer, 
                                       AI_OFFSET_X, PLAYER_OFFSET_X, BOARD_OFFSET_Y);
                                       
                    if (g.running) {
                        show_ranking_screen(renderer, g.difficulty);
                    }
                    g.running = false;
                    continue;
                }

                // AI behavior
                logic_ai_move(&g.ai_board,
                              g.difficulty,
                              dt,
                              g.config);

                // mismatch → flip cards back after delay
                if (g.mismatch_active) {

                    g.mismatch_timer -= dt;
                    if (g.mismatch_timer <= 0.f) {

                        g.mismatch_active = false;

                        Card *a = &g.player_board.cards[g.mismatch_r1][g.mismatch_c1];
                        Card *b = &g.player_board.cards[g.mismatch_r2][g.mismatch_c2];

                        a->is_flipped = false;
                        a->animating  = false;
                        a->anim_t     = 0.0f;

                        b->is_flipped = false;
                        b->animating  = false;
                        b->anim_t     = 0.0f;
                    }
                }

                // -------------------------
                // Win / Lose checks
                // -------------------------
                // -------------------------
                // Win / Lose checks
                // -------------------------
                int win_status = check_win_status(&g.player_board, &g.ai_board, 
                                                g.player_combo, g.ai_combo);

                if (win_status != WIN_NONE) {
                    g.show_result = true;
                    
                    if (win_status == WIN_PLAYER) {
                        g.player_win = true;
                        audio_stop_timer();
                        audio_play_win();
                        
                        float clear = 60.f - g.time_left;
                        if (clear < 0.f) clear = 0.f;
                        
                        io_save_ranking(nickname, clear, g.difficulty);
                    } else {
                        g.player_win = false;
                        audio_stop_timer();
                        audio_play_lose();
                    }
                    
                    // treat as loss: show ranking (no save)
                    show_game_end_result(&g, renderer, 
                                       AI_OFFSET_X, PLAYER_OFFSET_X, BOARD_OFFSET_Y);
                                       
                    if (g.running) {
                        show_ranking_screen(renderer, g.difficulty);
                    }
                    g.running = false;
                    continue;
                }
            }

            // -------------------------
            // Rendering
            // -------------------------
            render_clear();

            render_board(&g.ai_board,     AI_OFFSET_X,     BOARD_OFFSET_Y);
            render_board(&g.player_board, PLAYER_OFFSET_X, BOARD_OFFSET_Y);

            render_ui(renderer,
                      g.time_left,
                      g.ai_combo, g.player_combo,
                      g.show_result, g.player_win);

            render_present();

            SDL_Delay(16);
        }

        audio_stop_timer();
        audio_stop_bgm();
    }
}
