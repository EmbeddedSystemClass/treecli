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

#include "treecli_parser.h"
#include "lineedit.h"

#define assert u_assert

#ifndef _TREECLI_SHELL_H_
#define _TREECLI_SHELL_H_

#ifndef TREECLI_SHELL_LINE_LEN
#define TREECLI_SHELL_LINE_LEN 200
#endif

#ifndef TREECLI_SHELL_DEFAULT_HOSTNAME
#define TREECLI_SHELL_DEFAULT_HOSTNAME "cli"
#endif

#ifndef TREECLI_SHELL_DEFAULT_PROMPT_COLOR
#define TREECLI_SHELL_DEFAULT_PROMPT_COLOR LINEEDIT_FG_COLOR_GREEN
#endif

#ifndef TREECLI_SHELL_DEFAULT_ERROR_COLOR
#define TREECLI_SHELL_DEFAULT_ERROR_COLOR LINEEDIT_FG_COLOR_RED
#endif

/**
 * Treecli shell structure holding single shell context. The structure shouldn't
 * be accessed directly.
 */
struct treecli_shell {

	/**
	 * Treecli parser context used for command parsing and autocompletion
	 * during shell session.
	 */
	struct treecli_parser parser;

	/**
	 * A context for the line editing library providing a convenient interface
	 * for single line editing, autocompletion and history over simple serial
	 * interfaces (eg. socket, serial line).
	 */
	struct lineedit line;

	/**
	 * Print handler function is called with print_handler_ctx structure member
	 * passed as ctx parameter whenever the shell needs to write anything to
	 * its console output. It must be set prior to any other operations on
	 * the shell context.
	 */
	int32_t (*print_handler)(const char *line, void *ctx);
	void *print_handler_ctx;

	/**
	 * Set autocomplete to nonzero value and autocomplete_at to command line
	 * position where autocompletion is requested. Autocompletion will be
	 * performed on next parser call. Internal temporary variable.
	 */
	uint8_t autocomplete;
	uint32_t autocomplete_at;

	const char *hostname;
	uint8_t prompt_color;
	uint8_t error_color;
};


/**
 * @brief Initialize the shell in the previously allocated treecli_shell structure.
 *
 * Internal shell components (lineedit and parser) are also initialized. Print
 * handler has to be set for correct shell operation. It is used to print all
 * output from line editor and commands. Keypress function can be used afterwards
 * to pass input to the shell.
 *
 * @param sh A treecli shell to initialize. Cannot be NULL.
 * @param top Top node of a configuration tree. Cannot be NULL.
 *
 * @return TREECLI_SHELL_INIT_OK on success or
 *         TREECLI_SHELL_INIT_FAILED otherwise.
 */
int32_t treecli_shell_init(struct treecli_shell *sh, const struct treecli_node *top);
#define TREECLI_SHELL_INIT_OK 0
#define TREECLI_SHELL_INIT_FAILED -1


/**
 * @brief Free previously allocated shell and all associated resources.
 *
 * @param sh A treecli shell to free. Cannot be NULL.
 *
 * @return TREECLI_SHELL_FREE_OK if all resources were freed successfully or
 *         TREECLI_SHELL_FREE_FAILED otherwise.
 */
int32_t treecli_shell_free(struct treecli_shell *sh);
#define TREECLI_SHELL_FREE_OK 0
#define TREECLI_SHELL_FREE_FAILED -1


/**
 * @brief Set shell print handler.
 *
 * Print handler is used to output strings to console from command exec callbacks
 * and line editor. No action can be performed prior to setting print handler.
 *
 * @param sh A treecli shell context. Cannot be NULL.
 * @param print_handler Pointer to print handler function to be called whenever
 *                      output printing is requested. Function is called with ctx
 *                      parameter set to @a ctx.
 * @param ctx Context of print handler function which will be passed to it on
 *            each call.
 *
 * @return TREECLI_SHELL_SET_PRINT_HANDLER_OK if print handler function was set
 *                                            successfully or
 *         TREECLI_SHELL_SET_PRINT_HANDLER_FAILED otherwise.
 */
int32_t treecli_shell_set_print_handler(struct treecli_shell *sh, int32_t (*print_handler)(const char *line, void *ctx), void *ctx);
#define TREECLI_SHELL_SET_PRINT_HANDLER_OK 0
#define TREECLI_SHELL_SET_PRINT_HANDLER_FAILED -1


/**
 * @brief Callback function called from treecli parser to output a command prompt.
 *
 * @param le Line editor context from which is the callback called.
 * @param ctx Context of the callback function. Cast to shell context in this case.
 *
 * @return Zero on success, negative integer on failure.
 */
int32_t treecli_shell_prompt_callback(struct lineedit *le, void *ctx);


/**
 * @brief Callback function called from treecli parser to print command output.
 *
 * @param line Pointer to string to print. Cannot be NULL.
 * @param ctx Context of the callback function. Cast to shell context in this case.
 *
 * @return Zero on success, negative integer on failure.
 */
int32_t treecli_shell_print_handler(const char *line, void *ctx);


/**
 * @brief Callback function called from treecli parser to handle matches which
 *        occur during command parsing.
 *
 * @param token Pointer to string with (partially) matched token. Cannot be NULL.
 * @param ctx Context of the callback function. Cast to shell context in this case.
 *
 * @return Zero on success, negative integer on failure.
 */
int32_t treecli_shell_match_handler(const char *token, void *ctx);


/**
 * @brief Callback function called from treecli parser to handle best matched
 *        token during command parsing.
 *
 * @param token Matched token. Cannot be NULL.
 * @param token_len Possible length of the matched part to which the token can
 *                  be autocompleted.
 * @param match_pos Position in parsed command on which (partial) match occured.
 * @param match_len Length of the matched part.
 * @param ctx Context of the callback function. Cast to shell context in this case.
 *
 * @return Zero on success, negative integer on failure.
 */
int32_t treecli_shell_best_match_handler(const char *token, uint32_t token_len, uint32_t match_pos, uint32_t match_len, void *ctx);


/**
 * @brief Print parsing result with a line showing error position.
 *
 * @param sh A treecli shell context. Cannot be NULL.
 * @param res Return value from treecli_parser_parse_line function.
 *
 * @return TREECLI_SHELL_PRINT_PARSER_RESULT_OK on success or
 *         TREECLI_SHELL_PRINT_PARSER_RESULT_FAILED otherwise.
 */
int32_t treecli_shell_print_parser_result(struct treecli_shell *sh, int32_t res);
#define TREECLI_SHELL_PRINT_PARSER_RESULT_OK 0
#define TREECLI_SHELL_PRINT_PARSER_RESULT_FAILED -1


/**
 * @brief A function to be called on every keypress registered on the input.
 *
 * All internal shell processing and command parsing is triggered by calling
 * this single function. Shell can be implemented as a neverending loop waiting
 * for input characters from console and passing them to keypress function.
 *
 * @param sh A treecli shell context. Cannot be NULL.
 * @param c Character to be processed.
 *
 * @return TREECLI_SHELL_KEYPRESS_OK on success or
 *         TREECLI_SHELL_KEYPRESS_FAILED otherwise.
 */
int32_t treecli_shell_keypress(struct treecli_shell *sh, int c);
#define TREECLI_SHELL_KEYPRESS_OK 0
#define TREECLI_SHELL_KEYPRESS_FAILED -1





#endif


