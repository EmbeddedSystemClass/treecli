
#include "treecli_parser.h"
#include "lineedit.h"

#ifndef _TREECLI_SHELL_H_
#define _TREECLI_SHELL_H_

#ifndef TREECLI_SHELL_LINE_LEN
#define TREECLI_SHELL_LINE_LEN 200
#endif


struct treecli_shell {
	struct treecli_parser parser;
	struct lineedit line;

	int32_t (*print_handler)(const char *line, void *ctx);
	void *print_handler_ctx;

	uint32_t autocomplete;
	uint32_t autocomplete_at;
};




int32_t treecli_shell_init(struct treecli_shell *sh, const struct treecli_node *top);
#define TREECLI_SHELL_INIT_OK 0
#define TREECLI_SHELL_INIT_FAILED -1

int32_t treecli_shell_free(struct treecli_shell *sh);
#define TREECLI_SHELL_FREE_OK 0
#define TREECLI_SHELL_FREE_FAILED -1

int32_t treecli_shell_set_print_handler(struct treecli_shell *sh, int32_t (*print_handler)(const char *line, void *ctx), void *ctx);
#define TREECLI_SHELL_SET_PRINT_HANDLER_OK 0
#define TREECLI_SHELL_SET_PRINT_HANDLER_FAILED -1

int32_t treecli_shell_prompt_callback(struct lineedit *le, void *ctx);
int32_t treecli_shell_print_handler(const char *line, void *ctx);
int32_t treecli_shell_match_handler(const char *token, void *ctx);
int32_t treecli_shell_best_match_handler(const char *token, uint32_t token_len, uint32_t match_pos, uint32_t match_len, void *ctx);

int32_t treecli_shell_print_parser_result(struct treecli_shell *sh, int32_t res);
#define TREECLI_SHELL_PRINT_PARSER_RESULT_OK 0
#define TREECLI_SHELL_PRINT_PARSER_RESULT_FAILED -1

int32_t treecli_shell_keypress(struct treecli_shell *sh, int c);
#define TREECLI_SHELL_KEYPRESS_OK 0
#define TREECLI_SHELL_KEYPRESS_FAILED -1
#define TREECLI_SHELL_KEYPRESS_QUIT -2





#endif


