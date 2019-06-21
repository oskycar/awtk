#include "tkc/str.h"
#include "gtest/gtest.h"
#include "mledit/mledit.h"
#include "base/text_edit.h"
#include "base/window_manager.h"

TEST(TextEdit, basic) {
  str_t str;
  widget_t* w = mledit_create(NULL, 10, 20, 30, 40);
  text_edit_t* text_edit = text_edit_create(w, FALSE);
  text_edit_set_canvas(text_edit, WINDOW_MANAGER(window_manager())->canvas);

  widget_set_text(w, L"hello");
  ASSERT_EQ(text_edit_set_select(text_edit, 1, 2), RET_OK);
  ASSERT_EQ(text_edit_paste(text_edit, L"123", 3), RET_OK);

  str_init(&str, 0);
  str_from_wstr(&str, w->text.str);
  ASSERT_STREQ(str.str, "h123llo");

  str_reset(&str);
  widget_destroy(w);
  text_edit_destroy(text_edit);
}
