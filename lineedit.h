/**
 * Copyright (c) 2014, Marek Koza (qyx@krtko.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#define LINEEDIT_FG_COLOR_BLACK 30
#define LINEEDIT_FG_COLOR_RED 31
#define LINEEDIT_FG_COLOR_GREEN 32
#define LINEEDIT_FG_COLOR_YELLOW 33
#define LINEEDIT_FG_COLOR_BLUE 34
#define LINEEDIT_FG_COLOR_MAGENTA 35
#define LINEEDIT_FG_COLOR_CYAN 36
#define LINEEDIT_FG_COLOR_WHITE 37


enum lineedit_escape {
	ESC_NONE, ESC_ESC, ESC_CSI, ESC_OSC
};

struct lineedit {
	uint32_t cursor;
	char *text;
	uint32_t len;
	enum lineedit_escape escape;
	uint32_t csi_escape_mod;
	char pwchar;

	int32_t (*print_handler)(const char *line, void *ctx);
	void *print_handler_ctx;

	int32_t (*prompt_callback)(struct lineedit *le, void *ctx);
	void *prompt_callback_ctx;
	uint32_t prompt_len;
};

enum lineedit_escape_seq {
	ESC_CURSOR_LEFT,
	ESC_CURSOR_RIGHT,
	ESC_COLOR,
	ESC_DEFAULT,
	ESC_BOLD,
	ESC_CURSOR_SAVE,
	ESC_CURSOR_RESTORE,
	ESC_ERASE_LINE_END

};





int32_t lineedit_escape_print(struct lineedit *le, enum lineedit_escape_seq esc, int param);
#define LINEEDIT_ESCAPE_PRINT_OK 0
#define LINEEDIT_ESCAPE_PRINT_FAILED -1

int32_t lineedit_init(struct lineedit *le, uint32_t line_len);
#define LINEEDIT_INIT_OK 0
#define LINEEDIT_INIT_FAILED -1

int32_t lineedit_free(struct lineedit *le);
#define LINEEDIT_FREE_OK 0
#define LINEEDIT_FREE_FAILED -1

int32_t lineedit_keypress(struct lineedit *le, int c);
#define LINEEDIT_OK 0
#define LINEEDIT_ENTER -1
#define LINEEDIT_TAB -2

int32_t lineedit_backspace(struct lineedit *le);
#define LINEEDIT_BACKSPACE_OK 0
#define LINEEDIT_BACKSPACE_FAILED -1

int32_t lineedit_insert_char(struct lineedit *le, int c);
#define LINEEDIT_INSERT_CHAR_OK 0
#define LINEEDIT_INSERT_CHAR_FAILED -1

int32_t lineedit_set_print_handler(struct lineedit *le, int32_t (*print_handler)(const char *line, void *ctx), void *ctx);
#define LINEEDIT_SET_PRINT_HANDLER_OK 0
#define LINEEDIT_SET_PRINT_HANDLER_FAILED -1

int32_t lineedit_set_prompt_callback(struct lineedit *le, int32_t (*prompt_callback)(struct lineedit *le, void *ctx), void *ctx);
#define LINEEDIT_SET_PROMPT_CALLBACK_OK 0
#define LINEEDIT_SET_PROMPT_CALLBACK_FAILED -1

int32_t lineedit_refresh(struct lineedit *le);
#define LINEEDIT_REFRESH_OK 0
#define LINEEDIT_REFRESH_FAILED -1

int32_t lineedit_get_cursor(struct lineedit *le, uint32_t *cursor);
#define LINEEDIT_GET_CURSOR_OK 0
#define LINEEDIT_GET_CURSOR_FAILED -1

int32_t lineedit_set_cursor(struct lineedit *le, uint32_t cursor);
#define LINEEDIT_SET_CURSOR_OK 0
#define LINEEDIT_SET_CURSOR_FAILED -1

int32_t lineedit_get_line(struct lineedit *le, char **text);
#define LINEEDIT_GET_LINE_OK 0
#define LINEEDIT_GET_LINE_FAILED -1

int32_t lineedit_set_line(struct lineedit *le, const char *text);
#define LINEEDIT_SET_LINE_OK 0
#define LINEEDIT_SET_LINE_FAILED -1

int32_t lineedit_clear(struct lineedit *le);
#define LINEEDIT_CLEAR_OK 0
#define LINEEDIT_CLEAR_FAILED -1

int32_t lineedit_insert(struct lineedit *le, const char *text);
#define LINEEDIT_INSERT_OK 0
#define LINEEDIT_INSERT_FAILED -1




#endif

