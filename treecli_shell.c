#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "lineedit.h"
#include "treecli_parser.h"
#include "treecli_shell.h"



int32_t treecli_shell_init(struct treecli_shell *sh, const struct treecli_node *top) {
	assert(sh != NULL);

	sh->print_handler = NULL;
	sh->print_handler_ctx = NULL;

	/* initialize embedded command parser */
	treecli_parser_init(&(sh->parser), top);
	treecli_parser_set_print_handler(&(sh->parser), treecli_shell_print_handler, (void *)sh);
	sh->parser.allow_exec = 1;

	/* initialize line editing library */
	lineedit_init(&(sh->line), TREECLI_SHELL_LINE_LEN);
	lineedit_set_print_handler(&(sh->line), treecli_shell_print_handler, (void *)sh);
	lineedit_set_prompt_callback(&(sh->line), treecli_shell_prompt_callback, (void *)sh);
	lineedit_set_line(&(sh->line), "abcd");

	return TREECLI_SHELL_INIT_OK;
}


int32_t treecli_shell_free(struct treecli_shell *sh) {
	assert(sh != NULL);
	treecli_parser_free(&(sh->parser));
	lineedit_free(&(sh->line));
	
	return TREECLI_SHELL_FREE_OK;
}


int32_t treecli_shell_set_print_handler(struct treecli_shell *sh, int32_t (*print_handler)(const char *line, void *ctx), void *ctx) {
	assert(sh != NULL);
	assert(print_handler != NULL);
	
	sh->print_handler = print_handler;
	sh->print_handler_ctx = ctx;

	lineedit_refresh(&(sh->line));
	
	return TREECLI_SHELL_SET_PRINT_HANDLER_OK;
}


int32_t treecli_shell_prompt_callback(struct lineedit *le, void *ctx) {
	assert(le != NULL);
	assert(ctx != NULL);

	struct treecli_shell *sh = (struct treecli_shell *)ctx;

	if (sh->print_handler) {
		sh->print_handler("cli ", sh->print_handler_ctx);
		treecli_parser_pos_print(&(sh->parser));
		sh->print_handler(" > ", sh->print_handler_ctx);
	}

	return 0;
}


int32_t treecli_shell_print_handler(const char *line, void *ctx) {
	assert(line != NULL);
	assert(ctx != NULL);
	
	struct treecli_shell *sh = (struct treecli_shell *)ctx;
	
	if (sh->print_handler) {
		sh->print_handler(line, sh->print_handler_ctx);
	}

	return 0;
}


int32_t treecli_shell_print_parser_result(struct treecli_shell *sh, int32_t res) {
	assert(sh != NULL);
	if (sh->print_handler == NULL) {
		return TREECLI_SHELL_KEYPRESS_FAILED;
	}

	if (res == TREECLI_PARSER_PARSE_LINE_OK) {
		/* Everything went good, do not print anything, just move on. */
		return TREECLI_SHELL_PRINT_PARSER_RESULT_OK;
	} else {
		/* Error occured, print error position. */
		for (uint32_t i = 0; i < sh->parser.error_pos; i++) {
			sh->print_handler("-", sh->print_handler_ctx);
		}
		sh->print_handler("^\n", sh->print_handler_ctx);
		
		if (res == TREECLI_PARSER_PARSE_LINE_FAILED) {
			sh->print_handler("error: command parsing failed\n", sh->print_handler_ctx);
			return TREECLI_SHELL_PRINT_PARSER_RESULT_OK;
		}
		if (res == TREECLI_PARSER_PARSE_LINE_MULTIPLE_MATCHES) {
			sh->print_handler("error: multiple matches\n", sh->print_handler_ctx);
			return TREECLI_SHELL_PRINT_PARSER_RESULT_OK;
		}
		if (res == TREECLI_PARSER_PARSE_LINE_NO_MATCHES) {
			sh->print_handler("error: no match\n", sh->print_handler_ctx);
			return TREECLI_SHELL_PRINT_PARSER_RESULT_OK;
		}
		if (res == TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE) {
			sh->print_handler("error: cannot change working position\n", sh->print_handler_ctx);
			return TREECLI_SHELL_PRINT_PARSER_RESULT_OK;
		}
	}
	
	return TREECLI_SHELL_PRINT_PARSER_RESULT_FAILED;
}


int32_t treecli_shell_keypress(struct treecli_shell *sh, int c) {
	assert(sh != NULL);
	if (sh->print_handler == NULL) {
		return TREECLI_SHELL_KEYPRESS_FAILED;
	}

	int32_t ret = lineedit_keypress(&(sh->line), c);

	if (ret == LINEEDIT_ENTER) {
		/* Always move to another line before parsing. */
		sh->print_handler("\r\n", sh->print_handler_ctx);

		/* Line editing is finished (ENTER pressed), get line from line
		 * edit library and try to parse it */
		char *cmd;
		lineedit_get_line(&(sh->line), &cmd);
		int32_t parser_ret = treecli_parser_parse_line(&(sh->parser), cmd);

		
		treecli_shell_print_parser_result(sh, parser_ret);

		lineedit_clear(&(sh->line));
		lineedit_refresh(&(sh->line));
		
		return TREECLI_SHELL_KEYPRESS_OK;
	}

	if (ret == LINEEDIT_TAB) {
		sh->print_handler("\r\nerror: autocompletion unimplemented\n", sh->print_handler_ctx);
		lineedit_refresh(&(sh->line));
	
		return TREECLI_SHELL_KEYPRESS_OK;
	}

	return TREECLI_SHELL_KEYPRESS_OK;
}

