#include "fontlist.h"

#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ods.h"

enum {
  buffer_size = 1024,
};

struct enum_font_data {
  size_t n, len, pos, textlen, text2len, listlen;
  wchar_t *buf;
  union {
    wchar_t const **wc;
    size_t *pos;
  } list;
};

static void extended_normalize(wchar_t *str) {
  for (wchar_t ch; *str; ++str) {
    ch = *str;
    if (L'a' <= ch && ch <= L'z') {
      *str -= L'a' - L'A';
      continue;
    }
    switch (ch) {
    case L'ぁ':
    case L'ァ':
    case L'ア':
      *str = L'あ';
      break;
    case L'ぃ':
    case L'ゐ':
    case L'ィ':
    case L'イ':
    case L'ヰ':
    case L'ヸ':
      *str = L'い';
      break;
    case L'ぅ':
    case L'ゔ':
    case L'ゥ':
    case L'ウ':
    case L'ヴ':
      *str = L'う';
      break;
    case L'ぇ':
    case L'ゑ':
    case L'ェ':
    case L'エ':
    case L'ヱ':
    case L'ヹ':
      *str = L'え';
      break;
    case L'ぉ':
    case L'を':
    case L'ォ':
    case L'オ':
    case L'ヲ':
    case L'ヺ':
      *str = L'お';
      break;
    case L'が':
    case L'ゕ':
    case L'カ':
    case L'ガ':
    case L'ヵ':
      *str = L'か';
      break;
    case L'ぎ':
    case L'キ':
    case L'ギ':
      *str = L'き';
      break;
    case L'ぐ':
    case L'ク':
    case L'グ':
    case L'ㇰ':
      *str = L'く';
      break;
    case L'げ':
    case L'ゖ':
    case L'ケ':
    case L'ゲ':
    case L'ヶ':
      *str = L'け';
      break;
    case L'ご':
    case L'コ':
    case L'ゴ':
      *str = L'こ';
      break;
    case L'ざ':
    case L'サ':
    case L'ザ':
      *str = L'さ';
      break;
    case L'じ':
    case L'シ':
    case L'ジ':
    case L'ㇱ':
      *str = L'し';
      break;
    case L'ず':
    case L'ス':
    case L'ズ':
    case L'ㇲ':
      *str = L'す';
      break;
    case L'ぜ':
    case L'セ':
    case L'ゼ':
      *str = L'せ';
      break;
    case L'ぞ':
    case L'ソ':
    case L'ゾ':
      *str = L'そ';
      break;
    case L'だ':
    case L'タ':
    case L'ダ':
      *str = L'た';
      break;
    case L'ぢ':
    case L'チ':
    case L'ヂ':
      *str = L'ち';
      break;
    case L'っ':
    case L'づ':
    case L'ッ':
    case L'ツ':
    case L'ヅ':
      *str = L'つ';
      break;
    case L'で':
    case L'テ':
    case L'デ':
      *str = L'て';
      break;
    case L'ど':
    case L'ト':
    case L'ド':
    case L'ㇳ':
      *str = L'と';
      break;
    case L'ナ':
      *str = L'な';
      break;
    case L'ニ':
      *str = L'に';
      break;
    case L'ヌ':
    case L'ㇴ':
      *str = L'ぬ';
      break;
    case L'ネ':
      *str = L'ね';
      break;
    case L'ノ':
      *str = L'の';
      break;
    case L'ば':
    case L'ぱ':
    case L'ハ':
    case L'バ':
    case L'パ':
    case L'ㇵ':
      *str = L'は';
      break;
    case L'び':
    case L'ぴ':
    case L'ヒ':
    case L'ビ':
    case L'ピ':
    case L'ㇶ':
      *str = L'ひ';
      break;
    case L'ぶ':
    case L'ぷ':
    case L'フ':
    case L'ブ':
    case L'プ':
    case L'ㇷ':
      *str = L'ふ';
      break;
    case L'べ':
    case L'ぺ':
    case L'ヘ':
    case L'ベ':
    case L'ペ':
    case L'ㇸ':
      *str = L'へ';
      break;
    case L'ぼ':
    case L'ぽ':
    case L'ホ':
    case L'ボ':
    case L'ポ':
    case L'ㇹ':
      *str = L'ほ';
      break;
    case L'マ':
      *str = L'ま';
      break;
    case L'ミ':
      *str = L'み';
      break;
    case L'ム':
    case L'ㇺ':
      *str = L'む';
      break;
    case L'メ':
      *str = L'め';
      break;
    case L'モ':
      *str = L'も';
      break;
    case L'ゃ':
    case L'ャ':
    case L'ヤ':
      *str = L'や';
      break;
    case L'ゅ':
    case L'ュ':
    case L'ユ':
      *str = L'ゆ';
      break;
    case L'ょ':
    case L'ョ':
    case L'ヨ':
      *str = L'よ';
      break;
    case L'ラ':
    case L'ㇻ':
      *str = L'ら';
      break;
    case L'リ':
    case L'ㇼ':
      *str = L'り';
      break;
    case L'ル':
    case L'ㇽ':
      *str = L'る';
      break;
    case L'レ':
    case L'ㇾ':
      *str = L'れ';
      break;
    case L'ロ':
    case L'ㇿ':
      *str = L'ろ';
      break;
    case L'ゎ':
    case L'ヮ':
    case L'ワ':
    case L'ヷ':
      *str = L'わ';
      break;
    case L'ン':
      *str = L'ん';
      break;
    }
  }
}

