// ===============================================
//  io.c — FINAL STABLE VERSION
//  Config 안정 파싱 + Ranking 안정 저장/정렬
// ===============================================

#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ----------------------------------------------------
//  (1) JSON-like 라인 파싱 함수
// ----------------------------------------------------
static int parse_json_line(const char *line, const char *key, float *fval, int *ival)
{
    char pattern_f[128];
    char pattern_i[128];

    snprintf(pattern_f, sizeof(pattern_f), " \"%s\" : %%f", key);
    snprintf(pattern_i, sizeof(pattern_i), " \"%s\" : %%d", key);

    if (fval && sscanf(line, pattern_f, fval) == 1)
        return 1;

    if (ival && sscanf(line, pattern_i, ival) == 1)
        return 1;

    return 0;
}


// ----------------------------------------------------
//  (2) config.json 로드
// ----------------------------------------------------
void io_load_config(Config *cfg)
{
    FILE *fp = fopen("data/config.json", "r");
    if (!fp) {

        printf("config.json 없음 → 기본값 사용 + 생성\n");

        cfg->ai[DIFF_EASY].accuracy = 0.55f;
        cfg->ai[DIFF_EASY].reaction_ms = 800;

        cfg->ai[DIFF_NORMAL].accuracy = 0.75f;
        cfg->ai[DIFF_NORMAL].reaction_ms = 550;

        cfg->ai[DIFF_HARD].accuracy = 0.90f;
        cfg->ai[DIFF_HARD].reaction_ms = 350;

        // 자동 생성
        fp = fopen("data/config.json", "w");
        if (fp) {
            fprintf(fp,
                "{\n"
                "   \"easy_accuracy\"      : %.2f,\n"
                "   \"easy_reaction_ms\"   : %d,\n"
                "   \"normal_accuracy\"    : %.2f,\n"
                "   \"normal_reaction_ms\" : %d,\n"
                "   \"hard_accuracy\"      : %.2f,\n"
                "   \"hard_reaction_ms\"   : %d\n"
                "}\n",
                cfg->ai[DIFF_EASY].accuracy,
                cfg->ai[DIFF_EASY].reaction_ms,
                cfg->ai[DIFF_NORMAL].accuracy,
                cfg->ai[DIFF_NORMAL].reaction_ms,
                cfg->ai[DIFF_HARD].accuracy,
                cfg->ai[DIFF_HARD].reaction_ms
            );
            fclose(fp);
        }
        return;
    }

    char line[256];

    while (fgets(line, sizeof(line), fp)) {

        parse_json_line(line, "easy_accuracy", &cfg->ai[DIFF_EASY].accuracy, NULL);
        parse_json_line(line, "easy_reaction_ms", NULL, &cfg->ai[DIFF_EASY].reaction_ms);

        parse_json_line(line, "normal_accuracy", &cfg->ai[DIFF_NORMAL].accuracy, NULL);
        parse_json_line(line, "normal_reaction_ms", NULL, &cfg->ai[DIFF_NORMAL].reaction_ms);

        parse_json_line(line, "hard_accuracy", &cfg->ai[DIFF_HARD].accuracy, NULL);
        parse_json_line(line, "hard_reaction_ms", NULL, &cfg->ai[DIFF_HARD].reaction_ms);
    }

    fclose(fp);
}



// ----------------------------------------------------
//  (3) CSV 안전 저장 (헤더 보장 + 콤마 제거)
// ----------------------------------------------------
void io_save_ranking(const char *nickname, float clear_time, Difficulty diff)
{
    FILE *check = fopen("data/ranking.csv", "r");
    if (!check) {
        // CSV 자동 생성
        FILE *init = fopen("data/ranking.csv", "w");
        if (init) {
            fprintf(init, "Name,Time,Difficulty\n");
            fclose(init);
        }
    }
    else fclose(check);

    // 닉네임에서 쉼표 제거
    char safe_name[64];
    strncpy(safe_name, nickname, sizeof(safe_name) - 1);
    safe_name[sizeof(safe_name)-1] = '\0';
    for (int i = 0; safe_name[i]; i++)
        if (safe_name[i] == ',') safe_name[i] = '_';

    FILE *fp = fopen("data/ranking.csv", "a");
    if (!fp) {
        printf("Ranking save failed!\n");
        return;
    }

    fprintf(fp, "%s,%.2f,%d\n", safe_name, clear_time, diff);
    fclose(fp);
}



// ----------------------------------------------------
//  (4) 랭킹로드 + 조건부 정렬
// ----------------------------------------------------
static int compare_rank(const void *a, const void *b)
{
    const RankingEntry *A = (const RankingEntry*)a;
    const RankingEntry *B = (const RankingEntry*)b;

    if (A->time < B->time) return -1;
    if (A->time > B->time) return 1;
    return 0;
}


int io_load_ranking(RankingEntry *entries, int max, Difficulty diff)
{
    FILE *fp = fopen("data/ranking.csv", "r");
    if (!fp) return 0;

    char line[256];

    // 첫 줄 헤더 스킵
    fgets(line, sizeof(line), fp);

    int count = 0;

    while (fgets(line, sizeof(line), fp)) {

        // 공백/빈줄 처리
        if (strlen(line) < 3) continue;

        // BOM 제거 (윈도우 CSV 대응)
        if ((unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB &&
            (unsigned char)line[2] == 0xBF)
        {
            memmove(line, line+3, strlen(line)-2);
        }

        char name[64];
        float t;
        int d;

        if (sscanf(line, "%63[^,],%f,%d", name, &t, &d) == 3) {

            if (d == diff) {
                if (count < max) {
                    strncpy(entries[count].nickname, name,
                            sizeof(entries[count].nickname) - 1);
                    entries[count].nickname[sizeof(entries[count].nickname)-1] = '\0';

                    entries[count].time = t;
                    count++;
                }
            }
        }
    }

    fclose(fp);

    // ★ 클리어 시간 오름차순 정렬
    qsort(entries, count, sizeof(RankingEntry), compare_rank);

    return count;
}
