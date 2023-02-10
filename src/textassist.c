#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>

#include <commctrl.h>

#include "aviutl.h"

#include "fontlist.h"
#include "ods.h"
#include "version.h"

#define TEXTASSIST_NAME "\x83\x65\x83\x4C\x83\x58\x83\x67\x95\xD2\x8F\x57\x95\xE2\x8F\x95"
#define TEXTASSIST_NAME_WIDE L"テキスト編集補助"
#define CAPTION (TEXTASSIST_NAME_WIDE VERSION_WIDE)

enum {
  tag_type_unknown,
  tag_type_color,
  tag_type_position,
  tag_type_font,
  tag_type_speed,
  tag_type_wait,
  tag_type_clear,
};

struct tag_color {
  uint32_t color[2];
};

struct tag_position {
  float x, y, z;
  bool x_relative, y_relative, z_relative;
};

struct tag_font {
  int size;
  wchar_t name[64];
  bool bold, italic;
};

struct tag_speed {
  float v;
};

struct tag_wait {
  float v;
  bool per_char;
};

struct tag_clear {
  float v;
  bool per_char;
};

struct tag {
  int type;
  int pos;
  int len;
  int value_pos[3];
  int value_len[3];
  union {
    struct tag_color color;
    struct tag_position position;
    struct tag_font font;
    struct tag_speed speed;
    struct tag_wait wait;
    struct tag_clear clear;
  } value;
};

static int find_char_reverse(wchar_t const *const str, int const pos, wchar_t ch) {
  for (int i = pos; i >= 0; --i) {
    if (str[i] == ch) {
      return i;
    }
  }
  return -1;
}

static inline bool is_dec(wchar_t const c) { return L'0' <= c && c <= L'9'; }

static inline bool is_hex(wchar_t const c) {
  return (L'0' <= c && c <= L'9') || (L'a' <= c && c <= L'f') || (L'A' <= c && c <= L'F');
}

static inline bool is_float(wchar_t const c, bool *const found_dot) {
  if (is_dec(c)) {
    return true;
  }
  if (c == L'.') {
    if (*found_dot) {
      return false;
    }
    *found_dot = true;
    return true;
  }
  return false;
}

static inline bool has_sign(wchar_t const c) { return c == L'+' || c == L'-'; }

static inline int saturatei(int const v, int const minv, int const maxv) {
  return v < minv ? minv : v > maxv ? maxv : v;
}

static inline float saturatef(float const v, float const minv, float const maxv) {
  return v < minv ? minv : v > maxv ? maxv : v;
}

static int sprint_int(wchar_t *const buf, int const v, bool const omit_zero) {
  if (omit_zero && !v) {
    buf[0] = L'\0';
    return 0;
  }
  return wsprintfW(buf, L"%d", v);
}

static int sprint_float(wchar_t *const buf, float const v, bool const omit_zero) {
  int const i = (int)v;
  int const f = (int)(v * 10 * (v < 0 ? -1 : 1)) % 10;
  if (!f) {
    if (omit_zero && !i) {
      buf[0] = L'\0';
      return 0;
    }
    return wsprintfW(buf, L"%d", i);
  }
  return wsprintfW(buf, L"%d.%d", i, f);
}

