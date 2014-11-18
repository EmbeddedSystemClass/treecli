#ifndef _TERMINAL_H_
#define _TERMINAL_H_

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

