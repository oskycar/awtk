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

#include "text_edit.h"

#define CHAR_SPACING 1
#define FONT_BASELINE 1.25f
#define STB_TEXTEDIT_CHARTYPE wchar_t
#define STB_TEXTEDIT_NEWLINE (wchar_t)('\n')
#define STB_TEXTEDIT_STRING text_edit_t 

#include "stb/stb_textedit.h"

static void text_edit_layout(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i) {
  uint32_t i = 0;
  uint32_t x = 0;
  canvas_t* c = str->c;
  uint32_t font_size = c->font_size;
  wstr_t* text = &(str->widget->text);

  for (i = start_i; i < text->size; i++) {
    wchar_t* p = text->str + i;
    if (*p == STB_TEXTEDIT_NEWLINE) {
      i++;
      break;
    }
    x += canvas_measure_text(c, p, 1) + 1;
  }

  row->x0 = 0;
  row->x1 = x;
  row->ymin = 0;
  row->ymax = font_size;
  row->num_chars = i - start_i;
  row->baseline_y_delta = font_size * FONT_BASELINE;
}

static int text_edit_remove(STB_TEXTEDIT_STRING* str, int pos, int num) {
  wstr_t* text = &(str->widget->text);
  wstr_remove(text, pos, num);

  return TRUE;
}

static int text_edit_get_char_width(STB_TEXTEDIT_STRING* str, int pos, int offset) {
  wstr_t* text = &(str->widget->text);

  return canvas_measure_text(str->c, text->str + pos + offset, 1);
}

static int text_edit_insert(STB_TEXTEDIT_STRING* str, int pos, 
    STB_TEXTEDIT_CHARTYPE* newtext, int num) {
  wstr_t* text = &(str->widget->text);
  wstr_insert(text, pos, newtext, num);

  return TRUE;
}

#define KEYDOWN_BIT 0x80000000
#define STB_TEXTEDIT_STRINGLEN(str) ((str)->widget->text.size)
#define STB_TEXTEDIT_LAYOUTROW text_edit_layout
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

typedef struct _text_edit_impl_t {
  text_edit_t text_edit;
  
  STB_TexteditState state;
} text_edit_impl_t;

#define DECL_IMPL(te) text_edit_impl_t* impl = (text_edit_impl_t*)(te)

text_edit_t* text_edit_create(widget_t* widget, bool_t signle_line) {
  text_edit_impl_t* impl = NULL;
  return_value_if_fail(widget != NULL, NULL);

  impl = TKMEM_ZALLOC(text_edit_impl_t);
  return_value_if_fail(impl != NULL, NULL);

  impl->text_edit.widget = widget; 
  stb_textedit_initialize_state(&(impl->state), signle_line);

  return (text_edit_t*)impl;
}

ret_t text_edit_set_canvas(text_edit_t* text_edit, canvas_t* canvas) {
  return_value_if_fail(text_edit != NULL && canvas != NULL, RET_BAD_PARAMS);

  text_edit->c = canvas;

  return RET_OK;
}

static point_t text_edit_normalize_point(text_edit_t* text_edit, xy_t x, xy_t y) {
  point_t point = {x, y};

  widget_t* widget = text_edit->widget;
  widget_to_local(widget, &point); 

  return point;
}


ret_t text_edit_click(text_edit_t* text_edit, xy_t x, xy_t y) {
  point_t point;
  DECL_IMPL(text_edit);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);
  
  point = text_edit_normalize_point(text_edit, x, y);
  widget_prepare_text_style(text_edit->widget, text_edit->c);
  stb_textedit_click(text_edit, &(impl->state), point.x, point.y);

  return RET_OK;
}

ret_t text_edit_drag(text_edit_t* text_edit, xy_t x, xy_t y) {
  point_t point;
  DECL_IMPL(text_edit);
  return_value_if_fail(impl != NULL, RET_BAD_PARAMS);

  point = text_edit_normalize_point(text_edit, x, y);
  widget_prepare_text_style(text_edit->widget, text_edit->c);
  stb_textedit_drag(text_edit, &(impl->state), point.x, point.y);

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
    default:
      break;
  }

  if (evt->ctrl) {
    char c = key;
    if(c == 'z') {
      stb_textedit_key(text_edit, state, STB_TEXTEDIT_K_UNDO);
    } else if(c == 'y') {
      stb_textedit_key(text_edit, state, STB_TEXTEDIT_K_REDO);
    } else if(c == 'x') {
      stb_textedit_cut(text_edit, state);
    } else if(c == 'v') {
      //stb_textedit_cut(text_edit, state);
    }

    return RET_OK;
  }

  if (evt->shift) {
    key |= STB_TEXTEDIT_K_SHIFT;
  }

  stb_textedit_key(text_edit, state, key);

  return RET_OK;
}

ret_t text_edit_destroy(text_edit_t* text_edit) {
  return_value_if_fail(text_edit != NULL, RET_BAD_PARAMS);

  TKMEM_FREE(text_edit);

  return RET_OK;
}

ret_t text_edit_set_info(text_edit_t* text_edit, const text_edit_info_t* info) {
  DECL_IMPL(text_edit);
  return_value_if_fail(text_edit != NULL && info != NULL, RET_BAD_PARAMS);
 
  impl->state.cursor = info->cursor;
  impl->state.select_end = info->select_end;
  impl->state.select_start = info->select_start;

  return RET_OK;
}

ret_t text_edit_get_info(text_edit_t* text_edit, text_edit_info_t* info) {
  DECL_IMPL(text_edit);
  return_value_if_fail(text_edit != NULL && info != NULL, RET_BAD_PARAMS);

  memset(info, 0x00, sizeof(text_edit_info_t));

  info->char_spacing = CHAR_SPACING; 
  info->cursor = impl->state.cursor;
  info->newline = STB_TEXTEDIT_NEWLINE; 
  info->select_end = impl->state.select_end;
  info->select_start = impl->state.select_start;
  if(text_edit->c != NULL) {
    info->line_height = text_edit->c->font_size * FONT_BASELINE;
  }

  return RET_OK;
}

ret_t text_edit_paste(text_edit_t* text_edit, wchar_t* str, uint32_t size) {
  DECL_IMPL(text_edit);
  return_value_if_fail(text_edit != NULL && str != NULL, RET_BAD_PARAMS);

  stb_textedit_paste(text_edit, &(impl->state), str, size);

  return RET_OK;
}