static bool parse_tag(wchar_t const *const str, int const len, int const pos, struct tag *tag) {
  if (len - pos < 3 || str[pos] != L'<') {
    return false;
  }
  int type = tag_type_unknown;
  switch (str[pos + 1]) {
  case L'#':
    type = tag_type_color;
    break;
  case L'p':
    type = tag_type_position;
    break;
  case L's':
    type = tag_type_font;
    break;
  case L'r':
    type = tag_type_speed;
    break;
  case L'w':
    type = tag_type_wait;
    break;
  case L'c':
    type = tag_type_clear;
    break;
  default:
    return false;
  }
  int end = pos + 2, token = 0;
  bool found_dot = false;
  int value_pos[3] = {pos + 2, -1, -1};
  int value_len[3] = {0, 0, 0};
  for (; end < len; ++end) {
    if (str[end] == L',') {
      value_len[token] = end - value_pos[token];
      ++token;
      if (token == 3) {
        return false; // too many tokens
      }
      found_dot = false;
      value_pos[token] = end + 1;
      continue;
    }
    // '<' is not allowed in most cases, except for font name.
    if (str[end] == L'<' && (type != tag_type_font || (type == tag_type_font && token != 1))) {
      return false;
    }
    if (str[end] == L'>') {
      value_len[token] = end - value_pos[token];
      if (token == 0 && value_len[0] == 0) {
        value_pos[0] = -1;
      }
      ++token;
      // final check
      switch (type) {
      case tag_type_color:
        if (token > 2 || (value_len[0] != 0 && value_len[0] != 6) || (value_len[1] != 0 && value_len[1] != 6)) {
          return false;
        }
        break;
      case tag_type_position:
        if (token == 1) {
          return false;
        }
        break;
      case tag_type_font:
        if (value_len[1] > 255) {
          return false; // too long
        }
      }
      tag->type = type;
      tag->pos = pos;
      tag->len = end - pos + 1;
      tag->value_pos[0] = value_pos[0];
      tag->value_pos[1] = value_pos[1];
      tag->value_pos[2] = value_pos[2];
      tag->value_len[0] = value_len[0];
      tag->value_len[1] = value_len[1];
      tag->value_len[2] = value_len[2];
      switch (type) {
      case tag_type_color:
        tag->value.color.color[0] = tag->value_len[0] == 0 ? 0 : (uint32_t)wcstol(str + tag->value_pos[0], NULL, 16);
        tag->value.color.color[1] = tag->value_len[1] == 0 ? 0 : (uint32_t)wcstol(str + tag->value_pos[1], NULL, 16);
        break;
      case tag_type_position:
        tag->value.position.x = tag->value_len[0] == 0 ? 0 : wcstof(str + tag->value_pos[0], NULL);
        tag->value.position.y = tag->value_len[1] == 0 ? 0 : wcstof(str + tag->value_pos[1], NULL);
        tag->value.position.z = tag->value_len[2] == 0 ? 0 : wcstof(str + tag->value_pos[2], NULL);
        tag->value.position.x_relative = tag->value_len[0] > 0 && has_sign(str[tag->value_pos[0]]);
        tag->value.position.y_relative = tag->value_len[1] > 0 && has_sign(str[tag->value_pos[1]]);
        tag->value.position.z_relative = tag->value_len[2] > 0 && has_sign(str[tag->value_pos[2]]);
        break;
      case tag_type_font:
        tag->value.font.size = tag->value_len[0] == 0 ? 0 : wcstol(str + tag->value_pos[0], NULL, 10);
        if (tag->value_len[1] > 0) {
          memcpy(tag->value.font.name, str + tag->value_pos[1], sizeof(WCHAR) * (size_t)tag->value_len[1]);
        }
        tag->value.font.name[tag->value_len[1]] = L'\0';
        tag->value.font.bold =
            tag->value_len[2] > 0 && find_char_reverse(str + tag->value_pos[2], tag->value_len[2] - 1, L'B') != -1;
        tag->value.font.italic =
            tag->value_len[2] > 0 && find_char_reverse(str + tag->value_pos[2], tag->value_len[2] - 1, L'I') != -1;
        break;
      case tag_type_speed:
        tag->value.speed.v = tag->value_len[0] == 0 ? 0 : wcstof(str + tag->value_pos[0], NULL);
        break;
      case tag_type_wait:
        tag->value.wait.per_char = tag->value_len[0] > 0 && str[tag->value_pos[0]] == '*';
        tag->value.wait.v =
            tag->value_len[0] == 0 ? 0 : wcstof(str + tag->value_pos[0] + (tag->value.wait.per_char ? 1 : 0), NULL);
        break;
      case tag_type_clear:
        tag->value.clear.per_char = tag->value_len[0] > 0 && str[tag->value_pos[0]] == '*';
        tag->value.clear.v =
            tag->value_len[0] == 0 ? 0 : wcstof(str + tag->value_pos[0] + (tag->value.clear.per_char ? 1 : 0), NULL);
        break;
      }
      return true;
    }
    if (type == tag_type_color) {
      switch (token) {
      case 0:
      case 1:
        if (!is_hex(str[end])) {
          return false;
        }
        break;
      default:
        return false;
      }
    } else if (type == tag_type_position) {
      switch (token) {
      case 0:
      case 1:
        if (((str[end] == L'+' || str[end] == L'-') && value_pos[token] != end) && !is_float(str[end], &found_dot)) {
          return false;
        }
        break;
      default:
        return false;
      }
    } else if (type == tag_type_font) {
      switch (token) {
      case 0: // size
        if (!is_dec(str[end])) {
          return false;
        }
        break;
      case 1: // name
        break;
      case 2: // style
        if (str[end] != L'I' && str[end] != L'B') {
          return false;
        }
        break;
      default:
        return false;
      }
    } else if (type == tag_type_speed) {
      switch (token) {
      case 0:
        if (!is_float(str[end], &found_dot)) {
          return false;
        }
        break;
      default:
        return false;
      }
    } else if (type == tag_type_wait || type == tag_type_clear) {
      switch (token) {
      case 0:
        if ((str[end] == L'*' && value_pos[token] != end) && !is_float(str[end], &found_dot)) {
          return false;
        }
        break;
      default:
        return false;
      }
    }
  }
  return false;
}

static int get_caret_tag_value_index(struct tag const *const tag, int const pos) {
  for (int i = 0; i < 3; ++i) {
    if (tag->value_pos[i] != -1 && tag->value_pos[i] <= pos && pos <= tag->value_pos[i] + tag->value_len[i]) {
      return i;
    }
  }
  return -1;
}

