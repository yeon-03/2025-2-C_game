// src/io.h
#ifndef IO_H
#define IO_H

#include "common.h"

void io_load_config(Config *config);
void io_save_ranking(const char *nickname, float time, Difficulty diff);
int  io_load_ranking(RankingEntry *entries, int max_entries, Difficulty diff);

#endif
