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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lineedit.h"
#include "treecli_parser.h"
#include "treecli_shell.h"

int32_t treecli_shell_init(struct treecli_shell *sh, const struct treecli_node *top) {
	assert(sh != NULL);

	/* initialize member variables */
	sh->print_handler = NULL;
	sh->print_handler_ctx = NULL;
	sh->autocomplete = 0;
	sh->autocomplete_at = 0;
	sh->hostname = TREECLI_SHELL_DEFAULT_HOSTNAME;
	sh->prompt_color = TREECLI_SHELL_DEFAULT_PROMPT_COLOR;
	sh->error_color = TREECLI_SHELL_DEFAULT_ERROR_COLOR;

	/* initialize embedded command parser */
	if (treecli_parser_init(&(sh->parser), top) != TREECLI_PARSER_INIT_OK) {
		return TREECLI_SHELL_INIT_FAILED;
	}
	if (treecli_parser_set_print_handler(&(sh->parser), treecli_shell_print_handler, (void *)sh) != TREECLI_PARSER_SET_PRINT_HANDLER_OK) {
		return TREECLI_SHELL_INIT_FAILED;
	}
	if (treecli_parser_set_match_handler(&(sh->parser), treecli_shell_match_handler, (void *)sh) != TREECLI_PARSER_SET_MATCH_HANDLER_OK) {
		return TREECLI_SHELL_INIT_FAILED;
	}
	if (treecli_parser_set_best_match_handler(&(sh->parser), treecli_shell_best_match_handler, (void *)sh) != TREECLI_PARSER_SET_BEST_MATCH_HANDLER_OK) {
		return TREECLI_SHELL_INIT_FAILED;
	}

	/* initialize line editing library */
	if (lineedit_init(&(sh->line), TREECLI_SHELL_LINE_LEN) != LINEEDIT_INIT_OK) {
		return TREECLI_SHELL_INIT_FAILED;
	}
	if (lineedit_set_print_handler(&(sh->line), treecli_shell_print_handler, (void *)sh) != LINEEDIT_SET_PRINT_HANDLER_OK) {
		return TREECLI_SHELL_INIT_FAILED;
	}
	if (lineedit_set_prompt_callback(&(sh->line), treecli_shell_prompt_callback, (void *)sh) != LINEEDIT_SET_PROMPT_CALLBACK_OK) {
		return TREECLI_SHELL_INIT_FAILED;
	}

	return TREECLI_SHELL_INIT_OK;
}


int32_t treecli_shell_free(struct treecli_shell *sh) {
	assert(sh != NULL);

	/* Do not return if failed, do our best to free as much as possible. */
	int32_t ret = TREECLI_SHELL_FREE_OK;

	if (treecli_parser_free(&(sh->parser)) != TREECLI_PARSER_FREE_OK) {
		ret = TREECLI_SHELL_FREE_FAILED;
	}
	if (lineedit_free(&(sh->line)) != LINEEDIT_FREE_OK) {
		ret = TREECLI_SHELL_FREE_FAILED;
	}

	return ret;
}


int32_t treecli_shell_set_print_handler(struct treecli_shell *sh, int32_t (*print_handler)(const char *line, void *ctx), void *ctx) {
	assert(sh != NULL);
	assert(print_handler != NULL);

	sh->print_handler = print_handler;
	sh->print_handler_ctx = ctx;

	/* Current line with command prompt is displayed when print handler is made
	 * available. If we cannot refresh actually edited line, something is wrong. */
	if (lineedit_refresh(&(sh->line)) != LINEEDIT_REFRESH_OK) {
		return TREECLI_SHELL_SET_PRINT_HANDLER_FAILED;
	}

	return TREECLI_SHELL_SET_PRINT_HANDLER_OK;
}