static bool find_tag(PCWSTR str, int const len, int const pos, struct tag *const tag) {
  int tag_pos = find_char_reverse(str, pos, L'<');
  if (tag_pos == -1) {
    return false; // tag not found
  }
  if (!parse_tag(str, len, tag_pos, tag)) {
    return false; // invalid tag
  }
  if (pos <= tag->pos || tag->pos + tag->len - 1 < pos) {
    // not inside
    return false;
  }
  return true;
}

static inline int
choice_by_arrow_up_downi(int const keyCode, int const up, int const down, int const shift_up, int const shift_down) {
  bool const shift = GetKeyState(VK_SHIFT) < 0;
  switch (keyCode) {
  case VK_UP:
    return shift ? shift_up : up;
  case VK_DOWN:
    return shift ? shift_down : down;
  }
  return 0;
}

static inline float choice_by_arrow_up_downf(
    int const keyCode, float const up, float const down, float const shift_up, float const shift_down) {
  bool const shift = GetKeyState(VK_SHIFT) < 0;
  switch (keyCode) {
  case VK_UP:
    return shift ? shift_up : up;
  case VK_DOWN:
    return shift ? shift_down : down;
  }
  return 0.f;
}

static bool increment_tag_color(struct tag *const tag, int const pos, int const keyCode) {
  int const v = choice_by_arrow_up_downi(keyCode, 1, -1, 16, -16);
  if (!v) {
    return false;
  }
  int const idx = get_caret_tag_value_index(tag, pos);
  if (idx != 0 && idx != 1) {
    return false;
  }
  int r[2] = {0}, g[2] = {0}, b[2] = {0};
  for (int i = 0; i < 2; ++i) {
    r[i] = (tag->value.color.color[i] >> 16) & 0xff;
    g[i] = (tag->value.color.color[i] >> 8) & 0xff;
    b[i] = (tag->value.color.color[i] >> 0) & 0xff;
  }
  switch (pos - tag->value_pos[idx]) {
  case 0:
  case 1:
    r[idx] = saturatei(r[idx] + v, 0, 255);
    break;
  case 3:
    g[idx] = saturatei(g[idx] + v, 0, 255);
    break;
  case 5:
  case 6:
    b[idx] = saturatei(b[idx] + v, 0, 255);
    break;
  }
  for (int i = 0; i < 2; ++i) {
    tag->value.color.color[i] = (uint32_t)(r[i] << 16) | (uint32_t)(g[i] << 8) | (uint32_t)b[i];
  }
  return true;
}

static bool increment_tag_position(struct tag *const tag, int const pos, int const keyCode) {
  (void)pos;
  float const v = GetKeyState(VK_SHIFT) < 0 ? 10.f : 1.f;
  switch (keyCode) {
  case VK_LEFT:
    tag->value.position.x =
        saturatef(tag->value.position.x - v, tag->value.position.x_relative ? (float)INT_MIN : 0, (float)INT_MAX);
    break;
  case VK_RIGHT:
    tag->value.position.x =
        saturatef(tag->value.position.x + v, tag->value.position.x_relative ? (float)INT_MIN : 0, (float)INT_MAX);
    break;
  case VK_UP:
    tag->value.position.y =
        saturatef(tag->value.position.y - v, tag->value.position.y_relative ? (float)INT_MIN : 0, (float)INT_MAX);
    break;
  case VK_DOWN:
    tag->value.position.y =
        saturatef(tag->value.position.y + v, tag->value.position.y_relative ? (float)INT_MIN : 0, (float)INT_MAX);
    break;
  default:
    return false;
  }
  return true;
}

static struct font_list g_font_name_list = {0};

static PCWSTR choice_similar_font(struct font_list *fl, HWND hwnd, PCWSTR s) {
  DWORD caret_start = 0, caret_end = 0;
  SendMessageW(hwnd, EM_GETSEL, (WPARAM)&caret_start, (LPARAM)&caret_end);
  LRESULT r = SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)caret_start, 0);
  POINT pt = {LOWORD(r), HIWORD(r)};
  if (!ClientToScreen(hwnd, &pt)) {
    ods(L"ClientToScreen failed");
    return NULL;
  }

  struct font_similar *similar = font_get_similar(fl, s);
  if (!similar) {
    ods(L"failed to get a list of similar font names");
    return NULL;
  }
  HMENU h = CreatePopupMenu();
  if (!h) {
    odshr(HRESULT_FROM_WIN32(GetLastError()), L"CreatePopupMenu failed");
    goto failed;
  }
  for (size_t i = 0; i < fl->num && i < 10; ++i) {
    if (!AppendMenuW(h, MF_ENABLED | MF_STRING, (UINT_PTR)i + 1, fl->sorted[similar[i].idx])) {
      odshr(HRESULT_FROM_WIN32(GetLastError()), L"AppendMenu failed");
      goto failed;
    }
  }
  int const id =
      TrackPopupMenu(h, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
  DestroyMenu(h);

  PCWSTR ret = id ? fl->sorted[similar[id - 1].idx] : NULL;
  free(similar);
  return ret;

failed:
  if (similar) {
    free(similar);
  }
  DestroyMenu(h);
  return NULL;
}

