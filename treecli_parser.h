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


#ifndef _TREECLI_PARSER_H_
#define _TREECLI_PARSER_H_

#include <stdint.h>
#include <stdbool.h>


/**
 * Custom assert definition. In an embedded environment, it can be made void or
 * modified according to custom needs.
 */
#ifndef u_assert
#define u_assert(e) ((e) ? (0) : (u_assert_func(#e, __FILE__, __LINE__)))
#endif

#ifndef TREECLI_TREE_MAX_DEPTH
#define TREECLI_TREE_MAX_DEPTH 8
#endif

#ifndef TREECLI_PARSER_AVAILABLE_SUBNODES
#define TREECLI_PARSER_AVAILABLE_SUBNODES "Available subnodes:\n"
#endif

#ifndef TREECLI_PARSER_AVAILABLE_COMMANDS
#define TREECLI_PARSER_AVAILABLE_COMMANDS "Available commands:\n"
#endif

#ifndef TREECLI_PARSER_HELP_UNAVAILABLE
#define TREECLI_PARSER_HELP_UNAVAILABLE "<help unavailable>"
#endif

#ifndef TREECLI_DNODE_MAX_COUNT
#define TREECLI_DNODE_MAX_COUNT 100
#endif

#ifndef TREECLI_DNODE_MAX_NAME_LEN
#define TREECLI_DNODE_MAX_NAME_LEN 100
#endif


enum treecli_value_type {
	TREECLI_VALUE_UINT32,
	TREECLI_VALUE_INT32,
	TREECLI_VALUE_STR,
	TREECLI_VALUE_TIME,
	TREECLI_VALUE_DATE,
	TREECLI_VALUE_DATA,
	TREECLI_VALUE_PHYS,
	TREECLI_VALUE_BOOL,
};


struct treecli_parser;
struct treecli_parser_pos;
struct treecli_parser_pos_level;


struct treecli_command {

	const char *name;
	const char *help;
	const struct treecli_command *next;

	int32_t (*exec)(struct treecli_parser *parser, void *exec_context);
	void *exec_context;
};

struct treecli_value {
	const char *name;
	const char *help;

	/**
	 * Pointer to previously allocated variable with the actual value.
	 * It is optional and could be ommited (value getter/setter can be used
	 * instead). If neither one is specified, value always reads as (uint32)0,
	 * write has ho effect.
	 */
	void *value;

	/**
	 * Value assigned after initialization. It has no effect if value getter
	 * is valid.
	 */
	void *default_value;

	/**
	 * Type of value or value getter/setter.
	 */
	enum treecli_value_type value_type;

	const char *units;

	/* TODO: value boundaries */
	/* TODO: valid values enum/iterator */

	/**
	 * Getter function can be used instead of reference to manipulated variable.
	 * Value getter (get callback function) is called every time a value needs
	 * to be determined and set function is called when a value assignment
	 * is requested. Both functions are called with get_set_context passed
	 * as their ctx argument.
	 * */
	int32_t (*get)(struct treecli_parser *parser, void *ctx, struct treecli_value *value, void *buf, size_t *len);
	int32_t (*set)(struct treecli_parser *parser, void *ctx, struct treecli_value *value, void *buf, size_t len);
	void *get_set_context;

	const struct treecli_value *next;
};

/**
 * Single node in the tree hierarchy. Nodes can be chained together on the same
 * level using "next" field (creating linked list of nodes, last one must have it
 * set to NULL). There must be only one top level node (conventionally named "/")
 * which is used to reference the whole tree.
 */
struct treecli_node {
	char *name;
	char *help;

	const struct treecli_node *(*subnodes)[];
	const struct treecli_dnode *(*dsubnodes)[];
	const struct treecli_command *(*commands)[];
	const struct treecli_value *(*values)[];

	const struct treecli_node *next;
};


/**
 * Dnode specifies how are node childs generated during runtime. It can be used
 * to dynamically generate configuration subtrees for components not known at
 * compile time (for example - network interfaces).
 */
struct treecli_dnode {
	const char *name;

