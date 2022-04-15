#pragma once

#include <stdbool.h>
#include <wchar.h>

struct font_list {
  size_t num;
  wchar_t **sorted;
};

struct font_similar {
  int idx;
  int score;
};

bool font_list_create(struct font_list *const fl);
void font_list_destroy(struct font_list *const fl);

int font_list_index_of(struct font_list const *const fl, wchar_t const *const s);
struct font_similar *font_get_similar(struct font_list const *const fl, wchar_t const *const s);