static bool increment_tag_font(HWND hwnd, struct tag *tag, int const pos, int const keyCode) {
  int const idx = get_caret_tag_value_index(tag, pos);
  switch (idx) {
  case 0: {
    int const v = choice_by_arrow_up_downi(keyCode, 1, -1, 10, -10);
    if (!v) {
      return false;
    }
    tag->value.font.size = saturatei(tag->value.font.size + v, 0, INT_MAX);
  }
    return true;
  case 1: {
    int const fidx = font_list_index_of(&g_font_name_list, tag->value.font.name);
    if (fidx != -1) {
      int const v = choice_by_arrow_up_downi(keyCode, 1, -1, 10, -10);
      if (!v) {
        return false;
      }
      wcscpy(tag->value.font.name, g_font_name_list.sorted[saturatei(fidx + v, 0, (int)g_font_name_list.num - 1)]);
      return true;
    }

    PCWSTR s = choice_similar_font(&g_font_name_list, hwnd, tag->value.font.name);
    if (!s) {
      return false;
    }
    wcscpy(tag->value.font.name, s);
  }
    return true;
  case 2: {
    int const v = choice_by_arrow_up_downi(keyCode, 1, -1, 1, -1);
    if (!v) {
      return false;
    }
    int style = 0;
    if (tag->value.font.bold) {
      style |= 1;
    }
    if (tag->value.font.italic) {
      style |= 2;
    }
    style += v;
    tag->value.font.bold = style & 1;
    tag->value.font.italic = style & 2;
  }
    return true;
  }
  return false;
}

static bool increment_tag_speed(struct tag *tag, int const pos, int const keyCode) {
  (void)pos;
  float const v = choice_by_arrow_up_downf(keyCode, 0.1f, -0.1f, 1.f, -1.f);
  if (v == 0.f) {
    return false;
  }
  tag->value.speed.v = saturatef(tag->value.speed.v + v, 0, (float)INT_MAX);
  return true;
}

static bool increment_tag_wait(struct tag *tag, int const pos, int const keyCode) {
  (void)pos;
  float const v = choice_by_arrow_up_downf(keyCode, 0.1f, -0.1f, 1.f, -1.f);
  if (v == 0.f) {
    return false;
  }
  tag->value.speed.v = saturatef(tag->value.wait.v + v, 0, (float)INT_MAX);
  return true;
}

static bool increment_tag_clear(struct tag *tag, int const pos, int const keyCode) {
  (void)pos;
  float const v = choice_by_arrow_up_downf(keyCode, 0.1f, -0.1f, 1.f, -1.f);
  if (v == 0.f) {
    return false;
  }
  tag->value.speed.v = saturatef(tag->value.clear.v + v, 0, (float)INT_MAX);
  return true;
}

static bool increment_tag(HWND hwnd, struct tag *tag, int const pos, int const keyCode) {
  switch (tag->type) {
  case tag_type_color:
    return increment_tag_color(tag, pos, keyCode);
  case tag_type_position:
    return increment_tag_position(tag, pos, keyCode);
  case tag_type_font:
    return increment_tag_font(hwnd, tag, pos, keyCode);
  case tag_type_speed:
    return increment_tag_speed(tag, pos, keyCode);
  case tag_type_wait:
    return increment_tag_wait(tag, pos, keyCode);
  case tag_type_clear:
    return increment_tag_clear(tag, pos, keyCode);
  }
  return false;
}

static int sprint_tag_color(wchar_t *buf, struct tag *tag) {
  wchar_t color[2][8] = {{L'\0'}, {L'\0'}};
  if (tag->value_len[0] == 6) {
    wsprintfW(color[0], L"%06x", tag->value.color.color[0]);
  }
  if (tag->value_len[1] == 6) {
    wsprintfW(color[1], L"%06x", tag->value.color.color[1]);
  }
  if (tag->value_pos[1] != -1) {
    return wsprintfW(buf, L"<#%s,%s>", color[0], color[1]);
  }
  if (tag->value_pos[0] != -1) {
    return wsprintfW(buf, L"<#%s>", color[0]);
  }
  return wsprintfW(buf, L"<#>");
}

static inline int fcompare(float x, float y, float tolerance) {
  return (x > y + tolerance) ? 1 : (y > x + tolerance) ? -1 : 0;
}
#define fcmp(x, op, y, tolerance) ((fcompare((x), (y), (tolerance)))op 0)