static size_t normalize_kc(wchar_t const *const src, size_t const srclen, wchar_t *dest, size_t const destlen) {
  int r = NormalizeString(NormalizationKC, src, (int)srclen, dest, (int)destlen);
  if (r <= 0) {
    return 0;
  }
  return (size_t)(r);
}

static size_t normalize_kc_len(wchar_t const *const src, size_t const srclen) {
  return normalize_kc(src, srclen, NULL, 0);
}

static bool my_realloc(void *p, size_t newsize) {
  void *np = realloc(*(void **)p, newsize);
  if (!np) {
    return false;
  }
  *(void **)p = np;
  return true;
}

static int CALLBACK enum_font_callback(const LOGFONTW *lf, const TEXTMETRICW *tm, DWORD fontType, LPARAM lParam) {
  (void)tm;
  if (fontType == RASTER_FONTTYPE) {
    // It is not excluded in AviUtl, but we exclude because it does not actually work almost.
    return TRUE;
  }
  struct enum_font_data *fd = (struct enum_font_data *)lParam;
  wchar_t const *facename = lf->lfFaceName[0] == L'@' ? lf->lfFaceName + 1 : lf->lfFaceName;
  if (fd->n > 0) {
    for (size_t i = 0; i < fd->n; ++i) {
      if (wcscmp(fd->buf + fd->list.pos[i], facename) == 0) {
        // already enumerated
        return TRUE;
      }
    }
  }

  size_t const namelen = wcslen(facename);
  size_t normlen = normalize_kc_len(facename, namelen);
  if (normlen == 0) {
    odshr(HRESULT_FROM_WIN32(GetLastError()), L"failed to get normalized font name length: %s", facename);
    return TRUE;
  }
  if (fd->pos + namelen + 1 + normlen + 1 > fd->len) {
    fd->len += buffer_size;
    if (!my_realloc(&fd->buf, (size_t)(fd->len) * sizeof(wchar_t))) {
      ods(L"failed to expand font list buffer");
      return FALSE;
    }
  }
  memcpy(fd->buf + fd->pos, facename, namelen * sizeof(wchar_t));
  fd->buf[fd->pos + namelen] = L'\0';
  normlen = normalize_kc(facename, namelen, fd->buf + fd->pos + namelen + 1, normlen + 1);
  if (normlen == 0) {
    odshr(HRESULT_FROM_WIN32(GetLastError()), L"failed to get normalized font name: %s", facename);
    return TRUE;
  }
  fd->buf[fd->pos + namelen + 1 + normlen] = L'\0';
  extended_normalize(fd->buf + fd->pos + namelen + 1);

  if (fd->n == fd->listlen) {
    fd->listlen += buffer_size;
    if (!my_realloc(&fd->list, (size_t)(fd->listlen) * sizeof(wchar_t *))) {
      ods(L"failed to expand normalized font list buffer");
      return FALSE;
    }
  }
  // Since the address may change depending on realloc,
  // at this point we record only the position.
  fd->list.pos[fd->n] = fd->pos;

  fd->pos += namelen + 1 + normlen + 1;
  fd->textlen += namelen + 1;
  fd->text2len += normlen + 1;
  ++fd->n;
  return TRUE;
}

