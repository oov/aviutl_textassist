#pragma once

#include <stdbool.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct font_list
{
  int num;
  PZPCWSTR sorted;
};

struct font_similar
{
  int idx;
  int score;
};

bool font_list_create(struct font_list *fl);
void font_list_destroy(struct font_list *fl);

int font_list_index_of(struct font_list *fl, PCWSTR s);
struct font_similar *font_get_similar(struct font_list *fl, PCWSTR s);