static int sprint_tag_position(wchar_t *buf, struct tag *tag) {
  wchar_t x[16] = {L'+'};
  wchar_t y[16] = {L'+'};
  wchar_t z[16] = {L'+'};
  sprint_float(
      x + (tag->value.position.x_relative && tag->value.position.x >= 0 ? 1 : 0), tag->value.position.x, false);
  sprint_float(
      y + (tag->value.position.y_relative && tag->value.position.y >= 0 ? 1 : 0), tag->value.position.y, false);
  sprint_float(
      z + (tag->value.position.z_relative && tag->value.position.z >= 0 ? 1 : 0), tag->value.position.z, false);
  if (tag->value_pos[2] != -1) {
    if (tag->value.position.x_relative && fcmp(tag->value.position.x, ==, 0, 1e-16f) &&
        tag->value.position.y_relative && fcmp(tag->value.position.y, ==, 0, 1e-16f) &&
        tag->value.position.z_relative && fcmp(tag->value.position.z, ==, 0, 1e-16f)) {
      buf[0] = L'\0';
      return 0;
    }
    return wsprintfW(buf, L"<p%s,%s,%s>", x, y, z);
  }
  if (tag->value_pos[1] != -1) {
    if (tag->value.position.x_relative && fcmp(tag->value.position.x, ==, 0, 1e-16f) &&
        tag->value.position.y_relative && fcmp(tag->value.position.y, ==, 0, 1e-16f)) {
      buf[0] = L'\0';
      return 0;
    }
    return wsprintfW(buf, L"<p%s,%s>", x, y, z);
  }
  return -1;
}

static int sprint_tag_font(wchar_t *buf, struct tag *tag) {
  wchar_t size[16] = {L'\0'};
  wchar_t style[4] = {L'\0'};
  sprint_int(size, tag->value.font.size, true);
  if (tag->value.font.bold && tag->value.font.italic) {
    wcscat(style, L"BI");
  } else if (tag->value.font.bold) {
    wcscat(style, L"B");
  } else if (tag->value.font.italic) {
    wcscat(style, L"I");
  }
  if (tag->value_pos[2] != -1) {
    return wsprintfW(buf, L"<s%s,%s,%s>", size, tag->value.font.name, style);
  }
  if (tag->value_pos[1] != -1) {
    return wsprintfW(buf, L"<s%s,%s>", size, tag->value.font.name);
  }
  return wsprintfW(buf, L"<s%s>", size);
}

static int sprint_tag_speed(wchar_t *buf, struct tag *tag) {
  wchar_t v[16] = {L'\0'};
  sprint_float(v, tag->value.speed.v, true);
  return wsprintfW(buf, L"<r%s>", v);
}

static int sprint_tag_wait(wchar_t *buf, struct tag *tag) {
  wchar_t v[16] = {L'*'};
  sprint_float(v + (tag->value.wait.per_char ? 1 : 0), tag->value.wait.v, true);
  return wsprintfW(buf, L"<w%s>", v);
}

static int sprint_tag_clear(wchar_t *buf, struct tag *tag) {
  wchar_t v[16] = {L'*'};
  sprint_float(v + (tag->value.clear.per_char ? 1 : 0), tag->value.clear.v, true);
  return wsprintfW(buf, L"<c%s>", v);
}

static int sprint_tag(wchar_t *buf, struct tag *tag) {
  switch (tag->type) {
  case tag_type_color:
    return sprint_tag_color(buf, tag);
  case tag_type_position:
    return sprint_tag_position(buf, tag);
  case tag_type_font:
    return sprint_tag_font(buf, tag);
  case tag_type_speed:
    return sprint_tag_speed(buf, tag);
  case tag_type_wait:
    return sprint_tag_wait(buf, tag);
  case tag_type_clear:
    return sprint_tag_clear(buf, tag);
  }
  return -1;
}

static wchar_t *get_text_from_window(HWND hwnd, int *length) {
  int const len = (int)GetWindowTextLengthW(hwnd);
  if (!len) {
    return NULL; // empty or error
  }
  wchar_t *str = realloc(NULL, sizeof(WCHAR) * (size_t)(len + 1));
  if (!str) {
    ods(L"failed to allocate text buffer");
    return NULL;
  }
  if (GetWindowTextW(hwnd, str, (int)(len + 1)) == 0) {
    odshr(HRESULT_FROM_WIN32(GetLastError()), L"GetWindowText failed");
    free(str);
    return NULL;
  }
  str[len] = L'\0';
  *length = len;
  return str;
}