	int32_t (*create)(struct treecli_parser *parser, uint32_t index, struct treecli_node *node, void *ctx);
	void *create_context;

	const struct treecli_dnode *next;
};


/**
 * One level in hierarchical tree structure can be described by a statically
 * initialized node or dynamically created node (dnode specification and its index).
 */
struct treecli_parser_pos_level {
	const struct treecli_node *node;
	const struct treecli_dnode *dnode;
	uint32_t dnode_index;
};

/**
 * The whole path in the hierarchical tree structure from its root up to the
 * current working node is described as an array of nodes (static or dynamic) and
 * index of actual working node (which also determines the tree depth).
 */
struct treecli_parser_pos {
	struct treecli_parser_pos_level levels[TREECLI_TREE_MAX_DEPTH];
	uint32_t depth;
};

/**
 * Parsing can be done in many different modes. These values can be OR'ed together
 * to specify which actions should be taken during command parsing.
 */
enum treecli_parser_mode {
	TREECLI_PARSER_DEFAULT = 0,
	TREECLI_PARSER_ALLOW_MATCHES = 1,
	TREECLI_PARSER_ALLOW_SUGGESTIONS = 2,
	TREECLI_PARSER_ALLOW_BEST_MATCH = 4,
	TREECLI_PARSER_ALLOW_EXEC = 8
};

enum treecli_parser_context {
	TREECLI_PARSER_CONTEXT_NODE = 0,
	TREECLI_PARSER_CONTEXT_VALUE_OPERATOR,
	TREECLI_PARSER_CONTEXT_VALUE_LITERAL,
};

enum treecli_match_type {
	TREECLI_MATCH_TYPE_NODE = 0,
	TREECLI_MATCH_TYPE_VALUE,
	TREECLI_MATCH_TYPE_COMMAND,
};

struct treecli_parser {
	const struct treecli_node *top;
	struct treecli_parser_pos pos;

	int32_t (*print_handler)(const char *line, void *ctx);
	void *print_handler_ctx;

	int32_t (*match_handler)(const char *token, enum treecli_match_type match_type, void *ctx);
	void *match_handler_ctx;

	int32_t (*best_match_handler)(const char *token, uint32_t token_len, uint32_t match_pos, uint32_t match_len, void *ctx);
	void *best_match_handler_ctx;

	uint32_t error_pos;
	uint32_t error_len;

	enum treecli_parser_mode mode;

	const struct treecli_value *parsing_value;
	enum treecli_parser_context parsing_context;

	void *context;
};

struct treecli_matches {
	uint32_t count;
	const struct treecli_node *subnode;
	const struct treecli_dnode *dsubnode;
	uint32_t dsubnode_index;
	const struct treecli_command *command;
	const struct treecli_value *value;

	char best_match[100];
	uint32_t best_match_len;
};


int32_t treecli_print_tree(const struct treecli_node *top, int32_t indent);
#define TREECLI_PRINT_TREE_OK 0
#define TREECLI_PRINT_TREE_FAILED -1

int32_t treecli_parser_pos_print(struct treecli_parser *parser);
#define TREECLI_PARSER_POS_PRINT_OK 0
#define TREECLI_PARSER_POS_PRINT_FAILED -1

/**
 * Search command line and try to get next token. Token is a word describing one
 * subnode, command or value name consisting of alphanumeric characters (lower and
 * uppoer case), underscore, dash, dot and slash.
 * Input line position is being incremented during search and after execution it
 * points to a position where the search can continue (this apply also if function
 * fails).
 *
 * @param parser TODO: remove
 * @param pos Pointer to initial string position where the search should begin.
 * @param token Pointer which will point to start of the token after successful
 *              execution.
 * @param len Length of the token.
 *
 * @return TREECLI_TOKEN_GET_OK if valid token was found or
 *         TREECLI_TOKEN_GET_UNEXPECTED if invalid characters were found or
 *         TREECLI_TOKEN_GET_NONE if end of line was reached or
 *         TREECLI_TOKEN_GET_FAILED otherwise.
 */