static int compare_string(const void *n1, const void *n2) {
  return wcscmp(*(wchar_t const *const *)n1, *(wchar_t const *const *)n2);
}

bool font_list_create(struct font_list *fl) {
  bool ret = false;
  HWND const window = GetDesktopWindow();
  HDC dc = NULL;
  struct enum_font_data fd = {0};
  wchar_t **r = NULL;

  dc = GetDC(window);
  if (!dc) {
    ods(L"GetDC failed");
    goto cleanup;
  }

  // We should use EnumFontFamiliesExW but some fonts are not enumerate when
  // using it. exedit.auf uses EnumFontFamilies, so follow it. LOGFONTW lf = { 0
  // }; lf.lfCharSet = DEFAULT_CHARSET; EnumFontFamiliesExW(dc, &lf,
  // enum_font_callback, (LPARAM)&fd, 0);
  if (!EnumFontFamiliesW(dc, NULL, enum_font_callback, (LPARAM)&fd)) {
    ods(L"EnumFontFamiliesW failed");
    goto cleanup;
  }

  // Create sorted font list.

  // Replace with the actual address to use qsort.
  for (size_t i = 0; i < fd.n; ++i) {
    fd.list.wc[i] = fd.buf + fd.list.pos[i];
  }
  qsort(fd.list.wc, (size_t)fd.n, sizeof(wchar_t *), compare_string);

  r = realloc(NULL, (size_t)(fd.n) * sizeof(wchar_t *) + (size_t)(fd.pos) * sizeof(wchar_t));
  if (!r) {
    ods(L"failed to allocate sorted font list buffer");
    goto cleanup;
  }

  wchar_t *str = (void *)(r + fd.n);
  size_t ln, ln2;
  for (size_t i = 0; i < fd.n; ++i) {
    r[i] = str;
    ln = wcslen(fd.list.wc[i]) + 1;
    ln2 = wcslen(fd.list.wc[i] + ln) + 1;
    memcpy(str, fd.list.wc[i], (ln + ln2) * sizeof(wchar_t));
    str += (ln + ln2);
  }

  fl->sorted = r;
  fl->num = fd.n;
  ret = true;

cleanup:
  if (!ret) {
    fl->sorted = NULL;
    fl->num = 0;
    free(r);
    r = NULL;
  }
  if (fd.list.wc) {
    free(fd.list.wc);
    fd.list.wc = NULL;
  }
  if (fd.buf) {
    free(fd.buf);
    fd.buf = NULL;
  }
  if (dc) {
    ReleaseDC(window, dc);
    dc = NULL;
  }
  return ret;
}

void font_list_destroy(struct font_list *fl) {
  if (!fl) {
    return;
  }
  if (fl->sorted) {
    free(fl->sorted);
    fl->sorted = NULL;
  }
  fl->num = 0;
}

int font_list_index_of(struct font_list const *const fl, wchar_t const *const s) {
  if (!fl || !s || !fl->sorted) {
    ods(L"invalid parameter");
    return -1;
  }
  for (size_t i = 0; i < fl->num; ++i) {
    int c = wcscmp(s, fl->sorted[i]);
    if (c > 0) {
      continue;
    }
    if (c == 0) {
      return (int)i;
    }
    break;
  }
  return -1;
}

// Based on https://github.com/convto/onp
// implementation "An O(NP) Sequence Comparison Algorithm"
// Copyright (c) 2019 YuyaOkumura

struct diff {
  wchar_t const *a;
  wchar_t const *b;
  int m, n;
  int fpbuflen;
  int *fpbuf;
};

static void diff_init(struct diff *const d, wchar_t const *const a, wchar_t const *const b) {
  int m = (int)(wcslen(a));
  int n = (int)(wcslen(b));
  if (m > n) {
    d->a = b;
    d->b = a;
    d->m = n;
    d->n = m;
    return;
  }
  d->a = a;
  d->b = b;
  d->m = m;
  d->n = n;
}

