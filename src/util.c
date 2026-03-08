// ===============================================
//  util.c — FINAL STABLE VERSION
//  안정 난수 + 안전 타임스탬프
// ===============================================

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ----------------------------------------------------------
//   난수 시드 — 더 안전한 seeding
//   SDL_GetTicks() + time + rand 섞어서 seed 고정 방지
// ----------------------------------------------------------
void util_seed(unsigned int seed)
{
    unsigned int mix = (unsigned int)time(NULL)
                     ^ (seed * 0x85ebca6b)
                     ^ (rand() * 0xc2b2ae35);

    srand(mix);
}


// ----------------------------------------------------------
//   [min, max] 정수 랜덤 — 방어 코드 강화
// ----------------------------------------------------------
int util_random_int(int min, int max)
{
    if (max < min) {
        int tmp = max;
        max = min;
        min = tmp;
    }

    // rand() 품질 향상을 위해 여러 번 섞기 (unsigned로 처리하여 음수 방지)
    unsigned int r = (unsigned int)rand() ^ ((unsigned int)rand() << 7) ^ ((unsigned int)rand() >> 3);

    return min + (int)(r % (unsigned int)(max - min + 1));
}


// ----------------------------------------------------------
//   [0.0, 1.0] 실수 랜덤 — 더 높은 정밀도
// ----------------------------------------------------------
float util_random_float(void)
{
    // 31bit 사용
    unsigned int r = ((unsigned int)rand() << 15) ^ rand();
    return (float)r / (float)0x7FFFFFFF;   // 최대값 2147483647
}


// ----------------------------------------------------------
//   타임스탬프 문자열 (스레드 안전 버전)
// ----------------------------------------------------------
void util_get_timestamp(char *buffer, size_t size)
{
    if (!buffer || size == 0) return;

    time_t t = time(NULL);

#if defined(_WIN32)
    struct tm tm_info;
    localtime_s(&tm_info, &t);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);
#else
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);
#endif
}
