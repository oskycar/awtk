/**
 * File:   text_edit.h
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

#ifndef TK_TEXT_EDIT_H
#define TK_TEXT_EDIT_H

#include "base/widget.h"

BEGIN_C_DECLS

/**
 * @class text_edit_t
 */
typedef struct _text_edit_t {
  canvas_t* c;
  widget_t* widget;
} text_edit_t;

typedef struct _text_edit_info_t {
  int32_t cursor;
  wchar_t newline;
  int32_t char_spacing;
  int32_t select_start;
  int32_t select_end;
  int32_t line_height;
} text_edit_info_t;

/**
 * @method text_edit_create
 * 创建text_edit对象
 * @param {widget_t*} widget 控件
 * @param {boo_t} single_line 单行。
 *
 * @return {widget_t*} 对象。
 */
text_edit_t* text_edit_create(widget_t* widget, bool_t signle_line);

/**
 * @method text_edit_set_canvas
 * 设置canvas对象。
 * @param {text_edit_t*} text_edit text_edit对象。
 * @param {canvas_t*} c canvas对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_edit_set_canvas(text_edit_t* text_edit, canvas_t* canvas);

/**
 * @method text_edit_click
 * click
 * @param {text_edit_t*} text_edit text_edit对象。
 * @param {xy_t} x x坐标。
 * @param {xy_t} y y坐标。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_edit_click(text_edit_t* text_edit, xy_t x, xy_t y);

/**
 * @method text_edit_paste
 * paste
 * @param {text_edit_t*} text_edit text_edit对象。
 * @param {wchar_t*} str 文本。
 * @param {uint32_t} size 文本长度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_edit_paste(text_edit_t* text_edit, wchar_t* str, uint32_t size);

/**
 * @method text_edit_drag
 * drag
 * @param {text_edit_t*} text_edit text_edit对象。
 * @param {xy_t} x x坐标。
 * @param {xy_t} y y坐标。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_edit_drag(text_edit_t* text_edit, xy_t x, xy_t y);

/**
 * @method text_edit_key_down
 * key_down
 * @param {text_edit_t*} text_edit text_edit对象。
 * @param {key_event_t*} evt event
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_edit_key_down(text_edit_t* text_edit, key_event_t* evt);

/**
 * @method text_edit_set_info
 * set_info
 * @param {text_edit_t*} text_edit text_edit对象。
 * @param {const text_edit_info_t*} info text edit info;
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_edit_set_info(text_edit_t* text_edit, const text_edit_info_t* info);

/**
 * @method text_edit_get_info
 * get_info
 * @param {text_edit_t*} text_edit text_edit对象。
 * @param {text_edit_info_t*} info text edit info;
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_edit_get_info(text_edit_t* text_edit, text_edit_info_t* info);

/**
 * @method text_edit_destroy
 * 销毁text_edit对象。
 * @param {text_edit_t*} text_edit text_edit对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t text_edit_destroy(text_edit_t* text_edit);

END_C_DECLS

#endif /*TK_TEXT_EDIT_H*/