int32_t treecli_shell_prompt_callback(struct lineedit *le, void *ctx) {
	assert(le != NULL);
	assert(ctx != NULL);

	uint32_t len = 0;

	struct treecli_shell *sh = (struct treecli_shell *)ctx;

	if (sh->print_handler) {
		/* Use shell print handler to output command prompt. Prompt
		 * length have to be returned back to line editor to let it know
		 * where actual editing begins. */
		/* TODO: system/host name should be printed instead */
		lineedit_escape_print(le, ESC_COLOR, sh->prompt_color);
		lineedit_escape_print(le, ESC_BOLD, 0);
		sh->print_handler(sh->hostname, sh->print_handler_ctx);
		sh->print_handler(" ", sh->print_handler_ctx);
		len += 1 + strlen(sh->hostname);

		lineedit_escape_print(le, ESC_DEFAULT, 0);
		lineedit_escape_print(le, ESC_COLOR, sh->prompt_color);

		/* print actual working position in the tree */
		uint32_t ret = treecli_parser_pos_print(&(sh->parser));
		if (ret > 0) {
			len += ret;
		}

		lineedit_escape_print(le, ESC_BOLD, 0);
		sh->print_handler(" > ", sh->print_handler_ctx);
		lineedit_escape_print(le, ESC_DEFAULT, 0);
		len += 3;

	} else {
		/* negative number should be returned if error occurs. */
		return -1;
	}

	return len;
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


int32_t treecli_shell_match_handler(const char *token, enum treecli_match_type match_type, void *ctx) {
	assert(token != NULL);
	assert(ctx != NULL);

	struct treecli_shell *sh = (struct treecli_shell *)ctx;

	if (match_type == TREECLI_MATCH_TYPE_VALUE) {
		lineedit_escape_print(&(sh->line), ESC_COLOR, LINEEDIT_FG_COLOR_YELLOW);
	} else if (match_type == TREECLI_MATCH_TYPE_COMMAND) {
		lineedit_escape_print(&(sh->line), ESC_COLOR, LINEEDIT_FG_COLOR_BLUE);
	} else {
		lineedit_escape_print(&(sh->line), ESC_COLOR, LINEEDIT_FG_COLOR_GREEN);
	}
	treecli_shell_print_handler(token, (void *)sh);
	treecli_shell_print_handler(" ", (void *)sh);
	lineedit_escape_print(&(sh->line), ESC_DEFAULT, 0);

	return 0;
}


int32_t treecli_shell_best_match_handler(const char *token, uint32_t token_len, uint32_t match_pos, uint32_t match_len, void *ctx) {
	assert(token != NULL);
	assert(ctx != NULL);

	struct treecli_shell *sh = (struct treecli_shell *)ctx;

	//~ printf("best match token=%s token_len=%d match_start=%d match_len=%d\n", token, token_len, match_pos, match_len);

	if ((match_pos + match_len) == sh->autocomplete_at && sh->autocomplete) {

		if (match_len < token_len) {
			//~ printf("autocomplete %s len=%d\n", token + match_len, token_len - match_len);
			for (uint32_t i = 0; i < (token_len - match_len); i++) {
				lineedit_insert_char(&(sh->line), token[match_len + i]);
			}

			if (strlen(token) == token_len) {
				lineedit_insert_char(&(sh->line), ' ');
			}

		}
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
		/* Error occured, print error position.
		 * TODO: shift depending on prompt length*/
		lineedit_escape_print(&(sh->line), ESC_COLOR, sh->error_color);
		//~ lineedit_escape_print(&(sh->line), ESC_BOLD, 0);
		for (uint32_t i = 0; i < (sh->parser.error_pos + sh->line.prompt_len); i++) {
			sh->print_handler("-", sh->print_handler_ctx);
		}
		sh->print_handler("^\n", sh->print_handler_ctx);

		if (res == TREECLI_PARSER_PARSE_LINE_FAILED) {
			sh->print_handler("error: command parsing failed\n", sh->print_handler_ctx);
		}
		if (res == TREECLI_PARSER_PARSE_LINE_MULTIPLE_MATCHES) {
			sh->print_handler("error: multiple matches\n", sh->print_handler_ctx);
		}
		if (res == TREECLI_PARSER_PARSE_LINE_NO_MATCHES) {
			sh->print_handler("error: no match\n", sh->print_handler_ctx);
		}
		if (res == TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE) {
			sh->print_handler("error: cannot change working position\n", sh->print_handler_ctx);
		}
		if (res == TREECLI_PARSER_PARSE_LINE_EXPECTING_VALUE) {
			sh->print_handler("error: value expected\n", sh->print_handler_ctx);
		}
		if (res == TREECLI_PARSER_PARSE_LINE_UNEXPECTED_TOKEN) {
			sh->print_handler("error: unexpected token\n", sh->print_handler_ctx);
		}
		if (res == TREECLI_PARSER_PARSE_LINE_COMMAND_FAILED) {
			sh->print_handler("error: command execution failed\n", sh->print_handler_ctx);
		}
		if (res == TREECLI_PARSER_PARSE_LINE_VALUE_FAILED) {
			sh->print_handler("error: value parsing failed\n", sh->print_handler_ctx);
		}
		lineedit_escape_print(&(sh->line), ESC_DEFAULT, 0);
		return TREECLI_SHELL_PRINT_PARSER_RESULT_OK;

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
		treecli_parser_set_mode(&(sh->parser), TREECLI_PARSER_ALLOW_EXEC);
		int32_t parser_ret = treecli_parser_parse_line(&(sh->parser), cmd);

		/* Print parsing results and prepare lineedit for new command. */
		treecli_shell_print_parser_result(sh, parser_ret);
		lineedit_clear(&(sh->line));
		lineedit_refresh(&(sh->line));

		return TREECLI_SHELL_KEYPRESS_OK;
	}

	if (ret == LINEEDIT_TAB) {
		/* always move to next line after <tab> press */
		sh->print_handler("\r\n", sh->print_handler_ctx);

		/* we are autocompleting only at cursor position - get it */
		uint32_t cursor;
		if (lineedit_get_cursor(&(sh->line), &cursor) != LINEEDIT_GET_CURSOR_OK) {
			/* ignore keypress on error */
			return TREECLI_SHELL_KEYPRESS_OK;
		}

		char *cmd;
		lineedit_get_line(&(sh->line), &cmd);

		/* Setup the parser to do nothing. */
		treecli_parser_set_mode(&(sh->parser), TREECLI_PARSER_DEFAULT);

		/* try to parse whole command with execution disabled to find out
		 * position of multiple matches */
		int32_t parser_ret = treecli_parser_parse_line(&(sh->parser), cmd);

		if (parser_ret == TREECLI_PARSER_PARSE_LINE_MULTIPLE_MATCHES) {
			if (cursor == (sh->parser.error_pos + sh->parser.error_len)) {

				/* run parser again to display suggestions */

				treecli_parser_set_mode(&(sh->parser), TREECLI_PARSER_ALLOW_MATCHES);
				sh->autocomplete = 0;
				sh->autocomplete_at = cursor;
				parser_ret = treecli_parser_parse_line(&(sh->parser), cmd);

				sh->print_handler("\r\n", sh->print_handler_ctx);

				/* and run the parser for the third time to
				 * possibly autocomplete suggested tokens */
				treecli_parser_set_mode(&(sh->parser), TREECLI_PARSER_ALLOW_MATCHES | TREECLI_PARSER_ALLOW_BEST_MATCH);
				sh->autocomplete = 1;
				sh->autocomplete_at = cursor;
				parser_ret = treecli_parser_parse_line(&(sh->parser), cmd);

			} else {
				treecli_shell_print_parser_result(sh, parser_ret);
			}
		} else if (parser_ret == TREECLI_PARSER_PARSE_LINE_OK) {

			treecli_parser_set_mode(&(sh->parser), TREECLI_PARSER_ALLOW_SUGGESTIONS);
			sh->autocomplete = 0;
			sh->autocomplete_at = cursor;
			parser_ret = treecli_parser_parse_line(&(sh->parser), cmd);

			sh->print_handler("\r\n", sh->print_handler_ctx);

			treecli_parser_set_mode(&(sh->parser), TREECLI_PARSER_ALLOW_BEST_MATCH);
			sh->autocomplete = 1;
			sh->autocomplete_at = cursor;
			parser_ret = treecli_parser_parse_line(&(sh->parser), cmd);

		} else {
			treecli_shell_print_parser_result(sh, parser_ret);
		}

		lineedit_refresh(&(sh->line));

		return TREECLI_SHELL_KEYPRESS_OK;
	}

	return TREECLI_SHELL_KEYPRESS_OK;
}


int32_t treecli_shell_set_parser_context(struct treecli_shell *sh, void *context) {
	u_assert(sh != NULL);
	u_assert(context != NULL);

	/* Set context of the parser directly. */
	if (treecli_parser_set_context(&(sh->parser), context) != TREECLI_PARSER_SET_CONTEXT_OK) {
		return TREECLI_SHELL_SET_PRINT_HANDLER_FAILED;
	}

	return TREECLI_SHELL_SET_PRINT_HANDLER_OK;
}