static bool insert_tag(HWND hwnd) {
  DWORD caret_start = 0, caret_end = 0;
  SendMessageW(hwnd, EM_GETSEL, (WPARAM)&caret_start, (LPARAM)&caret_end);
  LRESULT r = SendMessageW(hwnd, EM_POSFROMCHAR, (WPARAM)caret_end, 0);
  POINT pt = {LOWORD(r), HIWORD(r)};
  if ((uint32_t)r == 0xffffffff) {
    // Move the caret to the end of the selection
    SendMessageW(hwnd, EM_SETSEL, (WPARAM)caret_start, (LPARAM)caret_end);
    SendMessageW(hwnd, EM_SCROLLCARET, 0, 0);
    if (!GetCaretPos(&pt)) {
      odshr(HRESULT_FROM_WIN32(GetLastError()), L"GetCaretPos failed");
      return false;
    }
  }
  if (!ClientToScreen(hwnd, &pt)) {
    odshr(HRESULT_FROM_WIN32(GetLastError()), L"ClientToScreen failed");
    return false;
  }

  HMENU h = CreatePopupMenu();
  if (!h) {
    odshr(HRESULT_FROM_WIN32(GetLastError()), L"CreatePopupMenu failed");
    return false;
  }
  if (caret_start == caret_end) {
    AppendMenuW(h, MF_ENABLED | MF_STRING, 1, L"色の変更 <#000000,ffffff>");
    AppendMenuW(h, MF_ENABLED | MF_STRING, 2, L"フォント <s32,ＭＳ Ｐゴシック,BI>");
    AppendMenuW(h, MF_ENABLED | MF_STRING, 3, L"座標指定 <p0,0>");
    AppendMenuW(h, MF_ENABLED | MF_STRING, 4, L"表示速度 <r1>");
    AppendMenuW(h, MF_ENABLED | MF_STRING, 5, L"表示ウェイト <w1>");
    AppendMenuW(h, MF_ENABLED | MF_STRING, 6, L"表示クリア <c1>");
  } else {
    AppendMenuW(h, MF_ENABLED | MF_STRING, 1, L"色の変更 <#000000,ffffff> ～ <#>");
    AppendMenuW(h, MF_ENABLED | MF_STRING, 2, L"フォント <s32,ＭＳ Ｐゴシック,BI> ～ <s>");
    AppendMenuW(h, MF_ENABLED | MF_STRING, 4, L"表示速度 <r1> ～ <r>");
  }

  int id = TrackPopupMenu(h, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
  DestroyMenu(h);
  if (!id) {
    return false;
  }
  PCWSTR left = NULL, right = NULL;
  switch (id) {
  case 1:
    left = L"<#000000,ffffff>";
    if (caret_start != caret_end) {
      right = L"<#>";
    }
    break;
  case 2:
    left = L"<s32,ＭＳ Ｐゴシック,>";
    if (caret_start != caret_end) {
      right = L"<s>";
    }
    break;
  case 3:
    left = L"<p0,0>";
    break;
  case 4:
    left = L"<r1>";
    if (caret_start != caret_end) {
      right = L"<r>";
    }
    break;
  case 5:
    left = L"<w1>";
    break;
  case 6:
    left = L"<c1>";
    break;
  default:
    return false;
  }
  int len = 0;
  wchar_t *str = get_text_from_window(hwnd, &len);
  if (!str) {
    return false;
  }
  wchar_t *str2 = realloc(NULL, (size_t)(len + 64) * sizeof(WCHAR));
  int pos = 0, l = 0;

  if (caret_start > 0) {
    memcpy(str2 + pos, str, caret_start * sizeof(WCHAR));
    pos += caret_start;
  }
  if (left) {
    l = (int)(wcslen(left));
    memcpy(str2 + pos, left, (size_t)(l) * sizeof(WCHAR));
    pos += l;
  }
  if (caret_start != caret_end) {
    l = (int)(caret_end - caret_start);
    memcpy(str2 + pos, str + caret_start, (size_t)(l) * sizeof(WCHAR));
    pos += l;
  }
  if (right) {
    l = (int)(wcslen(right));
    memcpy(str2 + pos, right, (size_t)(l) * sizeof(WCHAR));
    pos += l;
  }
  if ((int)caret_end != len) {
    l = (int)((DWORD)(len)-caret_end);
    memcpy(str2 + pos, str + caret_end, (size_t)(l) * sizeof(WCHAR));
    pos += l;
  }
  str2[pos] = L'\0';
  SetWindowTextW(hwnd, str2);
  l = (int)(wcslen(left));
  caret_start += (DWORD)l;
  caret_end += (DWORD)l;
  SendMessageW(hwnd, EM_SETSEL, (WPARAM)caret_start, (LPARAM)caret_end);
  SendMessageW(hwnd, EM_SCROLLCARET, 0, 0);
  free(str2);
  free(str);
  return true;
}

static bool support_input(HWND hwnd, WPARAM keyCode) {
  DWORD caret_start = 0, caret_end = 0;
  SendMessageW(hwnd, EM_GETSEL, (WPARAM)&caret_start, (LPARAM)&caret_end);
  if (caret_start != caret_end) {
    return false; // do nothing if text is selected
  }

  int len = 0;
  wchar_t *str = get_text_from_window(hwnd, &len);
  wchar_t *str2 = NULL;
  if (!str) {
    return false;
  }

  struct tag tag = {0};
  // find tags, left side first
  if (!find_tag(str, len, (int)caret_start, &tag)) {
    // It seems current caret is not inside any tag.
    switch (keyCode) {
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
      // generate relative position tag
      tag.pos = (int)caret_start;
      tag.len = 0;
      tag.type = tag_type_position;
      tag.value_pos[0] = (int)caret_start;
      tag.value_pos[1] = (int)caret_start;
      tag.value_pos[2] = -1;
      tag.value_len[0] = 0;
      tag.value_len[1] = 0;
      tag.value_len[2] = 0;
      tag.value.position.x = 0;
      tag.value.position.y = 0;
      tag.value.position.z = 0;
      tag.value.position.x_relative = true;
      tag.value.position.y_relative = true;
      tag.value.position.z_relative = false;
      break;
    }
  }

  // ods("tag type:%d pos:%d len:%d v0:%d %d v1:%d %d v2:%d %d", tag.type,
  // tag.pos, tag.len, tag.value_pos[0], tag.value_len[0], tag.value_pos[1],
  // tag.value_len[1], tag.value_pos[2], tag.value_len[2]);

  if ((keyCode == VK_LEFT || keyCode == VK_RIGHT) && tag.type != tag_type_position) {
    int const idx = get_caret_tag_value_index(&tag, (int)caret_start);
    if (idx == -1) {
      goto failed;
    }
    int newpos = -1;
    if (tag.type != tag_type_color) {
      int const newidx = saturatei(idx + (keyCode == VK_LEFT ? -1 : 1), 0, 2);
      if (tag.value_pos[newidx] == -1) {
        goto failed;
      }
      newpos = tag.value_pos[newidx] + tag.value_len[newidx];
    } else {
      int colorpos = (int)(caret_start)-tag.value_pos[idx];
      int newidx = idx;
      int newcolorpos = colorpos;
      if (keyCode == VK_LEFT) {
        switch (colorpos) {
        case 0:
        case 1:
          --newidx;
          newcolorpos = 5;
          break;
        case 3:
          newcolorpos = 1;
          break;
        case 5:
        case 6:
          newcolorpos = 3;
          break;
        }
      } else {
        switch (colorpos) {
        case 0:
        case 1:
          newcolorpos = 3;
          break;
        case 3:
          newcolorpos = 5;
          break;
        case 5:
        case 6:
          ++newidx;
          newcolorpos = 1;
          break;
        }
      }
      if (newidx == -1 || newidx == 2 || tag.value_pos[newidx] == -1 || tag.value_len[newidx] != 6) {
        goto failed;
      }
      newpos = tag.value_pos[newidx] + newcolorpos;
    }
    SendMessageW(hwnd, EM_SETSEL, (WPARAM)newpos, (LPARAM)newpos);
    SendMessageW(hwnd, EM_SCROLLCARET, 0, 0);
    free(str);
    return false;
  }

  if (!increment_tag(hwnd, &tag, (int)caret_start, (int)keyCode)) {
    goto failed; // no change
  }

  // 256 is large enough to store generated tag.
  str2 = realloc(NULL, sizeof(WCHAR) * (size_t)(len - tag.len + 256));
  if (!str2) {
    ods(L"failed to allocate modified text buffer");
    goto failed;
  }
  memcpy(str2, str, (size_t)(tag.pos) * sizeof(WCHAR));
  int const newlen = sprint_tag(str2 + tag.pos, &tag);
  if (newlen == -1) {
    goto failed;
  }
  memcpy(str2 + tag.pos + newlen, str + tag.pos + tag.len, (size_t)(len - tag.pos - tag.len + 1) * sizeof(WCHAR));
  if (!SetWindowTextW(hwnd, str2)) {
    odshr(HRESULT_FROM_WIN32(GetLastError()), L"SetWindowText failed");
    goto failed;
  }

  // re-parse to calculate new caret position
  int newpos = (int)caret_start;
  if (newlen > 0) {
    if (tag.type != tag_type_color) {
      int const oldidx = get_caret_tag_value_index(&tag, (int)caret_start);
      struct tag newtag = {0};
      if (!parse_tag(str2, tag.pos + newlen, tag.pos, &newtag)) {
        ods(L"why failed?");
        goto failed;
      }
      newpos = newtag.value_pos[oldidx] + newtag.value_len[oldidx];
    }
  } else {
    newpos = tag.pos;
  }
  SendMessageW(hwnd, EM_SETSEL, (WPARAM)newpos, (LPARAM)newpos);
  SendMessageW(hwnd, EM_SCROLLCARET, 0, 0);
  free(str);
  free(str2);
  return true;

failed:
  if (str) {
    free(str);
  }
  if (str2) {
    free(str2);
  }
  return false;
}

static HWND g_exedit_window = NULL;

static LRESULT WINAPI subclassed_edit_control_window_proc(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR uid_subclass, DWORD_PTR ref_data) {
  (void)uid_subclass;
  (void)ref_data;
  switch (message) {
  case WM_SYSKEYDOWN:
    if (wparam == VK_DOWN || wparam == VK_UP || wparam == VK_LEFT || wparam == VK_RIGHT) {
      if (support_input(hwnd, wparam)) {
        UpdateWindow(hwnd);
        SendMessageW(g_exedit_window, WM_COMMAND, (WPARAM)(MAKELONG(GetDlgCtrlID(hwnd), EN_CHANGE)), (LPARAM)hwnd);
        return 0;
      }
    } else if (wparam == 0x54) {
      if (insert_tag(hwnd)) {
        UpdateWindow(hwnd);
        SendMessageW(g_exedit_window, WM_COMMAND, (WPARAM)(MAKELONG(GetDlgCtrlID(hwnd), EN_CHANGE)), (LPARAM)hwnd);
        return 0;
      }
    }
    break;
  }
  return DefSubclassProc(hwnd, message, wparam, lparam);
}

static LRESULT WINAPI subclassed_exedit_window_proc(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR uid_subclass, DWORD_PTR ref_data) {
  (void)uid_subclass;
  (void)ref_data;
  switch (message) {
  case WM_COMMAND:
    switch (HIWORD(wparam)) {
    case EN_SETFOCUS:
      if ((GetWindowLongPtrW((HWND)lparam, GWL_STYLE) & ES_MULTILINE) == ES_MULTILINE) {
        if (!SetWindowSubclass((HWND)lparam, subclassed_edit_control_window_proc, (UINT_PTR)&g_font_name_list, 0)) {
          ods(L"拡張編集ウィンドウのサブクラス化に失敗しました");
        }
      }
      break;
    case EN_KILLFOCUS:
      if ((GetWindowLongPtrW((HWND)lparam, GWL_STYLE) & ES_MULTILINE) == ES_MULTILINE) {
        RemoveWindowSubclass((HWND)lparam, subclassed_edit_control_window_proc, (UINT_PTR)&g_font_name_list);
      }
      break;
    }
    break;
  }
  return DefSubclassProc(hwnd, message, wparam, lparam);
}

static void initialize(HWND hwnd, void *editp, FILTER *fp) {
  (void)editp;
  (void)fp;

  if (!font_list_create(&g_font_name_list)) {
    ods(L"failed to initialize font list");
  }

  g_exedit_window = FindWindowW(L"ExtendedFilterClass", NULL);
  if (!g_exedit_window) {
    MessageBoxW(hwnd, L"拡張編集のウィンドウが見つけられませんでした", CAPTION, MB_ICONERROR);
    return;
  }
  if (!SetWindowSubclass(
          g_exedit_window, (SUBCLASSPROC)subclassed_exedit_window_proc, (UINT_PTR)&g_font_name_list, 0)) {
    MessageBoxW(hwnd, L"拡張編集ウィンドウのサブクラス化に失敗しました", CAPTION, MB_ICONERROR);
    return;
  }
}

static void finalize(HWND hwnd, void *editp, FILTER *fp) {
  (void)hwnd;
  (void)editp;
  (void)fp;
  if (g_exedit_window) {
    RemoveWindowSubclass(g_exedit_window, subclassed_exedit_window_proc, (UINT_PTR)&g_font_name_list);
  }
  g_exedit_window = NULL;

  font_list_destroy(&g_font_name_list);
}

static BOOL textassist_wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void *editp, FILTER *fp) {
  (void)wparam;
  (void)lparam;
  switch (message) {
  case WM_FILTER_INIT:
    CreateWindowExW(0,
                    L"STATIC",
                    L"将来的には設定項目が増えるかも……",
                    WS_CHILD | WS_VISIBLE,
                    0,
                    0,
                    360,
                    360,
                    hwnd,
                    (HMENU)101,
                    NULL,
                    NULL);
    PostMessageW(hwnd, WM_APP + 1, 0, 0);
    break;
  case WM_FILTER_EXIT:
    finalize(hwnd, editp, fp);
    break;
  case WM_APP + 1:
    initialize(hwnd, editp, fp);
    break;
  }
  return FALSE;
}

EXTERN_C FILTER_DLL __declspec(dllexport) * *__stdcall GetFilterTableList(void);
EXTERN_C FILTER_DLL __declspec(dllexport) * *__stdcall GetFilterTableList(void) {
  static FILTER_DLL textassist_filter = (FILTER_DLL){
      .flag =
          FILTER_FLAG_ALWAYS_ACTIVE | FILTER_FLAG_EX_INFORMATION | FILTER_FLAG_DISP_FILTER | FILTER_FLAG_WINDOW_SIZE,
      .x = 360 | FILTER_WINDOW_SIZE_CLIENT,
      .y = 360 | FILTER_WINDOW_SIZE_CLIENT,
      .name = TEXTASSIST_NAME,
      .func_WndProc = textassist_wndproc,
      .information = TEXTASSIST_NAME " " VERSION,
  };
  static FILTER_DLL *filter_list[] = {&textassist_filter, NULL};
  return (FILTER_DLL **)&filter_list;
}
