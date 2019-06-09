/**
 * File:   mledit.h
 * Author: AWTK Develop Team
 * Brief:  mledit
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
#include "mledit/mledit.h"
#include "base/input_method.h"

static ret_t mledit_get_prop(widget_t* widget, const char* name, value_t* v) {
  mledit_t* mledit = MLEDIT(widget);
  return_value_if_fail(mledit != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  return RET_NOT_FOUND;
}

static ret_t mledit_set_prop(widget_t* widget, const char* name, const value_t* v) {
  mledit_t* mledit = MLEDIT(widget);
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_TEXT)) {
    text_edit_info_t info;
    wstr_from_value(&(widget->text), v);

    text_edit_get_info(mledit->model, &info);
    info.cursor = widget->text.size;
    text_edit_set_info(mledit->model, &info);
  }

  return RET_NOT_FOUND;
}

static ret_t mledit_on_destroy(widget_t* widget) {
  mledit_t* mledit = MLEDIT(widget);
  return_value_if_fail(widget != NULL && mledit != NULL, RET_BAD_PARAMS);

  text_edit_destroy(mledit->model);

  return RET_OK;
}

static ret_t mledit_paint_caret(widget_t* widget, canvas_t* c) {
  mledit_t* mledit = MLEDIT(widget);
  color_t cursor_color = color_init(0xff, 0, 0, 0xff);

  if (mledit->caret_visible) {
    uint32_t x = mledit->caret.x;
    uint32_t y = mledit->caret.y;
    uint32_t font_size = c->font_size;

    canvas_set_stroke_color(c, cursor_color);
    canvas_draw_vline(c, x, y, font_size);
  }
  mledit->caret_visible = !mledit->caret_visible;
  mledit->caret_visible = TRUE;

  return RET_OK;
}

static ret_t mledit_on_paint_self(widget_t* widget, canvas_t* c) {
  uint32_t i = 0;
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t line_height = 0;
  text_edit_info_t info;
  wstr_t* s = &(widget->text);
  mledit_t* mledit = MLEDIT(widget);
  widget_prepare_text_style(widget, c);
  color_t normal = color_init(0, 0, 0, 0xff);
  color_t select = color_init(0xff, 0, 0, 0xff);

  text_edit_set_canvas(mledit->model, c);
  text_edit_get_info(mledit->model, &info);

  line_height = info.line_height;
  for (i = 0; i < s->size; i++) {
    wchar_t* p = s->str + i;
    uint32_t char_w = canvas_measure_text(c, p, 1);

    if (i == info.cursor) {
      mledit->caret.x = x;
      mledit->caret.y = y;
      mledit_paint_caret(widget, c);
    }

    if (i >= info.select_start && i < info.select_end) {
      canvas_set_text_color(c, select);
    } else {
      canvas_set_text_color(c, normal);
    }
    canvas_draw_text(c, p, 1, x, y);

    x += char_w + info.char_spacing;
    if (*p == info.newline) {
      x = 0;
      y += line_height;
    }
  }

  if (i == info.cursor) {
    mledit->caret.x = x;
    mledit->caret.y = y;
    mledit_paint_caret(widget, c);
  }

  return RET_OK;
}

static ret_t mledit_commit_str(widget_t* widget, const char* str) {
  wchar_t wstr[32];
  mledit_t* mledit = MLEDIT(widget);
  utf8_to_utf16(str, wstr, ARRAY_SIZE(wstr));

  text_edit_paste(mledit->model, wstr, wcslen(wstr));

  return RET_OK;
}

static ret_t mledit_request_input_method_on_window_open(void* ctx, event_t* e) {
  input_method_request(input_method(), WIDGET(ctx));

  return RET_REMOVE;
}

static ret_t mledit_request_input_method(widget_t* widget) {
  if (widget_is_window_opened(widget)) {
    input_method_request(input_method(), widget);
  } else {
    widget_t* win = widget_get_window(widget);
    if (win != NULL) {
      widget_on(win, EVT_WINDOW_OPEN, mledit_request_input_method_on_window_open, widget);
    }
  }

  return RET_OK;
}

static ret_t mledit_on_event(widget_t* widget, event_t* e) {
  uint32_t type = e->type;
  mledit_t* mledit = MLEDIT(widget);
  return_value_if_fail(widget != NULL && mledit != NULL, RET_BAD_PARAMS);

  widget_invalidate(widget, NULL);
  switch (type) {
    case EVT_POINTER_DOWN: {
      pointer_event_t evt = *(pointer_event_t*)e;
      text_edit_click(mledit->model, evt.x, evt.y);
      break;
    }
    case EVT_POINTER_DOWN_ABORT: {
      break;
    }
    case EVT_POINTER_MOVE: {
      pointer_event_t evt = *(pointer_event_t*)e;
      if (evt.pressed) {
        text_edit_drag(mledit->model, evt.x, evt.y);
      }
      break;
    }
    case EVT_POINTER_UP: {
      break;
    }
    case EVT_KEY_DOWN: {
      text_edit_key_down(mledit->model, (key_event_t*)e);
      break;
    }
    case EVT_IM_COMMIT: {
      im_commit_event_t* evt = (im_commit_event_t*)e;
      mledit_commit_str(widget, evt->text);
      break;
    }
    case EVT_KEY_UP: {

      break;
    }
    case EVT_BLUR: {
      // input_method_request(input_method(), NULL);
      break;
    }
    case EVT_FOCUS: {
      // mledit_request_input_method(widget);
      break;
    }
    case EVT_WHEEL: {
      break;
    }
    case EVT_PROP_CHANGED: {
      break;
    }
    case EVT_VALUE_CHANGING: {
      break;
    }
    default:
      break;
  }

  return RET_OK;
}

static const char* s_mledit_properties[] = {};

TK_DECL_VTABLE(mledit) = {.size = sizeof(mledit_t),
                          .type = WIDGET_TYPE_MLEDIT,
                          .clone_properties = s_mledit_properties,
                          .persistent_properties = s_mledit_properties,
                          .parent = TK_PARENT_VTABLE(widget),
                          .create = mledit_create,
                          .on_paint_self = mledit_on_paint_self,
                          .set_prop = mledit_set_prop,
                          .get_prop = mledit_get_prop,
                          .on_event = mledit_on_event,
                          .on_destroy = mledit_on_destroy};

widget_t* mledit_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  widget_t* widget = widget_create(parent, TK_REF_VTABLE(mledit), x, y, w, h);
  mledit_t* mledit = MLEDIT(widget);
  return_value_if_fail(mledit != NULL, NULL);

  mledit->model = text_edit_create(widget, FALSE);
  ENSURE(mledit->model != NULL);

  return widget;
}

widget_t* mledit_cast(widget_t* widget) {
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, mledit), NULL);

  return widget;
}