int32_t treecli_token_get(struct treecli_parser *parser, const char **pos, const char **token, uint32_t *len);
#define TREECLI_TOKEN_GET_OK 0
#define TREECLI_TOKEN_GET_FAILED -1
#define TREECLI_TOKEN_GET_NONE -2
#define TREECLI_TOKEN_GET_UNEXPECTED -3

/**
 * Initializes parser context. It is used to parse lines of configuration, execute
 * commands and manipulate configuration variables. Parser operates on a
 * configuration tree defined by its top level node.
 *
 * @param parser A treecli parser context to initialize.
 * @param top Top node of configuration structure used during parsing.
 *
 * @return TREECLI_PARSER_INIT_OK on success or
 *         TREECLI_PARSER_INIT_FAILED otherwise.
 */
int32_t treecli_parser_init(struct treecli_parser *parser, const struct treecli_node *top);
#define TREECLI_PARSER_INIT_OK 0
#define TREECLI_PARSER_INIT_FAILED -1

/**
 * Frees previously created parser context. Nothing to do here yet, placeholder.
 *
 * @param parser A parser context to free.
 *
 * @return TREECLI_PARSER_FREE_OK.
 */
int32_t treecli_parser_free(struct treecli_parser *parser);
#define TREECLI_PARSER_FREE_OK 0
#define TREECLI_PARSER_FREE_FAILED -1

/**
 * Function parses one line of commands and performs configuration tree traversal
 * to set active node for command execution, sets or reads values and executes
 * commands (if requested).
 *
 * @param parser A parser context used to do command parsing.
 * @param line String with node names, commands and value set/get specifications.
 *
 * @return TREECLI_PARSER_PARSE_LINE_OK if the whole line was parsed successfully.
 */
int32_t treecli_parser_parse_line(struct treecli_parser *parser, const char *line);
#define TREECLI_PARSER_PARSE_LINE_OK 0
#define TREECLI_PARSER_PARSE_LINE_FAILED -1
#define TREECLI_PARSER_PARSE_LINE_NO_MATCHES -2
#define TREECLI_PARSER_PARSE_LINE_MULTIPLE_MATCHES -3
#define TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE -4
#define TREECLI_PARSER_PARSE_LINE_COMMAND_FAILED -5
#define TREECLI_PARSER_PARSE_LINE_VALUE_FAILED -6
#define TREECLI_PARSER_PARSE_LINE_EXPECTING_VALUE -7
#define TREECLI_PARSER_PARSE_LINE_UNEXPECTED_TOKEN -8


int32_t treecli_parser_set_print_handler(struct treecli_parser *parser, int32_t (*print_handler)(const char *line, void *ctx), void *ctx);
#define TREECLI_PARSER_SET_PRINT_HANDLER_OK 0
#define TREECLI_PARSER_SET_PRINT_HANDLER_FAILED -1

int32_t treecli_parser_set_match_handler(struct treecli_parser *parser, int32_t (*match_handler)(const char *token, enum treecli_match_type match_type, void *ctx), void *ctx);
#define TREECLI_PARSER_SET_MATCH_HANDLER_OK 0
#define TREECLI_PARSER_SET_MATCH_HANDLER_FAILED -1

int32_t treecli_parser_set_best_match_handler(struct treecli_parser *parser, int32_t (*best_match_handler)(const char *token, uint32_t token_len, uint32_t match_pos, uint32_t match_len, void *ctx), void *ctx);
#define TREECLI_PARSER_SET_BEST_MATCH_HANDLER_OK 0
#define TREECLI_PARSER_SET_BEST_MATCH_HANDLER_FAILED -1

uint32_t treecli_parser_strmatch(const char *s1, const char *s2);

int32_t treecli_parser_resolve_match(struct treecli_parser *parser, struct treecli_matches *matches, const char *token);
#define TREECLI_PARSER_RESOLVE_MATCH_OK 0
#define TREECLI_PARSER_RESOLVE_MATCH_FAILED -1

int32_t treecli_parser_try_match(struct treecli_parser *parser, struct treecli_matches *matches, const char *token, uint32_t len, const char *str);
#define TREECLI_PARSER_TRY_MATCH_OK 0
#define TREECLI_PARSER_TRY_MATCH_FAILED -1

