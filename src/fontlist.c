#include "fontlist.h"

#include "ods.h"
#include <stdlib.h>
#include <stdbool.h>

enum
{
  BUFFER_SIZE = 1024,
};

struct enum_font_data
{
  int n, len, pos, textlen, text2len, listlen;
  PWSTR buf;
  PZPCWSTR list;
};

static void extended_normalize(PWSTR str)
{
  for (WCHAR ch; *str; ++str)
  {
    ch = *str;
    if (L'a' <= ch && ch <= L'z')
    {
      *str -= L'a' - L'A';
      continue;
    }
    switch (ch)
    {
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

static int CALLBACK enum_font_callback(const LOGFONTW *lf, const TEXTMETRICW *tm, DWORD fontType, LPARAM lParam)
{
  (void)tm;
  if (fontType == RASTER_FONTTYPE)
  {
    // It is not excluded in AviUtl, but we exclude because it does not actually work almost.
    return TRUE;
  }
  struct enum_font_data *fd = (struct enum_font_data *)lParam;
  PCWSTR facename = lf->lfFaceName[0] == L'@' ? lf->lfFaceName + 1 : lf->lfFaceName;
  if (fd->n > 0)
  {
    for (int i = 0; i < fd->n; ++i)
    {
      if (lstrcmpW(fd->buf + (int)fd->list[i], facename) == 0)
      {
        return TRUE;
      }
    }
  }

  const int namelen = lstrlenW(facename);
  int namelen2 = NormalizeString(NormalizationKC, facename, namelen, NULL, 0);
  if (fd->pos + namelen + 1 + namelen2 + 1 > fd->len)
  {
    fd->len += BUFFER_SIZE;
    fd->buf = realloc(fd->buf, fd->len * sizeof(WCHAR));
  }
  memcpy(fd->buf + fd->pos, facename, namelen * sizeof(WCHAR));
  fd->buf[fd->pos + namelen] = L'\0';
  namelen2 = NormalizeString(NormalizationKC, facename, namelen, fd->buf + fd->pos + namelen + 1, namelen2 + 1);
  fd->buf[fd->pos + namelen + 1 + namelen2] = L'\0';
  extended_normalize(fd->buf + fd->pos + namelen + 1);

  if (fd->n == fd->listlen)
  {
    fd->listlen += BUFFER_SIZE;
    fd->list = realloc(fd->list, fd->listlen * sizeof(PCWSTR));
  }
  // Since the address may change depending on realloc,
  // at this point we record only the position.
  fd->list[fd->n] = (PCWSTR)fd->pos;

  fd->pos += namelen + 1 + namelen2 + 1;
  fd->textlen += namelen + 1;
  fd->text2len += namelen2 + 1;
  ++fd->n;
  return TRUE;
}

static int compare_string(const void *n1, const void *n2)
{
  return lstrcmpW(*(PCWSTR *)n1, *(PCWSTR *)n2);
}

struct font_list *font_list_create(void)
{
  HWND window = GetDesktopWindow();
  HDC dc = GetDC(window);
  struct enum_font_data fd = {0};

  // We should use EnumFontFamiliesExW but some fonts are not enumerate when using it.
  // exedit.auf uses EnumFontFamilies, so follow it.
  // LOGFONTW lf = { 0 };
  // lf.lfCharSet = DEFAULT_CHARSET;
  // EnumFontFamiliesExW(dc, &lf, enum_font_callback, (LPARAM)&fd, 0);
  EnumFontFamiliesW(dc, NULL, enum_font_callback, (LPARAM)&fd);

  // Create sorted font list.

  // Replace with the actual address to use qsort.
  for (int i = 0; i < fd.n; ++i)
  {
    fd.list[i] = fd.buf + (int)fd.list[i];
  }
  qsort(fd.list, fd.n, sizeof(PCWSTR), compare_string);

  PZPCWSTR r = realloc(NULL, (fd.n + 1) * sizeof(PCWSTR) + fd.pos * sizeof(WCHAR));
  PWSTR str = (void *)(r + fd.n + 1);
  int ln, ln2;
  for (int i = 0; i < fd.n; ++i)
  {
    r[i] = str;
    ln = lstrlenW(fd.list[i]) + 1;
    ln2 = lstrlenW(fd.list[i] + ln) + 1;
    memcpy(str, fd.list[i], (ln + ln2) * sizeof(WCHAR));
    str += (ln + ln2);
  }
  r[fd.n] = NULL;

  free(fd.list);
  free(fd.buf);
  ReleaseDC(window, dc);

  struct font_list *fl = realloc(NULL, sizeof(struct font_list));
  fl->sorted = r;
  fl->num = fd.n;
  return fl;
}

void font_list_destroy(struct font_list *fl)
{
  if (fl->sorted)
  {
    free(fl->sorted);
    fl->sorted = NULL;
  }
  free(fl);
}

int font_list_index_of(struct font_list *fl, PCWSTR s)
{
  for (int i = 0, c; i < fl->num; ++i)
  {
    c = lstrcmpW(s, fl->sorted[i]);
    if (c > 0)
    {
      continue;
    }
    if (c == 0)
    {
      return i;
    }
    break;
  }
  return -1;
}

// Based on https://github.com/convto/onp
// implementation "An O(NP) Sequence Comparison Algorithm"
// Copyright (c) 2019 YuyaOkumura

struct diff
{
  PCWSTR a, b;
  int m, n;
  int fpbuflen;
  int *fpbuf;
};

static void diff_init(struct diff *d, PCWSTR a, PCWSTR b)
{
  int m = lstrlenW(a);
  int n = lstrlenW(b);
  if (m > n)
  {
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

static int diff_snake(struct diff *d, int k, int y)
{
  int x = y - k;
  while (x < d->m && y < d->n && d->a[x] == d->b[y])
  {
    ++x;
    ++y;
  }
  return y;
}

static inline int diff_max(int x, int y)
{
  return x > y ? x : y;
}

static int diff_distance(struct diff *d)
{
  int offset = d->m + 1;
  int delta = d->n - d->m;
  int fplen = d->m + d->n + 3;
  if (d->fpbuflen < fplen)
  {
    d->fpbuflen = fplen;
    d->fpbuf = realloc(d->fpbuf, d->fpbuflen * sizeof(int));
  }
  int *fp = d->fpbuf;
  for (int i = 0; i < fplen; ++i)
  {
    fp[i] = -1;
  }
  for (int p = 0;; ++p)
  {
    // -p <= k <= delta - 1
    for (int k = -p; k <= delta - 1; ++k)
    {
      fp[k + offset] = diff_snake(d, k, diff_max(fp[k - 1 + offset] + 1, fp[k + 1 + offset]));
    }
    // d.delta + 1 <= k <= d.delta + p
    for (int k = delta + p; k >= delta + 1; --k)
    {
      fp[k + offset] = diff_snake(d, k, diff_max(fp[k - 1 + offset] + 1, fp[k + 1 + offset]));
    }
    // d.delta == k
    fp[delta + offset] = diff_snake(d, delta, diff_max(fp[delta - 1 + offset] + 1, fp[delta + 1 + offset]));
    if (fp[delta + offset] == d->n)
    {
      return delta + 8 * p;
    }
  }
}

static int compare_distance(const void *n1, const void *n2)
{
  float f1 = ((struct font_similar *)n1)->score;
  float f2 = ((struct font_similar *)n2)->score;
  return f1 == f2 ? 0 : f1 > f2 ? 1
                                : -1;
}

struct font_similar *font_get_similar(struct font_list *fl, PCWSTR s)
{
  // make normalized input
  int slen = lstrlenW(s);
  int snlen = NormalizeString(NormalizationKC, s, slen, NULL, 0);
  PWSTR sn = realloc(NULL, (snlen + 1) * sizeof(WCHAR));
  snlen = NormalizeString(NormalizationKC, s, slen, sn, snlen);
  sn[snlen] = L'\0';
  extended_normalize(sn);

  struct diff diff = {0};
  diff.fpbuflen = 128;
  diff.fpbuf = realloc(NULL, diff.fpbuflen * sizeof(int));

  struct font_similar *sim = realloc(NULL, fl->num * sizeof(struct font_similar));
  for (int i = 0; i < fl->num; ++i)
  {
    diff_init(&diff, sn, fl->sorted[i] + lstrlenW(fl->sorted[i]) + 1);
    sim[i].idx = i;
    sim[i].score = diff_distance(&diff);
  }
  qsort(sim, fl->num, sizeof(struct font_similar), compare_distance);

  free(diff.fpbuf);
  diff.fpbuf = NULL;
  diff.fpbuflen = 0;
  free(sn);
  return sim;
}
