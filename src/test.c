#include "textassist.c"

#include "acutest.h"

static void test_sprint_float(void) {
  struct {
    wchar_t const *expected;
    float input;
    bool omit_zero;
  } cases[] = {
      {L"0", 0.f, false},
      {L"", 0.f, true},
      {L"0.5", 0.5f, false},
      {L"-0.5", -0.5f, false},
      {L"1", 1.f, false},
      {L"-1", -1.f, false},
      {L"2.2", 2.1f + 0.1f, false},
  };
  wchar_t buf[64];
  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    TEST_CASE_("#%d %ls", i, cases[i].expected);
    sprint_float(buf, cases[i].input, cases[i].omit_zero);
    TEST_CHECK(wcscmp(buf, cases[i].expected) == 0);
    TEST_MSG("expected: %ls, got: %ls", cases[i].expected, buf);
  }
}

static void test_parse_tag_position(void) {
  struct {
    wchar_t const *input;
    bool result;
    int value_pos[3];
    int value_len[3];
  } cases[] = {
      {L"<p>", false, {0, 0, 0}, {0, 0, 0}},
      {L"<p0>", false, {0, 0, 0}, {0, 0, 0}},
      {L"<p0,>", true, {2, 4, -1}, {1, 0, 0}},
      {L"<p+1>", false, {0, 0, 0}, {0, 0, 0}},
      {L"<p+1,+2>", true, {2, 5, -1}, {2, 2, 0}},
      {L"<p+1,+2,+3>", true, {2, 5, 8}, {2, 2, 2}},
      {L"<p+1,+2,+3,>", false, {0, 0, 0}, {0, 0, 0}},
  };
  struct tag t;
  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    TEST_CASE_("#%d %ls", i, cases[i].input);
    bool const r = parse_tag(cases[i].input, (int)(wcslen(cases[i].input)), 0, &t);
    TEST_CHECK(r == cases[i].result);
    if (r) {
      TEST_CHECK(t.type == tag_type_position);
      TEST_CHECK(t.pos == 0);
      TEST_CHECK(t.len == (int)(wcslen(cases[i].input)));
      for (int j = 0; j < 3; j++) {
        TEST_CHECK(t.value_pos[j] == cases[i].value_pos[j]);
        TEST_MSG("t.value_pos[%d] expected: %d, got: %d", j, cases[i].value_pos[j], t.value_pos[j]);
        TEST_CHECK(t.value_len[j] == cases[i].value_len[j]);
        TEST_MSG("t.value_len[%d] expected: %d, got: %d", j, cases[i].value_len[j], t.value_len[j]);
      }
    }
  }
}

TEST_LIST = {
    {"test_sprint_float", test_sprint_float},
    {"test_parse_tag_position", test_parse_tag_position},
    {NULL, NULL},
};
