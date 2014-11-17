

#ifndef _TREECLI_PARSER_H_
#define _TREECLI_PARSER_H_



#define TREECLI_VALUE_UINT32 0

#define TREECLI_TREE_MAX_DEPTH 8



struct treecli_parser;
struct treecli_parser_pos;
struct treecli_parser_pos_level;

/**
 * Dnode specifies how are node childs generated during runtime. It can be used
 * to dynamically generate configuration subtrees for components not known at
 * compile time (for example - network interfaces).
 */
struct treecli_dnode {
	
	const struct treecli_dnode *next;
};


struct treecli_command {
	
	/* TODO:
	 * function pointer to execute the command
	 */
	const char *name;
	const char *help;
	const struct treecli_command *next;

	int32_t (*exec)(struct treecli_parser *parser, void *exec_context);
	void *exec_context;
};

struct treecli_value {
	const char *name;
	const char *help;

	/* Pointer to variable with actual value. Optional, could be ommited
	 * (value getter/setter can be used instead). If neither one is specified,
	 * value always reads as (uint32)0, write has ho effect */
	void *value;
	
	/* Value assigned after initialization. It has no effect if value getter
	 * is valid. */
	void *default_value;
	
	/* Type of value or value getter/setter */
	int value_type;
	
	/* TODO: value getter/setter functions */
	/* TODO: value boundaries */
	/* TODO: valid values enum/iterator */

	const struct treecli_value *next;
};

/**
 * Single node in the tree hierarchy. Nodes can be chained together on the same
 * level using "next" field (creating linked list of nodes, last one must have it
 * set to NULL). There must be only one top level node (conventionally named "/")
 * which is used to reference the whole tree.
 */
struct treecli_node {
	/* TODO:
	 */
	
	const char *name;
	const char *help;
	
	const struct treecli_node *subnodes;
	const struct treecli_dnode *dsubnodes;
	const struct treecli_command *commands;
	const struct treecli_value *values;
	
	const struct treecli_node *next;
};





/**
 * One level in hierarchical tree structure can be described by a statically
 * initialized node or dynamically created node (dnode specification and its index).
 */
struct treecli_parser_pos_level {
	const struct treecli_node *node;
	const struct treecli_node *dnode;
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


struct treecli_parser {
	const struct treecli_node *top;
	struct treecli_parser_pos pos;
	
	int32_t (*print_handler)(const char *line, void *ctx);
	void *print_handler_ctx;
	
	uint32_t error_pos;
	
	int allow_exec;
};

struct treecli_matches {
	uint32_t count;
	const struct treecli_node *subnode;
	const struct treecli_dnode *dsubnode;
	const struct treecli_command *command;
	const struct treecli_value *value;
	
	
};





int32_t treecli_print_tree(const struct treecli_node *top, int32_t indent);
#define TREECLI_PRINT_TREE_OK 0
#define TREECLI_PRINT_TREE_FAILED -1

int32_t treecli_parser_pos_print(const struct treecli_parser *parser);
#define TREECLI_PARSER_POS_PRINT_OK 0
#define TREECLI_PARSER_POS_PRINT_FAILED -1

int32_t treecli_token_get(struct treecli_parser *parser, char **pos, char **token, uint32_t *len);
#define TREECLI_TOKEN_GET_OK 0
#define TREECLI_TOKEN_GET_FAILED -1
#define TREECLI_TOKEN_GET_NONE -2
#define TREECLI_TOKEN_GET_UNEXPECTED -3

int32_t treecli_parser_init(struct treecli_parser *parser, const struct treecli_node *top);
#define TREECLI_PARSER_INIT_OK 0
#define TREECLI_PARSER_INIT_FAILED -1

int32_t treecli_parser_free(struct treecli_parser *parser);
#define TREECLI_PARSER_FREE_OK 0
#define TREECLI_PARSER_FREE_FAILED -1

int32_t treecli_parser_parse_line(struct treecli_parser *parser, const char *line);
#define TREECLI_PARSER_PARSE_LINE_OK 0
#define TREECLI_PARSER_PARSE_LINE_FAILED -1
#define TREECLI_PARSER_PARSE_LINE_NO_MATCHES -2
#define TREECLI_PARSER_PARSE_LINE_MULTIPLE_MATCHES -3
#define TREECLI_PARSER_PARSE_LINE_CANNOT_MOVE -4

int32_t treecli_parser_set_print_handler(struct treecli_parser *parser, int32_t (*print_handler)(const char *line, void *ctx), void *ctx);
#define TREECLI_PARSER_SET_PRINT_HANDLER_OK 0
#define TREECLI_PARSER_SET_PRINT_HANDLER_FAILED -1

int32_t treecli_parser_get_matches(struct treecli_parser *parser, char *token, uint32_t len, struct treecli_matches *matches);
#define TREECLI_PARSER_GET_MATCHES_HELP 5
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

int32_t treecli_parser_get_current_node(struct treecli_parser_pos *pos, struct treecli_node *node);
#define TREECLI_PARSER_GET_CURRENT_NODE_OK 0
#define TREECLI_PARSER_GET_CURRENT_NODE_ROOT -1
#define TREECLI_PARSER_GET_CURRENT_NODE_FAILED -2

int32_t treecli_parser_help(struct treecli_parser *parser);
#define TREECLI_PARSER_HELP_OK 0
#define TREECLI_PARSER_HELP_FAILED -1




#endif