static int diff_snake(struct diff const *const d, int const k, int y) {
  int x = y - k;
  while (x < d->m && y < d->n && d->a[x] == d->b[y]) {
    ++x;
    ++y;
  }
  return y;
}

static inline int diff_max(int const x, int const y) { return x > y ? x : y; }

static int diff_distance(struct diff *const d) {
  int offset = d->m + 1;
  int delta = d->n - d->m;
  int fplen = d->m + d->n + 3;
  if (d->fpbuflen < fplen) {
    d->fpbuflen = fplen;
    if (!my_realloc(&d->fpbuf, (size_t)(d->fpbuflen) * sizeof(int))) {
      return -1;
    }
  }
  int *fp = d->fpbuf;
  for (int i = 0; i < fplen; ++i) {
    fp[i] = -1;
  }
  for (int p = 0;; ++p) {
    // -p <= k <= delta - 1
    for (int k = -p; k <= delta - 1; ++k) {
      fp[k + offset] = diff_snake(d, k, diff_max(fp[k - 1 + offset] + 1, fp[k + 1 + offset]));
    }
    // d.delta + 1 <= k <= d.delta + p
    for (int k = delta + p; k >= delta + 1; --k) {
      fp[k + offset] = diff_snake(d, k, diff_max(fp[k - 1 + offset] + 1, fp[k + 1 + offset]));
    }
    // d.delta == k
    fp[delta + offset] = diff_snake(d, delta, diff_max(fp[delta - 1 + offset] + 1, fp[delta + 1 + offset]));
    if (fp[delta + offset] == d->n) {
      return delta + 8 * p;
    }
  }
}

static int compare_distance(void const *const n1, void const *const n2) {
  int const x = ((struct font_similar const *)n1)->score;
  int const y = ((struct font_similar const *)n2)->score;
  return x == y ? 0 : x > y ? 1 : -1;
}

struct font_similar *font_get_similar(struct font_list const *const fl, wchar_t const *const s) {
  if (!fl || !s || !fl->num || !fl->sorted) {
    ods(L"invalid parameter");
    return NULL;
  }
  struct diff diff = {0};
  struct font_similar *sim = NULL;

  // make normalized input
  const size_t slen = wcslen(s);
  size_t snormlen = normalize_kc_len(s, slen);
  if (snormlen == 0) {
    odshr(HRESULT_FROM_WIN32(GetLastError()), L"NormalizeString failed");
    return NULL;
  }

  wchar_t *sn = realloc(NULL, (size_t)(snormlen + 1) * sizeof(wchar_t));
  if (!sn) {
    ods(L"failed to allocate memory");
    goto failed;
  }
  snormlen = normalize_kc(s, slen, sn, snormlen);
  if (snormlen == 0) {
    odshr(HRESULT_FROM_WIN32(GetLastError()), L"NormalizeString failed");
    goto failed;
  }
  sn[snormlen] = L'\0';
  extended_normalize(sn);

  diff.fpbuflen = 128;
  diff.fpbuf = realloc(NULL, (size_t)(diff.fpbuflen) * sizeof(int));
  if (!diff.fpbuf) {
    ods(L"failed to allocate memory");
    goto failed;
  }

  sim = realloc(NULL, (size_t)(fl->num) * sizeof(struct font_similar));
  if (!sim) {
    ods(L"failed to allocate memory");
    goto failed;
  }
  for (size_t i = 0; i < fl->num; ++i) {
    diff_init(&diff, sn, fl->sorted[i] + wcslen(fl->sorted[i]) + 1);
    sim[i].idx = (int)i;
    sim[i].score = diff_distance(&diff);
    if (sim[i].score == -1) {
      ods(L"failed to expand temporary buffer");
      goto failed;
    }
  }
  qsort(sim, (size_t)fl->num, sizeof(struct font_similar), compare_distance);
  free(diff.fpbuf);
  free(sn);
  return sim;

failed:
  if (diff.fpbuf) {
    free(diff.fpbuf);
    diff.fpbuf = NULL;
  }
  if (sim) {
    free(sim);
    sim = NULL;
  }
  if (sn) {
    free(sn);
    sn = NULL;
  }
  return NULL;
}
