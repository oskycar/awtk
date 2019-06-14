/**
 * File:   text_edit.c
 * Author: AWTK Develop Team
 * Brief:  text_edit
 *
 * Copyright (c) 2018 - 2019  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2019-06-08 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utf8.h"
#include "tkc/utils.h"
#include "base/events.h"
#include "base/text_edit.h"
#include "base/line_break.h"
#include "base/clip_board.h"

#define CHAR_SPACING 1
#define FONT_BASELINE 1.25f
#define STB_TEXTEDIT_CHARTYPE wchar_t
#define STB_TEXTEDIT_NEWLINE (wchar_t)('\n')
#define STB_TEXTEDIT_STRING text_edit_t

#include "stb/stb_textedit.h"

typedef struct _text_layout_info_t {
  int32_t w;
  int32_t h;
  int32_t ox;
  int32_t oy;

  uint32_t virtual_w;
  uint32_t virtual_h;
  uint32_t widget_w;
  uint32_t widget_h;
  uint32_t margin_l;
  uint32_t margin_t;
  uint32_t margin_r;
  uint32_t margin_b;
} text_layout_info_t;

typedef struct _row_info_t {
  uint32_t offset;
  uint16_t length;
  uint16_t text_w;
} row_info_t;

typedef struct _rows_t {
  uint32_t size;
  uint32_t capacity;
  row_info_t info[1];
} rows_t;

typedef struct _text_edit_impl_t {
  text_edit_t text_edit;
  STB_TexteditState state;

  rows_t* rows;
  point_t caret;
  bool_t wrap_word;
  text_layout_info_t layout_info;
} text_edit_impl_t;

static ret_t widget_get_text_layout_info(widget_t* widget, text_layout_info_t* info) {
  value_t v;
  return_value_if_fail(widget != NULL && info != NULL, RET_BAD_PARAMS);

  value_set_int(&v, 0);

  //  memset(info, 0x00, sizeof(text_layout_info_t));

  info->widget_w = widget->w;
  info->widget_h = widget->h;
  info->virtual_w = widget->w;
  info->virtual_h = widget->h;

  if (widget_get_prop(widget, WIDGET_PROP_XOFFSET, &v) == RET_OK) {
    //    info->ox = value_int(&v);
  }

  if (widget_get_prop(widget, WIDGET_PROP_YOFFSET, &v) == RET_OK) {
    //    info->oy = value_int(&v);
  }

  if (widget_get_prop(widget, WIDGET_PROP_LEFT_MARGIN, &v) == RET_OK) {
    info->margin_l = value_int(&v);
  }

  if (widget_get_prop(widget, WIDGET_PROP_RIGHT_MARGIN, &v) == RET_OK) {
    info->margin_r = value_int(&v);
  }

  if (widget_get_prop(widget, WIDGET_PROP_TOP_MARGIN, &v) == RET_OK) {
    info->margin_t = value_int(&v);
  }

  if (widget_get_prop(widget, WIDGET_PROP_BOTTOM_MARGIN, &v) == RET_OK) {
    info->margin_b = value_int(&v);
  }

  info->margin_l = 16;
  info->margin_r = 8;
  info->margin_t = 23;
  info->margin_b = 5;

  info->w = info->widget_w - info->margin_l - info->margin_r;
  info->h = info->widget_h - info->margin_t - info->margin_b;

  return RET_OK;
}

static rows_t* rows_create(uint32_t capacity) {
  uint32_t msize = sizeof(rows_t) + capacity * sizeof(row_info_t);
  rows_t* rows = (rows_t*)TKMEM_ALLOC(msize);
  return_value_if_fail(rows != NULL, NULL);

  memset(rows, 0x00, msize);
  rows->capacity = capacity;

  return rows;
}

static row_info_t* rows_find_by_offset(rows_t* rows, uint32_t offset) {
  uint32_t i = 0;

  for (i = 0; i < rows->size; i++) {
    row_info_t* iter = rows->info + i;
    if (iter->offset == offset) {
      return iter;
    }
  }

  return NULL;
}

static ret_t rows_destroy(rows_t* rows) {
  return_value_if_fail(rows != NULL, RET_BAD_PARAMS);

  TKMEM_FREE(rows);

  return RET_OK;
}

#define DECL_IMPL(te) text_edit_impl_t* impl = (text_edit_impl_t*)(te)

static ret_t text_edit_set_caret_pos(text_edit_impl_t* impl, uint32_t x, uint32_t y,
                                     uint32_t font_size) {
  text_layout_info_t* layout_info = &(impl->layout_info);
  uint32_t caret_top = layout_info->margin_t + y;
  uint32_t caret_bottom = layout_info->margin_t + y + font_size;
  uint32_t caret_left = layout_info->margin_l + x;
  uint32_t caret_right = layout_info->margin_l + x + 1;

  uint32_t view_top = layout_info->oy + layout_info->margin_t;
  uint32_t view_bottom = layout_info->oy + layout_info->margin_t + layout_info->h;
  uint32_t view_left = layout_info->ox + layout_info->margin_l;
  uint32_t view_right = layout_info->ox + layout_info->margin_l + layout_info->w;

  impl->caret.x = x;
  impl->caret.y = y;

  if (view_top > caret_top) {
    layout_info->oy = caret_top - layout_info->margin_t;
  }

  if (view_bottom < caret_bottom) {
    layout_info->oy = caret_bottom - layout_info->h;
  }

  if (view_left > caret_left) {
    layout_info->ox = caret_left - layout_info->margin_l;
  }

  if (view_right < caret_right) {
    layout_info->ox = caret_right - layout_info->w;
  }

  return RET_OK;
}

static row_info_t* text_edit_layout_line(text_edit_t* text_edit, uint32_t row_num,
                                         uint32_t offset) {
  uint32_t i = 0;
  uint32_t x = 0;
  DECL_IMPL(text_edit);
  rows_t* rows = impl->rows;
  canvas_t* c = text_edit->c;
  wstr_t* text = &(text_edit->widget->text);
  STB_TexteditState* state = &(impl->state);
  row_info_t* row = impl->rows->info + row_num;
  uint32_t line_height = c->font_size * FONT_BASELINE;
  uint32_t y = line_height * row_num;
  text_layout_info_t* layout_info = &(impl->layout_info);

  memset(row, 0x00, sizeof(row_info_t));
  for (i = offset; i < text->size; i++) {
    wchar_t* p = text->str + i;
    break_type_t word_break= LINE_BREAK_NO;
    break_type_t line_break= LINE_BREAK_NO;
    uint32_t char_w = canvas_measure_text(c, p, 1) + CHAR_SPACING; 
    uint32_t next_char_w = canvas_measure_text(c, p+1, 1) + CHAR_SPACING; 

    if (i == state->cursor) {
      text_edit_set_caret_pos(impl, x, y, c->font_size);
    }

    line_break = line_break_check(*p, p[1]);
    if(line_break== LINE_BREAK_MUST) {
      i++;
      break;
    } 
    
    word_break = word_break_check(*p, p[1]);
    if((x + char_w) > layout_info->w) {
      break;
    } else if((x + char_w + next_char_w) >= layout_info->w) {
      if(line_break == LINE_BREAK_NO || word_break == LINE_BREAK_NO) {
        break;
      }
    }
        
    x += char_w;
  }

  if (i == state->cursor && state->cursor == text->size) {
    text_edit_set_caret_pos(impl, x, y, c->font_size);
  }

  row->text_w = x;
  row->offset = offset;
  row->length = i - offset;
  layout_info->virtual_h = tk_max(y, layout_info->widget_h);

  return row;
}

ret_t text_edit_layout(text_edit_t* text_edit) {
  uint32_t i = 0;
  uint32_t offset = 0;
  DECL_IMPL(text_edit);
  uint32_t max_rows = impl->rows->capacity;
  uint32_t size = text_edit->widget->text.size;
  text_layout_info_t* layout_info = &(impl->layout_info);

  impl->rows->size = 0;
  impl->caret.x = 0;
  impl->caret.y = 0;
  if (text_edit->c == NULL) {
    return RET_OK;
  }

  widget_get_text_layout_info(text_edit->widget, layout_info);
  while (offset < size && i < max_rows) {
    row_info_t* iter = text_edit_layout_line(text_edit, i, offset);
    if (iter == NULL || iter->length == 0) {
      break;
    }
    offset += iter->length;
    i++;
  }
  impl->rows->size = i;

  return RET_OK;
}

static void text_edit_layout_for_stb(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int offset) {
  DECL_IMPL(str);
  uint32_t font_size = str->c->font_size;
  text_layout_info_t* layout_info = &(impl->layout_info);
  row_info_t* info = rows_find_by_offset(impl->rows, offset);

  if (info != NULL) {
    row->x1 = info->text_w;
    row->num_chars = info->length;
  } else {
    row->x1 = 0;
    row->num_chars = 0;
  }

  row->x0 = 0;
  row->ymin = 0;
  row->ymax = font_size;
  row->baseline_y_delta = font_size * FONT_BASELINE;
}

static ret_t text_edit_paint_caret(text_edit_t* text_edit) {
  DECL_IMPL(text_edit);
  canvas_t* c = text_edit->c;
  widget_t* widget = text_edit->widget;
  color_t caret_color = color_init(0, 0xff, 0, 0xff);
  text_layout_info_t* layout_info = &(impl->layout_info);
  uint32_t x = layout_info->margin_l + impl->caret.x;
  uint32_t y = layout_info->margin_t + impl->caret.y;
  uint32_t rx = x - layout_info->ox;
  uint32_t ry = y - layout_info->oy;

  canvas_set_stroke_color(c, caret_color);
  canvas_draw_vline(c, rx, ry, c->font_size);

  return RET_OK;
}

static ret_t text_edit_paint_text(text_edit_t* text_edit) {
  uint32_t i = 0;
  uint32_t x = 0;
  uint32_t y = 0;
  canvas_t* c = text_edit->c;
  widget_t* widget = text_edit->widget;

  DECL_IMPL(text_edit);
  rows_t* rows = impl->rows;
  wstr_t* text = &(widget->text);
  style_t* style = widget->astyle;
  STB_TexteditState* state = &(impl->state);
  color_t black = color_init(0, 0, 0, 0xff);
  color_t white = color_init(0xf0, 0xf0, 0xf0, 0xff);

  color_t text_color = style_get_color(style, STYLE_ID_TEXT_COLOR, black);
  color_t select_bg_color = style_get_color(style, STYLE_ID_SELECTED_BG_COLOR, white);
  color_t select_text_color = style_get_color(style, STYLE_ID_SELECTED_TEXT_COLOR, black);

  uint32_t line_height = c->font_size * FONT_BASELINE;
  text_layout_info_t* layout_info = &(impl->layout_info);

  uint32_t view_top = layout_info->oy + layout_info->margin_t;
  uint32_t view_bottom = layout_info->oy + layout_info->margin_t + layout_info->h;
  uint32_t view_left = layout_info->ox + layout_info->margin_l;
  uint32_t view_right = layout_info->ox + layout_info->margin_l + layout_info->w;

  for (i = 0; i < rows->size; i++) {
    uint32_t k = 0;
    row_info_t* iter = rows->info + i;

    x = layout_info->margin_l;
    y = i * line_height + layout_info->margin_t;

    if ((y + c->font_size) < view_top) {
      continue;
    }

    if (y > view_bottom) {
      break;
    }

    for (k = 0; k < iter->length; k++) {
      uint32_t offset = iter->offset + k;
      wchar_t* p = text->str + offset;
      uint32_t char_w = canvas_measure_text(c, p, 1);

      if ((x + char_w) < view_left) {
        x += char_w + CHAR_SPACING;
        continue;
      }

      if (x > view_right) {
        break;
      }

      if (*p != STB_TEXTEDIT_NEWLINE) {
        uint32_t rx = x - layout_info->ox;
        uint32_t ry = y - layout_info->oy;

        if (offset >= state->select_start && offset < state->select_end) {
          canvas_set_fill_color(c, select_bg_color);
          canvas_fill_rect(c, rx, ry, char_w + CHAR_SPACING, c->font_size);

          canvas_set_text_color(c, select_text_color);
        } else {
          canvas_set_text_color(c, text_color);
        }

        canvas_draw_text(c, p, 1, rx, ry);
        x += char_w + CHAR_SPACING;
      }
    }
  }

  return RET_OK;
}

ret_t text_edit_paint(text_edit_t* text_edit, canvas_t* c) {
  return_value_if_fail(text_edit != NULL && c != NULL, RET_BAD_PARAMS);

  widget_prepare_text_style(text_edit->widget, c);

  if (text_edit->c != NULL) {
    text_edit->c = c;
  } else {
    text_edit->c = c;
    text_edit_layout(text_edit);
  }

  if (text_edit_paint_text(text_edit) == RET_OK) {
    DECL_IMPL(text_edit);
    STB_TexteditState* state = &(impl->state);
    text_layout_info_t* layout_info = &(impl->layout_info);

    canvas_draw_hline(c, layout_info->margin_l, layout_info->margin_t, layout_info->w);
    canvas_draw_hline(c, layout_info->margin_l, layout_info->margin_t + layout_info->h,
                      layout_info->w);
    canvas_draw_vline(c, layout_info->margin_l, layout_info->margin_t, layout_info->h);
    canvas_draw_vline(c, layout_info->margin_l + layout_info->w, layout_info->margin_t,
                      layout_info->h);

    if (state->select_start == state->select_end) {
      text_edit_paint_caret(text_edit);
    }
  }

  return RET_OK;
}

static int text_edit_remove(STB_TEXTEDIT_STRING* str, int pos, int num) {
  wstr_t* text = &(str->widget->text);
  wstr_remove(text, pos, num);

  return TRUE;
}

static int text_edit_get_char_width(STB_TEXTEDIT_STRING* str, int pos, int offset) {
  wstr_t* text = &(str->widget->text);

  return canvas_measure_text(str->c, text->str + pos + offset, 1) + 1;
}

static int text_edit_insert(STB_TEXTEDIT_STRING* str, int pos, STB_TEXTEDIT_CHARTYPE* newtext,
                            int num) {
  wstr_t* text = &(str->widget->text);
  wstr_insert(text, pos, newtext, num);

  return TRUE;
}

#define KEYDOWN_BIT 0x80000000
#define STB_TEXTEDIT_STRINGLEN(str) ((str)->widget->text.size)
#define STB_TEXTEDIT_LAYOUTROW text_edit_layout_for_stb
#define STB_TEXTEDIT_GETWIDTH(str, n, i) text_edit_get_char_width(str, n, i)
#define STB_TEXTEDIT_KEYTOTEXT(key) (((key)&KEYDOWN_BIT) ? 0 : (key))
#define STB_TEXTEDIT_GETCHAR(str, i) (((str)->widget->text).str[i])
#define STB_TEXTEDIT_IS_SPACE(ch) iswspace(ch)
#define STB_TEXTEDIT_DELETECHARS text_edit_remove
#define STB_TEXTEDIT_INSERTCHARS text_edit_insert

#define STB_TEXTEDIT_K_SHIFT 0x40000000
#define STB_TEXTEDIT_K_CONTROL 0x20000000
#define STB_TEXTEDIT_K_LEFT (KEYDOWN_BIT | 1)
#define STB_TEXTEDIT_K_RIGHT (KEYDOWN_BIT | 2)      // VK_RIGHT
#define STB_TEXTEDIT_K_UP (KEYDOWN_BIT | 3)         // VK_UP
#define STB_TEXTEDIT_K_DOWN (KEYDOWN_BIT | 4)       // VK_DOWN
#define STB_TEXTEDIT_K_LINESTART (KEYDOWN_BIT | 5)  // VK_HOME
#define STB_TEXTEDIT_K_LINEEND (KEYDOWN_BIT | 6)    // VK_END
#define STB_TEXTEDIT_K_TEXTSTART (STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_TEXTEND (STB_TEXTEDIT_K_LINEEND | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_DELETE (KEYDOWN_BIT | 7)     // VK_DELETE
#define STB_TEXTEDIT_K_BACKSPACE (KEYDOWN_BIT | 8)  // VK_BACKSPACE
#define STB_TEXTEDIT_K_UNDO (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | 'z')
#define STB_TEXTEDIT_K_REDO (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | 'y')
#define STB_TEXTEDIT_K_INSERT (KEYDOWN_BIT | 9)  // VK_INSERT
#define STB_TEXTEDIT_K_WORDLEFT (STB_TEXTEDIT_K_LEFT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_WORDRIGHT (STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_PGUP (KEYDOWN_BIT | 10)    // VK_PGUP -- not implemented
#define STB_TEXTEDIT_K_PGDOWN (KEYDOWN_BIT | 11)  // VK_PGDOWN -- not implemented

#define STB_TEXTEDIT_IMPLEMENTATION 1

#include "stb/stb_textedit.h"

text_edit_t* text_edit_create(widget_t* widget, bool_t signle_line) {
  text_edit_impl_t* impl = NULL;
  return_value_if_fail(widget != NULL, NULL);

  impl = TKMEM_ZALLOC(text_edit_impl_t);
  return_value_if_fail(impl != NULL, NULL);

  impl->wrap_word = !signle_line;
  impl->text_edit.widget = widget;
  stb_textedit_initialize_state(&(impl->state), signle_line);
  if (!signle_line) {
    text_edit_set_max_rows((text_edit_t*)impl, 100);
  }

  return (text_edit_t*)impl;
}

ret_t text_edit_set_max_rows(text_edit_t* text_edit, uint32_t max_rows) {
  DECL_IMPL(text_edit);
  return_value_if_fail(text_edit != NULL && max_rows > 1, RET_BAD_PARAMS);

  if (impl->rows != NULL && impl->rows->capacity < max_rows) {
    rows_destroy(impl->rows);
    impl->rows = NULL;
  }

  if (impl->rows == NULL) {
    impl->rows = rows_create(max_rows);
  }

  return RET_OK;
}

ret_t text_edit_set_canvas(text_edit_t* text_edit, canvas_t* canvas) {
  return_value_if_fail(text_edit != NULL && canvas != NULL, RET_BAD_PARAMS);

  text_edit->c = canvas;
  text_edit_layout(text_edit);

  return RET_OK;
}

static point_t text_edit_normalize_point(text_edit_t* text_edit, xy_t x, xy_t y) {
  point_t point = {x, y};
  DECL_IMPL(text_edit);
  text_layout_info_t* layout_info = &(impl->layout_info);

  widget_t* widget = text_edit->widget;
  widget_to_local(widget, &point);

  point.x = point.x - layout_info->margin_l + layout_info->ox;
  point.y = point.y - layout_info->margin_t + layout_info->oy;

  return point;
}

ret_t text_edit_click(text_edit_t* text_edit, xy_t x, xy_t y) {
  point_t point;
  DECL_IMPL(text_edit);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  point = text_edit_normalize_point(text_edit, x, y);
  widget_prepare_text_style(text_edit->widget, text_edit->c);
  stb_textedit_click(text_edit, &(impl->state), point.x, point.y);
  text_edit_layout(text_edit);

  return RET_OK;
}

ret_t text_edit_drag(text_edit_t* text_edit, xy_t x, xy_t y) {
  point_t point;
  DECL_IMPL(text_edit);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  point = text_edit_normalize_point(text_edit, x, y);
  widget_prepare_text_style(text_edit->widget, text_edit->c);
  stb_textedit_drag(text_edit, &(impl->state), point.x, point.y);
  text_edit_layout(text_edit);

  return RET_OK;
}

ret_t text_edit_key_down(text_edit_t* text_edit, key_event_t* evt) {
  uint32_t key = 0;
  wstr_t* text = NULL;
  DECL_IMPL(text_edit);
  STB_TexteditState* state = NULL;
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  key = evt->key;
  state = &(impl->state);
  text = &(text_edit->widget->text);

  switch (key) {
    case TK_KEY_RETURN: {
      key = STB_TEXTEDIT_NEWLINE;
      break;
    }
    case TK_KEY_LEFT: {
      key = STB_TEXTEDIT_K_LEFT;
      break;
    }
    case TK_KEY_RIGHT: {
      key = STB_TEXTEDIT_K_RIGHT;
      break;
    }
    case TK_KEY_DOWN: {
      key = STB_TEXTEDIT_K_DOWN;
      break;
    }
    case TK_KEY_UP: {
      key = STB_TEXTEDIT_K_UP;
      break;
    }
    case TK_KEY_HOME: {
      key = STB_TEXTEDIT_K_LINESTART;
      break;
    }
    case TK_KEY_END: {
      key = STB_TEXTEDIT_K_LINEEND;
      break;
    }
    case TK_KEY_DELETE: {
      key = STB_TEXTEDIT_K_DELETE;
      break;
    }
    case TK_KEY_BACKSPACE: {
      key = STB_TEXTEDIT_K_BACKSPACE;
      break;
    }
    case TK_KEY_INSERT: {
      key = STB_TEXTEDIT_K_INSERT;
      break;
    }
    case TK_KEY_LSHIFT:
    case TK_KEY_RSHIFT:
    case TK_KEY_LCTRL:
    case TK_KEY_RCTRL:
    case TK_KEY_LALT:
    case TK_KEY_RALT:
    case TK_KEY_COMMAND:
    case TK_KEY_MENU: {
      return RET_OK;
    }
    default: {
      if (!isprint((char)key)) {
        return RET_OK;
      }
      break;
    }
  }

  if (evt->ctrl) {
    char c = key;
    if (c == 'z') {
      stb_textedit_key(text_edit, state, STB_TEXTEDIT_K_UNDO);
    } else if (c == 'y') {
      stb_textedit_key(text_edit, state, STB_TEXTEDIT_K_REDO);
    } else if (c == 'x' || c == 'c') {
      if (state->select_end > state->select_start) {
        str_t str;
        wstr_t wstr;
        wchar_t* start = text->str + state->select_start;
        uint32_t size = state->select_end - state->select_start;

        wstr_init(&wstr, size + 1);
        wstr_append_with_len(&wstr, start, size);

        str_init(&str, 0);
        str_from_wstr(&str, wstr.str);
        clip_board_set_text(str.str);

        str_reset(&str);
        wstr_reset(&wstr);
      }
      if (c == 'x') {
        stb_textedit_cut(text_edit, state);
      }
    } else if (c == 'a') {
      state->select_start = 0;
      state->select_end = text->size;
    } else if (c == 'v') {
      value_t v;
      wstr_t str;
      const char* data = clip_board_get_text();
      if (data != NULL) {
        value_set_str(&v, data);
        wstr_init(&str, 0);
        wstr_from_value(&str, &v);
        wstr_normalize_newline(&str, STB_TEXTEDIT_NEWLINE);
        text_edit_paste(text_edit, str.str, str.size);
        wstr_reset(&str);
      }
    }

    return RET_OK;
  }

  if (evt->shift) {
    key |= STB_TEXTEDIT_K_SHIFT;
  }

  stb_textedit_key(text_edit, state, key);
  text_edit_layout(text_edit);

  return RET_OK;
}

ret_t text_edit_paste(text_edit_t* text_edit, wchar_t* str, uint32_t size) {
  DECL_IMPL(text_edit);
  return_value_if_fail(text_edit != NULL && str != NULL, RET_BAD_PARAMS);

  stb_textedit_paste(text_edit, &(impl->state), str, size);
  text_edit_layout(text_edit);

  return RET_OK;
}

ret_t text_edit_set_cursor(text_edit_t* text_edit, uint32_t cursor) {
  DECL_IMPL(text_edit);
  return_value_if_fail(text_edit != NULL, RET_BAD_PARAMS);

  impl->state.cursor = cursor;
  text_edit_layout(text_edit);

  return RET_OK;
}

ret_t text_edit_set_wrap_word(text_edit_t* text_edit, bool_t wrap_word) {
  DECL_IMPL(text_edit);
  return_value_if_fail(text_edit != NULL, RET_BAD_PARAMS);

  impl->wrap_word = wrap_word;
  text_edit_layout(text_edit);

  return RET_OK;
}

ret_t text_edit_set_select(text_edit_t* text_edit, uint32_t start, uint32_t end) {
  DECL_IMPL(text_edit);
  return_value_if_fail(text_edit != NULL, RET_BAD_PARAMS);

  impl->state.select_end = start;
  impl->state.select_start = end;
  text_edit_layout(text_edit);

  return RET_OK;
}

ret_t text_edit_destroy(text_edit_t* text_edit) {
  DECL_IMPL(text_edit);
  return_value_if_fail(text_edit != NULL, RET_BAD_PARAMS);

  rows_destroy(impl->rows);
  TKMEM_FREE(text_edit);

  return RET_OK;
}