int32_t treecli_parser_get_matches(struct treecli_parser *parser, const char *token, uint32_t len, struct treecli_matches *matches);
#define TREECLI_PARSER_GET_MATCHES_VALUE_LITERAL 8
#define TREECLI_PARSER_GET_MATCHES_VALUE_OPERATOR 7
#define TREECLI_PARSER_GET_MATCHES_HELP 6
#define TREECLI_PARSER_GET_MATCHES_DSUBNODE 5
#define TREECLI_PARSER_GET_MATCHES_SUBNODE 4
#define TREECLI_PARSER_GET_MATCHES_VALUE 3
#define TREECLI_PARSER_GET_MATCHES_COMMAND 2
#define TREECLI_PARSER_GET_MATCHES_TOP 1
#define TREECLI_PARSER_GET_MATCHES_UP 0
#define TREECLI_PARSER_GET_MATCHES_FAILED -1
#define TREECLI_PARSER_GET_MATCHES_MULTIPLE -2
#define TREECLI_PARSER_GET_MATCHES_NONE -3

int32_t treecli_parser_pos_move(struct treecli_parser_pos *pos, struct treecli_parser_pos_level *level);
#define TREECLI_PARSER_POS_MOVE_OK 0
#define TREECLI_PARSER_POS_MOVE_FAILED -1

int32_t treecli_parser_pos_up(struct treecli_parser_pos *pos);
#define TREECLI_PARSER_POS_UP_OK 0
#define TREECLI_PARSER_POS_UP_FAILED -1

int32_t treecli_parser_pos_root(struct treecli_parser_pos *pos);
#define TREECLI_PARSER_POS_ROOT_OK 0
#define TREECLI_PARSER_POS_ROOT_FAILED -1

int32_t treecli_parser_pos_copy(struct treecli_parser_pos *pos, struct treecli_parser_pos *src);
#define TREECLI_PARSER_POS_COPY_OK 0
#define TREECLI_PARSER_POS_COPY_FAILED -1

int32_t treecli_parser_pos_init(struct treecli_parser_pos *pos);
#define TREECLI_PARSER_POS_INIT_OK 0
#define TREECLI_PARSER_POS_INIT_FAILED -1

int32_t treecli_parser_get_current_node(struct treecli_parser *parser, struct treecli_node *node);
#define TREECLI_PARSER_GET_CURRENT_NODE_OK 0
#define TREECLI_PARSER_GET_CURRENT_NODE_ROOT -1
#define TREECLI_PARSER_GET_CURRENT_NODE_FAILED -2

int32_t treecli_parser_help(struct treecli_parser *parser);
#define TREECLI_PARSER_HELP_OK 0
#define TREECLI_PARSER_HELP_FAILED -1

int32_t treecli_parser_dnode_get_name(struct treecli_parser *parser, const struct treecli_dnode *dnode, uint32_t index, char *name);
#define TREECLI_PARSER_DNODE_GET_NAME_OK 0
#define TREECLI_PARSER_DNODE_GET_NAME_FAILED -1

int32_t treecli_parser_set_mode(struct treecli_parser *parser, enum treecli_parser_mode mode);
#define TREECLI_PARSER_SET_MODE_OK 0
#define TREECLI_PARSER_SET_MODE_FAILED -1

int32_t treecli_parser_value_to_str(struct treecli_parser *parser, char *s, const struct treecli_value *value, uint32_t max);
#define TREECLI_PARSER_VALUE_TO_STR_OK 0
#define TREECLI_PARSER_VALUE_TO_STR_FAILED -1

int32_t treecli_parser_str_to_value(struct treecli_parser *parser, const struct treecli_value *value, const char *s, uint32_t len);
#define TREECLI_PARSER_STR_TO_VALUE_OK 0
#define TREECLI_PARSER_STR_TO_VALUE_FAILED -1

int32_t treecli_parser_set_context(struct treecli_parser *parser, void *context);
#define TREECLI_PARSER_SET_CONTEXT_OK 0
#define TREECLI_PARSER_SET_CONTEXT_FAILED -1


#endif
