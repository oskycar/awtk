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
    wstr_from_value(&(widget->text), v);
    text_edit_set_cursor(mledit->model, widget->text.size);
  }

  return RET_NOT_FOUND;
}

static ret_t mledit_on_destroy(widget_t* widget) {
  mledit_t* mledit = MLEDIT(widget);
  return_value_if_fail(widget != NULL && mledit != NULL, RET_BAD_PARAMS);

  text_edit_destroy(mledit->model);

  return RET_OK;
}

static ret_t mledit_on_paint_self(widget_t* widget, canvas_t* c) {
  mledit_t* mledit = MLEDIT(widget);

  text_edit_paint(mledit->model, c);

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

static const char* s_mledit_properties[] = {WIDGET_PROP_TEXT};

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
